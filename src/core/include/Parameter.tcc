/* -*- C++ -*- */

#include "Element.h"

namespace MFM
{
  template <class CC>
  Parameter<CC>::Parameter(const VD& vd, const char* tag,
                           const char* name, const char* description)
    : m_vDesc(vd),
      m_tag(tag),
      m_name(name),
      m_description(description)
  {
    if (!m_tag || !m_name || !m_description)
    {
      FAIL(NULL_POINTER);
    }
    if (!strlen(m_tag) || !strlen(m_name))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
  }

  template <class CC>
  ElementParameter<CC>::ElementParameter(Element<CC> * elt, u32 type, const char * tag,
                                         const char * name, const char * description,
                                         s32 min, s32 vdef, s32 max)
    : Parameter<CC>(MakeVD(type, min, vdef, max), tag, name, description)
  {
    elt->GetElementParameters().AddParameter(this);
    this->Reset();
  }

  template <class CC>
  ElementParameter<CC>::ElementParameter(Element<CC> * elt, u32 type, const char * tag,
                                         const char * name, const char * description,
                                         u64 vdef)
    : Parameter<CC>(MakeLongVD(vdef), tag, name, description)
  {
    elt->GetElementParameters().AddParameter(this);
    this->Reset();
  }

  template <class CC>
  ElementParameterU32<CC>::ElementParameterU32(Element<CC> * elt, const char * tag,
                                               const char * name, const char * description,
                                               u32 min, u32 vdef, u32 max)
    : ElementParameter<CC>(elt, VD::U32, tag, name, description, (s32) min, (s32) vdef, (s32) max)
  { }

  template <class CC>
  ElementParameterS32<CC>::ElementParameterS32(Element<CC> * elt, const char * tag,
                                               const char * name, const char * description,
                                               s32 min, s32 vdef, s32 max)
    : ElementParameter<CC>(elt, VD::S32, tag, name, description, min, vdef, max)
  { }

  template <class CC>
  ElementParameterBool<CC>::ElementParameterBool(Element<CC> * elt,  const char * tag,
                                                 const char * name, const char * description,
                                                 bool vdef)
    : ElementParameter<CC>(elt, VD::BOOL, tag, name, description, 0, vdef?1:0, 1)
  { }

  template <class CC, u32 LENGTH>
  ElementParameterBits<CC,LENGTH>::ElementParameterBits(Element<CC> * elt,  const char * tag,
                                                        const char * name, const char * description,
                                                        u64 vdef)
    : ElementParameter<CC>(elt, VD::BITS, tag, name, description, vdef)
  { }

  template <class CC>
  AtomicParameter<CC>::AtomicParameter(Element<CC> * pl, const char * tag,
                                       const char * name, const char * description,
                                       const VD & vdesc)
    : Parameter<CC>(vdesc, tag, name, description)
  {
    pl->GetAtomicParameters().AddParameter(this);
  }

  template <class CC, VD::Type VT, u32 LEN, u32 POS>
  AtomicParameterType<CC,VT,LEN,POS>::AtomicParameterType(Element<CC> * pl, const char * tag,
                                                          const char * name, const char * description,
                                                          const VTYPE & vmin, const VTYPE & vdefault, const VTYPE & vmax)
    : AtomicParameter<CC>(pl, tag, name, description,
                          VD(VT, LEN, POS, (s32) vmin, (s32) vdefault, (s32) vmax)),
      m_vdefault(vdefault)
  { }

}

