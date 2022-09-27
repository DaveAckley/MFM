/* -*- C++ -*- */
#include "Fail.h"
#include "BitVector.h"
#include "UlamElement.h"
#include "UlamRefMutable.h"

namespace MFM {

  template <class EC>
  UlamRef<EC>::UlamRef(u32 pos, u32 len, BitStorage<EC>& stg,
		       const UlamClass<EC> * effself,
                       const UsageType usage,
		       const UlamContext<EC> & uc)
    : m_uc(uc)
    , m_effSelf(effself)
    , m_stg(stg)
    , m_pos(pos)
    , m_len(len)
    , m_usage(usage)
    , m_posToEff(pos)
    , m_vtableclassid(0)
    , m_posToDM(0)
    , m_prevur(NULL)
  {
    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    MFM_API_ASSERT_ARG(m_usage != PRIMITIVE || m_effSelf == 0); // Primitive usage has no effself
    MFM_API_ASSERT_ARG(m_usage != ARRAY || m_effSelf == 0); // Array usage has no effself
    MFM_API_ASSERT_ARG(m_usage != CLASSIC || m_effSelf != 0); // Classic usage has effself

    if(m_usage == ELEMENTAL)
      {
	MFM_API_ASSERT_ARG(pos >= T::ATOM_FIRST_STATE_BIT); //non-negative
	m_posToDM = (u32) (pos - T::ATOM_FIRST_STATE_BIT);
	m_posToEff = 0u;
      }
    else if(m_usage == PRIMITIVE)
      {
	m_posToEff = 0u; //no eff self
      }
    else if(m_usage == CLASSIC)
      {
	if(effself != NULL)
	  {
	    const UlamElement<EC> * eltptr = effself->AsUlamElement();
	    if(eltptr) //quark or transient, baseclass of element
	      {
		MFM_API_ASSERT_ARG(pos >= T::ATOM_FIRST_STATE_BIT); //non-negative
		m_posToEff = (u32) (pos - T::ATOM_FIRST_STATE_BIT); //(t3747, t41613)
	      }
	    //else m_posToEff == pos
	  }
	//else m_posToEff == pos
      }

    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
      {
	UpdateEffectiveSelf();
      }

    if(m_effSelf != NULL)
      {
	m_vtableclassid = m_effSelf->GetRegistrationNumber(); //init
      }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(u32 pos, u32 len, u32 postoeff, BitStorage<EC>& stg, const UlamClass<EC> * effself,
            const UsageType usage, const UlamContext<EC> & uc)
    : m_uc(uc)
    , m_effSelf(effself)
    , m_stg(stg)
    , m_pos(pos)
    , m_len(len)
    , m_usage(usage)
    , m_posToEff(postoeff)
    , m_vtableclassid(0)
    , m_posToDM(pos-postoeff)
    , m_prevur(NULL)
  {
    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    MFM_API_ASSERT_ARG(m_usage != PRIMITIVE || m_effSelf == 0); // Primitive usage has no effself
    MFM_API_ASSERT_ARG(m_usage != ARRAY || m_effSelf == 0); // Array usage has no effself
    MFM_API_ASSERT_ARG(m_usage != CLASSIC || m_effSelf != 0); // Classic usage has effself

    if(m_usage == ELEMENTAL)
      {
	MFM_API_ASSERT_ARG(pos >= T::ATOM_FIRST_STATE_BIT); //non-negative
	m_posToDM = (u32) (pos - postoeff - T::ATOM_FIRST_STATE_BIT);
      }
    else if(m_usage == PRIMITIVE)
      {
	m_posToEff = 0u; //no eff self
      }
    else if(m_usage == CLASSIC)
      {
	if(effself != NULL)
	  {
	    const UlamElement<EC> * eltptr = effself->AsUlamElement();
	    if(eltptr) //quark or transient, baseclass of element
	      {
		MFM_API_ASSERT_ARG(pos >= T::ATOM_FIRST_STATE_BIT); //non-negative
		m_posToDM = (u32) (pos - postoeff - T::ATOM_FIRST_STATE_BIT); //t3747,t41613
	      }
	    //else m_posToDM = pos
	  }
	//else m_posToDM == pos
      }

    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
      {
	UpdateEffectiveSelf();
      }

    if(m_effSelf != NULL)
      {
	m_vtableclassid = m_effSelf->GetRegistrationNumber(); //init
      }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef<EC> & existing, s32 posincr, u32 len, const UlamClass<EC> * effself, const UsageType usage)
    : m_uc(existing.m_uc)
    , m_effSelf(effself)
    , m_stg(existing.m_stg)
    , m_len(len)
    , m_vtableclassid(0)
    , m_posToDM(existing.m_posToDM)
    , m_prevur(NULL)
  {
    s32 newpos = posincr + (s32) existing.GetPos(); //e.g. pos -25 to start of atom of element ref
    MFM_API_ASSERT_ARG(newpos >= 0); //non-negative
    m_pos = (u32) newpos; //save as unsigned

    m_usage = usage; //save

    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    MFM_API_ASSERT_ARG(existing.m_usage != PRIMITIVE || m_usage == existing.m_usage);  // derived from PRIMITIVE can't change usage type
    MFM_API_ASSERT_ARG(m_usage != ARRAY || m_effSelf == 0); // Array usage has no effself
    MFM_API_ASSERT_ARG(m_usage != CLASSIC || m_effSelf != 0); // Classic usage has effself

    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }

    if((usage == ATOMIC) && (existing.m_usage == ELEMENTAL))
      m_posToEff = 0u; //== pos + t::atom_first_state_bit
    if((usage == ATOMIC) && (existing.m_usage == CLASSIC))
      {
	m_posToEff = 0u; //== pos + t::atom_first_state_bit
	m_posToDM = posincr; //t41610 transient DM, an atom
      }
    else if((usage == ELEMENTAL) && (existing.m_usage == ATOMIC))
      m_posToEff = 0u; //== pos - t::atom_first_state_bit
    else if((usage == CLASSIC) && (existing.m_usage == ATOMIC))
      m_posToEff = m_pos - T::ATOM_FIRST_STATE_BIT - m_posToDM; //== pos - t::atom_first_state_bit (t41360); to start of atom, not always start of stg (t41611);
    else if(m_effSelf && (m_effSelf != existing.m_effSelf))
      m_posToEff = 0u; //data member, new effSelf
    else if(usage == PRIMITIVE)
      m_posToEff = 0u;
    else //base class, same eff self
      {
	//negative when the new base is a subclass of old base (t41325)
	//MFM_API_ASSERT_ARG(posincr >= 0); //non-negative
	m_posToEff = existing.m_posToEff + posincr; //subtract from newpos for effself pos
      }

    if(m_effSelf != NULL)
      {
	m_vtableclassid = m_effSelf->GetRegistrationNumber(); //init
      }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef<EC> & existing, s32 posincr, u32 len)
    : m_uc(existing.m_uc)
    , m_effSelf(existing.m_effSelf)
    , m_stg(existing.m_stg)
    , m_len(len)
    , m_usage(existing.m_usage)
    , m_posToEff(existing.m_posToEff + posincr)
    , m_vtableclassid(0)
    , m_posToDM(existing.m_posToDM)
    , m_prevur(NULL)
  {
    s32 newpos = posincr + (s32) existing.GetPos(); //e.g. pos -25 to start of atom of element ref
    MFM_API_ASSERT_ARG(newpos >= 0); //non-negative
    m_pos = (u32) newpos; //save as unsigned

    MFM_API_ASSERT_ARG(m_pos + m_len <= m_stg.GetBitSize());
    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }

