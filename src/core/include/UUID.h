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
#include "ByteSource.h"
#include "ByteSerializable.h"
#include <string.h>    /* For strlen, strncpy */

#define MFM_UUID_FOR(label, apiVersion) UUID(label,(u32) apiVersion, (u32) MFM_BUILD_DATE, (u32) MFM_BUILD_TIME, UUID::ComputeConfigurationCode<CC>())

namespace MFM {

  class UUID : public ByteSerializable {
  public:
    static const u32 MAX_LABEL_LENGTH = 63;

    UUID() : m_configurationCode(0), m_apiVersion(0), m_hexDate(0), m_hexTime(0)
    {
      m_label[0] = '\0';
    }

    template <class CC>
    static u32 ComputeConfigurationCode() {
      u32 val = 0;
      val = (val<<4) + CC::ATOM_TYPE::ATOM_CATEGORY;
      val = (val<<4) + CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS;
      val = (val<<4) + CC::PARAM_CONFIG::ELEMENT_TABLE_BITS;
      val = (val<<4) + CC::PARAM_CONFIG::ELEMENT_DATA_SLOTS;
      val = (val<<8) + CC::PARAM_CONFIG::TILE_WIDTH;
      val = (val<<8) + CC::PARAM_CONFIG::BITS_PER_ATOM;
      return val;
    }

    UUID(const char * label, const u32 apiVersion, const u32 decDate, const u32 decTime, const u32 configCode)
      : m_configurationCode(configCode),
        m_apiVersion(apiVersion),
        m_hexDate(decDate), m_hexTime(decTime)
    {
      if (!label)
        FAIL(NULL_POINTER);
      strncpy(m_label, label, MAX_LABEL_LENGTH + 1);
    }

    UUID(ByteSource & bs) ;

    const char * GetLabel() const { return m_label; }
    u32 GetVersion() const { return m_apiVersion; }
    u32 GetHexDate() const { return m_hexDate; }
    u32 GetHexTime() const { return m_hexTime; }
    u32 GetConfigurationCode() const { return m_configurationCode; }

    bool CompatibleConfigurationCode(const UUID & other) const ;
    bool CompatibleLabel(const UUID & other) const ;
    bool CompatibleAPIVersion(const UUID & other) const ;
    bool CompatibleButStrictlyNewer(const UUID & other) const ;

    bool Compatible(const UUID & other) const ; //< CompatibleAPIVersion plus not older date

    void Print(ByteSink & bs) const ;

    Result PrintTo(ByteSink & bs, s32 argument = 0) {
      Print(bs);
      return SUCCESS;
    }

    bool Read(ByteSource & bs) ;

    Result ReadFrom(ByteSource & bs, s32 argument = 0) {
      if (Read(bs))
        return SUCCESS;
      return FAILURE;
    }

    bool Equals(const UUID & other) const {
      return *this == other;
    }

    bool operator==(const UUID & other) const ;

    bool operator!=(const UUID & other) const {
      return !(*this == other);
    }

    virtual ~UUID() { }

  private:

    s32 CompareDateOnly(const UUID & other) const ;

    char m_label[MAX_LABEL_LENGTH + 1];
    u32 m_configurationCode;
    u32 m_apiVersion;
    u32 m_hexDate;
    u32 m_hexTime;

  };

}

#endif /* UUID_H */
