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

#include "BitVector.h"
#include "ByteSerializable.h"
#include "OverflowableCharBufferByteSink.h"
#include "Util.h"

namespace MFM
{
  class Parameters
  {
   public:
    /**
     * An enumeration of all known types of Parameters .
     */
    enum KnownParameterTypes
    {
      /**
       * The KnownParameterTypes which represents a signed 32-bit
       * integer parameter.
       */
      S32_PARAMETER = 1,

      /**
       * The KnownParameterTypes which represents a boolean parameter.
       */
      BOOL_PARAMETER = 2,

      NEIGHBORHOOD_PARAMETER = 3,

      /**
       * Used to signify that a Parameter is not valid.
       */
      ILLEGAL_PARAMETER = 0
    };

    /**
     * An abstract class meant to be the superclass for all
     * Parameters .
     */
    class Parameter : public ByteSerializable
    {
     private:
      /**
       * A short name for this Parameter . This is used for saving
       * this Parameter to a config file, and therefore should be
       * unique to every other parameter tag.
       */
      const char * m_tag;

      /**
       * A name for this Parameter . This will be displayed when any
       * controls for this Parameter are displayed.
       */
      const char * m_name;

      /**
       * A description of this Parameter . At the moment, this is not
       * displayed with this Parameter .
       */
      const char * m_description;

      friend class Parameters;

      /**
       * A pointer to the parameter which should be considered
       * 'next'. Essentially, this makes every Parameter a linked
       * list node.
       */
      Parameter * m_next;

    public:
      /**
       * Constructs a new parameter with specified parameters.
       *
       * @param c The Parameters collection to add this Parameter to.
       *
       * @param tag The permanent Tag for this Parameter . This is used
       *            for saving this Parameter to a config file, and
       *            therefore should be unique to every other
       *            parameter tag.
       *
       * @param name The permanent Name of this Parameter . This will
       *             be displayed when any controls for this Parameter
       *             are displayed.
       *
       * @param description A permanent description of this Parameter
       *                    .  At the moment, this is not displayed
       *                    with this Parameter .
       */
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

      /**
       * Gets the KnownParameterType associated with this Parameter
       * . Each implementing subclass should use a unique
       * KnownParameterType.
       *
       * @returns the KnownParameterType associated with this
       *          Parameter .
       */
      virtual u32 GetParameterType() const = 0;

      /**
       * Prints this Parameter to a specified ByteSink . An IO saving
       * procedure will use this to save this object's state to
       * non-volatile memory and will be attempted to be revived
       * through the Read() method, so all state must be represented
       * through this method in some way.
       *
       * @param bs The ByteSink to print this Parameter to.
       */
      virtual void Print(ByteSink & bs) const = 0;

      /**
       * Reads this Parameter from a specified Bytesink . This should
       * return all state to this Parameter that would have been saved
       * through the Print() method.
       *
       * @param bs The ByteSink to read this Parameter from.
       */
      virtual bool Read(ByteSource & bs) = 0;

      /**
       * Deconstructs this Parameter. Default behavior does not do
       * anything.
       */
      virtual ~Parameter()
      { }

      /**
       * Gets the permanent Tag of this Parameter , usually used to
       * save this Parameter to some non-volatile IO device.
       *
       * @returns The permanent Tag of this Parameter .
       */
      const char* GetTag() const
      {
        return m_tag;
      }

      /**
       * Gets the permanent Name of this Parameter , normally used by
       * a GUI system during rendering of this Parameter .
       *
       * @returns the permanent Name of this Parameter .
       */
      const char* GetName() const
      {
        return m_name;
      }

      /**
       * Gets the permanent Description of this Parameter . At the
       * moment, this is not used.
       *
       * @returns The permanent Description of this Parameter
       */
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

    template<u32 R>
    class Neighborhood : public Parameter
    {
     public:
      enum
      {
        SITES = (R + 1) * (R + 1) + (R * R)
      };

