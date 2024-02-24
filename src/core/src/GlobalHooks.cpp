#include "GlobalHooks.h"

namespace MFM {
  GlobalHooks & GlobalHooks::getSingleton() {
    static GlobalHooks theDisgustingGlobalHooks;
    return theDisgustingGlobalHooks;
  }
}
