#include <stdlib.h> /* -*- C++ -*- */
#include "Dirs.h"
#include "MDist.h"

namespace MFM {

  template <class T, u32 R, u32 B>
  bool ElementTable<T,R,B>::FillSubWindowContaining(Point<s32>& pt, EventWindow<T,R>& window,
                                                    ElementType type, Dir corner)
  {
    Random & random = window.GetRandom();
    u32 startIdx = random.Create(R);
  
    Point<s32> srcPt;
    for(u32 i = 0; i < R; i++)
      {
        srcPt = MDist<R>::get().GetSEWindowPoint((startIdx + i) % R);

        srcPt = Dirs::FlipSEPointToCorner(srcPt,corner);

        if(f(&window.GetRelativeAtom(srcPt)) == type)
          {
            pt.Set(srcPt.GetX(), srcPt.GetY());
            return true;
          }
      }

    return false;
  }

  /* Fills 'indices' with the indices of a Sub-windows of all       */
  /* relative atoms which have type 'type' .                        */
  /* Once all indices are found, a -1 is inserted, like a null      */
  /* terminator.                                                    */
  template <class T, u32 R, u32 B>
  void ElementTable<T,R,B>::FillSubwindowIndices(s32* indices,
                                                 EventWindow<T,R>& window,
                                                 ElementType type,
                                                 Dir corner)
  {
    /* As long as R is a power of two,             */
    /* ((R * R) / 4) is the size of one sub-window. */
    Point<s32> srcPt;
    for(u32 i = 0; i < ((R * R) / 4); i++)
      {
        srcPt = MDist<R>::get().GetSEWindowPoint(i);

        srcPt = Dirs::FlipSEPointToCorner(srcPt, corner);

        if(f(&window.GetRelativeAtom(srcPt)) == type)
          {
            *indices = MDist<R>::get().FromPoint(srcPt, MANHATTAN_TABLE_EVENT);
            indices++;
          }
      }
    *indices = -1;
  }

  template <class T ,u32 R, u32 B>
  u32 ElementTable<T,R,B>::FoundIndicesCount(s32* indices)
  {
    u32 count = 0;
    while(indices[count] != -1)
      {
        count++;
      }
    return count;
  }

  template <class T, u32 R, u32 B>
  void ElementTable<T,R,B>::ReproduceVertically(EventWindow<T,R>& window, ElementType type)
  {
    Random & random = window.GetRandom();

    u32 cval = window.GetCenterAtom().ReadLowerBits();
    bool down = random.CreateBool();
    SPoint repPt(0, down ? R/2 : -(R/2));
    if(window.GetRelativeAtom(repPt).GeType() == ELEMENT_EMPTY)
      {
        window.SetRelativeAtom(repPt, T(type));
        window.GetRelativeAtom(repPt).WriteLowerBits(cval + (down ? 1 : -1));
      }
  }

  template <class T,u32 R, u32 B>
  s32 ElementTable<T,R,B>::GetIndex(u32 elementType) const
  {
    u32 slot = SlotFor(elementType);
    if (m_hash[slot] == 0) return -1;
    return (s32) slot;
  }

  template <class T,u32 R, u32 B>
  u32 ElementTable<T,R,B>::SlotFor(u32 elementType) const
  {
    u32 collide = 0;
    u32 slot = elementType;
    while (true) {
      slot %= SIZE;
      const Element<T,R> * elt = m_hash[slot];
      if (elt==0 || elt->GetType() == elementType) 
        return slot;   // Empty or match: This is the slot for you
      ++collide;
      slot = elementType+(collide*(1+collide))/2;
    } 
  }

  template <class T,u32 R, u32 B>
  void ElementTable<T,R,B>::Insert(const Element<T,R> & theElement)
  {
    u32 type = theElement.GetType();
    u32 slotFor = SlotFor(type);

    if (m_hash[slotFor] != 0) {

      if (m_hash[slotFor] != &theElement)
        FAIL(DUPLICATE_ELEMENT_TYPE);
      else return;  

    } else {
      if (++m_hashSlotsInUse > SIZE/2)
        FAIL(TOO_MANY_ELEMENT_TYPES);
      m_hash[slotFor] = &theElement;
    }
  }

  template <class T,u32 R, u32 B>
  const Element<T,R> * ElementTable<T,R,B>::Lookup(u32 elementType) 
  {
    return m_hash[SlotFor(elementType)];
  }

  template <class T,u32 R, u32 B>
  ElementTable<T,R,B>::ElementTable()
  {
    Reinit();
  }

  template <class T,u32 R, u32 B>
  void ElementTable<T,R,B>::Reinit()
  {
    m_hashSlotsInUse = 0;
    for (u32 i = 0; i < SIZE; ++i) 
      m_hash[i] = 0;
  }

} /* namespace MFM */
