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
#include "Tile.h"

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

    // We build this at ctor time rather than using a function-scoped
    // static and relying on the gcc's deadlock-prone static
    // initialization lock
    const T m_defaultAtom;

  public:

    // Element Data Slot names
    enum {
      DATUMS_CONSUMED_SLOT,
      TOTAL_BUCKET_ERROR_SLOT,
      DATA_SLOT_COUNT
    };

    const char* GetName() const { return "Consumer"; }

    static Element_Consumer THE_INSTANCE;
    static const u32 TYPE = 0xdad0;

    Element_Consumer() : m_defaultAtom(BuildDefaultAtom())
    {
    }

    u64 GetAndResetDatumsConsumed(Tile<CC> & t) const
    {
      ElementTable<CC> & et = t.GetElementTable();
      u64 * datap = et.GetDataIfRegistered(TYPE, DATA_SLOT_COUNT);
      if (!datap)
        return 0;

      u64 ret = datap[DATUMS_CONSUMED_SLOT];
      datap[DATUMS_CONSUMED_SLOT] = 0;
      return ret;
    }

    u64 GetAndResetBucketError(Tile<CC> & t) const
    {
      ElementTable<CC> & et = t.GetElementTable();
      u64 * datap = et.GetDataIfRegistered(TYPE, DATA_SLOT_COUNT);
      if (!datap)
        return 0;

      u64 ret = datap[TOTAL_BUCKET_ERROR_SLOT];
      datap[TOTAL_BUCKET_ERROR_SLOT] = 0;
      return ret;
    }

    const T BuildDefaultAtom() const {
      T defaultAtom(TYPE,0,0,Element_Reprovert<CC>::STATE_BITS);
      this->SetGap(defaultAtom,1); // Pack consumers adjacent
      return defaultAtom;
    }

    virtual const T & GetDefaultAtom() const
    {
      return m_defaultAtom;
    }

    virtual u32 PercentMovable(const T& you,
			       const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff202030;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      //      Random & random = window.GetRandom();
      this->ReproduceVertically(window);

      T self = window.GetCenterAtom();

      // Find nearest-on-right consumable, if any
      for (u32 i = 1; i < R; ++i) {
        SPoint consPt(i,0);
        if(window.GetRelativeAtom(consPt).GetType() == Element_Data<CC>::TYPE)
          {
            // Use the expanded info maintained by reprovert to
            // dynamically compute the bucket size.  That info can be
            // temporarily disrupted e.g. by DReg, so going this route
            // does increase the overall sorting error a bit.  But it's so
            // damn robust.  Turn off an output tile and it starts sorting

            u32 above = this->GetAbove(self,0);
            u32 below = this->GetBelow(self,0);
            u32 range = above + below + 1;
            const u32 bucketSize = DATA_MAXVAL / range;

            u32 val = Element_Data<CC>::THE_INSTANCE.GetDatum(window.GetRelativeAtom(consPt), 0);

            u32 minBucketVal = bucketSize * above;
            u32 maxBucketVal = bucketSize * (above + 1);
            u32 midpoint = (maxBucketVal+minBucketVal)/2;

            u32 diff;
            if (val < midpoint) diff = midpoint - val;
            else diff = val - midpoint;

            u32 bucketsOff = diff / bucketSize;

            Tile<CC> & tile = window.GetTile();
            ElementTable<CC> & et = tile.GetElementTable();

            u64 * datap = et.GetDataAndRegister(TYPE,DATA_SLOT_COUNT);
            ++datap[DATUMS_CONSUMED_SLOT];                 // Count datums consumed
            datap[TOTAL_BUCKET_ERROR_SLOT] += bucketsOff;  // Count total bucket error

            /*
              printf("[%d:%d:%d/bs %d>%d<%d=%d]Export!: %d %ld %ld %f\n",
              below,
              range,
              above,
              bucketSize,
              minBucketVal,
              maxBucketVal,
              bucketsOff,
              val,
              datap[DATUMS_CONSUMED_SLOT], datap[TOTAL_BUCKET_ERROR_SLOT],
              ((double)datap[TOTAL_BUCKET_ERROR_SLOT])/datap[DATUMS_CONSUMED_SLOT]);
            */
            window.SetRelativeAtom(consPt, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
            break;
          }
      }
    }

    static void Needed();

  };

  template <class CC>
  Element_Consumer<CC> Element_Consumer<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_CONSUMER_H */
