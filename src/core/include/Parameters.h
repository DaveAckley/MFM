/*                                              -*- mode:C++ -*-
  Parameters.h Abstract class for a set of configurable parameters
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
  \file Parameters.h Abstract class for a set of configurable parameters
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "ByteSerializable.h"
#include "OverflowableCharBufferByteSink.h"
#include "Util.h"

namespace MFM
{
  class Parameters
  {
   public:
    enum KnownParameterTypes
    {
      S32_PARAMETER = 1,
      BOOL_PARAMETER = 2,

      ILLEGAL_PARAMETER = 0
    };

    class Parameter : public ByteSerializable
    {
     private:
      const char * m_tag;
      const char * m_name;
      const char * m_description;

      friend class Parameters;
      Parameter * m_next;

    public:
      Parameter(Parameters& c, const char* tag, const char* name, const char* description)
        : m_tag(tag),
          m_name(name),
          m_description(description),
          m_next(0)
      {
        if (!m_tag || !m_name || !m_description)
        {
          FAIL(NULL_POINTER);
        }
        if (!strlen(m_tag) || !strlen(m_name))
        {
          FAIL(ILLEGAL_ARGUMENT);
        }
        c.AddParameter(this);
      }

      virtual u32 GetParameterType() const = 0;

      virtual void Print(ByteSink & bs) const = 0;

      virtual bool Read(ByteSource & bs) = 0;

      virtual ~Parameter()
      { }

      const char* GetTag() const
      {
        return m_tag;
      }

      const char* GetName() const
      {
        return m_name;
      }

      const char* GetDescription() const
      {
        return m_description;
      }

      Result PrintTo(ByteSink & bs, s32 argument = 0)
      {
        Print(bs);
        return SUCCESS;
      }

      Result ReadFrom(ByteSource & bs, s32 argument = 0)
      {
        if (Read(bs))
          return SUCCESS;
        return FAILURE;
      }

    };

    class S32 : public Parameter
    {
      s32 m_current;
      s32 m_min;
      s32 m_initial;
      s32 m_max;
      s32 m_snap;

    public:

      static S32 * Cast(Parameter * p)
      {
        return dynamic_cast<S32*>(p);
      }

      virtual u32 GetParameterType() const
      {
        return S32_PARAMETER;
      }

      virtual void Print(ByteSink & bs) const
      {
        bs.Print(m_current);
      }

      virtual bool Read(ByteSource & bs)
      {
        s32 val;
        if (!bs.Scan(val))
        {
          return false;
        }
        if (val < m_min || val > m_max)
        {
          return false;
        }

        m_current = val;
        return true;
      }

      S32(Parameters& c,
          const char* tag,
          const char* name,
          const char* description,
          s32 min, s32 initial, s32 max, s32 snap)
        : Parameter(c, tag, name, description),
          m_min(min),
          m_initial(initial),
          m_max(max),
          m_snap(snap)
      {
        if (m_max < m_min)
        {
          m_max = m_min;
        }
        SetValue(initial);
      }

      virtual s32 GetValue() const
      {
        return m_current;
      }

      virtual void SetValue(s32 value)
      {
        value = CLAMP(m_min, m_max, value);
        value -= value % m_snap;
        m_current = CLAMP(m_min, m_max, value);
      }

      virtual s32 GetMin() const
      {
        return m_min;
      }

      virtual s32 GetMax() const
      {
        return m_max;
      }

      u32 GetRange() const
      {
        return m_max - m_min + 1;
      }

      u32 MapValue(u32 max, s32 val) const
      {
        val = CLAMP(m_min, m_max, val);

        u32 zval = (u32) (val - m_min);

        return zval * max / GetRange();
      }

      /**
       * Gets the snapping resolution of a parameter. This is used to
       * snap a slider to a particular number of values. If not
       * specified, the default is one.
       *
       * @returns The snapping resolution of this Parameter.
       */
      virtual u32 GetSnap() const
      {
        return m_snap;
      }
    };

    class Bool : public Parameter
    {
      bool m_current;
      bool m_initial;

    public:

      static Bool * Cast(Parameter * p)
      {
        return dynamic_cast<Bool*>(p);
      }

      virtual u32 GetParameterType() const
      {
        return BOOL_PARAMETER;
      }

      virtual void Print(ByteSink & bs) const
      {
        bs.Print(m_current?"true":"false");
      }

      virtual bool Read(ByteSource & bs)
      {
        OString16 val;
        if (!bs.ScanIdentifier(val) || val.HasOverflowed())
        {
          return false;
        }

        if (val.Equals("true"))
        {
          m_current = true;
          return true;
        }

        if (val.Equals("false"))
        {
          m_current = false;
          return true;
        }

        return false;
      }

      Bool(Parameters& c,
          const char* tag,
          const char* name,
          const char* description,
          bool initial)
        : Parameter(c, tag, name, description),
          m_initial(initial)
      {
        SetValue(m_initial);
      }

      virtual bool GetValue() const
      {
        return m_current;
      }

      virtual void SetValue(bool value)
      {
        m_current = value;
      }

    };

    Parameters() :
      m_firstParameter(0)
    {
    }

    u32 GetParameterCount() const
    {
      u32 count = 0;
      Parameter * p = m_firstParameter;
      while (p)
      {
        ++count;
        p = p->m_next;
      }
      return count;
    }

    s32 GetParameterNumber(const Parameter * ap) const
    {
      MFM_API_ASSERT_NONNULL(ap);
      u32 number = 0;
      for (Parameter * p = m_firstParameter; p; ++number, p = p->m_next)
      {
        if (p == ap)
        {
          return (s32) number;
        }
      }
      return -1;
    }

    s32 GetParameterNumberFromTag(const char * tag) const
    {
      MFM_API_ASSERT_NONNULL(tag);
      u32 number = 0;
      for (const Parameter * p = m_firstParameter; p; ++number, p = p->m_next)
      {
        if (!strcmp(tag, p->GetTag()))
        {
          return (s32) number;
        }
      }
      return -1;
    }

    const Parameter * GetParameter(u32 index) const
    {
      Parameter * p = m_firstParameter;
      s32 left = (s32) index;

      while (p && --left >= 0) {
        p = p->m_next;
      }

      if (!p)
      {
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }
      return p;
    }

    Parameter * GetParameter(u32 index)
    {
      // Safe. But, barf.
      return
        const_cast<Parameter *>(static_cast<const Parameters*>(this)->GetParameter(index));
    }

    void AddParameter(Parameter * np)
    {
      if (!np)
      {
        FAIL(NULL_POINTER);
      }

      if (np->m_next)  // Already added somewhere
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      Parameter ** pPtr = &m_firstParameter;
      for (Parameter * p = *pPtr; p; pPtr = &p->m_next, p = p->m_next)
      {
        if (p == np)
        {
          FAIL(DUPLICATE_ENTRY);
        }
      }

      *pPtr = np;
    }

  private:
    Parameter* m_firstParameter;
  };
}

#endif /* PARAMETERS_H */
