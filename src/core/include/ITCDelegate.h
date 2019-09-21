/* -*- mode:C++ -*- */
/**
  ITCDelegate.h A stub representing intertile communications machinery
  Copyright (C) 2019 The T2 Tile Project.  All rights reserved.

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
  \file ITCDelegate.h A stub representing intertile communications machinery.
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \lgpl
 */
#ifndef ITCDELEGATE_H
#define ITCDELEGATE_H

#include "Point.h"   /* For SPoint */
#include "Packet.h" /* For PacketBuffer */

#include <stdlib.h>  /* for abort() */

namespace MFM
{
  template <class EC> class Tile; // FORWARD
  template <class EC> class EventWindow; // FORWARD
  template <class EC> class CacheProcessor; // FORWARD
  template <class EC> class LonglivedLock; // FORWARD

  template <class EC>
  struct ITCDelegate {
    virtual void InitializeTile(Tile<EC> & tile) = 0;
    virtual bool ConsiderEventWindow(EventWindow<EC> & ew) = 0;
    virtual bool AdvanceCommunicationPredicate(Tile<EC> & tile) = 0;
    virtual bool TryLock(LonglivedLock<EC> & lll, const CacheProcessor<EC> &cp) = 0;
    virtual bool Unlock(LonglivedLock<EC> & lll, const CacheProcessor<EC> &cp) = 0;
    virtual bool ReceivePacket(CacheProcessor<EC> &cp, PacketBuffer & to) = 0;
    //    virtual bool AcquireAllLocks(EventWindow<EC> & ew, const SPoint& tileCenter, const u32 eventWindowBoundary) = 0;
    virtual bool ShipBufferAsPacket(CacheProcessor<EC> & cp, PacketBuffer & pb) = 0;
    //    virtual bool AdvanceReceiving(CacheProcessor<EC> &cp) = 0;
    virtual bool IsConnected(const CacheProcessor<EC> &cp) = 0;
  };
} /* namespace MFM */

#endif /*ITCDELEGATE_H*/
