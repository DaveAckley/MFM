#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"

namespace MFM {

  template <class EC>
  void UlamElement<EC>::Print(ByteSink & bs, const T & atom, u32 flags) const
  {
    if (!flags) return;

    if (flags & PRINT_SYMBOL) bs.Printf("(%s)", this->GetAtomicSymbol());

    if (flags & PRINT_FULL_NAME) bs.Printf("%s",this->GetName());

    if (flags & PRINT_ATOM_BODY)
    {
      typedef typename EC::ATOM_CONFIG AC;
      T dup = atom; // Get mutable copy (lame AtomSerializer)
      AtomSerializer<AC> as(dup);
      bs.Printf(":%@", &as);
    }

    if (flags & (PRINT_MEMBER_VALUES|PRINT_MEMBER_NAMES|PRINT_MEMBER_TYPES))
    {
      bool opened = false;
      for (s32 i = 0; i < GetDataMemberCount(); ++i)
      {
        const UlamClassDataMemberInfo & dmi = GetDataMemberInfo((u32) i);
        UlamTypeInfo utin;
        if (!utin.InitFrom(dmi.m_mangledType))
          FAIL(ILLEGAL_STATE);

        // Skip size 0 members unless they reeeally want them
        if (utin.GetBitSize() == 0 && !(flags & PRINT_SIZE0_MEMBERS)) continue;

        if (!opened)
        {
          opened = true;
          bs.Printf("(");
        }
        else
        {
          bs.Printf(",");
        }
        if (flags & PRINT_MEMBER_TYPES)
        {
          utin.PrintPretty(bs);
          bs.Printf(" ");
        }

        if (flags & PRINT_MEMBER_NAMES)
        {
          bs.Printf("%s", dmi.m_dataMemberName);
          if (flags & PRINT_MEMBER_VALUES)
            bs.Printf("=");
        }

        if (flags & PRINT_MEMBER_VALUES)
        {
          // For starters just dig out the bits and print them
          u32 val = atom.GetBits().Read(dmi.m_bitPosition + T::ATOM_FIRST_STATE_BIT, utin.GetBitSize());

          if (utin.m_category == UlamTypeInfo::QUARK ||
              (utin.m_category == UlamTypeInfo::PRIM && utin.m_utip.GetPrimType() >= UlamTypeInfoPrimitive::BOOL))
            bs.Printf("0x%x", val);
          else
            bs.Printf("%d", val);
        }
      }
      if (opened)
        bs.Printf(")");
    }
  }
}
