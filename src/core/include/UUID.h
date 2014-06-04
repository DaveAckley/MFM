/*                                              -*- mode:C++ -*-
  UUID.h ID System for Elements
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
  \file UUID.h ID System for Elements
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef UUID_H
#define UUID_H

#include "itype.h"
#include "ByteSink.h"

#define MFM_UUID_FOR(label, apiVersion) UUID(label,apiVersion,MFM_BUILD_DATE,MFM_BUILD_TIME)

namespace MFM {

  struct UUID : public ByteSinkable {
    const char * const m_label;
    const u32 m_apiVersion;
    const u32 m_hexDate;
    const u32 m_hexTime;

    UUID(const char * label, const u32 apiVersion, const u32 hexDate, const u32 hexTime) ;

    const char * GetLabel() const { return m_label; }
    u32 GetVersion() const { return m_apiVersion; }
    u32 GetHexDate() const { return m_hexDate; }
    u32 GetHexTime() const { return m_hexTime; }

    bool CompatibleLabel(const UUID & other) const ;
    bool CompatibleAPIVersion(const UUID & other) const ;
    bool CompatibleButStrictlyNewer(const UUID & other) const ;

    void Print(ByteSink & bs) const ;

    void PrintTo(ByteSink & bs, s32 argument = 0) {
      Print(bs);
    }

    bool operator==(const UUID & other) const ;

    bool operator!=(const UUID & other) const {
      return !(*this == other);
    }

  private:
    s32 CompareDateOnly(const UUID & other) const ;
  };

}

#endif /* UUID_H */
