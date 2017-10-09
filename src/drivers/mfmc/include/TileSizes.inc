
/* Tile codes don't have to start with A, but whatever tiles sizes are
   configured must have consecutive letter codes!  Here we support B-F
   by default, or A-J if EXTRA_TILE_SIZES is defined.  Every tile size
   that is included implies another huge pile of templates for g++ to
   instantiate, slowing the MFM build time.
   Width,Height may be different;
 */

#ifdef EXTRA_TILE_SIZES
XX(A,24,24)
#endif /*EXTRA_TILE_SIZES*/

XX(B,32,32)
XX(C,40,40)
XX(D,54,32)
XX(E,72,72)
XX(F,98,98)
XX(G,136,136)

#ifdef EXTRA_TILE_SIZES
XX(H,188,188)
XX(I,264,264)
XX(J,370,370)
#endif /*EXTRA_TILE_SIZES*/
