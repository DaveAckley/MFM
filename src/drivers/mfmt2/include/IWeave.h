/* -*- C++ -*- */
#ifndef IWEAVE_H
#define IWEAVE_H

#include "Point.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {
  struct Weaver; // FORWARD
  struct IWeavePrivate; // FORWARD
  
  struct IWeave {
    Weaver & mWeaver;
    IWeavePrivate * mIWeavePrivatePtr;

    IWeave(Weaver& w) ;
    ~IWeave() ;
    void runInteractive() ;
    bool doCommand() ;
  private:
  };

}
#endif /* IWEAVE_H */
