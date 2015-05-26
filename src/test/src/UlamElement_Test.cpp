#include "assert.h"
#include "UlamElement_Test.h"
#include "itype.h"

namespace MFM {

  struct UlamTypeCase {
    const char * mangled;
    const char * pretty;
    UlamTypeCase(const char * m, const char * p) : mangled(m), pretty(p) { }
  };

  const UlamTypeCase allCases[] =
    {
      UlamTypeCase("Ut_10111i","Int(1)"),
      UlamTypeCase("Ut_102321y","Unary"),
      UlamTypeCase("Ut_102321u","Unsigned"),
      UlamTypeCase("Ut_102321t","Bits"),
      UlamTypeCase("Ut_102311t","Bits(31)"),
      UlamTypeCase("Ut_10191y","Unary(9)"),
      UlamTypeCase("Ut_10111b","Bool"),
      UlamTypeCase("Ut_10161b","Bool(6)"),
      UlamTypeCase("Ut_112321i","Int[1]"),
      UlamTypeCase("Ut_211121y","Unary(2)[11]"),
      UlamTypeCase("Ut_232111b","Bool[32]"),
      UlamTypeCase("Ut_18141t","Bits(4)[8]"),
      UlamTypeCase("Ut_19131b","Bool(3)[9]"),

      UlamTypeCase("Uq_10104Fail10","quark Fail"),
      UlamTypeCase("Ue_102419212Display64x3210","element Display64x32"),
      UlamTypeCase("Ue_102689214WindowServices10","element WindowServices"),
      UlamTypeCase("Uq_102115IntXY12102321u16102321u15","quark IntXY(Unsigned=6u,Unsigned=5u)"),
      UlamTypeCase("Uq_102115IntXY12102321i16102321u15","quark IntXY(Int=6,Unsigned=5u)"),
      UlamTypeCase("Uq_102115IntXY12102321in16102321u10","quark IntXY(Int=-6,Unsigned=0u)"),
      UlamTypeCase("Uq_102115IntXY12102321in10102321u10","quark IntXY(Int=-2147483648,Unsigned=0u)"),
      UlamTypeCase("Uq_102117LoCoord13102321u16102321u15102311t74043954","quark LoCoord(Unsigned=6u,Unsigned=5u,Bits(31)=0x3DB4B2)"),
      UlamTypeCase("Uq_102117LoCoord13102321u16102321u15102321t814043954","quark LoCoord(Unsigned=6u,Unsigned=5u,Bits=0xD64B32)")
    };

  void UlamElement_Test::Test_RunTests() {

    for (u32 i = 0; i < sizeof(allCases)/sizeof(allCases[0]); ++i) {
      CharBufferByteSource cbs(allCases[i].mangled, strlen(allCases[i].mangled));
      UlamTypeInfo utin;
      assert(utin.InitFrom(cbs));

      OString512 os;
      utin.PrintMangled(os);
      assert(!strcmp(allCases[i].mangled,os.GetZString()));

      os.Reset();
      utin.PrintPretty(os);
      assert(!strcmp(allCases[i].pretty,os.GetZString()));
    }
  }


} /* namespace MFM */
