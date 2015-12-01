#include "UlamClass.h"

namespace MFM {

  void UlamClass::addHex(ByteSink & bs, u64 val)
  {
    bs.Printf("/0x");
    bs.Print(val, Format::HEX);
  }
} //MFM
