#include "TimeoutAble.h"

#include "TimeQueue.h"

namespace MFM {
  TimeoutAble::TimeoutAble()
    : mTimeMS(0)
    , mNonce(0)
    , mOnTQ(0)
  {
  }

  TimeoutAble::~TimeoutAble() {
    if (mOnTQ) remove();
  }

  void TimeoutAble::insert(TimeQueue& onTQ, u32 thisFarInFuture, s32 fuzzbits) {
    MFM_API_ASSERT_NULL(mOnTQ);
    if (fuzzbits < 0) {
      fuzzbits = 0;
      if (thisFarInFuture > 0) {
        u32 leadingZeros = __builtin_clz(thisFarInFuture);
        fuzzbits = (32-leadingZeros)>>1;
        /*debug("%d clz %d fb %d rg %d",
          thisFarInFuture, leadingZeros, fuzzbits, 1<<fuzzbits);*/
      }
    }
    Random & random = onTQ.getRandom();
    mNonce = random.Create();
    mTimeMS = onTQ.now();

    if (thisFarInFuture > 0) {
      if (fuzzbits == 0)
        mTimeMS += thisFarInFuture;
      else {
        const u32 mask = (1<<fuzzbits)-1;
        const u32 bits = mNonce&mask;
        mNonce &= ~mask;

        s32 delta = (bits==mask) ? 0 : // Exclude max to balance +&-
          ((s32) bits) - ((s32) ((1<<(fuzzbits-1))-1));
        if (delta > (s32) thisFarInFuture) delta = thisFarInFuture;
        else if (-delta > (s32) thisFarInFuture) delta = -thisFarInFuture;
        const u32 fuzzedFuture = thisFarInFuture + delta;
        /*
        printf("%d -> %d (f=%f%%)\n",
               thisFarInFuture, fuzzedFuture,
               abs(100.0*delta/thisFarInFuture));
        */
        mTimeMS += fuzzedFuture;
      }
    } 

    mOnTQ = &onTQ;
    mOnTQ->insertRaw(*this);
  }
  
  void TimeoutAble::remove() {
    assert(mOnTQ);
    mOnTQ->removeRaw(*this);
    mOnTQ = 0;
  }

  void TimeoutAble::printTimeout(ByteSink & bs) const {
    if (mOnTQ == 0) bs.Printf("unsched");
    else {
      u32 when = getTimeout();
      u32 now = mOnTQ->now();
      if (time_before(when,now)) bs.Printf("expired");
      else {
        u32 delta = when-now;
        u32 ms = (delta)%1000;
        u32 sec = (delta/1000)%60;
        u32 min = (delta/1000/60)%60;
        u32 hr = (delta/1000/60/60)%24;
        u32 day = (delta/1000/60/60/24);
        bs.Printf("+");
        if (day!=0) bs.Printf("%dd",day);
        if (hr!=0) bs.Printf("%dh",hr);
        if (min!=0) bs.Printf("%dm",min);
        bs.Printf("%d.%03ds",sec,ms);
      }
    }
  }


}
