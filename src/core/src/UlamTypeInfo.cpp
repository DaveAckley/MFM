#include "UlamTypeInfo.h"

namespace MFM
{

  bool UlamTypeInfo::InitFrom(ByteSource & cbs)
  {
    cbs.SkipWhitespace();
    u8 code;
    if (cbs.Scanf("U%c_",&code) != 3) return false;

    Category cat = GetCategoryFromUCode(code);
    if (cat == UNKNOWN) return false;

    if (cat == PRIM)
    {
      if (!m_utip.InitFrom(cbs)) return false;
    }
    else
    {
      if (!m_utic.InitFrom(cbs)) return false;
    }
    m_category = cat;
    return true;
  }

  void UlamTypeInfo::PrintMangled(ByteSink & bs) const
  {
    if (IsUnknown())
      FAIL(ILLEGAL_STATE);

    bs.Printf("U%c_",GetUCodeFromCategory());

    if (IsPrimitive())
      m_utip.PrintMangled(bs);
    else
      m_utic.PrintMangled(bs);
  }

  void UlamTypeInfo::PrintPretty(ByteSink & bs) const
  {
    bs.Printf("%s",GetPrettyPrefixFromCategory(m_category));

    if (IsPrimitive()) m_utip.PrintPretty(bs);
    else if (IsClass()) m_utic.PrintPretty(bs);
    else FAIL(ILLEGAL_STATE);
  }

  bool UlamTypeInfoPrimitive::PrimTypeFromChar(const u8 ch, PrimType & result)
  {
    PrimType type;
    switch (ch) {
    case 'b': type = BOOL; break;
    case 'i': type = INT; break;
    case 't': type = BITS; break;
    case 'u': type = UNSIGNED; break;
    case 'v': type = VOID; break;
    case 'y': type = UNARY; break;
    case 's': type = STRING; break;
    default: return false;
    }
    result = type;
    return true;
  }

  u8 UlamTypeInfoPrimitive::CharFromPrimType(const PrimType type)
  {
    switch (type) {
    case BOOL: return 'b';
    case INT: return 'i';
    case BITS: return 't';
    case UNSIGNED: return 'u';
    case VOID: return 'v';
    case UNARY: return 'y';
    case STRING: return 's';
    }
    FAIL(ILLEGAL_ARGUMENT);
  }

  const char * UlamTypeInfoPrimitive::NameFromPrimType(const PrimType type)
  {
    switch (type) {
    case BOOL: return "Bool";
    case INT: return "Int";
    case BITS: return "Bits";
    case UNSIGNED: return "Unsigned";
    case VOID: return "Void";
    case UNARY: return "Unary";
    case STRING: return "String";
    }
    FAIL(ILLEGAL_ARGUMENT);
  }

  u32 UlamTypeInfoPrimitive::DefaultSizeFromPrimType(const PrimType type)
  {
    switch (type) {
    case BOOL: return 1;
    case INT: return 32;
    case BITS: return 32;
    case UNSIGNED: return 32;
    case VOID: return 0;
    case UNARY: return 32;
    case STRING: return 32;
    }
    FAIL(ILLEGAL_ARGUMENT);
  }

  bool UlamTypeInfoPrimitive::InitFrom(ByteSource & cbs)
  {
    u32 arraylen, bitsize, namelen;
    if (!cbs.Scan(arraylen, Format::LEX32, 0)) return false;
    if (!cbs.Scan(bitsize, Format::LEX32, 0)) return false;
    if (!cbs.Scan(namelen, Format::LEXHD, 0)) return false;
    if (namelen != 1) return false;

    s32 namech = cbs.Read();
    if (namech < 0) return false;
    PrimType type;
    if (!PrimTypeFromChar((u8) namech, type)) return false;

    m_primType = type;
    m_bitSize = bitsize;
    m_arrayLength = arraylen;

    return true;
  }

