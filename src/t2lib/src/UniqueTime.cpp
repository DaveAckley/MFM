#include "UniqueTime.h"
namespace MFM {
  UniqueTime UniqueTime::mStaticLast;

  void UniqueTime::printRelativePretty(ByteSink& bs, const UniqueTime & relativeto) const
  {
    UniqueTime diff = *this - relativeto;
    diff.printPretty(bs);
  }

  void UniqueTime::printPretty(ByteSink& bs) const {
    printPretty(mLocalTimestamp.tv_sec, mLocalTimestamp.tv_nsec, bs);
  }

  void UniqueTime::printPretty(u32 tv_sec, u32 tv_nsec, ByteSink& bs) {
    const u32 SEC_PER_MIN = 60;
    const u32 SEC_PER_HOUR = 60*60;
    const u32 SEC_PER_DAY = 60*60*24;
    if ((u32) tv_sec > SEC_PER_DAY) {
      u32 days = tv_sec/SEC_PER_DAY;
      tv_sec %= SEC_PER_DAY;
      bs.Printf("%dd ",days);
    }

    if ((u32) tv_sec > SEC_PER_HOUR) {
      u32 hours = tv_sec/SEC_PER_HOUR;
      tv_sec %= SEC_PER_HOUR;
      bs.Printf("%dh ",hours);
    }

    if ((u32) tv_sec > SEC_PER_MIN) {
      u32 mins = tv_sec/SEC_PER_MIN;
      tv_sec %= SEC_PER_MIN;
      bs.Printf("%dm ",mins);
    }

    if ((u32) tv_sec > 0) {
      bs.Printf("%ds ",tv_sec);
    }

    u32 msecs = (u32) (tv_nsec/1000000.0);
    bs.Printf("%dms",msecs);
  }
}
