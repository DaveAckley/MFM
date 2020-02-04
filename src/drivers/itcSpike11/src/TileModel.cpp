#include "TileModel.h"

namespace MFM {
  u32 TileModel::getLevel() const {
    return mLevel;
  }

  u32 TileModel::getGeneration() const {
    return mGeneration;
  }

  const char * TileModel::getMFZVersionString() const { // result meaningless if getLevel()==0
    return mMFZVersionString.GetZString();
  }

  void TileModel::input(Event& evt) {
    LOG.Message("Tile EVENT INPUT?");
  }

  void TileModel::output(ByteSink& bs) {
    bs.Printf("L%d gen %d\n",getLevel(),getGeneration());
    if (getMFZVersionString())
      bs.Printf("RUNNING %s\n",getMFZVersionString());
  }

  void TileModel::update() {
    if (mRandom.OneIn(10000)) {
      switch (mLevel) {
      case 0:
        ++mLevel;
        {
          u32 mfzversion = mRandom.Between(0,3);
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
        if (mRandom.OneIn(50)) {
          ++mGeneration;
          mLevel = 0;
          LOG.Message("Tile Level %d: Quitting", mLevel);
        }
        break;
      default:
        LOG.Error("Tile Level %d UNRECOGNIZED", mLevel);
      }
    }
  }

  TileModel::TileModel(Random & random,SDLI & sdli)
    : mRandom(random)
    , mSDLI(sdli)
    , mLevel(0)
    , mGeneration(0)
  {
    mMFZVersionString.Reset();
    mMFZVersionString.Printf("--UNSET--");
  }

  TileModel::~TileModel() { }
}

