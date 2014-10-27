/* -*- C++ -*- */
#include "Fail.h"
#include "Dirs.h"
#include "Element_Empty.h"

namespace MFM
{

#if 0
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
#elsif 0 /* 'full' Four way diffusion */
  template <class CC>
  void Element<CC>::Diffuse(EventWindow<CC>& window) const
  {
    Random & random = window.GetRandom();
    Tile<CC>& tile = window.GetTile();
    SPoint pick = SPoint(0,0);
    u32 pickWeight = 0;
    const MDist<R> md = MDist<R>::get();

    for (u32 idx = md.GetFirstIndex(0); idx <= md.GetLastIndex(1); ++idx) {
      const SPoint sp = md.GetPoint(idx);
      T other = window.GetRelativeAtom(sp);
      const Element * elt = tile.GetElement(other.GetType());

      if (!other.IsSane() || !(elt = tile.GetElement(other.GetType()))) {
        if (sp.IsZero()) return;  // If ctr insane or undefined, we're done
        else continue;            // If nghbr, just ignore it
      }

      u32 thisWeight = elt->Diffusability(window, sp, SPoint(0,0));
      if (thisWeight > 0) {
        pickWeight += thisWeight;
        if (random.OddsOf(thisWeight, pickWeight))
          pick = sp;
      }
    }
    if (pick != SPoint(0,0))
      window.SwapAtoms(pick, SPoint(0, 0));
  }
#else  /* 'faster' weighted one way diffusion */
  template <class CC>
  void Element<CC>::Diffuse(EventWindow<CC>& window) const
  {
    Random & random = window.GetRandom();
    Tile<CC>& tile = window.GetTile();
    const MDist<R> & md = MDist<R>::get();

    SPoint sp;
    md.FillRandomSingleDir(sp, random);

    // Don't diffuse stuff into the great nowhere, but consider
    // 'bouncing' off the edge of the universe
    if (!window.IsLiveSite(sp)) {
      sp *= -1;
      if (!window.IsLiveSite(sp))  // Wow this is a tight universe!
        return;
    }

    T other = window.GetRelativeAtom(sp);
    const Element * elt = tile.GetElement(other.GetType());

    if (!other.IsSane() || !(elt = tile.GetElement(other.GetType())))
      return;       // Any confusion, let the engine sort it out first

    u32 thisWeight = elt->Diffusability(window, sp, SPoint(0,0));
    if (random.OddsOf(thisWeight, COMPLETE_DIFFUSABILITY))
      window.SwapAtoms(sp, SPoint(0, 0));
  }

#endif
}
