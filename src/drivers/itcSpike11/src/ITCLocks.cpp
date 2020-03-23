#include "ITCLocks.h"

#include <sys/types.h>  /* for open */
#include <sys/stat.h>  /* for open */
#include <fcntl.h>  /* for open */
#include <unistd.h> /* for close */
#include <errno.h>  /* for errno */
#include <string.h> /* for strerror */
#include "T2Utils.h"
#include "Random.h" /* for Random */

namespace MFM {

  const char * ITCLocks::LOCK_DEVICE_PATH = "/dev/itc/locks";
  const char * ITCLocks::LOCK_STATUS_PATH = "/sys/class/itc/status";
  const char * ITCLocks::ITC_STATUS_PATH = "/sys/class/itc_pkt/status";

  ITCLocks::ITCLocks()
    : mLockDeviceFD(-1)
    , mLockStatusFD(-1)
    , mITCStatusFD(-1)
    , mConnectedLockset(0)
    , mConnectedLocksetRefreshTime(0)
  {
    resetCounters();
  }

  void ITCLocks::fakeEvent() {
    static Random random;
    static u32 hits = 0;
    static u32 misses = 0;
    const u32 TILE_HEIGHT = 40u;
    const u32 TILE_WIDTH = 60u;
    const UPoint tileSize(TILE_WIDTH, TILE_HEIGHT);
    u8 locksTaken;
    UPoint site(random, TILE_WIDTH, TILE_HEIGHT);
    LOG.Message("FAKEVENT");
    if (tryLocksFor(4u, site, tileSize, 4u, locksTaken)) {
      ++hits;
      for (volatile s32 i = 1000*random.Create(1000); i >= 0; --i) { }
      freeLocks();
    } else ++misses;
    u32 tot = hits + misses;
    if (tot > 0 && (tot % 1000) == 0)
      LOG.Message("FVX tries %d nhits %d/%3f%%, misses %d/%3f%%"
                  ,tot
                  ,hits, 100.0*hits/tot
                  ,misses, 100.0*misses/tot
                  );
  }

#if 0
  void ITCLocks::OLDfakeEvent() {
    enum { DIR_ET = 0, DIR_SE, DIR_SW, DIR_WT, DIR_NW, DIR_NE };
    static u8 eventSets[] = {
      (1<<DIR_ET)
      ,(1<<DIR_ET)|(1<<DIR_SE)
      ,(1<<DIR_SE)
      ,(1<<DIR_SE)|(1<<DIR_SW)
      ,(1<<DIR_SW)
      ,(1<<DIR_SW)|(1<<DIR_WT)
      ,(1<<DIR_WT)
      ,(1<<DIR_WT)|(1<<DIR_NW)
      ,(1<<DIR_NW)
      ,(1<<DIR_NW)|(1<<DIR_NE)
      ,(1<<DIR_NE)
      ,(1<<DIR_NE)|(1<<DIR_ET)
      ,0  /* 1/3rd time no locks taken*/
      ,0
      ,0
      ,0
    };

    u8 byte = eventSets[random()%(sizeof(eventSets)/sizeof(eventSets[0]))];
    tryLock(byte);
  }
#endif

  void ITCLocks::resetCounters() {
    LOG.Message("Resetting ITCLocks counters");

    mLocksetAttempts = 0;
    mLocksetAcquisitions = 0;
    mUnreadyErrors = 0;
    mContestedErrors = 0;
    mTimeoutErrors = 0;
    mFailedErrors = 0;
  }

  bool ITCLocks::open() {
    s32 fd;
    fd = ::open(LOCK_DEVICE_PATH, O_RDWR);
    if (fd < 0) {
      LOG.Message("Couldn't open '%s': %s", LOCK_DEVICE_PATH, strerror(errno));
      return false;
    }
    mLockDeviceFD = fd;
    fd = ::open(LOCK_STATUS_PATH, O_RDONLY);
    if (fd < 0) {
      LOG.Message("Couldn't open '%s': %s", LOCK_STATUS_PATH, strerror(errno));
      close();
      return false;
    }
    mLockStatusFD = fd;

    fd = ::open(ITC_STATUS_PATH, O_RDONLY);
    if (fd < 0) {
      LOG.Message("Couldn't open '%s': %s", ITC_STATUS_PATH, strerror(errno));
      close();
      return false;
    }
    mITCStatusFD = fd;

    LOG.Message("ITCLocks devices opened");
    return true;
  }

  bool ITCLocks::close() {
    s32 count = 0;
    if (mLockDeviceFD >= 0) {
      ::close(mLockDeviceFD);
      mLockDeviceFD = -1;
      ++count;
    }
    if (mLockStatusFD >= 0) {
      ::close(mLockStatusFD);
      mLockStatusFD = -1;
      ++count;
    }
    if (mITCStatusFD >= 0) {
      ::close(mITCStatusFD);
      mITCStatusFD = -1;
      ++count;
    }
    LOG.Message("ITCLocks %d device(s) closed",count);
    return count > 0;
  }