    if(m_effSelf != NULL)
      {
	m_vtableclassid = m_effSelf->GetRegistrationNumber(); //init
      }
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef<EC> & existing, s32 effselfoffset, u32 len, const UsageType usage, bool applydelta)
    : m_uc(existing.m_uc)
    , m_effSelf(existing.m_effSelf)
    , m_stg(existing.m_stg)
    , m_len(len)
    , m_usage(usage)
    , m_vtableclassid(0)
    , m_posToDM(existing.m_posToDM)
    , m_prevur(NULL)
  {
    MFM_API_ASSERT_ARG(effselfoffset >= 0); //non-negative
    MFM_API_ASSERT_ARG(applydelta); //always true, de-ambiguity arg

    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }

    if(m_effSelf != NULL)
      {
	m_vtableclassid = m_effSelf->GetRegistrationNumber(); //init
      }

    //virtual func override class ref, from existing calling ref
    ApplyDelta(existing.GetEffectiveSelfPos(), effselfoffset, len);
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef<EC> & existing, u32 vownedfuncidx, const UlamClass<EC> & origclass, VfuncPtr & vfuncref)
    : m_uc(existing.m_uc)
    , m_effSelf(existing.m_effSelf)
    , m_stg(existing.m_stg)
    , m_len(existing.m_len)
    , m_usage(existing.m_usage)
    , m_vtableclassid(existing.m_vtableclassid)
    , m_posToDM(existing.m_posToDM)
    , m_prevur(& existing)
  {
    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
    }

    const u32 origclassid = origclass.GetRegistrationNumber();
    s32 candidate = -1;
    findMostSpecificNonDominatedVTClassIdInCallstack(existing, vownedfuncidx, origclassid, candidate);
    MFM_API_ASSERT(candidate>=0, NOT_FOUND);

    const UlamClassRegistry<EC> & ucr = existing.m_uc.GetUlamClassRegistry();
    const UlamClass<EC> * vtableclassptr = ucr.GetUlamClassOrNullByIndex(candidate);

    InitUlamRefForVirtualFuncCall(existing, vtableclassptr, vownedfuncidx, origclassid, vfuncref);
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef<EC> & existing, u32 vownedfuncidx, u32 origclassregnum, bool applydelta, VfuncPtr & vfuncref)
    : m_uc(existing.m_uc)
    , m_effSelf(existing.m_effSelf)
    , m_stg(existing.m_stg)
    , m_len(existing.m_len)
    , m_usage(existing.m_usage)
    , m_vtableclassid(existing.m_vtableclassid)
    , m_posToDM(existing.m_posToDM)
    , m_prevur(& existing)
  {
    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
      m_prevur = NULL;
      m_vtableclassid = m_effSelf->GetRegistrationNumber();
    }

    s32 candidate = -1;
    findMostSpecificNonDominatedVTClassIdInCallstack(existing, vownedfuncidx, origclassregnum, candidate);
    MFM_API_ASSERT(candidate>=0, NOT_FOUND);

    const UlamClassRegistry<EC> & ucr = existing.m_uc.GetUlamClassRegistry();
    const UlamClass<EC> * vtableclassptr = ucr.GetUlamClassOrNullByIndex(candidate);
    //applydelta is de-ambiguity arg
    InitUlamRefForVirtualFuncCall(existing, vtableclassptr, vownedfuncidx, origclassregnum, vfuncref);
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRef<EC> & existing, const UlamClass<EC> * vtclassptr, u32 vownedfuncidx, u32 origclassregnum, VfuncPtr & vfuncref)
    : m_uc(existing.m_uc)
    , m_effSelf(existing.m_effSelf)
    , m_stg(existing.m_stg)
    , m_len(existing.m_len)
    , m_usage(existing.m_usage)
    , m_posToDM(existing.m_posToDM)
    , m_prevur(& existing)
  {
    if ((m_usage == ATOMIC || m_usage == ELEMENTAL) && !m_effSelf)
    {
      UpdateEffectiveSelf();
      m_prevur = NULL;
    }

    InitUlamRefForVirtualFuncCall(existing, vtclassptr, vownedfuncidx, origclassregnum, vfuncref);
    m_vtableclassid = vtclassptr->GetRegistrationNumber(); //save!! newly specified vtable classid
  }

  template <class EC>
  UlamRef<EC>::UlamRef(const UlamRefMutable<EC> & muter)
    : m_uc(*checknonnulluc(muter.GetContextPtr()))
    , m_effSelf(muter.GetEffectiveSelfPtr())
    , m_stg(*checknonnullstg(muter.GetBitStoragePtr()))
    , m_pos(muter.GetPos())
    , m_len(muter.GetLen())
    , m_usage(muter.GetUsageType())
    , m_posToEff(muter.GetPosToEffectiveSelf())
    , m_vtableclassid(muter.GetVTableClassId())
    , m_posToDM(muter.GetPosToDataMember())
    , m_prevur(muter.GetPreviousUlamRefPtr())
  { }


  template <class EC>
  void UlamRef<EC>::InitUlamRefForVirtualFuncCall(const UlamRef<EC> & ur, const UlamClass<EC> * vtclassptr, u32 vownedfuncidx, u32 origclassregnum, VfuncPtr & vfuncref)
  {
    MFM_API_ASSERT_NONNULL(vtclassptr); //could be same as effSelf

    //check VTable class is/related to origclass
    if(!vtclassptr->internalCMethodImplementingIs(origclassregnum))
      FAIL(BAD_VIRTUAL_CALL);

    const UlamClass<EC> * effSelf = ur.GetEffectiveSelf();
    MFM_API_ASSERT_NONNULL(effSelf);

    //check effSelf is/related to VTable class
    if(!effSelf->internalCMethodImplementingIs(vtclassptr))
      FAIL(BAD_VIRTUAL_CALL);

    //3 VTable accesses for: originating class' start, vfunc entry, and its override class
    const u32 origclassvtstart = vtclassptr->GetVTStartOffsetForClassByRegNum(origclassregnum);
    vfuncref = vtclassptr->getVTableEntry(vownedfuncidx + origclassvtstart); //return ref to virtual function ptr
    const UlamClass<EC> * ovclassptr = vtclassptr->getVTableEntryUlamClassPtr(vownedfuncidx + origclassvtstart);
    MFM_API_ASSERT_NONNULL(ovclassptr);

    //relative to effSelf
    const s32 ovclassrelpos = effSelf->internalCMethodImplementingGetRelativePositionOfBaseClass(ovclassptr);
    MFM_API_ASSERT(ovclassrelpos >= 0, PURE_VIRTUAL_CALLED);

    const u32 ovclasslen = (ovclassptr == effSelf) ? ovclassptr->GetClassLength() : ovclassptr->GetClassDataMembersSize(); //use baseclass size when incomplete obj, not element.
    ApplyDelta(ur.GetEffectiveSelfPos(), ovclassrelpos, ovclasslen);

    m_usage = ovclassptr->AsUlamElement() ? ELEMENTAL : CLASSIC;
  } //InitUlamRefForVirtualFuncCall

  template <class EC>
  void UlamRef<EC>::ApplyDelta(s32 existingeffselfpos, s32 effselfoffset, u32 len)
  {
    MFM_API_ASSERT_ARG(effselfoffset >= 0); //non-negative

    //virtual func override class ref, from existing calling ref
    s32 newpos = existingeffselfpos + effselfoffset;
    MFM_API_ASSERT_ARG(newpos >= 0); //non-negative
    m_pos = (u32) newpos; //save as unsigned
    m_posToEff = (u32) effselfoffset; //subtract from newpos for pos of effself; (t3735)
    m_len = len;
    MFM_API_ASSERT_ARG((u32)(m_pos + m_len - m_posToEff) <= m_stg.GetBitSize());
  }

  template <class EC>
  bool UlamRef<EC>::findMostSpecificNonDominatedVTClassIdInCallstack(const UlamRef<EC> & existing, const u32 vownedfuncidx, const u32 origclassregnum, s32& candidateid) const
  {
    bool gotit = false;
    //start with first call, m_prevur is NULL, look into vtable of effSelf
    if(m_prevur != NULL)
      gotit = m_prevur->findMostSpecificNonDominatedVTClassIdInCallstack(existing, vownedfuncidx, origclassregnum, candidateid);

    if(!gotit)
      {
	const UlamClassRegistry<EC> & ucr = m_uc.GetUlamClassRegistry();
	const UlamClass<EC> * vtableclassptr = ucr.GetUlamClassOrNullByIndex(m_vtableclassid);
	MFM_API_ASSERT_NONNULL(vtableclassptr); //first one checked is effSelf

	//3 VTable accesses for: originating class' start, vfunc entry, and its override class
	const u32 origclassvtstart = vtableclassptr->GetVTStartOffsetForClassByRegNum(origclassregnum);
	const UlamClass<EC> * ovclassptr = vtableclassptr->getVTableEntryUlamClassPtr(vownedfuncidx + origclassvtstart);
	MFM_API_ASSERT_NONNULL(ovclassptr);

	if(candidateid < 0)
	  candidateid = ovclassptr->GetRegistrationNumber();
	else
	  {
	    if(!ovclassptr->IsTheEmptyClass()) //i.e. not Empty -> "pure" (t41397)
	      {
		const u32 ovid = ovclassptr->GetRegistrationNumber();
		if(!ovclassptr->internalCMethodImplementingIs(candidateid))
		  {
		    //here, ovclass not subclass of candidate, but may be a baseclass of candidate..
		    const UlamClass<EC> * candidateclassptr = ucr.GetUlamClassOrNullByIndex(candidateid);
		    MFM_API_ASSERT_NONNULL(candidateclassptr);
		    if(!candidateclassptr->internalCMethodImplementingIs(ovid)) //&&
		      {
			//here, candidate ALSO not subclass of override,
			//t.f. candidate was not more specific (t41403)
			candidateid = ovid; // non-dominated
		      }
		    // else no change to candidateid because candidate is already the more-specific
		  }
		else //ovclass is a subclass of candidate, hence more-specific
		  {
		    candidateid = ovid; // non-dominated
		  }
	      } //Empty override neither dominating nor more-specific.
	  }
	gotit = ((s32) m_vtableclassid == candidateid); //forsure gotit, when implemented here
      }
    //else gotit short-circuit, candidateid is set

    MFM_API_ASSERT(candidateid >= 0, UNINITIALIZED_VALUE);
    return gotit;
  } //findMostSpecificNonDominatedVTClassIdInCallstack

  template <class EC>
  void UlamRef<EC>::UpdateEffectiveSelf()
  {
    m_effSelf = LookupUlamElementTypeFromAtom();
  }

  template <class EC>
  void UlamRef<EC>::CheckEffectiveSelf() const
  {
    if (m_usage == ATOMIC || m_usage == ELEMENTAL)
    {
      const UlamClass<EC> * eltptr = LookupUlamElementTypeFromAtom();
      MFM_API_ASSERT((eltptr->internalCMethodImplementingIs(m_effSelf)), STALE_ATOM_REF);
    }
  }

  template <class EC>
  const UlamClass<EC>* UlamRef<EC>::LookupUlamElementTypeFromAtom() const
  {
    MFM_API_ASSERT_STATE(m_usage == ATOMIC || m_usage == ELEMENTAL);
    T a = ReadAtom();
    MFM_API_ASSERT(a.IsSane(),INCONSISTENT_ATOM);
    u32 etype = a.GetType();
    const UlamClass<EC> * eltptr = m_uc.LookupUlamElementTypeFromContext(etype);
    MFM_API_ASSERT_STATE(eltptr);
    return eltptr;
  }

  template <class EC>
  u32 UlamRef<EC>::GetType() const
  {
    const UlamClass<EC> * effSelf = GetEffectiveSelf();
    MFM_API_ASSERT_ARG(effSelf);
    const UlamElement<EC> * eltptr = effSelf->AsUlamElement();
    if(!eltptr) return T::ATOM_UNDEFINED_TYPE; //quark
    return eltptr->GetType();
  } //GetType

  template <class EC>
  typename EC::ATOM_CONFIG::ATOM_TYPE UlamRef<EC>::CreateAtom() const
  {
    const UlamClass<EC> * effSelf = GetEffectiveSelf();
    MFM_API_ASSERT_ARG(effSelf);
    const UlamElement<EC> * eltptr = effSelf->AsUlamElement();
    if(!eltptr) FAIL(ILLEGAL_ARGUMENT);
    u32 len = eltptr->GetClassLength();
    AtomBitStorage<EC> atmp(eltptr->GetDefaultAtom());
    atmp.WriteBig(0u + T::ATOM_FIRST_STATE_BIT, len, m_stg.ReadBig(GetPos(), len));
    return atmp.ReadAtom();
  }

  template <class EC>
  void UlamRef<EC>::Print(const UlamClassRegistry<EC>&uc, ByteSink& bs, u32 printFlags) const
  {
    if (!m_effSelf)
    {
      bs.Printf("UlamRef[pos=%d,len=%d,NULL]", m_pos, m_len);
      return;
    }

    const UlamElement<EC> * ue = m_effSelf->AsUlamElement();
    if (ue)
    {
      const T atom = this->ReadAtom();
      ue->Print(uc, bs, atom, printFlags, m_pos);
      return;
    }

    // If this isn't an ulam element, MFM doesn't have name info for
    // its type, but we can still print its class members
    m_effSelf->PrintClassMembers(uc, bs, m_stg, printFlags, GetEffectiveSelfPos());
  }

} //MFM