      static Neighborhood* Cast(Parameter* p)
      {
        return dynamic_cast<Neighborhood*>(p);
      }

     private:

      BitVector<SITES> m_sites;

     public:
      Neighborhood(Parameters& c,
                   const char* tag,
                   const char* name,
                   const char* description)
        : Parameter(c, tag, name, description)
      { }

      bool ReadBit(const u32 index) const
      {
        return m_sites.ReadBit(index);
      }

      void SetBit(const u32 index)
      {
        m_sites.SetBit(index);
      }

      void ClearBit(const u32 index)
      {
        m_sites.ClearBit(index);
      }

      virtual u32 GetParameterType() const
      {
        return NEIGHBORHOOD_PARAMETER;
      }

      virtual void Print(ByteSink& bs) const
      {
        m_sites.PrintBinary(bs);
      }

      virtual bool Read(ByteSource& bs)
      {
        return m_sites.ReadBinary(bs);
      }
    };

    /**
     * A Parameter representing a modifiable signed 32-bit integer
     * value.
     */
    class S32 : public Parameter
    {
      /**
       * The mutable current signed 32-bit value that this Parameter
       * represents.
       */
      s32 m_current;

      /**
       * The permanent minimum signed 32-bit value that this Parameter
       * represents.
       */
      const s32 m_min;

      /**
       * The permanent initial signed 32-bit value that this Parameter
       * represented at construction.
       */
      const s32 m_initial;

      /**
       * The permanent maximum signed 32-bit value that this Parameter
       * represents.
       */
      const s32 m_max;

      /**
       * The permanent snapping resolution of this Parameter .
       */
      const s32 m_snap;

    public:

      /**
       * Casts a specified Parameter pointer to an S32 type Parameter
       * pointer.
       *
       * @param p The Parameter pointer to case to an S32 type
       *          Parameter pointer.
       *
       * @returns an S32 type Parameter pointer which is the result of
       *          a \c dynamic_cast of \c p .
       */
      static S32* Cast(Parameter * p)
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

      /**
       * Constructs a new S32 type Parameter with specified bounds and
       * default value.
       *
       * @param c The Parameters collection to add this Parameter to.
       *
       * @param tag The permanent Tag for this Parameter . This is used
       *            for saving this Parameter to a config file, and
       *            therefore should be unique to every other
       *            parameter tag.
       *
       * @param name The permanent Name of this Parameter . This will
       *             be displayed when any controls for this Parameter
       *             are displayed.
       *
       * @param description A permanent description of this Parameter
       *                    .  At the moment, this is not displayed
       *                    with this Parameter .
       *
       * @param min The permanent minimum signed 32-bit value which
       *            this Parameter may represent.
       *
       * @param initial The initial signed 32-bit value which this
       *                Parameter represents. If this value is less
       *                than \c min, it will be set to \c min .
       *
       * @param max The permanent maximum signed 32-bit value which
       *            this Parameter may represent.
       *
       * @param snap The permanent snap resolution of this
       *             parameter. If this parameter changes, it is
       *             floored to a multiple of this snapping value.
       */
      S32(Parameters& c,
          const char* tag,
          const char* name,
          const char* description,
          s32 min, s32 initial, s32 max, s32 snap)
        : Parameter(c, tag, name, description),
          m_min(min),
          m_initial(initial),
          m_max(max < min ? min : max),
          m_snap(snap)
      {
        SetValue(initial);
      }

      /**
       * Gets the signed 32-bit value which this Parameter is
       * currently representing.
       *
       * @returns The signed 32-bit value which this Parameter is
       * currently representing.
       */
      virtual s32 GetValue() const
      {
        return m_current;
      }

