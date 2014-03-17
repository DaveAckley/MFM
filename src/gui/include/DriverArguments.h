#ifndef DRIVERARGUMENTS_H   /* -*- C++ -*- */
#define DRIVERARGUMENTS_H

#include "itype.h"

namespace MFM { 

  class DriverArguments
  {
  private:
    u32 m_seed;

  public:
    DriverArguments() : m_seed(0) { }
    DriverArguments(int argc, char **argv) : m_seed(0)
    {
      ProcessArguments(argc,argv);
    }

    int ProcessArguments(int argc, char **argv) ;

    u32 GetSeed() const { return m_seed; }

  };
} /* namespace MFM */

#endif /* DRIVERARGUMENTS_H */
