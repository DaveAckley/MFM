#include "ITC.h"

namespace MFM {
  ITC::ITC()
    : mRandom(0)
    , mMFMIO(0)
    , mDir6(U8_MAX)
    , mLevel(0)
    , mStage(0)
    , mUpdateTimeout(0)
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
      LOG.Warning("ITC bad level packet lvl%d stage%d",theirLevel,theirStage);
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
          LOG.Message("FORFGDS AT %d%d", mLevel, mStage);
          // We need compatible MFZs to advance
          TileModel & tm = getTileModel();
          if (tm.getLevel() > 0) {
            const char * mfzv = tm.getMFZVersionString();
            LOG.Message("XXX SEND %s", mfzv);
            FAIL(INCOMPLETE_CODE);
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
      LOG.Message("ITC: Crashing from L%d%d to L%d0",
                  mLevel,mStage,theirLevel);
      while (theirLevel < mLevel) {
        incrementLevel(-1);
      }
      sendLevelPacket(); // Announce our final position
    }

    //// NOW, ALL OTHER UP CASES
    else if (theirLevel < mLevel) {
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
    LOG.Message("ITC: To stage L%d%d", mLevel, mStage);
  }

  void ITC::incrementLevel(s32 amt) {
    if (amt == 0) return;
    s32 newLevel = mLevel + amt;
    if (newLevel < 0 || newLevel > 2) { // XXXX LEVEL NUMBER COUNT
      return;
    }
    mLevel = (u32) newLevel;
    mStage = 0;
    LOG.Message("ITC: %s to L%d0", amt > 0 ? "Up" : "Down", newLevel);
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
    LOG.Warning("ITC: UNRECOGNIZED Packet len=%d s='%s'",
                pb.GetLength(),z);
  }

  void ITC::update() {
    if (mUpdateTimeout == 0) {
      if (sendLevelPacket()) 
        mUpdateTimeout = getRandom().Between(1000,10000);
      else
        mUpdateTimeout = getRandom().Between(0,100);
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

    if (0) { /* COND */ }

    //// FIRST, WAITING FOR TILE MFMR 
    else if (mLevel == 0 && mStage == 1 && tlevel > 0) {
      // Tile has an MFZ running.
      incrementStageAndAnnounce(1); // Up to stage 2
    }

    //// SECOND, WATCHING FOR TILE BAILOUT
    else if (mLevel == 1 && tlevel == 0) {
      incrementLevel(-1);
      sendLevelPacket();
    }

    //// THIRD, TILE LEVEL 0 IS BORING
    else if (tlevel == 0) {
      // Nothing
    }

    else if (1) {
      LOG.Message("Uncovered handleTileChange %d",tm.getLevel());
    }
    
  }

  bool ITC::sendLevelPacket() {
    MFM_API_ASSERT(mLevel <= 3, ILLEGAL_STATE);
    MFM_API_ASSERT(mStage <= 3, ILLEGAL_STATE);
    u8 dir8 = mapDir6ToDir8(mDir6);
    unsigned char buf[4];
    buf[0] = 0xa0|(dir8&0x7); // 0xa==ROUTED MFM packet
    buf[1] = 'l';             // 'MFM::PacketType::LEVEL'
    buf[2] = '0'+mLevel;
    buf[3] = '0'+mStage;
    //LOG.Message("SLP L%d%d to %d",mLevel,mStage,dir8);
    return getMFMIO().trySendPacket(buf,4);
  }

  ITC::~ITC() { }
}
