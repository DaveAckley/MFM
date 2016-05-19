#include "UlamClass.h"

namespace MFM {

  template<class EC>
  void DebugPrint(const UlamClassRegistry<EC>& uc, const UlamRef<EC>& ur, ByteSink& out)
  {
    const u32 printFlags =
      UlamClassPrintFlags::PRINT_MEMBER_NAMES |
      UlamClassPrintFlags::PRINT_MEMBER_VALUES |
      UlamClassPrintFlags::PRINT_RECURSE_QUARKS;

    ur.Print(uc, out, printFlags);
  }

  template<class EC>
  void DebugPrint(const UlamClassRegistry<EC>&uc, const UlamRef<EC>& ur)
  {
    DebugPrint(uc,ur,STDOUT);
  }

  template<class EC>
  void InitDebugTools()
  {
    UlamClassRegistry<EC> ucr;
    AtomBitStorage<EC> bs;
    UlamRef<EC> ur(0,0,bs,0);
    DebugPrint(ucr, ur, DevNullByteSink);
  }
}