      /**
       * Sets the signed 32-bit value which this Parameter is
       * currently representing to a specified value. This value will
       * be clamped between the minimum and maximum values of this
       * Parameter , then flooed to the nearest multiple of the snap
       * value of this parameter.
       *
       * @param value The new signed 32-bit value which the Parameter
       *              is wished to represent.
       */
      virtual void SetValue(s32 value)
      {
        value = CLAMP(m_min, m_max, value);
        value -= value % m_snap;
        m_current = CLAMP(m_min, m_max, value);
      }

      /**
       * Gets the permanent minimum value which this Parameter may
       * represent.
       *
       * @returns The permanent minimum value which this Parameter may
       *          represent.
       */
      virtual s32 GetMin() const
      {
        return m_min;
      }

      /**
       * Gets the permanent maximum value which this Parameter may
       * represent.
       *
       * @returns The permanent maximum value which this Parameter may
       *          represent.
       */
      virtual s32 GetMax() const
      {
        return m_max;
      }

      /**
       * Gets the size of the permanent range of values which this
       * Parameter may represent.
       *
       * @returns The size of the permanent range of values which this
       *          Parameter may represent.
       */
      u32 GetRange() const
      {
        return m_max - m_min + 1;
      }

      /**
       * Maps the value of this Parameter to a seperate range of
       * values. This is useful, for example, for rendering this value
       * on a line where the range of pixels is not the same as the
       * range of values inside this Parameter .
       *
       * @param max The maximum value of the range to map to.
       *
       * @param val The value to map using the range held by this Parameter .
       *
       * @returns \c val , mapped to the range held by this Parameter,
       *          multiplied by \c max .
       */
      u32 MapValue(u32 max, s32 val) const
      {
        val = CLAMP(m_min, m_max, val);

        u32 zval = (u32) (val - m_min);

        return zval * max / GetRange();
      }

      /**
       * Gets the permanent snapping resolution of this Parameter
       * . Every time the signed 32-bit value which this Parameter
       * represents changes, the new value will be floored to a
       * multiple of this snapping resolution.
       *
       * @returns The permanent snapping resolution of this Parameter
       * .
       */
      virtual u32 GetSnap() const
      {
        return m_snap;
      }
    };

    /**
     * A Parameter representing a modifiable boolean value.
     */
    class Bool : public Parameter
    {
      /**
       * The current boolean value which this Parameter represents.
       */
      bool m_current;

      /**
       * The permanent boolean vlue which this Parameter reprsented at
       * construction.
       */
      const bool m_initial;

    public:


      /**
       * Casts a specified Parameter pointer to a Bool type Parameter
       * pointer.
       *
       * @param p The Parameter pointer to case to a Bool type
       *          Parameter pointer.
       *
       * @returns a Bool type Parameter pointer which is the result of
       *          a \c dynamic_cast of \c p .
       */
      static Bool* Cast(Parameter* p)
      {
        return dynamic_cast<Bool*>(p);
      }

      virtual u32 GetParameterType() const
      {
        return BOOL_PARAMETER;
      }

      virtual void Print(ByteSink& bs) const
      {
        bs.Print(m_current?"true":"false");
      }

      virtual bool Read(ByteSource& bs)
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

      /**
       * Constructs a new S32 type Parameter with specified bounds and
       * default value.
       *
       * @param c The Parameters collection to add this Parameter to.
       *
       * @param tag The permanent Tag for this Parameter . This is used
       *            for saving this Parameter to a config file, and
       *            therefore should be unique to every other
       *            parameter tag.
       *
       * @param name The permanent Name of this Parameter . This will
       *             be displayed when any controls for this Parameter
       *             are displayed.
       *
       * @param description A permanent description of this Parameter
       *                    .  At the moment, this is not displayed
       *                    with this Parameter .
       *
       * @param initial The initial boolean value which this Parameter
       *                represents.
       */
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

      /**
       * Gets the boolean value that this Parameter currently
       * represents.
       *
       * @returns The boolean value that this Parameter currently
       * represents.
       */
      virtual bool GetValue() const
      {
        return m_current;
      }

