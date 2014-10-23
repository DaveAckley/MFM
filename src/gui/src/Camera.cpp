#include "Camera.h"

#include <stdlib.h>    /* for malloc, free */
#include <png.h>

/* libpng is ghetto and needs these */
static void libpng_warning(png_structp context, png_const_charp msg)
{
  fprintf(stderr, "[libpng] WARNING: %s\n", msg);
}
static void libpng_error(png_structp context, png_const_charp msg)
{
  fprintf(stderr, "[libpng] ERROR: %s\n", msg);
}

namespace MFM
{
  Camera::Camera()
  {
    m_recording = false;
  }

  void Camera::ToggleRecord()
  {
    SetRecording(!m_recording);
  }

  bool Camera::IsRecording()
  {
    return m_recording;
  }

  void Camera::SetRecording(bool recording)
  {
    m_recording = recording;
    if(recording)
    {
      m_currentFrame = 0;
    }
  }

  bool Camera::DrawSurface(SDL_Surface* sfc, const char * pngDirPath) const
  {
    // m_currentFrame++
    bool ret = SavePNG(pngDirPath, sfc);

    //    SDL_Flip(sfc);
    return ret;
  }

  // Currently unused..
  u32 Camera::GetPNGColorType(SDL_Surface* sfc)
  {
    u32 ctype = PNG_COLOR_MASK_COLOR;

    if(sfc->format->palette)
    {
      ctype |= PNG_COLOR_MASK_PALETTE;
    }
    else if(sfc->format->Amask)
    {
      ctype |= PNG_COLOR_MASK_ALPHA;
    }

    return ctype;
  }

  u32 Camera::SavePNG(const char* filename, SDL_Surface* sfc) const
  {
    FILE* fp = fopen(filename, "wb");
    if(fp == NULL)
    {
      fprintf(stderr, "[Camera::SavePNG] Can't open %s\n.", filename);
      return 1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
						  NULL, libpng_error, libpng_warning);
    if(!png_ptr)
    {
      fprintf(stderr, "[Camera::SavePNG] Can't create png_ptr .\n");
      fclose(fp);
      return 2;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      fprintf(stderr, "[Camera::SavePNG] Can't create info_ptr .\n");
      fclose(fp);
      return 3;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      /* Looks like libpng will tell us if this fails. */
      fclose(fp);
      return 4;
    }

    png_init_io(png_ptr, fp);

    //    u32 ctype = GetPNGColorType(sfc);
    u32 ctype = PNG_COLOR_TYPE_RGB_ALPHA;
    png_set_IHDR(png_ptr, info_ptr, sfc->w, sfc->h, 8, ctype, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
    png_set_bgr(png_ptr);
    png_set_packing(png_ptr);

    png_bytep* rows = (png_bytep*)malloc(sizeof(png_bytep) * sfc->h);

    for(s32 i = 0; i < sfc->h; i++)
    {
      rows[i] = (png_bytep)((u8*)sfc->pixels + i * sfc->pitch);
    }
    png_write_image(png_ptr, rows);
    png_write_end(png_ptr, info_ptr);

    free(rows);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
  }
}
