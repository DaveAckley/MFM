/* -*- C++ -*- */
#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"
#include "UlamRef.h"
#include "UlamClass.h"
#include "UlamClassRegistry.h"

namespace MFM {

  template <class EC>
  void UlamElement<EC>::Behavior(EventWindow<EC>& window) const
  {
    Tile<EC> & tile = window.GetTile();
    UlamContext<EC> uc;
    uc.SetTile(tile);

    u32 sym = m_info ? m_info->GetSymmetry(uc) : PSYM_DEG000L;
    window.SetSymmetry((PointSymmetry) sym);

    AtomRefBitStorage<EC> atbs(window.GetCenterAtomSym());
    UlamRef<EC> ur(T::ATOM_FIRST_STATE_BIT, this->GetClassLength(), atbs, this);

    // how to do an ulam virtual function call in c++
    typedef void (* Uf_6behave) (const UlamContext<EC>&, UlamRef<EC>& );
    ((Uf_6behave) this->getVTableEntry(BEHAVE_VTABLE_INDEX)) (uc, ur);
  }

  template <class EC>
  void UlamElement<EC>::Print(const UlamClassRegistry<EC> & ucr, ByteSink & bs, const T & atom, u32 flags, u32 basestatepos) const
  {
    if (!flags) return;

    if (flags & UlamClass<EC>::PRINT_SYMBOL) bs.Printf("(%s) ", this->GetAtomicSymbol());

    if (flags & UlamClass<EC>::PRINT_FULL_NAME) bs.Printf("%s",this->GetName());

    if (flags & UlamClass<EC>::PRINT_INDENTED_LINES) bs.Printf("\n ");

    if (flags & UlamClass<EC>::PRINT_ATOM_BODY)
    {
      typedef typename EC::ATOM_CONFIG AC;
      T dup = atom; // Get mutable copy (lame AtomSerializer)
      AtomSerializer<AC> as(dup);
      bs.Printf(":%@", &as);
    }

    if (flags & UlamClass<EC>::PRINT_INDENTED_LINES) bs.Printf("\n");

    this->UlamClass<EC>::PrintClassMembers(ucr,bs,atom,flags,basestatepos);
  }
} //MFM
