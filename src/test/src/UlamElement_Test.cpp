#include "assert.h"
#include "UlamElement_Test.h"
#include "itype.h"

namespace MFM {

  struct UlamTypeCase {
    const char * mangled;
    const char * pretty;
    UlamTypeCase(const char * m, const char * p) : mangled(m), pretty(p) { }
  };

  const UlamTypeCase primCases[] =
    {
      UlamTypeCase("10111i","Int(1)"),
      UlamTypeCase("102321y","Unary"),
      UlamTypeCase("102321u","Unsigned"),
      UlamTypeCase("102321t","Bits"),
      UlamTypeCase("102311t","Bits(31)"),
      UlamTypeCase("10191y","Unary(9)"),
      UlamTypeCase("10111b","Bool"),
      UlamTypeCase("10161b","Bool(6)"),
      UlamTypeCase("112321i","Int[1]"),
      UlamTypeCase("211121y","Unary(2)[11]"),
      UlamTypeCase("232111b","Bool[32]"),
      UlamTypeCase("18141t","Bits(4)[8]"),
      UlamTypeCase("19131b","Bool(3)[9]")

    };

  const UlamTypeCase classCases[] =
    {
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
    {
      for (u32 i = 0; i < sizeof(primCases)/sizeof(primCases[0]); ++i) {
        CharBufferByteSource cbs(primCases[i].mangled, strlen(primCases[i].mangled));
        UlamTypeInfoPrimitive utip;
        assert(utip.InitFrom(cbs));

        OString512 os;
        utip.PrintMangled(os);
        assert(!strcmp(primCases[i].mangled,os.GetZString()));

        os.Reset();
        utip.PrintPretty(os);
        assert(!strcmp(primCases[i].pretty,os.GetZString()));
      }
    }
    {
      for (u32 i = 0; i < sizeof(classCases)/sizeof(classCases[0]); ++i) {
        CharBufferByteSource cbs(classCases[i].mangled, strlen(classCases[i].mangled));
        UlamTypeInfoClass utic;
        assert(utic.InitFrom(cbs));

        OString512 os;
        utic.PrintMangled(os);
        assert(!strcmp(classCases[i].mangled,os.GetZString()));

        os.Reset();
        utic.PrintPretty(os);
        assert(!strcmp(classCases[i].pretty,os.GetZString()));
      }
    }
  }

} /* namespace MFM */
