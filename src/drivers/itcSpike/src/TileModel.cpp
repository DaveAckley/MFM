#include "TileModel.h"

namespace MFM {
  u32 TileModel::getLevel() const {
    return mLevel;
  }

  const char * TileModel::getMFZVersionString() const { // result meaningless if getLevel()==0
    return mMFZVersionString.GetZString();
  }

  void TileModel::update() {
    if (mRandom.OneIn(10000)) {
      switch (mLevel) {
      case 0:
        ++mLevel;
        {
          u32 mfzversion = mRandom.Between(0,2);
          mMFZVersionString.Reset();
          mMFZVersionString.Printf("MFZ%d",mfzversion);
          LOG.Message("Tile Level %d: Starting %s", mLevel, getMFZVersionString());
        }
        break;
      case 1:
        ++mLevel;
        LOG.Message("Tile Level %d: Accepting events", mLevel);
        break;
      case 2:
        if (mRandom.OneIn(20)) {
          mLevel = 0;
          LOG.Message("Tile Level %d: Quitting", mLevel);
        }
        break;
      default:
        LOG.Error("Tile Level %d UNRECOGNIZED", mLevel);
      }
    }
  }

  TileModel::TileModel(Random & random)
    : mRandom(random)
    , mLevel(0)
  {
    mMFZVersionString.Reset();
    mMFZVersionString.Printf("--UNSET--");
  }

  TileModel::~TileModel() { }
}

