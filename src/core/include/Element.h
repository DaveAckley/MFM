#ifndef ELEMENT_H /* -*- C++ -*- */
#define ELEMENT_H

#include "itype.h"
#include "EventWindow.h"
#include "Dirs.h"

namespace MFM
{
  typedef u32 ElementType;

  /**
   * ELEMENT_EMPTY is recognized at the element/elementtable level.
   */
  static const u32 ELEMENT_EMPTY  = 0x0;

  template <class T, u32 R>
  class Element
  {
  private:
    
  protected:

    static SPoint VNNeighbors[4];
    
    void FlipSEPointToCorner(SPoint& readPt, SPoint& outPt, Dir corner) const;

    bool FillAvailableVNNeighbor(EventWindow<T,R>& window, SPoint& pt) const;

    bool FillPointWithType(EventWindow<T,R>& window, 
			   SPoint& pt, SPoint* relevants, u32 relevantCount,
			   Dir rotation, ElementType type) const;
      
    void Diffuse(EventWindow<T,R>& window) const;

  public:

    Element() 
    { }

    u32 GetType() const { return GetDefaultAtom().GetType(); }
    
    virtual const char * GetName() const = 0;

    virtual void Behavior(EventWindow<T,R>& window) const = 0;

    virtual const T & GetDefaultAtom() const = 0;

    virtual u32 DefaultPhysicsColor() const = 0;
    
  };
}
  
#include "Element.tcc"

#endif /* ELEMENT_H */
