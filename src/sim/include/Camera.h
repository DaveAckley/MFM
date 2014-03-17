#ifndef CAMERA_H     /* -*- C++ -*- */
#define CAMERA_H

#include "itype.h"
#include <SDL/SDL.h>

namespace MFM
{

  /**
   * A class which represents a layer that a SDL_Surface* must go
   * through in order to be drawn to the screen. This allows a user to
   * intercept the image as it is being drawn in order to capture the
   * images in the form of a PNG sequence to make primitive videos.
   *
   * At the moment, this only supports writing 10 million frames. This
   * is a whole lot, but keep this in mind if wanting to make a really
   * long video.
   */
  class Camera
  {
  private:

    static const u32 VIDEO_NAME_MAX_LENGTH = 64;

    bool m_recording;

    u32 m_currentFrame;

    char m_current_vid_dir[VIDEO_NAME_MAX_LENGTH];

    u32 GetPNGColorType(SDL_Surface* sfc);

    u32 SavePNG(char* filename, SDL_Surface* sfc);

  public:

    Camera();

    void ToggleRecord();

    bool IsRecording();

    void SetRecording(bool recording);

    void DrawSurface(SDL_Surface* sfc);
  };
}

#endif /* CAMERA_H */
