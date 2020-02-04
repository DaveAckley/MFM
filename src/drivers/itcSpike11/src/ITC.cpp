#include "ITC.h"
#include "Logger.h"

namespace MFM {
  ITC::ITC()
    : mRandom(0)
    , mMFMIO(0)
    , mDir6(U8_MAX)
    , mLevel(0)
    , mStage(0)
    , mUpdateTimeout(0)
    , mKnownIncompatible(false)
    , mBackoffInterval(STARTING_BACKOFF_INTERVAL)
    , mTileGeneration(U32_MAX)
    , mPacketsSent(0)
    , mPacketsDropped(0)
  { }

  void ITC::setRandom(Random& random) {
    MFM_API_ASSERT_NULL(mRandom);
    mRandom = &random;
  }
  
  void ITC::setMFMIO(MFMIO& mfmio) {
    MFM_API_ASSERT_NULL(mMFMIO);
    mMFMIO = &mfmio;
  }
  
  void ITC::setTileModel(TileModel& tileModel) {
    MFM_API_ASSERT_NULL(mTileModel);
    mTileModel = &tileModel;
  }
  
  void ITC::setDir6(u8 dir6) {
    MFM_API_ASSERT(mDir6 == U8_MAX, ILLEGAL_STATE);
    MFM_API_ASSERT(dir6 < 6, ILLEGAL_ARGUMENT);
    mDir6 = dir6;
  }

  void ITC::handleLevelPacket(u8 theirLevel, u8 theirStage, const char * args, u32 arglen) {
    if (theirLevel > 3 || theirStage > 2) {
      warn("Bad level packet lvl%d stage%d",theirLevel,theirStage);
      return;
    }
    if (0) { /* COND */ }
    //// FIRST, SAME-LEVEL CASES
    else if (theirLevel == mLevel) {
      if (0) { /* COND */ } 

      //// (S1) We were waiting to hear they're here 
      else if (mStage == 0) {
        incrementStageAndAnnounce(1);
      }

      //// (S2) I'm ready for next level when you are
      else if (mStage == 2 && theirStage == 2) {
        if (mLevel == 0) {
          // We need compatible MFZs to advance
          TileModel & tm = getTileModel();
          if (tm.getLevel() > 0) {
            const char * usmfzv = tm.getMFZVersionString();
            u32 ourlen = strlen(usmfzv);
            if (ourlen == arglen && strncmp(usmfzv,args,arglen)==0) {
              message("COMPATIBLE! %s",usmfzv);
              sendLevelPacket(); // Make sure they hear the good news
              incrementLevel(1);
            } else {
              if (mKnownIncompatible) 
                bumpBackoff();
              else {
                mKnownIncompatible = true;
                message("INCOMPATIBLE %s", usmfzv);
              }
            }
          }
        }
      }
      //// (S3) Your packet gives me no new information
      else { /* DROP IT */ }
    }

    //// HERE, THEY'RE ONE DOWN AND EXITING, WHILE WE'RE ENTERING  
    else if (theirLevel == mLevel-1 && theirStage == 2 && mStage == 0) {
      /* That's OK.  Just wait it out */
    }

    //// NOW, ALL OTHER DOWN CASES
    else if (theirLevel < mLevel) {
      // Bummer.  We have to crash back to their level.
      message("Crashing from L%d%d to L%d0",
              mLevel,mStage,theirLevel);
      while (theirLevel < mLevel) {
        incrementLevel(-1);
      }
      sendLevelPacket(); // Announce our final position
    }

    //// NOW, ALL OTHER UP CASES
    else if (theirLevel > mLevel) {
      sendLevelPacket(); // Reannounce our current level
    }

    //// NO OTHER CASES SHOULD EXIST
    else {
      FAIL(ILLEGAL_STATE);
    }
  }

  void ITC::incrementStageAndAnnounce(u32 amt) {
    if (amt == 0) return;
    s32 newStage = mStage + amt;
    if (newStage < 0 || newStage > 2) { // XXX getStageNumberCount()
      return;
    }
    mStage = newStage;
    sendLevelPacket();
    message("To stage L%d%d", mLevel, mStage);
  }

  u32 ITC::getLevel() const { return mLevel; }

  u32 ITC::getStage() const { return mStage; }

  bool ITC::getKnownIncompatible() const { return mKnownIncompatible; }

  void ITC::incrementLevel(s32 amt) {
    if (amt == 0) return;
    s32 newLevel = mLevel + amt;
    if (newLevel < 0 || newLevel > 2) { // XXXX LEVEL NUMBER COUNT
      return;
    }
    configureLevel(newLevel);
    message("%s to L%d0", amt > 0 ? "Up" : "Down", newLevel);
  }

  void ITC::configureLevel(u32 newLevel) {
    mLevel = newLevel;
    mStage = 0;
    mKnownIncompatible = false;
    mBackoffInterval = STARTING_BACKOFF_INTERVAL;
    mUpdateTimeout = 0;
  }

  void ITC::handleInboundPacket(PacketBuffer & pb) {
    const char * z = pb.GetZString();
    u32 len = pb.GetLength();
    if (len == 0) {
      LOG.Warning("ITC: Zero length packet rcvd");
      return;
    }
    if (z[0] == 'l') {         // 'MFM::PacketType::LEVEL'
      if (len < 3) {
        LOG.Warning("ITC: Short (%d) level packet rcvd", len);
        return;
      }
      handleLevelPacket(z[1]-'0',z[2]-'0',&z[3],len-3);
      return;
    }
    warn("UNRECOGNIZED Packet len=%d s='%s'", pb.GetLength(),z);
  }

