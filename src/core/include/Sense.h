/*                                              -*- mode:C++ -*-
  Sense.h Sensorimotor support
  Copyright (C) 2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file Sense.h Sensorimotor support
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef SENSE_H
#define SENSE_H

#include "itype.h"
#include "ByteSink.h"
#include "AtomSerializer.h"

namespace MFM {

  enum SiteTouchType {
    TOUCH_TYPE_NONE,
    TOUCH_TYPE_PROXIMITY,
    TOUCH_TYPE_LIGHT,
    TOUCH_TYPE_HEAVY
  };

  struct SiteTouchSensor {
    SiteTouchType m_touchType;
    u64 m_lastTouchEventCount;

    void Clear() 
    {
      m_touchType = TOUCH_TYPE_NONE;
      m_lastTouchEventCount = 0;
    }

    void Touch(SiteTouchType type, u64 eventCount)
    {
      m_touchType = type;
      m_lastTouchEventCount = eventCount;
    }

    SiteTouchType RecentTouch(u64 relativeToEventCount) const
    {
      return RecentTouch(relativeToEventCount, 10);
    }

    SiteTouchType RecentTouch(u64 relativeToEventCount, u32 withinSiteEvents) const
    {
      if (m_lastTouchEventCount + withinSiteEvents >= relativeToEventCount)
        return m_touchType;
      return TOUCH_TYPE_NONE;
    }

    template <class AC>
    void SaveConfig(ByteSink& bs, AtomTypeFormatter<AC> & atf) const
    {
      bs.Printf(",%D", m_touchType);
      bs.Print(m_lastTouchEventCount, Format::LXX64);
    }

    template <class AC>
    bool LoadConfig(LineCountingByteSource & bs, AtomTypeFormatter<AC> & atf)
    {
      u32 tmp_m_touchType;
      if (2 != bs.Scanf(",%D", &tmp_m_touchType)) return false;

      u64 tmp_m_lastTouchEventCount;
      if (!bs.Scan(tmp_m_lastTouchEventCount, Format::LXX64)) return false;

      m_touchType = (SiteTouchType) tmp_m_touchType;
      m_lastTouchEventCount = tmp_m_lastTouchEventCount;
      return true;
    }

  };

  struct SiteSensors {
    SiteTouchSensor m_touchSensor;

    void Clear() 
    {
      m_touchSensor.Clear();
    }

    void Touch(SiteTouchType type, u64 eventCount)
    {
      m_touchSensor.Touch(type, eventCount);
    }

    SiteTouchType RecentTouch(u64 eventCount) const
    {
      return m_touchSensor.RecentTouch(eventCount);
    }

    template<class AC>
    void SaveConfig(ByteSink& bs, AtomTypeFormatter<AC> & atf) const
    {
      m_touchSensor.SaveConfig(bs,atf);
    }

    template<class AC>
    bool LoadConfig(LineCountingByteSource & bs, AtomTypeFormatter<AC> & atf)
    {
      return m_touchSensor.LoadConfig(bs,atf);
    }


  };
}

#endif /* SENSE_H */
