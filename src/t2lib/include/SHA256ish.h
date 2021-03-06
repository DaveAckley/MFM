/*                                              -*- mode:C++ -*-
  SHA256ish.h Heavily bashed-over but possibly-still-conforming SHA26 code.  Modified from
  Original code Copyright (c) 2021 Jérémy LAMBERT (SystemGlitch) - SEE LICENSE AT BOTTOM OF THIS FILE
  Modifications Copyright (C) 2021 Living Computation Foundation.  All rights reserved.

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
  \file SHA256ish.h SHA256-inspired digest code
  \author Jérémy LAMBERT (SystemGlitch)
  \author David H. Ackley.
  \date (C) 2021 All rights reserved.
  \lgpl
 */

#ifndef SHA256ISH_H
#define SHA256ISH_H

#include "itype.h"
#include "ByteSink.h"
#include "T2Types.h"

namespace MFM {
class SHA256ish {
public:
  SHA256ish();
  void addByte(const u8 byte) { addBytes(&byte,1u); }
  void addBytes(const u8 * data, u32 length);
  bool digest(Bytes32 out);
  bool digest(ByteSink& to, bool ashex = true);

private:
  bool m_digested;
  u8  m_data[64];
  u32 m_blocklen;
  u64 m_bitlen;
  u32 m_state[8]; //A, B, C, D, E, F, G, H

  static inline u32 rotr(u32 x, u32 n) {
    return (x >> n) | (x << (32 - n));
  }

  static inline u32 choose(u32 e, u32 f, u32 g) {
    return (e & f) ^ (~e & g);
  }

  static inline u32 majority(u32 a, u32 b, u32 c) {
    return (a & (b | c)) | (b & c);
  }

  static inline u32 sig0(u32 x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
  }

  static inline u32 sig1(u32 x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
  }

  void transform();
  void pad();
  void revert(u8 * hash);
};
}
#endif  /* SHA256ISH_H */

/**
MIT License

Copyright (c) 2021 Jérémy LAMBERT (SystemGlitch)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