  void ITC::bumpBackoff() {
    if (mBackoffInterval < 1000000) {
      mBackoffInterval = getRandom().Between(mBackoffInterval,3*mBackoffInterval);
      message("Backoff interval = %d", mBackoffInterval);
    }
  }

  void ITC::resetBackoff() {
    mBackoffInterval = getRandom().Between(STARTING_BACKOFF_INTERVAL,2*STARTING_BACKOFF_INTERVAL);
    mUpdateTimeout = 0;
  }
  
  void ITC::update() {
    TileModel & tm = getTileModel();
    if (tm.getGeneration() != mTileGeneration) {
      /*
      message("Noticed tile gen %d (%d/%d)",
              tm.getGeneration(),
              mPacketsSent,
              mPacketsDropped);
      */
      handleTileChange();
      return;
    }
    if (mUpdateTimeout == 0) {
      if (!sendLevelPacket()) {
        bumpBackoff();
      }
      mUpdateTimeout = getRandom().Between(mBackoffInterval,2*mBackoffInterval);
    } else {
      --mUpdateTimeout;
      PacketBuffer pb;
      u8 dir6 = getDir6();
      u8 dir8 = mapDir6ToDir8(dir6);
      //    LOG.Message("UPDATE UPDAT dir6=%d, dir8=%d",dir6,dir8);
      if (getMFMIO().tryReceivePacket(dir8,pb)) {
        handleInboundPacket(pb);
      } else {
        handleTileChange();
      }
    }
  }

  void ITC::handleTileChange() {
    TileModel & tm = getTileModel();
    u32 tlevel = tm.getLevel();
    u32 tgen = tm.getGeneration();

    if (mTileGeneration != tgen) {
      if (mLevel != 0 || mStage != 0) {
        configureLevel(0);
      }
      mTileGeneration = tgen;
    }
    
    if (0) { /* COND */ }

    //// FIRST, WAITING FOR TILE MFMR AND NGBR LIVENESS
    else if (mLevel == 0 && mStage == 1 && tlevel > 0) {
      // Tile has an MFZ running.
      incrementStageAndAnnounce(1); // Up to stage 2
    }

    else if (mLevel == 0 && mStage != 1 && tlevel > 0) {
      // L00: Init startup
      // L02: Waiting to hear from neighbor
      // either way just wait
    }

    else if (mLevel == 1 && mStage == 0 && tlevel > 0) {
      // L10: Waiting for nghbr to get to 1
    }

    else if (mLevel == 1 && mStage == 1 && tlevel > 0) {
      // L11: All good here
    }

    //// SECOND, WATCHING FOR TILE BAILOUT
    else if (mLevel > 0 && tlevel == 0) {
      while (mLevel > 0) {
        incrementLevel(-1);
        sendLevelPacket();
      }
    }

    //// THIRD, TILE LEVEL 0 IS BORING
    else if (tlevel == 0) {
      // Nothing
    }

    else if (1) {
      message("Uncovered handleTileChange tile=L%d ITC=L%d%d",
              tlevel, mLevel, mStage);
    }
    
  }

  bool ITC::sendLevelPacket() {
    MFM_API_ASSERT(mLevel <= 3, ILLEGAL_STATE);
    MFM_API_ASSERT(mStage <= 3, ILLEGAL_STATE);
    u8 dir8 = mapDir6ToDir8(mDir6);
    u32 len = 4; // default
    unsigned char buf[256];
    buf[0] = 0xa0|(dir8&0x7); // 0xa==ROUTED MFM packet
    buf[1] = 'l';             // 'MFM::PacketType::LEVEL'
    buf[2] = '0'+mLevel;
    buf[3] = '0'+mStage;
    // L02 gets mfz name
    if (mLevel == 0 && mStage == 2) {
      TileModel & tm = getTileModel();
      const char * mfzv = tm.getMFZVersionString();
      for (; *mfzv && len < 256; buf[len++] = *mfzv++) { }
    }
    
    bool ret = getMFMIO().trySendPacket(buf,len);
    if (ret) ++mPacketsSent;
    else ++mPacketsDropped;
    return ret;
  }

  void ITC::message(const char * format, ... ) {
    va_list ap;
    va_start(ap, format);
    vreport(Logger::MESSAGE, format, ap);
    va_end(ap);
  }


  void ITC::warn(const char * format, ... ) {
    va_list ap;
    va_start(ap, format);
    vreport(Logger::MESSAGE, format, ap);
    va_end(ap);
  }

  void ITC::error(const char * format, ... ) {
    va_list ap;
    va_start(ap, format);
    vreport(Logger::MESSAGE, format, ap);
    va_end(ap);
  }

  void ITC::vreport(u32 msglevel, const char * format, va_list & ap) {
    OString256 buf;
    u8 dir8 = mapDir6ToDir8(mDir6);
    buf.Printf("ITC/%s%d%d: ", Dirs::GetCode(dir8), mLevel, mStage);
    buf.Vprintf(format, ap);
    LOG.Log((Logger::Level) msglevel, "%s", buf.GetZString());
  }

  ITC::~ITC() { }
}
