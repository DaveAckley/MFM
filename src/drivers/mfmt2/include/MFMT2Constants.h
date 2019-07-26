#ifndef MFMT2CONSTANTS_H
#define MFMT2CONSTANTS_H

namespace MFM
{
  enum { MENU_WIDTH = 480, MENU_HEIGHT = 320 };

  enum DriverOp {
    CANCEL_OP,      // Dismiss driver op menu
    RESET_OP,       // Reload MFMT2 initial state
    QUIT_OP,        // Quit MFMT2
    REBOOT_OP,      // Reboot tile
    SHUTDOWN_OP,    // Turn off tile power
    GLOBAL_OP,      // Broadcast next op to grid
    CLEAR_OP,       // Set all sites to empty
    T2VIZ_OP,       // Start t2viz stats displayer
    MHZ300_OP,      // Set CPU frequency to 300MHz
    MHZ600_OP,      // Set CPU frequency to 600MHz
    MHZ720_OP,      // Set CPU frequency to 720MHz
    MHZ800_OP,      // Set CPU frequency to 800MHz
    MHZ1000_OP,     // Set CPU frequency to 1Gz
    DRIVER_OP_COUNT
  };

  enum {
    EVENT_HISTORY_SIZE = 100000,
    SECS_PER_STATUS = 5,
    FLASH_TRAFFIC_TTL = 30,
    DRIVER_OP_DELAY = 3
  };
}

#endif /* MFMT2CONSTANTS_H */
