#include "memory.h"

#include <stddef.h>

#include <psx/malloc.h>
#include <psx/stdio.h>

static memory_manager_t *g_MEMORY_MANAGER = NULL;

static void *g_MEMORY_MANAGER_PAD = NULL;

#define MEM_ALIGN ((u32)sizeof(void *))
#define MEM_ALIGN_UP(value) ((((u32)(value)) + MEM_ALIGN - 1U) & ~(MEM_ALIGN - 1U))
#define BLOCK_PAYLOAD_OFFSET ((u32)offsetof(memory_block_t, m_Next))
#define BLOCK_MIN_PAYLOAD (3U * MEM_ALIGN)
#define BLOCK_MIN_SIZE (BLOCK_PAYLOAD_OFFSET + 3U * MEM_ALIGN)
#define MEM_MANAGER_HEADER (MEM_ALIGN_UP((u32)offsetof(memory_manager_t, m_Blocks)))

#define BLOCK_SIZE(block) ((block)->m_Header & 0x0FFFFFFF)
#define BLOCK_NEXT(block) ((memory_block_t *)((u8 *)(block) + BLOCK_SIZE(block)))
#define BLOCK_FOOTER(block) (*(memory_block_t **)((u8 *)(block) - MEM_ALIGN))
#define BLOCK_FROM_PAYLOAD(payload) ((memory_block_t *)((u8 *)(payload) - BLOCK_PAYLOAD_OFFSET))
#define BLOCK_PAYLOAD(block) ((void *)&(block)->m_Next)

static s32 g_MEMORY_LOCK = 0;

memory_manager_t *memory_create_manager(u32 Size, s32 Unused) {
    (void)Unused;

    long pool_size;
    memory_manager_t *manager;

    pool_size = Size;
    if (pool_size < 0x400U) {
        pool_size = 0x400;
    }

    pool_size = (long)((u32)pool_size & ~(MEM_ALIGN - 1U));
    manager = psyq_malloc_malloc(MEM_MANAGER_HEADER + pool_size + MEM_ALIGN);
    if (manager != NULL) {
        manager->m_Pool = (u8 *)manager + MEM_MANAGER_HEADER;
        manager->m_PoolSize = (s32)pool_size;
        memory_setup_manager(manager);
    } else {
        printf("bMemPMgr = %p, poolSize = %ld in BMemPMgrInit\n", manager, pool_size);
    }
    return manager;
}

void memory_set_manager(memory_manager_t *Manager) {
    g_MEMORY_MANAGER = Manager;
}

void memory_set_lock(s32 Value) {
    g_MEMORY_LOCK = Value;
}

s32 memory_is_locked(void) {
    return g_MEMORY_LOCK;
}

void memory_free_raw(void *Ptr) {
    psyq_malloc_free(Ptr);
}

void memory_setup_manager(memory_manager_t *ManagerParam) {
    memory_manager_t *manager;
    memory_block_t *block;
    memory_block_t *end_block;

    manager = g_MEMORY_MANAGER;
    if (manager == NULL) {
        manager = ManagerParam;
    }
    manager->m_Ready = 1;
    block = manager->m_Pool;
    manager->m_Head = block;
    manager->m_Tail = block;
    block->m_Header = manager->m_PoolSize | 0x40000000;
    manager->m_Head->m_Next = NULL;
    manager->m_Tail->m_Prev = NULL;
    end_block = BLOCK_NEXT(block);
    BLOCK_FOOTER(end_block) = block;
    end_block->m_Header = 0x80000000;
}

