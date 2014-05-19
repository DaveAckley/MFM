/* -*- C++ -*- */

namespace MFM {

  template <class UNSIGNED_TYPE>
  void ByteSink::PrintInBase(UNSIGNED_TYPE n, u32 base, s32 width, u8 pad) {
    if (base < 2 || base > 36)
      FAIL(ILLEGAL_ARGUMENT);

    u8 buf[8 * sizeof(UNSIGNED_TYPE)]; // Worst case is binary at 8 u8's per byte
    u32 i = 0;

    do {
      buf[i++] = (u8) (n % base);
      n /= base;
    } while (n > 0);

    if (width >= 0) {
      u32 uwidth = (u32) width;
      while (uwidth > i) {
        WriteByte(pad);
        --uwidth;
      }
    } /* XXX Left justified field widths NYI */

    for (; i > 0; i--)
      WriteByte(buf[i - 1] < 10 ?
                '0' + buf[i - 1] :
                'A' + buf[i - 1] - 10);
  }

}
