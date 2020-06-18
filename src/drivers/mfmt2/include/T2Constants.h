#ifndef T2CONSTANTS_H
#define T2CONSTANTS_H

/** Maximum number of active event windows in flight */
#define MAX_EWSLOT 32

/** Tile width in sites (including cache) */
/*#define T2TILE_WIDTH 68*/
#define T2TILE_WIDTH (48+8)

/** Tile height in sites (including cache) */
/*#define T2TILE_HEIGHT 48*/
#define T2TILE_HEIGHT (32+8)

#define MAX_EVENT_WINDOW_RADIUS 4

#define CACHE_LINES (MAX_EVENT_WINDOW_RADIUS)

/** Tile width in sites (excluding caches) */
#define T2TILE_OWNED_WIDTH (T2TILE_WIDTH - 2 * CACHE_LINES)

/** Tile height in sites (excluding caches) */
#define T2TILE_OWNED_HEIGHT (T2TILE_HEIGHT - 2 * CACHE_LINES)

/* values for 68x48
#define T2_SITE_TO_DIT_X 1971
#define T2_SITE_TO_DIT_Y 1971
#define T2_SITE_TO_DIT_W (8*256)
#define T2_SITE_TO_DIT_H (8*256)
*/
#define T2_DISPLAY_RAW_PIXEL_WIDTH 480
#define T2_DISPLAY_RAW_PIXEL_HEIGHT 320
#define T2_DISPLAY_PIXEL_WIDTH 462  /* FROM wconfig.pl/txt */
#define T2_DISPLAY_PIXEL_HEIGHT 308  /* FROM wconfig.pl/txt */
#define T2_DISPLAY_DIT_PER_PIX 256   /* FROM Drawing.h */

#define T2_SITE_TO_DIT_X ((T2_DISPLAY_PIXEL_WIDTH*T2_DISPLAY_DIT_PER_PIX)/T2TILE_OWNED_WIDTH)
#define T2_SITE_TO_DIT_Y ((T2_DISPLAY_PIXEL_HEIGHT*T2_DISPLAY_DIT_PER_PIX)/T2TILE_OWNED_HEIGHT)
#define T2_SITE_DRAW_WIDTH_PIXELS (T2_DISPLAY_RAW_PIXEL_WIDTH/T2TILE_OWNED_WIDTH)
#define T2_SITE_DRAW_HEIGHT_PIXELS (T2_DISPLAY_RAW_PIXEL_HEIGHT/T2TILE_OWNED_HEIGHT)
#define T2_SITE_TO_DIT_W (T2_SITE_DRAW_WIDTH_PIXELS*T2_DISPLAY_DIT_PER_PIX)
#define T2_SITE_TO_DIT_H (T2_SITE_DRAW_HEIGHT_PIXELS*T2_DISPLAY_DIT_PER_PIX)

#define T2_SITE_IS_CACHE(u32x,u32y)             \
  (((u32x)<CACHE_LINES)                         \
   || ((u32y)<CACHE_LINES)                      \
   || ((u32x)>=T2TILE_WIDTH-CACHE_LINES)        \
   || ((u32y)>=T2TILE_HEIGHT-CACHE_LINES))      \

#define T2_SITE_IS_VISIBLE_OR_CACHE(u32x,u32y)  \
  (((u32x)<2*CACHE_LINES)                       \
   || ((u32y)<2*CACHE_LINES)                    \
   || ((u32x)>=T2TILE_WIDTH-2*CACHE_LINES)      \
   || ((u32y)>=T2TILE_HEIGHT-2*CACHE_LINES))    \

#define T2_SITE_IS_VISIBLE(u32x,u32y)           \
  (T2_SITE_IS_VISIBLE_OR_CACHE(u32x,u32y) &&    \
   !T2_SITE_IS_CACHE(u32x,u32y))                \
  

//////MISC DEFINES
#define CIRCUITS_PER_ACTIVE_EW 2


/////////FAKE PHYSICS
#define T2_PHONY_DREG_TYPE 1
#define T2_PHONY_RES_TYPE  2


//////////////////////////ALSO MACROS///////////////////////

#include <stdio.h>   // For printf and friends
#include <string.h>  // For strerror
#include <stdarg.h>  // For va_args
#include <stdlib.h>  // For abort

#include "Logger.h"  // For Level
#include "LineCountingByteSource.h"  // For LineCountingByteSource

#define xstr(s) str(s)
#define str(s) #s

#define DIE(MSG) fatal("%s:%d: (%s) XXX " #MSG "\n",__FILE__,__LINE__,__FUNCTION__)
#define DIE_UNIMPLEMENTED() DIE(IMPLEMENT ME)

namespace MFM {
  void vlog(Logger::Level level, const char * fmt, va_list ap) ;
  void message(const char * fmt, ...) ;
  void debug(const char * fmt, ...) ;
  void warn(const char * fmt, ...) ;
  void error(const char * fmt, ...) ;
  [[noreturn]] void fatal(const char * fmt, ...) ;

  void vout(LineCountingByteSource& lcbs,
            Logger::Level level,
            const char * fmt,
            va_list ap) ;
  void message(LineCountingByteSource& lcbs, const char * fmt, ...) ;
  void debug(LineCountingByteSource& lcbs, const char * fmt, ...) ;
  void warn(LineCountingByteSource& lcbs, const char * fmt, ...) ;
  void error(LineCountingByteSource& lcbs, const char * fmt, ...) ;
  [[noreturn]] void fatal(LineCountingByteSource& lcbs, const char * fmt, ...) ;
}

namespace MFM {

#define ALL_RESOURCE_TYPES()  \
    XX(GRID_VOLTAGE)YY        \
    XX(CORE_TEMPERATURE)YY    \
    XX(EDGE_TEMPERATURE)YY    \
    XX(AMBIENT_LIGHT)YY       \
    XX(DISK_SPACE)ZZ          \
    
    enum ResourceType {
#define XX(a) RT_##a
#define YY ,
#define ZZ
      ALL_RESOURCE_TYPES()                       
#undef XX
#undef YY
#undef ZZ
    };

#define ALL_RESOURCE_LEVELS()  \
    XX(VERY_LOW)YY             \
    XX(LOW)YY                  \
    XX(ACCEPTABLE)YY           \
    XX(HIGH)YY                 \
    XX(VERY_HIGH)ZZ            \

    enum ResourceLevel {
#define XX(a) RL_##a
#define YY ,
#define ZZ
      ALL_RESOURCE_LEVELS()                       
#undef XX
#undef YY
#undef ZZ
    };

  const char * resourceTypeName(ResourceType rt) ;

  const char * resourceLevelName(ResourceLevel rl) ;
}

#endif /*T2CONSTANTS_H*/
