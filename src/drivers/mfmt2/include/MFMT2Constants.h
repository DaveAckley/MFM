#ifndef MFMT2CONSTANTS_H
#define MFMT2CONSTANTS_H

namespace MFM
{
  enum { MENU_WIDTH = 480, MENU_HEIGHT = 320 };

  enum DriverOp {
    CANCEL_OP,
    RESET_OP,
    QUIT_OP,
    REBOOT_OP,
    SHUTDOWN_OP,
    GLOBAL_OP,
    DRIVER_OP_COUNT
  };

  enum {
    EVENT_HISTORY_SIZE = 100000,
    SECS_PER_STATUS = 5,
    FLASH_TRAFFIC_TTL = 30,
    DRIVER_OP_DELAY = 8
  };
}

#endif /* MFMT2CONSTANTS_H */
