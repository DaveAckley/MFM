/* -*- C++ -*- */

#include "Element.h"

namespace MFM
{

  template <class EC>
  Parameter<EC>::Parameter(const VD& vd, const char* tag,
                           const char* name, const char* description)
    : m_vDesc(vd),
      m_tag(StripThroughHexSpaceIfExists(tag)),
      m_name(name),
      m_description(description)
  {
    MFM_API_ASSERT_NONNULL(m_tag);
    MFM_API_ASSERT_NONNULL(m_name);
    MFM_API_ASSERT_NONNULL(m_description);
    MFM_API_ASSERT_ARG(strlen(m_tag) > 0 && strlen(m_name) > 0);
  }

  template <class EC>
  ElementParameter<EC>::ElementParameter(Element<EC> * elt, u32 type, const char * tag,
                                         const char * name, const char * description,
                                         s32 min, s32 vdef, s32 max)
    : Parameter<EC>(MakeVD(type, min, vdef, max), tag, name, description)
  {
    elt->GetElementParameters().AddParameter(this);
    this->Reset();
  }

  template <class EC>
  ElementParameter<EC>::ElementParameter(Element<EC> * elt, u32 type, const char * tag,
                                         const char * name, const char * description,
                                         u64 vdef)
    : Parameter<EC>(MakeLongVD(vdef), tag, name, description)
  {
    elt->GetElementParameters().AddParameter(this);
    this->Reset();
  }

  template <class EC>
  ElementParameterU32<EC>::ElementParameterU32(Element<EC> * elt, const char * tag,
                                               const char * name, const char * description,
                                               u32 min, u32 vdef, u32 max)
    : ElementParameter<EC>(elt, VD::U32, tag, name, description, (s32) min, (s32) vdef, (s32) max)
  { }


  template <class EC>
  ElementParameterUnary<EC>::ElementParameterUnary(Element<EC> * elt, const char * tag,
                                                   const char * name, const char * description,
                                                   u32 min, u32 vdef, u32 max)
    : ElementParameter<EC>(elt, VD::UNARY, tag, name, description, (s32) min, (s32) vdef, (s32) max)
  { }

  template <class EC>
  ElementParameterS32<EC>::ElementParameterS32(Element<EC> * elt, const char * tag,
                                               const char * name, const char * description,
                                               s32 min, s32 vdef, s32 max)
    : ElementParameter<EC>(elt, VD::S32, tag, name, description, min, vdef, max)
  { }

  template <class EC>
  ElementParameterBool<EC>::ElementParameterBool(Element<EC> * elt,  const char * tag,
                                                 const char * name, const char * description,
                                                 bool vdef)
    : ElementParameter<EC>(elt, VD::BOOL, tag, name, description, 0, vdef?1:0, 1)
  { }

  template <class EC, u32 LENGTH>
  ElementParameterBits<EC,LENGTH>::ElementParameterBits(Element<EC> * elt,  const char * tag,
                                                        const char * name, const char * description,
                                                        u64 vdef)
    : ElementParameter<EC>(elt, VD::BITS, tag, name, description, vdef)
  { }

  template <class EC>
  AtomicParameter<EC>::AtomicParameter(Element<EC> * pl, const char * tag,
                                       const char * name, const char * description,
                                       const VD & vdesc)
    : Parameter<EC>(vdesc, tag, name, description)
  {
    pl->GetAtomicParameters().AddParameter(this);
  }

  template <class EC, VD::Type VT, u32 LEN, u32 POS>
  AtomicParameterType<EC,VT,LEN,POS>::AtomicParameterType(Element<EC> * pl, const char * tag,
                                                          const char * name, const char * description,
                                                          const VTYPE & vmin, const VTYPE & vdefault, const VTYPE & vmax)
    : AtomicParameter<EC>(pl, tag, name, description,
                          VD(VT, LEN, POS, (s32) vmin, (s32) vdefault, (s32) vmax)),
      m_vdefault(vdefault)
  { }

}