  void UlamTypeInfoPrimitive::PrintMangled(ByteSink & bs) const
  {
    bs.Printf("%D%D1%c", m_arrayLength, m_bitSize, CharFromPrimType(GetPrimType()));
  }

  void UlamTypeInfoPrimitive::PrintPretty(ByteSink & bs) const
  {
    bs.Printf("%s", NameFromPrimType(GetPrimType()));

    if (m_bitSize != DefaultSizeFromPrimType(GetPrimType()))
      bs.Printf("(%d)", m_bitSize);

    if (m_arrayLength > 0)
      bs.Printf("[%d]", m_arrayLength);
  }

  bool UlamTypeInfoClass::InitFrom(ByteSource & cbs)
  {
    u32 arraylen, bitsize, namelen, parms;
    if (!cbs.Scan(arraylen, Format::LEX32, 0)) return false;
    if (!cbs.Scan(bitsize, Format::LEX32, 0)) return false;
    if (!cbs.Scan(namelen, Format::LEXHD, 0)) return false;

    OStringClassName name;
    for (u32 i = 0; i < namelen; ++i)
    {
      s32 ch = cbs.Read();
      if (ch < 0) return false;
      name.WriteByte((u8) ch);
    }

    if (!cbs.Scan(parms, Format::LEX32, 0)) return false;

    UlamTypeInfoClassParameterArray parameters;
    for (u32 i = 0; i < parms; ++i)
    {
      UlamTypeInfoParameter uticp;
      if (!uticp.m_parameterType.InitFrom(cbs)) return false;

      if (uticp.m_parameterType.m_primType == UlamTypeInfoPrimitive::STRING)
	{
	  u32 strlen;
	  if (cbs.Scanf("%02x",&strlen) != 1) return false;
	  uticp.m_stringValue.Reset();
	  uticp.m_stringValue.WriteByte((u8) strlen); // length into [0]
	  for (u32 j = 0; j < strlen; ++j) {
	    u32 chr;
	    if (cbs.Scanf("%02x",&chr) != 1) return false;
	    uticp.m_stringValue.WriteByte((u8) chr);
	  }
	}
      else
	{
	  u32 arrayloop = uticp.m_parameterType.m_arrayLength;
	  if(arrayloop > MAX_CLASS_PARAMETER_ARRAY_LENGTH) return false;
	  arrayloop = ((arrayloop == 0) ? 1 : arrayloop);

	  for(u32 j = 0; j < arrayloop; j++)
	    {
	      s32 ch = cbs.Read();
	      if (ch < 0) return false;
	      if (ch != 'n') cbs.Unread();

	      if (!cbs.Scan(uticp.m_value[j], Format::LEX32, 0)) return false;
	      if (ch == 'n') {
		if (uticp.m_value[j] == 0)
		  uticp.m_value[j] = (u32) S32_MIN;
		else
		  uticp.m_value[j] = (u32) -uticp.m_value[j];
	      }
	    }
	}

      if (i < MAX_CLASS_PARAMETERS)
        parameters[i] = uticp;
    }

    m_name = name;
    m_arrayLength = arraylen;
    m_bitSize = bitsize;
    m_classParameterCount = parms;

    for (u32 i = 0; i < MAX_CLASS_PARAMETERS; ++i)
      if (i < m_classParameterCount)
        m_classParameters[i] = parameters[i];

    return true;
  }

