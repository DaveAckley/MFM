/* -*- C++ -*- */
#include "Fail.h"
#include "Tile.h"
#include "Random.h"
#include "EventWindow.h"
#include "Base.h"
#include "UlamRef.h"
#include "UlamClass.h"
#include "UlamClassRegistry.h"
#include "UlamContextEvent.h"
#include "UlamContextRestricted.h"

namespace MFM {

  template <class EC>
  u32 UlamElement<EC>::GetEventWindowBoundary() const
  {
    if (!m_info) return Super::GetEventWindowBoundary();
    return m_info->GetEventWindowBoundary();
  }

  template <class EC>
  void UlamElement<EC>::Behavior(EventWindow<EC>& window) const
  {
    Tile<EC> & tile = window.GetTile();
    const ElementTable<EC> & et = tile.GetElementTable();
    UlamContextEvent<EC> uc(et);
    uc.SetTile(tile);

    u32 sym = m_info ? m_info->GetSymmetry(uc) : PSYM_DEG000L;
    window.SetSymmetry((PointSymmetry) sym);

    UlamRef<EC> ur(T::ATOM_FIRST_STATE_BIT, this->GetClassLength(), window.GetCenterAtomBitStorage(), this, UlamRef<EC>::ELEMENTAL, uc);

    // how to do an ulam virtual function call in c++
    typedef void (* Uf_6behave) (const UlamContext<EC>&, UlamRef<EC>& );
    ((Uf_6behave) this->getVTableEntry(BEHAVE_VTABLE_INDEX)) (uc, ur);
  }

  template <class EC>
  u32 UlamElement<EC>::GetAtomColor(const ElementTable<EC> & et, const UlamClassRegistry<EC> & ucr, const T& atom, u32 selector) const
  {
    if (selector == 0)
      return GetElementColor();

    UlamContextRestricted<EC> uc(et,ucr);
    T temp(atom);
    Ui_Ut_102321u<EC> sel(selector);
    AtomBitStorage<EC> atbs(temp);
    UlamRef<EC> ur(T::ATOM_FIRST_STATE_BIT, this->GetClassLength(), atbs, this, UlamRef<EC>::ELEMENTAL, uc);

    // how to do an ulam virtual function call in c++
    typedef Ui_Ut_14181u<EC> (* Uf_8getColor11102321u) (const UlamContext<EC>&, UlamRef<EC>&, Ui_Ut_102321u<EC>& );
    Ui_Ut_14181u<EC> dynColor = ((Uf_8getColor11102321u) this->getVTableEntry(GETCOLOR_VTABLE_INDEX)) (uc, ur, sel);

    return dynColor.read();
  }

  template <class EC>
  u32 UlamElement<EC>::Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
  {
    if (nowAt == maybeAt || !m_info) return COMPLETE_DIFFUSABILITY;
    return
      COMPLETE_DIFFUSABILITY * m_info->GetPercentDiffusability() / 100;
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
