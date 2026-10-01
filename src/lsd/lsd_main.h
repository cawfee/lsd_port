#ifndef LSD_MAIN_H
#define LSD_MAIN_H

#include "types.h"

typedef enum {
    LSD_FILE_DRIVER_CD = 0x13,
    LSD_FILE_DRIVER_DEBUG = 0x23,
} lsd_file_driver_t;

typedef struct {
    lsd_file_driver_t m_FileDriverClass;
    s32 m_FrameSyncMode;
    s32 m_EnableMovie;
    s32 m_EnableLogo;
    s32 m_EnableMainMenu;
    s32 m_UnusedFlag;
} lsd_config_t;

// Get the default config for the game.
const lsd_config_t *lsd_default_config(void);

// Main call to run the game
// Takes in lsd_config_t
int lsd_main(const lsd_config_t *Config);

#endif /* LSD_MAIN_H */
