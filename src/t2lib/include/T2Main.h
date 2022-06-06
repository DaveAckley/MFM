/* -*- C++ -*- */
#ifndef T2MAIN_H
#define T2MAIN_H

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Logger.h"
#include "FileByteSink.h"

#include "TimeQueue.h"

namespace MFM {
  /// Base class of all T2 main programs, supplying basic services such as random numbers and time queues
  
  struct T2Main {

    u32 now() const { return mTimeQueue.now(); }
    TimeQueue & getTQ() { return mTimeQueue; }
    Random & getRandom() { return mRandom; }

    static inline T2Main & get() {
      static bool underConstruction;
      if (underConstruction) {
        // Go bare lib: LOG etc may not be available yet
        fprintf(stderr,"%s Reentry during construction\n",__PRETTY_FUNCTION__);
        FAIL(ILLEGAL_STATE);
      }
      underConstruction = true;
      {
        if (!mTheInstance) initInstance();
        underConstruction = false;
        return * mTheInstance;
      }
    }

  protected:
    static void initInstance() { new T2Main(); }

    T2Main()
      : mRandom()
      , mTimeQueue(this->getRandom())
    {
      MFM_API_ASSERT_NULL(mTheInstance);
      mTheInstance = this;
    }
    virtual ~T2Main();

    Random mRandom;
    TimeQueue mTimeQueue;

  private:
    static T2Main * mTheInstance;
  };
}

#endif /* T2MAIN_H */
