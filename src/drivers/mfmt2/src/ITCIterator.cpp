#include "ITCIterator.h"

namespace MFM {

  ITCIterator ITCIteration::begin() {
    ITCIterator ret(*this);
    return ret;
  }

  ITCIteration::~ITCIteration() {
    MFM_API_ASSERT_STATE(mIteratorsActive==0);
  }

}
