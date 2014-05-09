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


    /**
     * Determines how likely an Atom of this type is to be swapped
     * with during diffusal.
     *
     * @returns an integral percentage, from 0 to 100, describing the
     *          desire of an atom of this type to be moved; 0 being
     *          impossible to move and 100 being completely fine with
     *          moving.
     */
    virtual u32 PercentMovable(const T& you,
			       const T& me, const SPoint& offset) const = 0;

  public:

    Element()
    { }

    u32 GetType() const { return GetDefaultAtom().GetType(); }

    virtual const char * GetName() const = 0;

    virtual void Behavior(EventWindow<CC>& window) const = 0;

    virtual const T & GetDefaultAtom() const = 0;

    virtual u32 DefaultPhysicsColor() const = 0;

    virtual u32 LocalPhysicsColor(const T &, u32 selector) const {
      return DefaultPhysicsColor();
    }

    /**
     * On entry, the Atom at \a nowAt will be an instance of the type
     * of this Element.  How much does that atom like the idea that it
     * should be moved to (or remain at; the two SPoints might be
     * equal) location \a maybeAt?  Return 0 to totally hate the idea,
     * or greater than 0 to request that many chances for this choice
     * out of the sum of all choices.  Note that \a nowAt and \a
     * maybeAt are both relative to the EventWindow \a ew, but neither
     * is necessarily the center atom!
     *
     * By default all atoms are considered diffusable, and they return
     * DEFAULT_DIFFUSABILITY chances for every suggested move
     */
    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const {
      return DEFAULT_DIFFUSABILITY;
    }

    /**
       A utility method available to subclasses that specifies no
       diffusion is allowed, by specifying a 0 value for the
       Diffusability of every position except when \a nowAt == \a
       maybeAt.
     */
    u32 NoDiffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const {
      return (nowAt == maybeAt)? DEFAULT_DIFFUSABILITY : 0;
    }

    /**
     * A standard basis for specifying degrees of diffusability.
     */
    static const u32 DEFAULT_DIFFUSABILITY = 1000;

  };
}

#include "Element.tcc"

#endif /* ELEMENT_H */
