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
  };

  struct SiteSensors {
    SiteTouchSensor m_touchSensor;

    void Touch(SiteTouchType type, u64 eventCount)
    {
      m_touchSensor.Touch(type, eventCount);
    }

    SiteTouchType RecentTouch(u64 eventCount) const
    {
      return m_touchSensor.RecentTouch(eventCount);
    }
  };
}

#endif /* SENSE_H */
