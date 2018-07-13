#include "CastOps.h"

namespace MFM {

  u32 _BinOpDivideInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpDivideCs32WithBoundsCheck(cvala, cvalb);
      }
    return _Cs32ToInt32((cvala / cvalb), bitwidth);
  }

  u64 _BinOpDivideInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpDivideCs64WithBoundsCheck(cvala, cvalb);
      }
    return _Cs64ToInt64((cvala / cvalb), bitwidth);
  }

  u32 _BinOpModInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _Cs32ToInt32((cvala % cvalb), bitwidth);
  }

  u64 _BinOpModInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _Cs64ToInt64((cvala % cvalb), bitwidth);
  }

  u32 _BinOpDivideUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _Cu32ToUnary32(binvala / binvalb, bitwidth);
  }

  u64 _BinOpDivideUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _Cu64ToUnary64(binvala / binvalb, bitwidth);
  }

  u32 _BinOpModUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _Cu32ToUnary32(binvala % binvalb, bitwidth);
  }

  u64 _BinOpModUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _Cu64ToUnary64(binvala % binvalb, bitwidth);
  }

  u32 _BinOpDivideUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    //no special bounds checking needed for Unsigned division
    return _Cu32ToUnsigned32((cvala / cvalb), bitwidth);
  }

  u64 _BinOpDivideUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    //no special bounds checking needed for Unsigned division
    return _Cu64ToUnsigned64((cvala / cvalb), bitwidth);
  }

  u32 _BinOpModUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _Cu32ToUnsigned32((cvala % cvalb), bitwidth);
  }

  u64 _BinOpModUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _Cu64ToUnsigned64((cvala % cvalb), bitwidth);
  }

  u32 _BinOpDivideBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    u32 rtnval = _Cu32ToUnsigned32(binvala / binvalb, bitwidth);
    return _Unsigned32ToBool32(rtnval, bitwidth, bitwidth);
  }

  u64 _BinOpDivideBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    u64 rtnval = _Cu64ToUnsigned64(binvala / binvalb, bitwidth);
    return _Unsigned64ToBool64(rtnval, bitwidth, bitwidth);
  }

  u32 _BinOpModBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    u32 rtnval = _Cu32ToUnsigned32(binvala % binvalb, bitwidth);
    return _Unsigned32ToBool32(rtnval, bitwidth, bitwidth);
  }

  u64 _BinOpModBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    u64 rtnval = _Cu64ToUnsigned64(binvala % binvalb, bitwidth);
    return _Unsigned64ToBool64(rtnval, bitwidth, bitwidth);
  }

} //MFM
