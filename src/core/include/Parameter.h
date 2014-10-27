/*                                              -*- mode:C++ -*- */
/*
  Parameter.h A configurable parameter usable for Elements and Atoms
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
   \file Parameter.h A configurable parameter usable for Elements and Atoms
   \author Trent R. Small.
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef PARAMETER_H
#define PARAMETER_H

#include "ByteSerializable.h"
#include "OverflowableCharBufferByteSink.h"
#include "Util.h"
#include "VD.h"
#include "Atom.h"

namespace MFM
{
  template <class CC> class Element;  // FORWARD

  template <class CC> class AtomicParameter; // FORWARD
  template <class CC> class ElementParameter; // FORWARD

  template <class PARM> class Parameters; // FORWARD

  template <class CC> class AtomicParameters; // FORWARD
  template <class CC> class ElementParameters; // FORWARD

  template <class CC>
  class Parameter : public ByteSerializable
  {
  private:
    typedef typename CC::ATOM_TYPE T;

    /**
     * The value description this parameter is using.
     */
    const VD m_vDesc;

    /**
     * A short name for this Parameter . This is used for saving
     * this Parameter to a config file, and therefore should be
     * unique to every other parameter tag in the same category
     * (e.g., all the parameter tags of a single Element must be
     * different, but different Elements can have parameters with
     * the same tag without confusion).
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

  public:
    u32 GetType() const
    {
      return m_vDesc.m_type;
    }

    s32 GetMin() const
    {
      return m_vDesc.GetMin();
    }

    s32 GetDefault() const
    {
      return m_vDesc.GetDefault();
    }

    s32 GetMax() const
    {
      return m_vDesc.GetMax();
    }

    u32 GetRange() const
    {
      return GetMax() - GetMin() + 1;
    }

    u32 MapValue(u32 max, s32 val) const
    {
      val = CLAMP(GetMin(), GetMax(), val);

      u32 zval = (u32) (val - GetMin());

      return zval * max / GetRange();
    }

    virtual void Print(ByteSink & bs)
    {
      bs.Printf("Parameter[%s](\"%s\",\"%s\",\"%s\",%d,%d,%d)",
                VD::GetTypeName(this->GetType()),
                this->GetName(),
                this->GetTag(),
                this->GetDescription(),
                this->GetMin(),
                this->GetDefault(),
                this->GetMax());
    }

    void PrintValue(ByteSink &bs, const T & atom)
    {
      switch (this->GetType())
      {
      case VD::U32: bs.Print(m_vDesc.GetValueU32<CC>(atom)); break;
      case VD::S32: bs.Print(m_vDesc.GetValueS32<CC>(atom)); break;
      case VD::BOOL: bs.Print(m_vDesc.GetValueBool<CC>(atom)); break;
      case VD::UNARY: bs.Print(m_vDesc.GetValueUnary<CC>(atom)); break;
      default: FAIL(ILLEGAL_STATE);
      }
    }

    virtual bool ReadValue(ByteSource &bs, T & atom)
    {
      switch (this->GetType())
      {
      case VD::U32:
        {
          u32 val;
          if (!bs.Scan(val)) return false;
          m_vDesc.SetValueU32<CC>(atom,val);
          return true;
        }
      case VD::S32:
        {
          s32 val;
          if (!bs.Scan(val)) return false;
          m_vDesc.SetValueS32<CC>(atom,val);
          return true;
        }
      case VD::BOOL:
        {
          OString16 temp;
          if (!bs.ScanIdentifier(temp)) return false;
          if (temp.Equals("true"))
          {
            m_vDesc.SetValueBool<CC>(atom,true);
          }
          else if (temp.Equals("false"))
          {
            m_vDesc.SetValueBool<CC>(atom,false);
          }
          else
          {
            return false;
          }
          return true;
        }
      case VD::UNARY:
        {
          u32 val;
          if (!bs.Scan(val)) return false;
          m_vDesc.SetValueUnary<CC>(atom,val);
          return true;
        }
      default: FAIL(ILLEGAL_STATE);
      }
    }

    /////////
    /// Un-type-checked access treating all bit fields as s32

    s32 GetBitsAsS32(const T& atom) const
    {
      return m_vDesc.GetBitsAsS32<CC>(atom);
    }

    void SetBitsAsS32(T& atom, const s32 val) const
    {
      m_vDesc.SetBitsAsS32<CC>(atom, val);
    }

    /////////
    /// Un-type-checked access treating all bit fields as u64

    u64 GetBitsAsU64(const T& atom) const
    {
      return m_vDesc.GetBitsAsU64<CC>(atom);
    }

    void SetBitsAsU64(T& atom, const u64 val) const
    {
      m_vDesc.SetBitsAsU64<CC>(atom, val);
    }

    /////////
    /// Routines to (attempt to) apply this parameter to any given atom

    /////////
    /// u32 value

    bool LoadU32(const T& atom, u32 & store) const
    {
      if (this->GetType()==VD::U32)
      {
        store = m_vDesc.GetValueU32<CC>(atom);
        return true;
      }
      return false;
    }

    void StoreU32(T& atom, const u32 val) const
    {
      if (this->GetType()==VD::U32)
      {
        this->m_vDesc.SetValueU32(atom, val);
        return true;
      }
      return false;
    }

    /////////
    /// s32 value

    bool LoadS32(const T& atom, s32 & store) const
    {
      if (this->GetType()==VD::S32)
      {
        store = this->m_vDesc.GetValueS32(atom);
        return true;
      }
      return false;
    }

    void StoreS32(T& atom, const s32 val) const
    {
      if (this->GetType()==VD::S32)
      {
        this->m_vDesc.SetValueS32(atom, val);
        return true;
      }
      return false;
    }

    /////////
    /// bool value

    bool LoadBool(const T& atom, bool & store) const
    {
      if (this->GetType()==VD::BOOL)
      {
        store = m_vDesc.GetValueBool<CC>(atom);
        return true;
      }
      return false;
    }

    void StoreBool(T& atom, const bool val) const
    {
      if (this->GetType()==VD::BOOL)
      {
        this->m_vDesc.SetValueBool(atom, val);
        return true;
      }
      return false;
    }

    /////////
    /// unary value

    bool LoadUnary(const T& atom, u32 & store) const
    {
      if (this->GetType()==VD::UNARY)
      {
        store = this->m_vDesc.GetValueUnary(atom);
        return true;
      }
      return false;
    }

    void StoreUnary(T& atom, const u32 val) const
    {
      if (this->GetType()==VD::UNARY)
      {
        this->m_vDesc.SetValueUnary(atom, val);
        return true;
      }
      return false;
    }

    /**
     * Constructs a new parameter with specified parameters.
     *
     * @param c The Parameters collection to add this Parameter to.
     *
     * @param tag The permanent Tag for this Parameter . This is
     *            used for saving this Parameter to a config file,
     *            and therefore must be unique within its category
     *            (e.g., the parameters of a single Element ).
     *
     * @param name The permanent Name of this Parameter . This will
     *             be displayed when any controls for this Parameter
     *             are displayed.
     *
     * @param description A permanent description of this Parameter
     *                    .  At the moment, this is not displayed
     *                    with this Parameter .
     */
    Parameter(const VD & vd, const char* tag, const char* name, const char* description) ;

    const VD & GetVD() const
    {
      return m_vDesc;
    }

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
  };
}

