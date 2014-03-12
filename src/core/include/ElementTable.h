#ifndef ELEMENTTABLE_H      /* -*- C++ -*- */
#define ELEMENTTABLE_H

#include "BitVector.h"
#include "EventWindow.h"
#include "Dirs.h"
#include "itype.h"
#include "Element.h"

namespace MFM {

  template <class T,u32 R>
  class EventWindow;

  template <class T,u32 R,u32 B>
  class ElementTable
  {
  public:
    static const u32 SIZE = 1u<<B;

    void Insert(const Element<T,R> & theElement) ;

    u32 GetSize() const {
      return SIZE;
    }

    /**
     * Return -1 if elementType is not stored in this table, otherwise
     * return a number from 0.. GetSize()-1 representing the location
     * of this elementType in this table.
     */
    s32 GetIndex(u32 elementType) const ; 

    ElementTable();

    //  static ElementTable<T,R> & get();

    ~ElementTable() { }

    const Element<T,R> * Lookup(u32 elementType) ;

    void Execute(EventWindow<T,R>& window)
    { 
      u32 type = window.GetCenterAtom().GetType();
      if(type != ELEMENT_EMPTY) {
        const Element<T,R> * elt = Lookup(type);
        if (elt != 0)
          elt->Behavior(window);
        /* else WTH? */
      }
    }

    void FillAtom(T* atom, ElementType type);

    bool RegisterElement(const Element<T,R>& e)
    {
      Insert(e);
      return true;
    }

  private:

    static void FlipSEPointToCorner(Point<s32>& pt, Dir corner);

    /* Fills pt with the coordinates of a randomly selected          */
    /* Atom with the specified type. Returns false if there is none. */
    static bool FillSubWindowContaining(Point<s32>& pt, EventWindow<T,R>& window,
                                        ElementType type, 
                                        Dir corner);


    static void FillSubwindowIndices(s32* indices, EventWindow<T,R>& window,
                                     ElementType type, Dir corner);

    static u32 FoundIndicesCount(s32* indices);

    static void ReproduceVertically(EventWindow<T,R>& w, 
                                    ElementType type);
  

    u32 SlotFor(u32 elementType) const ; 

    const Element<T,R>* (m_hash[SIZE]);
    u32 m_hashSlotsInUse;
  
  };

} /* namespace MFM */

#include "ElementTable.tcc"

#endif /*ELEMENETTABLE_H*/