  void UlamTypeInfoClass::PrintMangled(ByteSink & bs) const
  {
    bs.Printf("%D%D%H%s",
              m_arrayLength,
              m_bitSize,
              m_name.GetLength(),
              m_name.GetZString());

    bs.Printf("%D",m_classParameterCount);

    for (u32 i = 0; i < m_classParameterCount; ++i)
    {
      if (i < MAX_CLASS_PARAMETERS)
	{
	  m_classParameters[i].m_parameterType.PrintMangled(bs);
	  if (m_classParameters[i].m_parameterType.GetPrimType() == UlamTypeInfoPrimitive::STRING)
	    {
	      const char * s = m_classParameters[i].m_stringValue.GetBuffer();
	      u32 len = *s++;
	      bs.Printf("%02x",len);
	      for (u32 j = 0; j < len; ++j)
		bs.Printf("%02x",s[j]);
	    }
	  else
	    {
	      u32 arrayloop = m_classParameters[i].m_parameterType.m_arrayLength;
	      if(arrayloop > MAX_CLASS_PARAMETER_ARRAY_LENGTH) FAIL(ILLEGAL_STATE);
	      arrayloop = ((arrayloop == 0) ? 1 : arrayloop);

	      for(u32 j = 0; j < arrayloop; j++)
		{
		  if (m_classParameters[i].m_parameterType.GetPrimType() == UlamTypeInfoPrimitive::INT
		      && ((s32) m_classParameters[i].m_value[j]) < 0)
		    {
		      if (((s32) m_classParameters[i].m_value[j]) == S32_MIN)
			bs.Printf("n%D",0);
		      else
			bs.Printf("n%D", - (s32) m_classParameters[i].m_value[j]);
		    }
		  else
		    bs.Printf("%D",m_classParameters[i].m_value[j]);
		}
	    }
	}
      else
	bs.Printf("0");
    }
  }

  void UlamTypeInfoClass::PrintPretty(ByteSink & bs) const
  {
    m_name.AppendTo(bs);

    if (m_classParameterCount==0)
      return;

    bs.Printf("(");
    for (u32 i = 0; i < m_classParameterCount; ++i)
    {
      if (i > 0) bs.Printf(",");
      if (i < MAX_CLASS_PARAMETERS)
      {
        m_classParameters[i].m_parameterType.PrintPretty(bs);

	if(m_classParameters[i].m_parameterType.GetPrimType() == UlamTypeInfoPrimitive::STRING)
	  {
	    bs.Printf("=");
	    bs.PrintDoubleQuotedCStringWithLength(m_classParameters[i].m_stringValue.GetBuffer());
	  }
	else
	  {
	    u32 arrayloop = m_classParameters[i].m_parameterType.m_arrayLength;
	    if(arrayloop > MAX_CLASS_PARAMETER_ARRAY_LENGTH) FAIL(ILLEGAL_STATE);

	    arrayloop = ((arrayloop == 0) ? 1 : arrayloop);
	    bs.Printf("=");
	    if(arrayloop > 1)
	      bs.Printf("{"); //start of array values
	    for(u32 j = 0; j < arrayloop; j++)
	      {
		if(j > 0)
		  bs.Printf(",");
		switch (m_classParameters[i].m_parameterType.GetPrimType())
		  {
		  case UlamTypeInfoPrimitive::INT:
		    bs.Printf("%d",m_classParameters[i].m_value[j]);
		    break;
		  case UlamTypeInfoPrimitive::BITS:
		    bs.Printf("0x%x",m_classParameters[i].m_value[j]);
		    break;
		  default:
		    bs.Printf("%uu",m_classParameters[i].m_value[j]);
		  }
	      }
	    if(arrayloop > 1)
	      bs.Printf("}"); //end of array
	  }
      }
      else
        bs.Printf("?");
    }
    bs.Printf(")"); //end of parameters
  }

  u64 UlamTypeInfoPrimitive::GetExtremeOfScalarType(bool wantMax) const
  {
    AssertScalar();
    switch (m_primType) {
    default:
      FAIL(ILLEGAL_STATE);
    case VOID:
      FAIL(ILLEGAL_STATE);
    case INT:
      if (wantMax) return _GetNOnes64(m_bitSize-1);
      return ((u64)1)<<(m_bitSize-1);
    case BITS: // Treat bits like unsigned even though they're not
    case UNSIGNED:
      if (wantMax) return _GetNOnes64(m_bitSize);
      return (u64) 0;
    case BOOL: // Treat bool as ordered even though it's not
      if (wantMax) return 1;
      return 0;
    case UNARY:
      if (wantMax) return m_bitSize;
      return 0;
    }
  }

} //MFM
