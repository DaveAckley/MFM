#include "TimeQueue.h"
#include "FileByteSink.h"
#include "T2Utils.h"

namespace MFM {
  void TimeQueue::dumpQueue() {
    dumpQueue(STDERR);
  }

  void TimeQueue::dumpQueue(ByteSink& bs) {
    printComma(getExpiredCount(), bs);
    bs.Printf(" [%d]\n", size());
    for (auto itr = mPQ.begin(); itr != mPQ.end(); ++itr) {
      TimeoutAble * ta = *itr;
      ta->printTimeout(bs);
      bs.Printf(" %s\n", ta->getName());
    }
  }

  TimeoutAble * TimeQueue::getEarliestExpired() {
    //dumpQueue();
    SetOfTimeoutAble::iterator min = mPQ.begin();
    if (min != mPQ.end()) {
      TimeoutAble* ta = *min;
      if (time_before_eq(ta->getTimeout(), now())) {
        ta->remove();
        ++mExpiredCount;
        return ta;
      }
    }
    return 0;
  }
  void TimeQueue::insertRaw(TimeoutAble & ta) {
    assert(ta.mOnTQ == this);
    mPQ.insert(&ta);
  }
  void TimeQueue::removeRaw(TimeoutAble & ta) {
    assert(ta.mOnTQ == this);
    mPQ.erase(&ta);
  }

  u32 TimeQueue::now() const {
    struct timespec time; 
    clock_gettime(CLOCK_REALTIME, &time); 
    u32 msnow = (u32) (1000u*time.tv_sec + time.tv_nsec/(1000u*1000u));
    static u32 lastmsnow = 0;
    if (lastmsnow) {
      u32 mspast = msnow - lastmsnow;
      if (mspast > 200) {
        LOG.Warning("Big MSPAST %d",mspast);
      }
    }
    lastmsnow = msnow;

    return msnow;
  }
  
}