void *memory_allocate_mem(u32 Size, void *Pool) {
    memory_manager_t *manager;
    memory_block_t *block;
    void *payload;
    memory_block_t *next_block;
    u32 block_size;
    u32 padded;

    memory_set_lock(1);
    payload = NULL;
    manager = g_MEMORY_MANAGER;
    if (manager == NULL) {
        manager = Pool;
    }
    if (Size != 0) {
        if (Size & (MEM_ALIGN - 1U)) {
            padded = Size + MEM_ALIGN;
            Size = padded - (Size & (MEM_ALIGN - 1U));
        }
        if (Size < BLOCK_MIN_PAYLOAD) {
            Size = BLOCK_MIN_PAYLOAD;
        }
        block = manager->m_Head;
        Size += BLOCK_PAYLOAD_OFFSET;
        while (block != NULL) {
            block_size = BLOCK_SIZE(block);
            if (block_size >= Size) {
                block->m_Header &= 0xBFFFFFFF;
                payload = BLOCK_PAYLOAD(block);
                if (block_size < Size + BLOCK_MIN_SIZE) {
                    next_block = BLOCK_NEXT(block);
                    next_block->m_Header &= 0x7FFFFFFF;
                    {
                        memory_block_t *prev_link;
                        memory_block_t *next_link;

                        prev_link = block->m_Prev;
                        next_link = block->m_Next;
                        if (next_link != NULL) {
                            next_link->m_Prev = prev_link;
                        } else {
                            manager->m_Tail = prev_link;
                        }
                    }
                    {
                        memory_block_t *prev_link;
                        memory_block_t *next_link;

                        prev_link = block->m_Prev;
                        next_link = block->m_Next;
                        if (prev_link != NULL) {
                            prev_link->m_Next = next_link;
                        } else {
                            manager->m_Head = next_link;
                        }
                    }
                } else {
                    block->m_Header = (block->m_Header & 0xF0000000) | Size;
                    next_block = BLOCK_NEXT(block);
                    next_block->m_Header = (block_size - Size) | 0x40000000;
                    next_block->m_Next = block->m_Next;
                    next_block->m_Prev = block->m_Prev;
                    {
                        memory_block_t *next_link = block->m_Next;

                        if (next_link != NULL) {
                            next_link->m_Prev = next_block;
                        } else {
                            manager->m_Tail = next_block;
                        }
                    }
                    {
                        memory_block_t *prev_link = block->m_Prev;

                        if (prev_link != NULL) {
                            prev_link->m_Next = next_block;
                        } else {
                            manager->m_Head = next_block;
                        }
                    }
                    BLOCK_FOOTER(BLOCK_NEXT(next_block)) = next_block;
                }
                break;
            }
            block = block->m_Next;
        }
    }
    memory_set_lock(0);
    return payload;
}

void *memory_free_mem(void *Ptr, void *Pool) {
    memory_manager_t *manager;
    memory_block_t *block;
    memory_block_t *next;
    u32 next_free;

    memory_set_lock(1);
    manager = g_MEMORY_MANAGER;
    if (manager == NULL) {
        manager = Pool;
    }
    if (Ptr != NULL) {
        block = BLOCK_FROM_PAYLOAD(Ptr);
        next = BLOCK_NEXT(block);
        next_free = next->m_Header & 0x40000000;
        if (block->m_Header & 0x80000000) {
            u32 freed_size = block->m_Header & 0x0FFFFFFF;

            block = BLOCK_FOOTER(block);
            block->m_Header = (block->m_Header & 0xF0000000) | (freed_size + (block->m_Header & 0x0FFFFFFF));
            {
                memory_block_t *prev_link = block->m_Prev;
                memory_block_t *next_link = block->m_Next;

                if (next_link != NULL) {
                    next_link->m_Prev = prev_link;
                } else {
                    manager->m_Tail = prev_link;
                }
            }
            {
                memory_block_t *next_link = block->m_Next;
                memory_block_t *prev_link = block->m_Prev;

                if (prev_link != NULL) {
                    prev_link->m_Next = next_link;
                } else {
                    manager->m_Head = next_link;
                }
            }
        }
        if (next_free) {
            u32 next_size = next->m_Header & 0x0FFFFFFF;

            block->m_Header = (block->m_Header & 0xF0000000) | (next_size + (block->m_Header & 0x0FFFFFFF));
            {
                memory_block_t *prev_link = next->m_Prev;
                memory_block_t *next_link = next->m_Next;

                if (next_link != NULL) {
                    next_link->m_Prev = prev_link;
                } else {
                    manager->m_Tail = prev_link;
                }
            }
            {
                memory_block_t *next_link = next->m_Next;
                memory_block_t *prev_link = next->m_Prev;

                next_size = prev_link != NULL;
                if (next_size) {
                    prev_link->m_Next = next_link;
                } else {
                    manager->m_Head = next_link;
                }
            }
            next = BLOCK_NEXT(block);
        }
        block->m_Next = manager->m_Head;
        manager->m_Head = block;
        block->m_Prev = NULL;
        if (block->m_Next != NULL) {
            block->m_Next->m_Prev = block;
        } else {
            manager->m_Tail = block;
        }
        BLOCK_FOOTER(next) = block;
        block->m_Header |= 0x40000000;
        next->m_Header |= 0x80000000;
    }
    memory_set_lock(0);
    return NULL;
}

void memory_nullsub3(void) {
    // unused
    (void)sizeof(g_MEMORY_MANAGER_PAD);
}
