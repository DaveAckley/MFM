
/* Tile codes don't have to start with A, but whatever tiles sizes are
   configured must have consecutive letter codes!  Here we support A-J
   by default, or A-M if EXTRA_TILE_SIZES is defined.  Every tile size
   that is included implies another huge pile of templates for g++ to
   instantiate, slowing the MFM build time.
   Width,Height values may be different (H-J);
   Usage: ./bin/mfms {ctr}|{{ctr}}
   where c is number of columns, t is Tile code, r is number of rows,
   	 and double braces is for a staggered grid layout
 */

#ifdef EXTRA_TILE_SIZES
#endif /*EXTRA_TILE_SIZES*/

XX(A,48+8,32+8)    /* T2 tile size doh! */
XX(B,32,32)
XX(C,40,40)
XX(D,54,54)
XX(E,72,72)
XX(F,98,98)
XX(G,136,136)
XX(H,60,40)
XX(I,80,40)
XX(J,128,76)


#ifdef EXTRA_TILE_SIZES
XX(K,188,188)
XX(L,264,264)
XX(M,370,370)
#endif /*EXTRA_TILE_SIZES*/