namespace MFM
{
  template <class CC> class Parameters; // FORWARD

  template <class CC>
  class ElementParameter : public Parameter<CC>
  {
    typedef Parameter<CC> Super;
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;

    friend class Parameters< ElementParameter<CC> >;
    ElementParameter * m_next;

    T m_storage;

    enum {
      BPA = P::BITS_PER_ATOM,
      PARM_LENGTH = 32,
      START_POS = BPA - PARM_LENGTH,

      LONG_PARM_LENGTH = 64,
      LONG_START_POS = BPA - LONG_PARM_LENGTH

    };

    static const VD MakeVD(u32 type, s32 min, s32 vdef, s32 max)
    {
      VD::AssertValidType(type);
      return VD(type, PARM_LENGTH, START_POS, min, vdef, max);
    }

    static const VD MakeLongVD(u64 vdef)
    {
      return VD(VD::BITS, LONG_PARM_LENGTH, LONG_START_POS, vdef);
    }

  public:
    // Make BitFields for faster access to the element storage
    typedef BitVector<BPA> BV;
    typedef BitField<BV, VD::U32, PARM_LENGTH, START_POS> FieldU32;
    typedef BitField<BV, VD::S32, PARM_LENGTH, START_POS> FieldS32;
    typedef BitField<BV, VD::BOOL, PARM_LENGTH, START_POS> FieldBool;
    typedef BitField<BV, VD::UNARY, PARM_LENGTH, START_POS> FieldUnary;
    typedef BitField<BV, VD::BITS, LONG_PARM_LENGTH, LONG_START_POS> FieldBits;

