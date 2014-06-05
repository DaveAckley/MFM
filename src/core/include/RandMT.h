// This is the ``Mersenne Twister'' random number generator MT19937, which
// generates pseudorandom integers uniformly distributed in 0..(2^32 - 1)
// starting from any odd seed in 0..(2^32 - 1).  This version is a recode
// by Shawn Cokus (Cokus@math.washington.edu) on March 8, 1998 of a version by
// Takuji Nishimura (who had suggestions from Topher Cooper and Marc Rieffel in
// July-August 1997).
//
// Effectiveness of the recoding (on Goedel2.math.washington.edu, a DEC Alpha
// running OSF/1) using GCC -O3 as a compiler: before recoding: 51.6 sec. to
// generate 300 million random numbers; after recoding: 24.0 sec. for the same
// (i.e., 46.5% of original time), so speed is now about 12.5 million random
// number generations per second on this machine.
//
// According to the URL <http://www.math.keio.ac.jp/~matumoto/emt.html>
// (and paraphrasing a bit in places), the Mersenne Twister is ``designed
// with consideration of the flaws of various existing generators,'' has
// a period of 2^19937 - 1, gives a sequence that is 623-dimensionally
// equidistributed, and ``has passed many stringent tests, including the
// die-hard test of G. Marsaglia and the load test of P. Hellekalek and
// S. Wegenkittl.''  It is efficient in memory usage (typically using 2506
// to 5012 bytes of static data, depending on data type sizes, and the code
// is quite short as well).  It generates random numbers in batches of 624
// at a time, so the caching and pipelining of modern systems is exploited.
// It is also divide- and mod-free.
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation (either version 2 of the License or, at your
// option, any later version).  This library is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY, without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU Library General Public License for more details.  You should have
// received a copy of the GNU Library General Public License along with this
// library; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307, USA.
//
// The code as Shawn received it included the following notice:
//
//   Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.  When
//   you use this, send an e-mail to <matumoto@math.keio.ac.jp> with
//   an appropriate reference to your work.
//
// It would be nice to CC: <Cokus@math.washington.edu> when you write.
//
// RandMT class created by Paul Gresham <gresham@mediavisual.com>
// There seems to be a slight performance deficit in process creation
// however I've not profiled the class to compare it with the straight
// C code.
//
// Use of a class removes many C nasties and also allows you to easily 
// create multiple generators.
// To compile on GNU a simple line is:
// g++ -O3 RandMT.cc -o RandMT
//

#ifndef _RANDMT_H_
#define _RANDMT_H_

namespace MFM {

typedef unsigned long uint32;

class RandMT {

  static const int N =          624;                // length of state vector
  static const int M =          397;                // a period parameter
  static const uint32 K =       0x9908B0DFU;        // a magic constant

  // If you want a single generator, consider using a singleton class 
  // instead of trying to make these static.
  uint32   state[N+1];  // state vector + 1 extra to not violate ANSI C
  uint32   *next;       // next random value is computed from here
  uint32   initseed;    //
  int      left;        // can *next++ this many times before reloading

  inline uint32 hiBit(uint32 u) { 
    return u & 0x80000000U;    // mask all but highest   bit of u
  }

  inline uint32 loBit(uint32 u) { 
    return u & 0x00000001U;    // mask all but lowest    bit of u
  }

  inline uint32 loBits(uint32 u) { 
    return u & 0x7FFFFFFFU;   // mask     the highest   bit of u
  }

  inline uint32 mixBits(uint32 u, uint32 v) {
    return hiBit(u)|loBits(v);  // move hi bit of u to hi bit of v
  }
  
  uint32 reloadMT(void) ;

  // MUST NOT PASS SEQUENTIAL SEEDS TO THIS VERSION
  void seedMT(uint32 seed) ;

public:
  RandMT() ;
  RandMT(uint32 seed) ;
  inline uint32 randomMT(void) ; 

  // ACKLEYHAX: This seeding function hacked for MFM
  void seedMT_MFM(uint32 s) ;

};

inline uint32 RandMT::randomMT(void) {
  uint32 y;

  if(--left < 0)
    return(reloadMT());

  y  = *next++;
  y ^= (y >> 11);
  y ^= (y <<  7) & 0x9D2C5680U;
  y ^= (y << 15) & 0xEFC60000U;
  return(y ^ (y >> 18));
}
} /* namespace MFM */

#endif // _RANDMT_H_



