#ifndef ELEMENT_H /* -*- C++ -*- */
#define ELEMENT_H

#include "itype.h"
#include "EventWindow.h"
#include "Dirs.h"

namespace MFM
{
  typedef u32 ElementType;

  template <class C> class Atom; // Forward declaration

  /**
   * ELEMENT_EMPTY is recognized at the element/elementtable level.
   */
  static const u32 ELEMENT_EMPTY  = 0x0;

  template <class CC>
  class Element
  {
  private:
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  protected:
    static const SPoint VNNeighbors[4];

    bool FillAvailableVNNeighbor(EventWindow<CC>& window, SPoint& pt) const;

    bool FillPointWithType(EventWindow<CC>& window,
			   SPoint& pt, const SPoint* relevants, u32 relevantCount,
			   Dir rotation, ElementType type) const;

    void Diffuse(EventWindow<CC>& window) const;

  public:

    Element()
    { }

    u32 GetType() const { return GetDefaultAtom().GetType(); }

    virtual const char * GetName() const = 0;

    virtual void Behavior(EventWindow<CC>& window) const = 0;

    virtual const T & GetDefaultAtom() const = 0;

    virtual u32 DefaultPhysicsColor() const = 0;

    /**
     * Determines how likely an Atom of this type is to be swapped
     * with during diffusal.
     *
     * @returns an integral percentage, from 0 to 100, describing the
     *          desire of an atom of this type to be moved; 0 being
     *          impossible to move and 100 being completely fine with
     *          moving.
     */
    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset)
    {
      return 100;
    }

    virtual u32 LocalPhysicsColor(const T &, u32 selector) const {
      return DefaultPhysicsColor();
    }

  };
}

#include "Element.tcc"

#endif /* ELEMENT_H */
