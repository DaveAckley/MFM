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
#include "OverflowableCharBufferByteSink.h"
#include "ZStringByteSource.h"
#include "ByteSerializable.h"
#include <string.h>    /* For strlen, strncpy */

#define MFM_UUID_FOR(label, elementVersion) \
  UUID(label,(u32) elementVersion, (u32) MFM_BUILD_DATE, (u32) MFM_BUILD_TIME, UUID::ComputeConfigurationCode<CC>())

namespace MFM {

  /**
   * A class representing a 'Universal Unique ID' for an Element.  A
   * UUID is meant to be a 'fingerprint' of an Element.
   */
  class UUID : public ByteSerializable {
  public:

    /**
       This is the version of the UUID system itself (not the version
       of any particular element).  This must and shall only be
       incremented when the serialized format of UUIDs changes.  The
       UUID API version must always remain the first component of the
       UUID after the element name, and if it mismatches what is
       expected, the UUID shall be declared incompatible without even
       reading the rest of the UUID.
     */
    static const u32 API_VERSION = 1;

    UUID() : m_configurationCode(0), m_elementVersion(0), m_uuidVersion(1), m_hexDate(0), m_hexTime(0)
    {
      m_label.Reset();
      m_label.GetZString();
    }

    /**
       ComputeFullConfigurationCode produces a 32 bit value that
       depends on all configuration parameters.  In API version 0
       (prior to v2.1.0), Element UUIDs included this code and
       required it to be identical between element versions.

       @sa ComputeConfigurationCode
     */
    template <class CC>
    static u32 ComputeFullConfigurationCode() {
      u32 val = 0;
      val = (val<<4) + CC::ATOM_TYPE::ATOM_CATEGORY;
      val = (val<<4) + CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS;
      val = (val<<4) + CC::PARAM_CONFIG::ELEMENT_TABLE_BITS;
      val = (val<<4) + CC::PARAM_CONFIG::ELEMENT_DATA_SLOTS;
      val = (val<<8) + CC::PARAM_CONFIG::TILE_WIDTH;
      val = (val<<8) + CC::PARAM_CONFIG::BITS_PER_ATOM;
      return val;
    }

    /**
       ComputeConfigurationCode produces a 16 bit value that depends
       on all configuration parameters that could plausibly affect
       Element behavior -- specifically, the ATOM_CATEGORY (e.g., the
       P3 Atom), the EVENT_WINDOW_RADIUS, and the BITS_PER_ATOM.  (All
       current ATOM_CATEGORY have specific requirements for
       BITS_PER_ATOM, so it's not exactly clear why the latter is
       included separately) dependent on the ATOM_CATEGORY).

       In API version 1, Element UUIDs include this code and require
       it to be identical between element versions.

       \sa ComputeFullConfigurationCode
       \since UUID API version 1
     */
    template <class CC>
    static u32 ComputeConfigurationCode() {
      u32 val = 0;
      val = (val<<4) + CC::ATOM_TYPE::ATOM_CATEGORY;
      val = (val<<4) + CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS;
      val = (val<<8) + CC::PARAM_CONFIG::BITS_PER_ATOM;
      return val;
    }

    UUID(const char * label, const u32 elementVersion, const u32 decDate, const u32 decTime, const u32 configCode)
      : m_configurationCode(configCode),
        m_elementVersion(elementVersion),
        m_uuidVersion(API_VERSION),
        m_hexDate(decDate), m_hexTime(decTime)
    {
      if (!label)
        FAIL(NULL_POINTER);
      if (!LegalLabel(label))
        FAIL(ILLEGAL_ARGUMENT);

      m_label.Reset();
      m_label.Print(label);
      m_label.GetZString(); // Ensure label is null-terminated
    }

    /**
     * Is \a label a legally-formatted label for an Element?  This
     * includes just the name, not the additional info following the
     * '-'.
     */
    static bool LegalLabel(const char * label)
    {
      if (!label)
        return false;

      OString64 buf;

      ZStringByteSource zbs(label);
      if (!zbs.ScanCamelIdentifier(buf))
        return false;

      if (zbs.Read() >= 0)  // Need EOF now
        return false;

      if (buf.HasOverflowed())
        return false;

      return true;
    }

    static bool LegalFilename(const char* label)
    {
      ZStringByteSource zbs(label);

      UUID temp;
      return 4==zbs.Scanf("%@.so",&temp);
    }

    UUID(ByteSource & bs) ;

    const char * GetLabel() const {
      return m_label.GetBuffer();  // m_label writers have ensured null-termination
    }
    u32 GetElementVersion() const { return m_elementVersion; }
    u32 GetUUIDVersion() const { return m_uuidVersion; }
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

    OString64 m_label;
    u32 m_configurationCode;
    u32 m_elementVersion;
    u32 m_uuidVersion;
    u32 m_hexDate;
    u32 m_hexTime;

  };

}

#endif /* UUID_H */
