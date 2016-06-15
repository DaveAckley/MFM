/* -*- C++ -*- */

#include "Fail.h"
#include "EventWindow.h"
#include "Tile.h"

namespace MFM {

  
  template <class EC>
  bool EventHistoryBuffer<EC>::StepBackward() 
  {
    if (m_cursor < 0) return false;  // not set up
    FAIL(INCOMPLETE_CODE);
  }

  template <class EC>
  u32 EventHistoryBuffer<EC>::CountEventsInHistory() const
  {
    if (!m_historyActive) return 0;
    u32 events = 0;
    for (u32 i = m_oldestEventStart; i != m_newestEventEnd; )
    {
      EventHistoryItem & item = m_historyBuffer[i];
      if (item.IsStart())
      {
        ++events;
        s32 next = EndOfEventStartedHere(i);
        MFM_API_ASSERT_STATE(next >= 0);
        i = (u32) next;
      }
      else if (item.IsEnd())
      {
        i = GetWrappedIndex(i + 1);
      }
    }
    return events;
  }

  template <class EC>
  void EventHistoryBuffer<EC>::ApplyDelta(bool toNewer, EventHistoryItem::DeltaItem & di, Tile<EC>& tile, const SPoint ctr)
  {
    const MDist<R> & md = MDist<R>::get();
    u32 site = di.m_site;
    if (site < md.GetSiteCount())
    {
      const SPoint pt = md.GetPoint(site) + ctr;
      T& atom = *tile.GetWritableAtom(pt);
      u32 idx = di.m_word;
      u32 val = toNewer ? di.m_newValue : di.m_oldValue;
      atom.GetBits().Write(idx*32, 32, val);
    }
    else
    {
      // base info
      FAIL(INCOMPLETE_CODE);
    }
    tile.NeedAtomRecount();
  }

  template <class EC>
  void EventHistoryBuffer<EC>::AddEventStart(const SPoint ctr) 
  {
    if (!m_historyActive) return;
    MFM_API_ASSERT_STATE(!m_makingEvent);
    EventHistoryItem & s = AllocateNextItem();
    m_makingEventStart = m_newestEventEnd; // well that's confusing
    s.MakeStart(ctr, ++m_eventsAdded);
    m_makingEvent = true;
    m_itemsInEvent = 0;
  }

  template <class EC>
  void EventHistoryBuffer<EC>::AddEventAtom(u32 siteInWindow, const T & oldAtom, const T & newAtom) 
  {
    if (!m_historyActive) return;
    MFM_API_ASSERT_STATE(m_makingEvent);
    RecordAtomChanges(siteInWindow, oldAtom, newAtom);
  }

  template <class EC>
  void EventHistoryBuffer<EC>::AddEventEnd()
  {
    if (!m_historyActive) return;
    MFM_API_ASSERT_STATE(m_makingEvent);

    if (m_itemsInEvent == 0) 
    {
      DeallocateCurrentStart(); 
    } 
    else
    {
      EventHistoryItem & s = m_historyBuffer[m_makingEventStart];
      EventHistoryItem & e = AllocateNextItem();
      ++m_itemsInEvent;
      e.MakeEnd(s, m_itemsInEvent);
      m_cursor = (s32) m_newestEventEnd;
      s.mHeaderItem.m_count = m_itemsInEvent;  // Point start header back to us
    }
    m_makingEvent = false;
  }

  template <class EC>
  void EventHistoryBuffer<EC>::AddEventWindow(const EventWindow<EC> & ew) 
  {
    if (!m_historyActive) return;
    const Tile<EC> & t = ew.GetTile();

    SPoint ctr = ew.GetCenterInTile();

    EventHistoryItem & s = AllocateNextItem();
    s.MakeStart(ctr, ++m_eventsAdded);
    m_itemsInEvent = 0;

    const MDist<R> & md = MDist<R>::get();

    for (u32 i = 0; i < ew.GetBoundedSiteCount(); ++i)
    {
      const SPoint pt = md.GetPoint(i) + ctr;
      RecordAtomChanges(i, *t.GetAtom(pt), ew.GetAtomDirect(i));
    }

    RecordBaseChanges(t.GetSite(ctr).GetBase(), ew.GetBase());
     if (m_itemsInEvent == 0) 
     {
       DeallocateCurrentStart(); // Fugedabowdit
     } else
     {
       EventHistoryItem & e = AllocateNextItem();
       ++m_itemsInEvent;
       e.MakeEnd(s, m_itemsInEvent);
       m_cursor = (s32) m_newestEventEnd;
       s.mHeaderItem.m_count = m_itemsInEvent;  // Point start header back to us
     }
   }

   template <class EC>
   void EventHistoryBuffer<EC>::RecordAtomChanges(u32 siteInWindow, const T& oldAtom, const T& newAtom) 
   {

     for (u32 i = 0; i < 96/32; ++i) 
     {
       u32 oldw = oldAtom.GetBits().Read(i*32,32);
       u32 neww = newAtom.GetBits().Read(i*32,32);
       if (oldw != neww)
       {
         EventHistoryItem & item = AllocateNextItem();
         ++m_itemsInEvent;
         item.MakeDelta(siteInWindow, m_itemsInEvent, i, oldw, neww);
       }
     }
   }

   template <class EC>
   void EventHistoryBuffer<EC>::RecordBaseChanges(const Base<AC>& oldBase, const Base<AC>& newBase) 
   {
     RecordAtomChanges(BASE_ATOM, oldBase.GetBaseAtom(), newBase.GetBaseAtom());
     RecordSensorChanges(oldBase.GetSensory(), newBase.GetSensory());
     if (oldBase.GetPaint() != newBase.GetPaint())
     {
       EventHistoryItem & item = AllocateNextItem();
       ++m_itemsInEvent;
       item.MakeDelta(BASE_PAINT, m_itemsInEvent, 0, oldBase.GetPaint(), newBase.GetPaint());
     }
   }

   template <class EC>
   void EventHistoryBuffer<EC>::RecordSensorChanges(const SiteSensors& oldSense, const SiteSensors& newSense) 
   {
     if (oldSense.m_touchSensor.m_touchType != newSense.m_touchSensor.m_touchType) 
     {
       EventHistoryItem & item = AllocateNextItem();
       ++m_itemsInEvent;
       item.MakeDelta(SITE_SENSORS, m_itemsInEvent, 0, oldSense.m_touchSensor.m_touchType, newSense.m_touchSensor.m_touchType);
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
         ++m_itemsInEvent;
         item.MakeDelta(SITE_SENSORS, m_itemsInEvent, i+1, o32, n32);
       }
     }
   }

   template <class EC>
   void EventHistoryBuffer<EC>::Print(ByteSink& bs)  const
   {
     bs.Printf("[EventHistoryBuffer(%p)", (void*) this);
     bs.Printf(",active=%d", m_historyActive);
     if (m_historyActive)
     {
       for (u32 i = m_oldestEventStart; i != m_newestEventEnd; )
       {
         EventHistoryItem & item = m_historyBuffer[i];
         bs.Printf("\n %d: ", i);
         item.Print(bs);
         if (item.IsStart())
         {
           s32 next = EndOfEventStartedHere(i);
           if (next >= 0)
           {
             i = (u32) next;
           }
         }
         i = GetWrappedIndex(i + 1);
       }
     }
     bs.Printf("]\n");
   }

} /* namespace MFM */
