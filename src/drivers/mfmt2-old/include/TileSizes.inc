
/* Tile codes don't have to start with A, but whatever tiles sizes are
   configured must have consecutive letter codes!  Here we support B-J
   by default, or A-M if EXTRA_TILE_SIZES is defined.  Every tile size
   that is included implies another huge pile of templates for g++ to
   instantiate, slowing the MFM build time.
   Width,Height values may be different (H-J);
   Usage: ./bin/mfms {ctr}|{{ctr}}
   where c is number of columns, t is Tile code, r is number of rows,
   	 and double braces is for a staggered grid layout
 */

/*XX(G,48,32)*/
XX(H,60,40)
XX(I,72,48)
XX(J,90,60)
