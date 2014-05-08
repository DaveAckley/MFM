/* -*- C++ -*- */
#include "Fail.h"
#include "Dirs.h"

namespace MFM
{
  template <class CC>
  const SPoint Element<CC>::VNNeighbors[4] =
  {
      SPoint(-1, 0), SPoint(1, 0), SPoint(0, -1), SPoint(0, 1)
  };

  /* Fills 'pt' with the value of a randomly selected empty von neumann */
  /* neighbor.                                                          */
  /* Returns false if there is no valid neighbor to be used.            */
  template <class CC>
  bool Element<CC>::FillAvailableVNNeighbor(EventWindow<CC>& window, SPoint& pt) const
  {
    return FillPointWithType(window, pt, VNNeighbors, 4, Dirs::SOUTHEAST, ELEMENT_EMPTY);
  }

  /* Master search method for finding atoms in regions in a window. If regions are
     symmetric about the origin, rotation does not make a difference.
   */
  template <class CC>
  bool Element<CC>::FillPointWithType(EventWindow<CC>& window,
				       SPoint& pt, const SPoint* relevants, u32 relevantCount,
				       Dir rotation, ElementType type) const
  {
    Random & random = window.GetRandom();
    u32 possibles = 0;
    for(u32 i = 0; i < relevantCount; i++)
    {
      SPoint current = Dirs::FlipSEPointToCorner(relevants[i], rotation);

      /* Live site?  Right type?  Lucky? */
      if(window.IsLiveSite(current) &&
	 (window.GetRelativeAtom(current).GetType() == type) &&
         random.OneIn(++possibles))
        pt = current;
    }
    return possibles > 0;
  }

#if 1
  template <class CC>
  void Element<CC>::Diffuse(EventWindow<CC>& window) const
  {
    SPoint pt;
    /* Make a pariring of direction to neighbor's move desires */
    Dir dirs[] = {Dirs::NORTH, Dirs::EAST, Dirs::SOUTH, Dirs::WEST};
    s32 desires[4] = {0};
    s32 totalDesire = 0;
    for(u32 i = 0; i < 4; i++)
    {
      Dirs::FillDir(pt, dirs[i]);
      u32 atomType= window.GetRelativeAtom(pt).GetType();

      desires[i] = (s32)window.GetTile().GetElementTable().Lookup(atomType)->
	PercentMovable(window.GetCenterAtom(), window.GetRelativeAtom(pt), pt);
      totalDesire += desires[i];
    }

    /* Pick a plausible swap partner */
    s32 swapIdx = (s32)window.GetTile().GetRandom().Create((u32)totalDesire);
    for(u32 i = 0; i < 4; i++)
    {
      if(swapIdx < desires[i])
      {
	Dirs::FillDir(pt, dirs[i]);
	window.SwapAtoms(pt, SPoint(0, 0));
	return;
      }
      swapIdx = MAX(swapIdx - desires[i], 0);
    }
  }
#else  /* Eight way diffusion */
  template <class CC>
  void Element<CC>::Diffuse(EventWindow<CC>& window) const
  {
    Random & random = window.GetRandom();
    SPoint pick;
    u32 picked = 0;
    const MDist<R> md = MDist<R>::get();

    for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(2); ++idx) {
      const SPoint sp = md.GetPoint(idx);
      if (sp.GetMaximumLength() > 1) continue;
      T other = window.GetRelativeAtom(sp);
      if (other.GetType() == ELEMENT_EMPTY && random.OneIn(++picked))
        pick = sp;
    }
    if (picked > 0)
      window.SwapAtoms(pick, SPoint(0, 0));
  }
#endif
}