    const T & GetAtom() const
    {
      return m_storage;
    }

    T & GetAtom()
    {
      return m_storage;
    }

    void PrintValue(ByteSink &bs)
    {
      Super::PrintValue(bs, m_storage);
    }

    virtual bool ReadValue(ByteSource &bs)
    {
      return Super::ReadValue(bs, m_storage);
    }

    ElementParameter * GetNextParameter()
    {
      return m_next;
    }

    const ElementParameter * GetNextParameter() const
    {
      return m_next;
    }

    ElementParameter(Element<CC> * elt, u32 type, const char * tag,
                     const char * name, const char * description,
                     s32 min, s32 vdef, s32 max) ;

    ElementParameter(Element<CC> * elt, u32 type, const char * tag,
                     const char * name, const char * description,
                     u64 vdef) ;

    /////////
    /// Un-type-checked access treating all bit fields as s32

    s32 GetBitsAsS32() const
    {
      return GetBitsAsS32(m_storage);
    }

    void SetBitsAsS32(const s32 val)
    {
      Parameter<CC>::SetBitsAsS32(m_storage, val);
    }

    u64 GetBitsAsU64() const
    {
      return GetBitsAsU64(m_storage);
    }

    void SetBitsAsU64(const u64 val)
    {
      Parameter<CC>::SetBitsAsU64(m_storage, val);
    }

    virtual void Reset()
    {
      if (this->GetVD().GetType()==VD::BITS)
      {
        this->SetBitsAsU64(this->GetVD().GetLongDefault());
      }
      else
      {
        this->SetBitsAsS32(this->GetVD().GetDefault());
      }
    }

    /////////
    /// u32 value

    u32 GetValueU32() const
    {
      return FieldU32::GetValue(m_storage);
    }

    void SetValueU32(const u32 val)
    {
      FieldU32::SetValue(m_storage, val);
    }

    /////////
    /// s32 value

    s32 GetValueS32() const
    {
      return FieldS32::GetValue(m_storage);
    }

    void SetValueS32(const s32 val)
    {
      FieldS32::SetValue(m_storage, val);
    }

    /////////
    /// bool value

    bool GetValueBool() const
    {
      return FieldBool::GetValue(m_storage);
    }

    void SetValueBool(const bool val)
    {
      FieldBool::SetValue(m_storage, val);
    }

    /////////
    /// unary value

    u32 GetValueUnary() const
    {
      return FieldUnary::GetValue(m_storage);
    }

    void SetValueUnary(const u32 val) const
    {
      FieldUnary::SetValue(m_storage, val);
    }

    /////////
    /// bits value

    u64 GetValueBits() const
    {
      u64 val;
      FieldBits::Load(m_storage, val);
      return val;
    }

    void SetValueBits(const u64 val) const
    {
      FieldBits::SetValue(m_storage, val);
    }
  };

  template <class CC>
  class ElementParameterU32 : public ElementParameter<CC>
  {
  public:
    ElementParameterU32(Element<CC> * elt, const char * tag,
                        const char * name, const char * description,
                        u32 min, u32 vdef, u32 max) ;
    u32 GetValue() const
    {
      return this->GetValueU32();
    }
    void SetValue(u32 val)
    {
      return this->SetValueU32(val);
    }

    virtual ByteSerializable::Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      byteSink.Printf("%u", this->GetValue());
      return ByteSerializable::SUCCESS;
    }

