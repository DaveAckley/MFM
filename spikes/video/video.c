#include <stdlib.h>
#include <unistd.h>
#include <png.h>
#include "SDL/SDL.h"

void fail(const char* msg, int code)
{
  fprintf(stderr, "%s\n", msg);
  exit(code);
}

void WriteSurface(SDL_Surface* sfc, const char* filename)
{
  FILE* fp = fopen(filename, "wb");

  if(!fp)
  {
    fail("[WriteSurface] Could not open file for writing.", 2);
  }

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png)
  {
    fail("[WriteSurface] Could not initialize png struct!", 3);
  }

  png_infop png_info = png_create_info_struct(png);
  if(!png_info)
  {
    fail("[WriteSurface] Could not initialize info struct!", 4);
  }

  if(setjmp(png_jmpbuf(png)))
  {
    fail("[WriteSurface] Could not set jump for jumpbuffer.", 5);
  }
  
  png_init_io(png, fp);
  
  if(setjmp(png_jmpbuf(png)))
  {
    fail("[WriteSurface] Could not set jump for jumpbuffer.", 6);
  }
  
  png_set_IHDR(png, png_info, sfc->w, sfc->h, sfc->format->BitsPerPixel,
	       PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png, png_info);

  if(setjmp(png_jmpbuf(png)))
  {
    fail("[WriteSurface] Could not set jump for jumpbuffer.", 7);
  }

  png_bytep* imgData = (png_bytep*)malloc(sizeof(png_bytep) * sfc->h);

  for(int y = 0; y < sfc->h; y++)
  {
    imgData[y] = (png_bytep)malloc(sfc->format->BytesPerPixel * sfc->w);

    for(int x = 0; x < sfc->w; x++)
    {	
      unsigned int pixel = ((unsigned int*)sfc->pixels)[y * sfc->w + x];
      for(int i = 0; i < sfc->format->BytesPerPixel; i++)
      {
	imgData[y][x + i] = (pixel >> (i * 8)) & 0xff;
      }
    }
  }
  
}

int main(int argc, char** argv)
{
  
  return 0;
}
