#ifndef ELEMENTTABLE_H      /* -*- C++ -*- */
#define ELEMENTTABLE_H

#include "BitVector.h"
#include "EventWindow.h"
#include "Dirs.h"
#include "itype.h"
#include "Element.h"

namespace MFM {

  template <class CC>
  class EventWindow;

  template <class CC>
  class ElementTable
  {
    // Extract short type names
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { B = P::ELEMENT_TABLE_BITS};
    enum { R = P::EVENT_WINDOW_RADIUS};

  public:
    static const u32 SIZE = 1u<<B;

    /**
     * Reinitialize this ElementTable to empty.
     */
    void Reinit() ;

    void Insert(const Element<CC> & theElement) ;

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

    ~ElementTable() { }

    const Element<CC> * Lookup(u32 elementType) ;

    void Execute(EventWindow<CC>& window)
    { 
      u32 type = window.GetCenterAtom().GetType();
      if(type != ELEMENT_EMPTY) {
        const Element<CC> * elt = Lookup(type);
        if (elt == 0) FAIL(UNKNOWN_ELEMENT);
        elt->Behavior(window);
      }
    }

    void FillAtom(T* atom, ElementType type);

    bool RegisterElement(const Element<CC>& e)
    {
      Insert(e);
      return true;
    }

  private:

    //    static void FlipSEPointToCorner(Point<s32>& pt, Dir corner);

    /* Fills pt with the coordinates of a randomly selected          */
    /* Atom with the specified type. Returns false if there is none. */
    static bool FillSubWindowContaining(Point<s32>& pt, EventWindow<CC>& window,
                                        ElementType type, 
                                        Dir corner);


    static void FillSubwindowIndices(s32* indices, EventWindow<CC>& window,
                                     ElementType type, Dir corner);

    static u32 FoundIndicesCount(s32* indices);

    static void ReproduceVertically(EventWindow<CC>& w, 
                                    ElementType type);
  

    u32 SlotFor(u32 elementType) const ; 

    const Element<CC>* (m_hash[SIZE]);
    u32 m_hashSlotsInUse;
  
  };

} /* namespace MFM */

#include "ElementTable.tcc"

#endif /*ELEMENETTABLE_H*/