    virtual ByteSerializable::Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      u32 val;
      if (byteSource.Scanf("%u", &val) != 1)
      {
        return ByteSerializable::FAILURE;
      }
      this->SetValue(val);
      return ByteSerializable::SUCCESS;
    }

  };

  template <class CC>
  class ElementParameterS32 : public ElementParameter<CC>
  {
  public:
    ElementParameterS32(Element<CC> * elt, const char * tag,
                        const char * name, const char * description,
                        s32 min, s32 vdef, s32 max) ;
    s32 GetValue() const
    {
      return this->GetValueS32();
    }
    void SetValue(s32 val)
    {
      return this->SetValueS32(val);
    }

    virtual ByteSerializable::Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      byteSink.Printf("%d", this->GetValue());
      return ByteSerializable::SUCCESS;
    }

    virtual ByteSerializable::Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      u32 val;
      if (byteSource.Scanf("%d", &val) != 1)
      {
        return ByteSerializable::FAILURE;
      }
      this->SetValue(val);
      return ByteSerializable::SUCCESS;
    }

  };

  template <class CC>
  class ElementParameterBool : public ElementParameter<CC>
  {
  public:
    ElementParameterBool(Element<CC> * elt, const char * tag,
                         const char * name, const char * description,
                         bool vdef) ;
    bool GetValue() const
    {
      return this->GetValueBool();
    }
    void SetValue(bool val)
    {
      return this->SetValueBool(val);
    }

    virtual ByteSerializable::Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      byteSink.Printf("%s", this->GetValue() ? "true" : "false");
      return ByteSerializable::SUCCESS;
    }

    virtual ByteSerializable::Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      OString16 temp;
      if (byteSource.ScanIdentifier(temp))
      {
        return ByteSerializable::FAILURE;
      }
      if (temp.Equals("true"))
      {
        this->SetValue(true);
        return ByteSerializable::SUCCESS;
      }
      if (temp.Equals("false"))
      {
        this->SetValue(false);
        return ByteSerializable::SUCCESS;
      }
      return ByteSerializable::FAILURE;
    }

  };

  template <class CC, u32 LENGTH>
  class ElementParameterBits : public ElementParameter<CC>
  {
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;

    enum {
      BPA = P::BITS_PER_ATOM,
      START_POS = BPA - LENGTH
    };
    typedef BitVector<BPA> BV;
    typedef BitField<BV, VD::BITS, LENGTH, START_POS> FieldBitsLength;

  public:
    ElementParameterBits(Element<CC> * elt, const char * tag,
                         const char * name, const char * description,
                         u64 vdef) ;
    u64 GetValue() const
    {
      u64 val;
      FieldBitsLength::Load(this->GetAtom(), val);
      return val;
    }

    void SetValue(u64 val)
    {
      FieldBitsLength::SetValue(this->GetAtom(), val);
    }

    void ClearBit(u32 bitnum)
    {
      FieldBitsLength::ClearBit(this->GetAtom(), bitnum);
    }
    void SetBit(u32 bitnum)
    {
      FieldBitsLength::SetBit(this->GetAtom(), bitnum);
    }
    bool GetBit(u32 bitnum) const
    {
      return FieldBitsLength::ReadBit(this->GetAtom(), bitnum);
    }
  };

  template <class CC, u32 SITES>
  class ElementParameterNeighborhood : public ElementParameterBits<CC,SITES>
  {
  public:
    ElementParameterNeighborhood(Element<CC> * elt, const char * tag,
                                 const char * name, const char * description,
                                 u64 vdef) :
      ElementParameterBits<CC,SITES>(elt, tag, name, description, vdef)
    { }

    virtual ByteSerializable::Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      return PrintU64AsNeighborhood(byteSink, this->GetValue());
    }

    virtual ByteSerializable::Result PrintU64AsNeighborhood(ByteSink & byteSink, u64 nghb)
    {
      byteSink.Print("[");
      bool firstOut = false;
      u32 start = SITES;
      for (u32 i = 0; i <= SITES; ++i) // '<=' for one extra loop at end
      {
        bool bitvalue = false;
        if (i < SITES)
        {
          bitvalue = (nghb>>((SITES-1)-i))&1;
        }

        if (bitvalue)
        {
          if (start == SITES)
          {
            // Starting a run
            start = i;
          }
        }
        else
        {
          if (start != SITES)
          {
            // Ending a run
            if (firstOut)
            {
              byteSink.Print(",");
            }
            else
            {
              firstOut = true;
            }
            byteSink.Print(start);
            if (start < i-1)
            {
              byteSink.Printf("-%d", i-1);
            }
            start = SITES;
          }
        }
      }
      byteSink.Print("]");
      return ByteSerializable::SUCCESS;
    }

    virtual ByteSerializable::Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      u64 ngb;
      ByteSerializable::Result res = ReadNeighborhoodAsU64(byteSource, ngb);
      if (res == ByteSerializable::SUCCESS)
      {
        this->SetValue(ngb);
      }
      return res;
    }

    virtual ByteSerializable::Result ReadNeighborhoodAsU64(ByteSource & byteSource, u64 & ngb)
    {
      const ByteSerializable::Result FAIL = ByteSerializable::FAILURE;

      byteSource.SkipWhitespace();
      if (byteSource.Read() != '[')
      {
        return FAIL;
      }
      u64 val = 0;
      while (true)
      {
        byteSource.SkipWhitespace();
        if (byteSource.Read() == ']')
        {
          ngb = val;
          return ByteSerializable::SUCCESS;
        }
        byteSource.Unread();
        s32 start;
        if (!byteSource.Scan(start))
        {
          return FAIL;
        }
        if (start < 0 || start >= (s32) SITES)
        {
          return FAIL;
        }
        s32 end = start;
        byteSource.SkipWhitespace();
        s32 delim = byteSource.Read();
        if (delim == '-')
        {
          if (!byteSource.Scan(end))
          {
            return FAIL;
          }
          if (end < 0 || end >= (s32) SITES || end < start)
          {
            return FAIL;
          }
          byteSource.SkipWhitespace();
          delim = byteSource.Read();
        }
        for (s32 bit = start; bit <= end; ++bit)
        {
          val |= 1L<<((SITES-1)-bit);
        }
        if (delim != ',')
        {
          if (delim == ']')
          {
            byteSource.Unread();
          }
          else
          {
            return FAIL;
          }
        } // else go around again
      }
    }

    bool ReadValue(ByteSource &bs)
    {
      return ReadFrom(bs) == ByteSerializable::SUCCESS;
    }

  };

  template <class CC>
  class AtomicParameter : public Parameter<CC>
  {
    typedef Parameter<CC> Super;
    typedef typename CC::ATOM_TYPE T;

    friend class Parameters< AtomicParameter<CC> >;
    AtomicParameter * m_next;

  public:
    virtual ByteSerializable::Result PrintTo(ByteSink & byteSink, s32 argument = 0)
    {
      return ByteSerializable::UNSUPPORTED;
    }

    virtual ByteSerializable::Result ReadFrom(ByteSource & byteSource, s32 argument = 0)
    {
      return ByteSerializable::UNSUPPORTED;
    }

    AtomicParameter * GetNextParameter()
    {
      return m_next;
    }

    const AtomicParameter * GetNextParameter() const
    {
      return m_next;
    }

    virtual void Print(ByteSink & io)
    {
      io.Print("AP(");
      Super::Print(io);
      io.Print(")");
    }

    virtual void Print(ByteSink & io, const T & atom) const = 0;

    virtual void Reset(T & atom) const = 0;

    AtomicParameter<CC>(Element<CC> * pl, const char * tag,
                        const char * name, const char * description,
                        const VD & vdesc) ;
  };

  template <class CC, VD::Type VT, u32 LEN, u32 POS>
  class AtomicParameterType : AtomicParameter<CC>
  {
  public:
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;

    typedef BitVector<P::BITS_PER_ATOM> BVA;
    typedef BitField<BVA, VT, LEN, POS> Field;

    // Lift BitField properties (particularly END) up to parameter level
    enum
    {
      BITS = Field::BITS,
      START = Field::START,
      LENGTH = Field::LENGTH,
      END = Field::END
    };

    typedef typename VTypeToType<VT>::TYPE VTYPE;

    const VTYPE m_vdefault;  // public, but const, whatever

    AtomicParameterType(Element<CC> * pl, const char * tag,
                        const char * name, const char * description,
                        const VTYPE & vmin, const VTYPE & vdefault, const VTYPE & vmax) ;

    VTYPE GetValue(const T & atom) const
    {
      return Field::GetValue(atom);
    }
    void SetValue(T & atom, const VTYPE val) const
    {
      Field::SetValue(atom,val);
    }

    virtual void Reset(T & atom) const
    {
      SetValue(atom, m_vdefault);
    }

    virtual void Print(ByteSink & io, const T & atom) const
    {
      atom.Print(io);
    }

  };

  template <class PARM>
  class Parameters
  {
  protected:

    PARM* m_firstParameter;

  public:
    Parameters() : m_firstParameter(0)
    { }

    /**
     * Gets the number of Parameters held by this Parameters
     * collection. Since this collection is a linked list, this runs
     * in O(n) time where n is the number of Parameters held.
     *
     * @returns The number of Parameters held by this Parameters
     *          collection.
     */
    u32 GetParameterCount() const
    {
      u32 count = 0;
      PARM* p = m_firstParameter;
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
    s32 GetParameterNumber(const PARM * ap) const
    {
      MFM_API_ASSERT_NONNULL(ap);
      u32 number = 0;
      for (PARM * p = m_firstParameter; p; ++number, p = p->m_next)
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
      for (const PARM* p = m_firstParameter; p; ++number, p = p->m_next)
      {
        if (!strcmp(tag, p->GetTag()))
        {
          return (s32) number;
        }
      }
      return -1;
    }

    /**
     * Gets the first Parameter in this Parameters collection, if
     * there is one.
     *
     * @returns A Pointer to the first Parameter in this Parameters
     *          collection, or NULL if there are none
     */
    const PARM* GetFirstParameter() const
    {
      return m_firstParameter;
    }

    /**
     * Gets the Parameter at a specified index in this Parameters
     * collection.
     *
     * @param index The index of the Parameter to get from this
     *              Parameters collection. If this number is not less
     *              than the number of Parameters held by this
     *              Parameters collection, this method FAILs with
     *              ARRAY_INDEX_OUT_OF_BOUNDS .
     *
     * @returns A Pointer to the Parameter held at the specified index
     *          in this Parameters collection.
     */
    const PARM* GetParameter(u32 index) const
    {
      PARM* p = m_firstParameter;
      s32 left = (s32) index;

      while (p && --left >= 0)
      {
        p = p->m_next;
      }

      if (!p)
      {
        FAIL(OUT_OF_BOUNDS);
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
    PARM * GetParameter(u32 index)
    {
      // Safe. But, barf.
      return
        const_cast<PARM *>(static_cast<const Parameters*>(this)->GetParameter(index));
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
    void AddParameter(PARM * np)
    {
      if (!np)
      {
        FAIL(NULL_POINTER);
      }

      if (np->m_next)  // Already added somewhere
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      PARM ** pPtr = &m_firstParameter;
      for (PARM * p = *pPtr; p; pPtr = &p->m_next, p = p->m_next)
      {
        if (p == np)
        {
          FAIL(DUPLICATE_ENTRY);
        }
      }

      *pPtr = np;
    }
  };

  template<class CC>
  class ElementParameters : public Parameters< ElementParameter<CC> >
  {
  public:

    /**
     * Reset all Parameters inside this Parameters collection to their default values
     */
    void Reset()
    {
      for (ElementParameter<CC> * p = this->m_firstParameter; p; p = p->m_next)
      {
        p->Reset();
      }
    }

  };

  template<class CC>
  class AtomicParameters : public Parameters< AtomicParameter<CC> >
  {
  public:
    typedef typename CC::ATOM_TYPE T;

    /**
     * On the given Atom, reset all AtomicParameters inside this
     * Parameters collection to their default values
     */
    void Reset(T & atom) const
    {
      for (AtomicParameter<CC> * p = this->m_firstParameter; p; p = p->m_next)
      {
        p->Reset(atom);
      }
    }
  };
}

#include "Parameter.tcc"

#endif /* PARAMETER_H */
