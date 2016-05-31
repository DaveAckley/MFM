/* -*- C++ -*- */

#include "Fail.h"
#include "Tile.h"

namespace MFM {

  template <class EC>
  void EventHistoryBuffer<EC>::AddEventWindow(const EventWindow<EC> & ew) 
  {
    if (!m_historyActive) return;
    const Tile<EC> & t = ew.GetTile();

    EventHistoryItem & s = AllocateNextItem();
    SPoint ctr = ew.GetCenterInTile();
    u32 siteInTile = t.GetSiteInTileNumber(ctr);
    u64 windowCount = ew.GetEventWindowsExecuted();
    s.MakeStart(siteInTile, windowCount);
    u32 itemsInEvent = 0;

    const MDist<R> & md = MDist<R>::get();

    for (u32 i = 0; i < ew.SITE_COUNT; ++i)
    {
      const SPoint pt = md.GetPoint(i) + ctr;
      itemsInEvent = RecordAtomChanges(i, *t.GetAtom(pt), ew.GetAtomDirect(i), itemsInEvent);
    }

    itemsInEvent = RecordBaseChanges(t.GetSite(ctr).GetBase(), ew.GetBase(), itemsInEvent);
     if (itemsInEvent == 0) 
     {
       DeallocateCurrentItem(); // Fugedabowdit
     } else
     {
       EventHistoryItem & e = AllocateNextItem();
       ++itemsInEvent;
       e.MakeEnd(itemsInEvent, windowCount);
     }
   }

   template <class EC>
   u32 EventHistoryBuffer<EC>::RecordAtomChanges(u32 siteInWindow, const T& oldAtom, const T& newAtom, u32 itemsInEvent) 
   {

     for (u32 i = 0; i < 96/32; ++i) 
     {
       u32 oldw = oldAtom.GetBits().Read(i*32,32);
       u32 neww = newAtom.GetBits().Read(i*32,32);
       if (oldw != neww)
       {
         EventHistoryItem & item = AllocateNextItem();
         ++itemsInEvent;
         item.MakeDelta(siteInWindow, itemsInEvent, i, oldw, neww);
       }
     }
     return itemsInEvent;
   }

   template <class EC>
   u32 EventHistoryBuffer<EC>::RecordBaseChanges(const Base<AC>& oldBase, const Base<AC>& newBase, u32 itemsInEvent) 
   {
     itemsInEvent = RecordAtomChanges(BASE_ATOM, oldBase.GetBaseAtom(), newBase.GetBaseAtom(), itemsInEvent);
     itemsInEvent = RecordSensorChanges(oldBase.GetSensory(), newBase.GetSensory(), itemsInEvent);
     if (oldBase.GetPaint() != newBase.GetPaint())
     {
       EventHistoryItem & item = AllocateNextItem();
       ++itemsInEvent;
       item.MakeDelta(BASE_PAINT, itemsInEvent, 0, oldBase.GetPaint(), newBase.GetPaint());
     }
     return itemsInEvent;
   }

   template <class EC>
   u32 EventHistoryBuffer<EC>::RecordSensorChanges(const SiteSensors& oldSense, const SiteSensors& newSense, u32 itemsInEvent) 
   {
     if (oldSense.m_touchSensor.m_touchType != newSense.m_touchSensor.m_touchType) 
     {
       EventHistoryItem & item = AllocateNextItem();
       ++itemsInEvent;
       item.MakeDelta(SITE_SENSORS, itemsInEvent, 0, oldSense.m_touchSensor.m_touchType, newSense.m_touchSensor.m_touchType);
     }
     u64 oldec = oldSense.m_touchSensor.m_lastTouchEventCount;
     u64 newec = newSense.m_touchSensor.m_lastTouchEventCount;
     for (u32 i = 0; i < 2; ++i) 
     {
       u32 o32 = (u32) (oldec>>(i*32));
       u32 n32 = (u32) (newec>>(i*32));
       if (o32 != n32) 
       {
         EventHistoryItem & item = AllocateNextItem();
         ++itemsInEvent;
         item.MakeDelta(SITE_SENSORS, itemsInEvent, i+1, o32, n32);
       }
     }
     
     return itemsInEvent;
   }
} /* namespace MFM */
