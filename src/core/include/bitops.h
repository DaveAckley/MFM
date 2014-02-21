#ifndef _BITOPS_H_             /* -*- mode: C++ -*- */
#define _BITOPS_H_

/* This is used by Random.h but needs to be integrated with BitVector.h! */

#include <limits>            // For numeric_limits<T>::is_signed
#include "itype.h"

namespace MFM {

  inline s32 _SignExtend32(u32 val, u32 bitwidth) {
    return ((s32)(val<<(32-bitwidth)))>>bitwidth;
  }

  inline s64 _SignExtend64(u64 val, u32 bitwidth) {
    return ((s64)(val<<(64-bitwidth)))>>bitwidth;
  }

  inline u32 _GetNOnes32(u32 bitwidth) {
    return (((u32)1)<<bitwidth)-1;
  }

  inline u64 _GetNOnes64(u32 bitwidth) {
    return (((u64)1)<<bitwidth)-1;
  }

  inline u32 _ShiftToBitNumber32(u32 value, u32 bitpos) {
    return value<<bitpos;
  }

  inline u64 _ShiftToBitNumber64(u32 value, u32 bitpos) {
    return ((u64) value)<<bitpos;
  }

  inline u32 _ShiftFromBitNumber32(u32 value, u32 bitpos) {
    return value>>bitpos;
  }

  inline u64 _ShiftFromBitNumber64(u64 value, u32 bitpos) {
    return value>>bitpos;
  }

  inline u32 _GetMask32(u32 bitpos, u32 bitwidth) {
    return _ShiftToBitNumber32(_GetNOnes32(bitwidth),bitpos);
  }

  inline u64 _GetMask64(u32 bitpos, u32 bitwidth) {
    return _ShiftToBitNumber64(_GetNOnes64(bitwidth),bitpos);
  }

  inline u32  _ExtractField32(u32 val, u32 bitpos,u32 bitwidth) {
    return _ShiftFromBitNumber32(val,bitpos)&_GetNOnes32(bitwidth);
  }

  inline u32  _ExtractUint32(u32 val, u32 bitpos,u32 bitwidth) {
    return _ExtractField32(val,bitpos,bitwidth);
  }

  inline s32  _ExtractSint32(u32 val, u32 bitpos,u32 bitwidth) {
    return _SignExtend32(_ExtractField32(val,bitpos,bitwidth),bitwidth);
  }

  inline u32 _getParity32(u32 v) {
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
  }

  // v must be <= 0x7fffffff
  inline u32 _getNextPowerOf2(u32 v) {
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    return v+1;
  }

}    

#endif  /* _BITOPS_H_ */
