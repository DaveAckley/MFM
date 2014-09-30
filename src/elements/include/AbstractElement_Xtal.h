/*                                              -*- mode:C++ -*-
  AbstractElement_Xtal.h Common attributes of simple Xtal forms
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file AbstractElement_Xtal.h Common attributes of simple Xtal forms
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTELEMENT_XTAL_H
#define ABSTRACTELEMENT_XTAL_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P3Atom.h"
#include "Element_Res.h"

namespace MFM
{
  template <class CC>
  class AbstractElement_Xtal : public Element<CC>
  {
  public:

    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum {
      R = P::EVENT_WINDOW_RADIUS,
      SITES = EVENT_WINDOW_SITES(R),
      BITS = P::BITS_PER_ATOM,

      //////
      // Element state fields

      XTAL_FIRST_FREE_POS = P3Atom<P>::P3_STATE_BITS_POS

    };

    typedef BitVector<SITES> XtalSites;

    static u32 GetIndex(const SPoint coord)
    {
      const MDist<R> & md = MDist<R>::get();
      s32 idx = md.FromPoint(coord, R);
      if (idx < 0)
      {
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }
      return (u32) idx;
    }

    static bool ReadBit(const XtalSites & sites, const SPoint coord)
    {
      return sites.ReadBit(GetIndex(coord));
    }

    static void WriteBit(XtalSites & sites, const SPoint coord, bool val)
    {
      sites.WriteBit(GetIndex(coord), val);
    }

    virtual void GetSites(T & atom, XtalSites & xites, EventWindow<CC>& window) const = 0;

    /**
     * Called when self and otherAtom are identical subtypes of
     * AbstractElement_Xtal, to check if there are any additional
     * details that might cause them to actually be different xtals.
     * Given the types are already known identical, IsSameXtal returns
     * true by default, but is overridden by, for example,
     * Element_Xtal_General.
     */
    virtual bool IsSameXtal(T & self, const T & otherAtom, EventWindow<CC>& window) const
    {
      return true;
    }

    virtual u32 GetSymI(T &atom, EventWindow<CC>& window) const = 0;

    AbstractElement_Xtal(const UUID & uuid) : Element<CC>(uuid)
    {
    }

    /**
       Xtals don't diffuse
     */
    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return this->NoDiffusability(ew, nowAt, maybeAt);
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    bool IsAbstractXtalType(EventWindow<CC>& window, u32 type) const
    {
      const Element<CC> * elt = window.GetTile().GetElement(type);
      return dynamic_cast<const AbstractElement_Xtal<CC>*>(elt) != 0;
    }

    struct SiteSampler {
      Random & random;
      u32 count;
      SPoint sampleSite;
      T sampleSiteContents;
      SiteSampler(Random & rand) : random(rand), count(0) { }
      void Sample(const SPoint & place, const T & content) {
        if (random.OneIn(++count))
        {
          sampleSite = place;
          sampleSiteContents = content;
        }
      }
    };

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();
      const u32 ourType = this->GetType();
      const MDist<R> & md = MDist<R>::get();

      T self = window.GetCenterAtom();

      // Find out what this Xtal looks like
      XtalSites xtalSites;
      GetSites(self, xtalSites, window);

      // Establish our symmetry before non-self access through window
      u32 symi = this->GetSymI(self, window);
      window.SetSymmetry((PointSymmetry) symi);

      // Xtal windows are considered to consist of two types of sites:
      // (1) 'Point' sites, which should ideally be occupied by xtals like us
      //      We count and sample empty, res, non-us-xtal and non-xtal point sites separately
      u32 totalPointSiteCount = 0;
      u32 totalConsistentPointSiteCount = 0;
      SiteSampler inconsistentEmptyPointSites(random);
      SiteSampler inconsistentResPointSites(random);
      SiteSampler inconsistentXtalPointSites(random);
      SiteSampler inconsistentNonxtalPointSites(random);

      // (2) 'Field' sites, which should ideally be empty or Res, but
      //      surely not xtal.  We count and sample empty, res, us,
      //      and non-us-xtal field sites separately
      u32 totalFieldSiteCount = 0;
      SiteSampler emptyFieldSites(random);
      SiteSampler resFieldSites(random);
      SiteSampler usFieldSites(random);
      SiteSampler nonusXtalFieldSites(random);

      // Scan event window _including_ self site
      for (u32 idx = md.GetFirstIndex(0); idx <= md.GetLastIndex(R); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        // First question: Is this a live site?
        if (!window.IsLiveSite(sp))
          continue;

        // Second question: Is this a point site or a field site?
        bool isPoint = xtalSites.ReadBit(idx) !=0 ;

        const T other = window.GetRelativeAtom(sp);
        const u32 otherType = other.GetType();

        if (isPoint) {
          ++totalPointSiteCount;

          // Next question: Is point site empty and thus potential growth spot?

          bool isEmpty = Element_Empty<CC>::THE_INSTANCE.IsType(otherType);

          if (isEmpty)
          {
            inconsistentEmptyPointSites.Sample(sp, other);
          }
          else
          {
            bool isRes = otherType == Element_Res<CC>::TYPE();
            if (isRes)
            {
              inconsistentResPointSites.Sample(sp, other);
            }
            else
            {
              // Next question: Is occupied point site _really_ one of us?
              if (otherType == ourType && this->IsSameXtal(self, other, window))
              {
                // Yes
                ++totalConsistentPointSiteCount;
              }
              else
              {
                // They are not one of us.  Are they _any_ std xtal?
                bool isXtal = IsAbstractXtalType(window, otherType);

                if (isXtal)
                {
                  inconsistentXtalPointSites.Sample(sp, other);
                }
                else
                {
                  inconsistentNonxtalPointSites.Sample(sp, other);
                }
              }
            }
          }
        }
        else
        {
          // This is a field site.  We'd like it to be empty or Res,
          // but we'd really like it not to be us-xtal
          ++totalFieldSiteCount;

          bool isRes = otherType == Element_Res<CC>::TYPE();
          if (isRes)
          {
            resFieldSites.Sample(sp, other);
          }
          else
          {
            bool isEmpty = Element_Empty<CC>::THE_INSTANCE.IsType(otherType);

            if (isEmpty)
            {
              emptyFieldSites.Sample(sp, other);
            }
            else
            {
              // Neither res nor empty.  Are they us?
              if (otherType == ourType && this->IsSameXtal(self, other, window))
              {
                usFieldSites.Sample(sp, other);
              }
              else
              {
                // No, are they any other std xtal?
                bool isXtal = IsAbstractXtalType(window, otherType);
                if (isXtal)
                {
                  nonusXtalFieldSites.Sample(sp, other);
                }
              }
            }
          }
        }
      }

      // Scan finished.  Let's decide what to do.

      // Cases in priority order: Do the first that applies, then
      // done.
      //
      // (0) Are we strictly more inconsistent (usFieldSites.count)
      //     than consistent (totalConsistentPointSiteCount) with our
      //     own kind?  If so, assume we're the problem, and res out.
      if (usFieldSites.count > totalConsistentPointSiteCount)
      {
        window.SetCenterAtom(Element_Res<CC>::THE_INSTANCE.GetDefaultAtom());
      }
      // (1) Is there a field us and a point empty?  If so, swap them
      else if (usFieldSites.count > 0 && inconsistentEmptyPointSites.count > 0)
      {
        window.SwapAtoms(usFieldSites.sampleSite, inconsistentEmptyPointSites.sampleSite);
      }
      // (2) Is there a point res?  If so, make it us
      else if (inconsistentResPointSites.count > 0)
      {
        window.SetRelativeAtom(inconsistentResPointSites.sampleSite, self);
      }
      // (3) Is there a point empty and a field res?  If so, swap and make the point us
      else if (resFieldSites.count > 0 && inconsistentEmptyPointSites.count > 0)
      {
        window.SwapAtoms(resFieldSites.sampleSite, inconsistentEmptyPointSites.sampleSite);
        window.SetRelativeAtom(inconsistentEmptyPointSites.sampleSite, self);
      }
      // (4) Is there a point non-us-xtal?  If so, res it out
      else if (inconsistentXtalPointSites.count > 0 && inconsistentEmptyPointSites.count > 0)
      {
        window.SetRelativeAtom(inconsistentXtalPointSites.sampleSite,
                               Element_Res<CC>::THE_INSTANCE.GetDefaultAtom());
      }

      /*
      if (inconsistentCount > 0)
      {
        // Next question: Are we strictly more consistent than inconsistent?
        if (consistentCount > inconsistentCount && !random.OneIn(xtalSiteCount + 1))
        {
          // Yes.  Punish selected loser
          window.SetRelativeAtom(anInconsistent, unmakeGuy);
        }
        else if (inconsistentCount > consistentCount)
        {
          // If we're strictly more inconsistent, res out and let them have us
          window.SetCenterAtom(Element_Res<CC>::THE_INSTANCE.GetDefaultAtom());
        }
      }
      else
      {
        // No inconsistencies.  Do we have something to make, and eat?
        if (makeCount > 0 && eatCount > 0)
        {
          window.SetRelativeAtom(toMake, makeGuy);
          window.SetRelativeAtom(toEat, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
        }
      }
      */
    }
  };
}

#endif /* ABSTRACTELEMENT_XTAL_H */
