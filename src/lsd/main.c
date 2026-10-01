#include <psx/kernel.h>

#include <stddef.h>

#include "lsd/lsd_main.h"
#include "lsd/memory/memory.h"

#define MEMORY_MANAGER_SIZE 0x166C00

static memory_manager_t *g_MEMORY_MANAGER_MAIN = NULL;
// static game_flow_t *g_GAME_FLOW;

const lsd_config_t *lsd_default_config(void) {
    static lsd_config_t config = {
        .m_FileDriverClass = LSD_FILE_DRIVER_CD, // 0x23 selects the null driver and crashes
        .m_FrameSyncMode = 0,
        .m_EnableMovie = 1,
        .m_EnableLogo = 1,
        .m_EnableMainMenu = 1,
        .m_UnusedFlag = 1,
    };

    return &config;
}

int lsd_main(const lsd_config_t *Config) {
    memory_manager_t *memory_manager = NULL;

    (void)Config;

    // Used to ensure psx is 2M and not developer
    SetMem(2);

    // Set up the games memory manager
    memory_manager = memory_create_manager(MEMORY_MANAGER_SIZE, 0);
    g_MEMORY_MANAGER_MAIN = memory_manager;
    memory_set_manager(memory_manager);

    return 0;
}
