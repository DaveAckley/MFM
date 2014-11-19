#ifndef EVENTWINDOW_TEST_H      /* -*- C++ -*- */
#define EVENTWINDOW_TEST_H

#include "Test_Common.h"

namespace MFM {

class EventWindow_Test
{

public:

  static void Test_EventWindowConstruction();

  static void Test_EventWindowNoLockOpen();

  static void Test_EventWindowWrite();

  static void Test_RunTests();
};
} /* namespace MFM */
#endif /*EVENTWINDOW_TEST_H*/
