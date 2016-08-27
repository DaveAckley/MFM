
/* Tile codes don't have to start with A, but whatever tiles sizes are
   configured must have consecutive letter codes!  Here we support B-F
   by default, or A-J if EXTRA_TILE_SIZES is defined.  Every tile size
   that is included implies another huge pile of templates for g++ to
   instantiate, slowing the MFM build time.
 */

#ifdef EXTRA_TILE_SIZES
XX(A,24,24)
#endif /*EXTRA_TILE_SIZES*/

XX(B,32,32)
XX(C,40,40)
XX(D,54,54)
XX(E,80,40)
XX(F,108,54)

#ifdef EXTRA_TILE_SIZES
XX(G,136,136)
XX(H,188,188)
XX(I,264,264)
XX(J,370,370)
#endif /*EXTRA_TILE_SIZES*/
