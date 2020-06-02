/* -*- C++ -*- */
#ifndef UNIQUETIME_H
#define UNIQUETIME_H

#include <time.h>

#include "itype.h"
#include "ByteSink.h"
#include "ByteSource.h"

namespace MFM {

  struct UniqueTime {
    static void printPretty(u32 sec, u32 nsec, ByteSink & bs) ;
    void printPretty(ByteSink& bs) const ;
    void printRelativePretty(ByteSink& bs, const UniqueTime & relativeto) const ;

    void Print(ByteSink& bs) const {
      bs.Printf("%l%l%c",
                mLocalTimestamp.tv_sec,
                mLocalTimestamp.tv_nsec,
                mUniquer);
    }
    bool Scan(ByteSource& bs) {
      struct timespec tmp;
      u8 uniquer;
      if (3 != bs.Scanf("%l%l%c",&tmp.tv_sec,&tmp.tv_nsec,&uniquer))
        return false;
      mLocalTimestamp = tmp;
      mUniquer = uniquer;
      return true;
    }
    static struct timespec now() {
      struct timespec time; 
      clock_gettime(CLOCK_REALTIME, &time);
      return time;
    }

    struct timespec getTimespec() const {
      return mLocalTimestamp;
    }

    u8 getUniquer() const {
      u8 ret = 0;
      if (this != &mStaticLast &&
          mLocalTimestamp.tv_sec == mStaticLast.mLocalTimestamp.tv_sec &&
          mLocalTimestamp.tv_nsec == mStaticLast.mLocalTimestamp.tv_nsec) {
        MFM_API_ASSERT_STATE(mStaticLast.mUniquer < 255);
        ret = 1+mStaticLast.mUniquer;
      }
      return ret;
    }

    UniqueTime()
      : mLocalTimestamp(now())
      , mUniquer(getUniquer())
    {
      mStaticLast = *this;
    }

    UniqueTime(const UniqueTime & ut)
      : mLocalTimestamp(ut.mLocalTimestamp)
      , mUniquer(ut.mUniquer)
    { }

    UniqueTime(const struct timespec ts, u8 uniquer)
      : mLocalTimestamp(ts)
      , mUniquer(uniquer)
    { }

    UniqueTime operator-(const UniqueTime& rhs) const {
      MFM_API_ASSERT_ARG(*this >= rhs);
      const u32 ONE_BILLION = 1000000000;
      u32 netsec = mLocalTimestamp.tv_sec - rhs.mLocalTimestamp.tv_sec;
      s32 netnanos = mLocalTimestamp.tv_nsec - rhs.mLocalTimestamp.tv_nsec;
      if (netnanos < 0) {
        --netsec;
        netnanos += ONE_BILLION;
        MFM_API_ASSERT_ARG(netnanos > 0);
      }
      s32 netuniquer = mUniquer - rhs.mUniquer;
      if (netuniquer < 0) {
        netuniquer = 0; // XXX ??
      } else if (netuniquer > 255) {
        netuniquer = 255;
      }
      struct timespec tmp;
      tmp.tv_sec = netsec;
      tmp.tv_nsec = netnanos;
      return UniqueTime(tmp,(u8) netuniquer);
    }

    bool operator>=(const UniqueTime& rhs) const {
      return !(*this < rhs);
    }
    bool operator==(const UniqueTime& rhs) const {
      return !(rhs < *this) && !(*this < rhs);
    }

    bool operator<(const UniqueTime& rhs) const {
      if (this == &rhs) return false;
      if (mLocalTimestamp.tv_sec < rhs.mLocalTimestamp.tv_sec) return true; 
      if (mLocalTimestamp.tv_sec > rhs.mLocalTimestamp.tv_sec) return false; 
      if (mLocalTimestamp.tv_nsec < rhs.mLocalTimestamp.tv_nsec) return true; 
      if (mLocalTimestamp.tv_nsec > rhs.mLocalTimestamp.tv_nsec) return false; 
      if (mUniquer < rhs.mUniquer) return true; 
      if (mUniquer > rhs.mUniquer) return false; 
      return false;  // Well, they're equal after all our efforts.
      //FAIL(ILLEGAL_STATE); // Not unique?
    }

  private:
    UniqueTime & operator=(const UniqueTime & ut) {
      mLocalTimestamp = ut.mLocalTimestamp;
      mUniquer = ut.mUniquer;
      return *this;
    }
    static UniqueTime mStaticLast;
    struct timespec mLocalTimestamp;
    u8 mUniquer;
  };

}
#endif /* UNIQUETIME_H */
