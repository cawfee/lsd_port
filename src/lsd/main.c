#include <psx/kernel.h>

#include "lsd/lsd_main.h"
#include "lsd/base/base.h"

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

    // Used to ensure psx is 2M and not developer
    SetMem(2);

    // Set up the games memory manager
    memory_manager = memory_create_manager(MEMORY_MANAGER_SIZE, 0);
    g_MEMORY_MANAGER_MAIN = memory_manager;
    memory_set_manager(memory_manager);

    // Create game flow
    // g_GAME_FLOW = game_flow_create(Config);

    // Create display and run
    // display = display_create();
    // g_GAME_FLOW->vtable->game_flow_init(g_GAME_FLOW, display, pad_create(0, 0));
    // g_GAME_FLOW->vtable->game_flow_execute_phases(g_GAME_FLOW);

    return 0;
}