      /**
       * Sets the boolean value that this Parameter currently
       * represents to a specified value.
       *
       * @param value The value that this Parameter will represent
       *              after calling this method.
       */
      virtual void SetValue(bool value)
      {
        m_current = value;
      }

    };

    /**
     * Constructs and initializes a Parameters collection as an empty collection.
     */
    Parameters() :
      m_firstParameter(0)
    {
    }

    /**
     * Gets the number of Parameters held by this Parameters
     * collection. Since this collection is a linked list, this runs
     * in O(n) time where n is the number of Parameters held.
     *
     * @returns The number of Prameters held by this Parameters
     *          collection.
     */
    u32 GetParameterCount() const
    {
      u32 count = 0;
      Parameter* p = m_firstParameter;
      while (p)
      {
        ++count;
        p = p->m_next;
      }
      return count;
    }

    /**
     * Gets the index of a specified Parameter inside this Parameters
     * collection.
     *
     * @param ap The Parameter to search this Parameters collection
     *           for. This method FAILs with NULL_POINTER if this
     *           argument is NULL .
     *
     * @returns The 0-based index of \c ap inside this Parameters
     *          collection, or \c -1 if it is not found.
     */
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

    /**
     * Gets the index of a Parameter held by this Parameters
     * collection by specifying its Parameter Tag.
     *
     * @param tag The Tag of the Parameter to look up. This method
     *            FAILs with NULL_POINTER if this argument is NULL .
     *
     * @returns The 0-based index of the Parameter whose tag is \c tag
     *          , or \c -1 if there is no such Parameter .
     */
    s32 GetParameterNumberFromTag(const char* tag) const
    {
      MFM_API_ASSERT_NONNULL(tag);
      u32 number = 0;
      for (const Parameter* p = m_firstParameter; p; ++number, p = p->m_next)
      {
        if (!strcmp(tag, p->GetTag()))
        {
          return (s32) number;
        }
      }
      return -1;
    }

    /**
     * Gets the Parameter at a specified index in this Parameters
     * collection.
     *
     * @param index The index of the Parameter to get from this
     *              Parameters collection. If this number is greater
     *              than the number of Parameters held by this
     *              Parameters collection, this method FAILs with
     *              ARRAY_INDEX_OUT_OF_BOUNDS .
     *
     * @returns A Pointer to the Parameter held at the specified index
     *          in this Parameters collection.
     */
    const Parameter* GetParameter(u32 index) const
    {
      Parameter* p = m_firstParameter;
      s32 left = (s32) index;

      while (p && --left >= 0)
      {
        p = p->m_next;
      }

      if (!p)
      {
        FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }
      return p;
    }

    /**
     * Gets a mutable version of the Parameter at a specified index in
     * this Parameters collection.
     *
     * @param index The index of the Parameter to get from this
     *              Parameters collection. If this number is greater
     *              than the number of Parameters held by this
     *              Parameters collection, this method FAILs with
     *              ARRAY_INDEX_OUT_OF_BOUNDS .
     *
     * @returns A Pointer to the Parameter held at the specified index
     *          in this Parameters collection.
     */
    Parameter * GetParameter(u32 index)
    {
      // Safe. But, barf.
      return
        const_cast<Parameter *>(static_cast<const Parameters*>(this)->GetParameter(index));
    }

    /**
     * Places a Parameter inside this Parameters collection, making it
     * available for any method which gets a Parameter.
     *
     * @param np A Pointer to the Parameter wished to add to this
     *           Parameters collection. This method FAILs with
     *           NULL_POINTER if this argument is null, and FAILs with
     *           ILLEGAL_ARGUMENT if this Parameter has its m_next
     *           field set already. It also FAILs with DUPLICATE_ENTRY
     *           if this Parameter is already held in this Parameters
     *           collection.
     */
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
    /**
     * A pointer to the first Parameter held by this Parameters
     * collection.
     */
    Parameter* m_firstParameter;
  };
}

#endif /* PARAMETERS_H */
