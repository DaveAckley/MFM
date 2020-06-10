#ifndef MFMT2CONSTANTS_H
#define MFMT2CONSTANTS_H

#define MFM_DEV_PATH "/dev/itc/mfm"

namespace MFM
{
  enum {
    MENU_WIDTH = 480,
    MENU_HEIGHT = 320,

    ITC_LENGTH = 200,
    ITC_WIDTH = 2,
    ITC_RIGHT_INDENT = 5
  };

  enum DriverOp {
    DRIVER_OP_MIN = 0x80,
    CANCEL_OP =     0x80,  // Dismiss driver op menu
    RESET_OP =      0x81,  // Reload MFMT2 initial state
    QUIT_OP =       0x82,  // Quit MFMT2
    REBOOT_OP =     0x83,  // Reboot tile
    SHUTDOWN_OP =   0x84,  // Turn off tile power
    GLOBAL_OP =     0x85,  // Broadcast next op to grid
    CLEAR_OP =      0x86,  // Set all sites to empty
    T2VIZ_OP =      0x87,  // Start t2viz stats displayer
    MHZ300_OP =     0x88,  // Set CPU frequency to 300MHz
    MHZ600_OP =     0x89,  // Set CPU frequency to 600MHz
    MHZ720_OP =     0x8a,  // Set CPU frequency to 720MHz
    MHZ800_OP =     0x8b,  // Set CPU frequency to 800MHz
    MHZ1000_OP =    0x8c,  // Set CPU frequency to 1Gz
    STATSON_OP =    0x8d,  // Show stats panel
    STATSOFF_OP =   0x8e,  // Don't show stats panel
    DRIVER_NO_OP =  0x8f,  // No op, do nothing
    DRIVER_OP_MAX = 0x8f
  };

  enum {
    EVENT_HISTORY_SIZE = 100000,
    SECS_PER_STATUS = 5,
    FLASH_TRAFFIC_TTL = 30,
    DRIVER_OP_DELAY = 2
  };
}

#endif /* MFMT2CONSTANTS_H */