  u8 ITCLocks::connected(bool allowRefresh) {
    time_t now = time(NULL);
    if (allowRefresh && now > mConnectedLocksetRefreshTime && mITCStatusFD >= 0) {
      ::lseek(mITCStatusFD, 0, SEEK_SET);
      u8 connected = 0x3f; /*assume all connected*/
      for (s32 i = 7; i >= 0; --i) {
        u8 dir6 = mapDir8ToDir6(i); // Returns DIR_COUNT on N/S
        u8 ch;
        if (read(mITCStatusFD,&ch,1) != 1) abort();
        if (ch=='0') connected &= ~(1<<dir6);
      }
      mConnectedLockset = connected;
      mConnectedLocksetRefreshTime = now;
    }
    return mConnectedLockset;
  }

  bool ITCLocks::tryLock(u8 lockset) {
    if (mLockDeviceFD < 0) return false;
    if (mLocksetAttempts > 2*1000*1000*1000) 
      resetCounters();
    if (0 == (mLocksetAttempts % 50000) && mLocksetAttempts > 0)  {
      LOG.Message("LSX atm %d, acq %d/%3f%%, unr %d/%3f%%, cns %d/%3f%%, tmo %d/%3f%%, oth %d/%3f%%"
                  ,mLocksetAttempts
                  ,mLocksetAcquisitions, 100.0*mLocksetAcquisitions/mLocksetAttempts
                  ,mUnreadyErrors,       100.0*mUnreadyErrors/mLocksetAttempts
                  ,mContestedErrors,     100.0*mContestedErrors/mLocksetAttempts
                  ,mTimeoutErrors,       100.0*mTimeoutErrors/mLocksetAttempts
                  ,mFailedErrors,        100.0*mFailedErrors/mLocksetAttempts
                  );
    }

    ++mLocksetAttempts;
    s32 status = write(mLockDeviceFD,&lockset,1);
    if (status > 0) {
      ++mLocksetAcquisitions;
      return true;
    }
    switch (errno) {
    case ENXIO: ++mUnreadyErrors; break;
    case EBUSY: ++mContestedErrors; break;
    case ETIME: ++mTimeoutErrors; break;
    default: ++mFailedErrors; break;
    }
    return false;
  }

  bool ITCLocks::tryLocksFor(const u32 EVENT_WINDOW_RADIUS, UPoint pt, UPoint tileSize, u32 ewRadius, u8 & locksetTaken) {
    const u8 maxLockset = maxLocksetFor(EVENT_WINDOW_RADIUS, pt, tileSize, ewRadius);
    const u8 connectedLockset = connected(true);
    const u8 neededLockset = maxLockset & connectedLockset;
    if (neededLockset == 0 || tryLock(neededLockset)) {
      locksetTaken = neededLockset;
      return true;
    }
    freeLocks();
    return false;
  }

  u8 ITCLocks::maxLocksetFor(const u32 EVENT_WINDOW_RADIUS, UPoint pt, UPoint tileSize, u32 ewRadius) {
    const u32 REACH = EVENT_WINDOW_RADIUS * 2 + ewRadius - 1;
    const u32 TILE_HEIGHT = tileSize.GetY();
    const u32 TILE_WIDTH = tileSize.GetX();
    u8 lockset = 0;

    if (pt.GetX() < REACH) {  //WT edge
      lockset |= 1<<DIR_WT;

      if (pt.GetY() < REACH)                     lockset |= 1<<DIR_NW; 
      else if (pt.GetY() >= TILE_HEIGHT - REACH) lockset |= 1<<DIR_SW;      

    } else if (pt.GetX() >= TILE_WIDTH - REACH) { //ET edge
        lockset |= 1<<DIR_ET;      

	if (pt.GetY() < REACH)                     lockset |= 1<<DIR_NE;      
	else if (pt.GetY() >= TILE_HEIGHT - REACH) lockset |= 1<<DIR_SE;      
    } else { // Neither ET nor WT edge

      if ((pt.GetX() >= TILE_WIDTH/2 - REACH) && (pt.GetX() < TILE_WIDTH/2 + REACH)) { //in the middle +/- REACH

        if (pt.GetY() < REACH)                     lockset |= (1<<DIR_NE)|(1<<DIR_NW);      
        else if (pt.GetY() >= TILE_HEIGHT - REACH) lockset |= (1<<DIR_SE)|(1<<DIR_SW);      

      } else if(pt.GetX() < TILE_WIDTH/2) { // Westside

        if (pt.GetY() < REACH)                     lockset |= 1<<DIR_NW;      
        else if (pt.GetY() >= TILE_HEIGHT - REACH) lockset |= 1<<DIR_SW;      

      } else { // Eastside

        MFM_API_ASSERT_STATE(pt.GetX() >=TILE_WIDTH/2); //sanity

        if (pt.GetY() < REACH)                     lockset |= 1<<DIR_NE;      
        else if (pt.GetY() >= TILE_HEIGHT - REACH) lockset |= 1<<DIR_SE;      
      }
    }

    return lockset;
  }    

  bool ITCLocks::freeLocks() { return tryLock(0); }

}
