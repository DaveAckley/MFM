/* -*- C++ -*- */
#ifndef ELEMENT_CONSUMER_H
#define ELEMENT_CONSUMER_H

#include "Element.h"       
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Data.h"
#include "Element_Empty.h"
#include "Element_Emitter.h" /* For DATA_MAXVAL, DATA_MINVAL */
#include "Element_Reprovert.h"
#include "itype.h"
#include "P1Atom.h"
#include "Util.h"

#include "ExperimentValues.h" /* for EXPERIMENT_WIDTH, EXPERIMENT_HEIGHT */

namespace MFM
{
  template <class CC>
  class Element_Consumer : public Element_Reprovert<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { W = P::TILE_WIDTH };

  public:
    const char* GetName() const { return "Consumer"; }

    static Element_Consumer THE_INSTANCE;
    static const u32 TYPE = 0xdada0;
    
    static u64 bucketHitCounts;
    static u64 bucketMissCounts;
    /*
    static const u32 STATE_HIT_IDX  = Element_Reprovert<CC>::STATE_BITS;
    static const u32 STATE_HIT_LEN  = 16;
    static const u32 STATE_MISS_IDX = STATE_HIT_IDX + STATE_HIT_LEN;
    static const u32 STATE_MISS_LEN = 16;
    static const u32 STATE_BITS     = STATE_MISS_IDX + STATE_MISS_LEN;
    

    u32 GetAndResetHits(T& atom, u32 badType) const
    {
      if(!Atom<CC>::IsType(atom,TYPE)) return badType;
      u32 hits = atom.GetStateField(STATE_HIT_IDX,STATE_HIT_LEN);
      atom.SetStateField(STATE_HIT_IDX,STATE_HIT_LEN,0);
      return hits;
    }

    u32 GetAndResetMisses(T& atom, u32 badType) const
    {
      if(!Atom<CC>::IsType(atom,TYPE)) return badType;
      u32 misses = atom.GetStateField(STATE_MISS_IDX,STATE_MISS_LEN);
      atom.SetStateField(STATE_MISS_IDX,STATE_MISS_LEN,0);
      return misses;
    }
    
    bool IncrementMisses(T& atom)
    {
      if(!Atom<CC>::IsType(atom,TYPE)) return false;
      return true;
    }
    */

    Element_Consumer() { Element_Consumer<CC>::bucketHitCounts = 
	                 Element_Consumer<CC>::bucketMissCounts = 0; }

    u64 GetAndResetHits() const
    {
      u64 val = Element_Consumer<CC>::bucketHitCounts;
      Element_Consumer<CC>::bucketHitCounts = 0;
      return val;
    }

    u64 GetAndResetMisses() const
    {
      u64 val = Element_Consumer<CC>::bucketMissCounts;
      Element_Consumer<CC>::bucketMissCounts = 0;
      return val;
    }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Reprovert<CC>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff101010;
    }
    
    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      this->ReproduceVertically(window);

      Point<s32> consPt;
      MDist<R>::get().FillRandomSingleDir(consPt, random);
      
      if(window.GetRelativeAtom(consPt).GetType() == Element_Data<CC>::TYPE)
      {
	u32 val = DATA_MAXVAL - Element_Data<CC>::THE_INSTANCE.GetDatum(window.GetRelativeAtom(consPt),0);
	u32 bnum = this->GetVertPos(window.GetCenterAtom(),0);

	const u32 bucketSize = (DATA_MAXVAL / EXPERIMENT_HEIGHT) * 5;
	u32 minBucketVal = bucketSize * bnum;
	u32 maxBucketVal = bucketSize * (bnum + 1);

	if(val < minBucketVal || val > maxBucketVal)
	{
	  /* D'oh, wrong bucket
	  
	  u32 missAmt = ABS((val - minBucketVal) / bucketSize);
	  u32 oldMisses = window.GetCenterAtom().GetStateField(STATE_MISS_IDX,STATE_MISS_LEN);
	  window.GetCenterAtom().SetStateField(STATE_MISS_IDX,STATE_MISS_LEN,missAmt + oldMisses);
	  */

	  Element_Consumer<CC>::bucketMissCounts++;
	  
	}
	else
	{
	  /*
	  u32 oldHits = window.GetCenterAtom().GetStateField(STATE_HIT_IDX,STATE_HIT_LEN);
	  window.GetCenterAtom().SetStateField(STATE_HIT_IDX,STATE_HIT_LEN,oldHits + 1);
	  */
	  Element_Consumer<CC>::bucketHitCounts++;

	}

	// something sort of constant at equil.?
	/* printf("[%3d]Export!: %d sum %d\n", bnum, val, 3*bnum+val); */
	window.SetRelativeAtom(consPt, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }

    static void Needed();
    
  };

  template <class CC>
  Element_Consumer<CC> Element_Consumer<CC>::THE_INSTANCE;

  template <class CC>
  u64 Element_Consumer<CC>::bucketMissCounts;

  template <class CC>
  u64 Element_Consumer<CC>::bucketHitCounts;

}

#endif /* ELEMENT_CONSUMER_H */
