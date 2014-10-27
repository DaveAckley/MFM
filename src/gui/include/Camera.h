/*                                              -*- mode:C++ -*-
  Camera.h Capture layer for an SDL_Surface*
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

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
  \file Camera.h Capture layer for an SDL_Surface*
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CAMERA_H
#define CAMERA_H

#include "itype.h"
#include "SDL.h"

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

    u32 SavePNG(const char* filename, SDL_Surface* sfc) const;

  public:

    Camera();

    void ToggleRecord();

    bool IsRecording();

    void SetRecording(bool recording);

    bool DrawSurface(SDL_Surface* sfc, const char * pngPath) const;
  };
}

#endif /* CAMERA_H */
