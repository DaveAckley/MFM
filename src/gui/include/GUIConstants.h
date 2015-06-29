/*                                              -*- mode:C++ -*-
  GUIConstants.h A place for constant and initial values
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file GUIConstants.h A place for constant and initial values
  \author Trent R. Small.
  \author Dave Ackley
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef GUICONSTANTS_H
#define GUICONSTANTS_H

namespace MFM
{

  enum GUIConstant {
    SCREEN_INITIAL_WIDTH = 1280,
    SCREEN_INITIAL_HEIGHT = 1024,

    FRAMES_PER_SECOND_IDLE = 15,  //< Redisplay rate when simulation paused

    CAMERA_SLOW_SPEED = 2,
    CAMERA_FAST_SPEED = 50,

    STATS_WINDOW_WIDTH = 320,

    STATS_START_WINDOW_WIDTH = STATS_WINDOW_WIDTH,
    STATS_START_WINDOW_HEIGHT = 120,

    MINIMAL_START_WINDOW_WIDTH = 1,
    MINIMAL_START_WINDOW_HEIGHT = 1,

    HELP_PANEL_COLUMNS = 100,
    HELP_PANEL_ROWS = 100,

    // Unrefactored magic out of ToolboxPanel.h
    MAX_GRIDTOOL_BUTTONS = 9,
    TOOLBOX_MAX_CONTROLLERS = 20,
    TOOLBOX_MAX_SLIDERS = 8,
    TOOLBOX_MAX_CHECKBOXES = 8,
    TOOLBOX_MAX_NEIGHBORHOODS = 8,
    ELEMENT_RENDER_SIZE = 32,
    ELEMENT_BIG_RENDER_SIZE = 48,
    ELEMENT_BOX_SIZE = 77,

    ELEMENTS_PER_ROW = 12,
    ELEMENT_ROWS = (ELEMENT_BOX_SIZE + ELEMENTS_PER_ROW - 1) / ELEMENTS_PER_ROW,
    NON_ELEMENT_ROWS = 3,
    TOTAL_ROWS = ELEMENT_ROWS + NON_ELEMENT_ROWS,
    BORDER_PADDING = 2,
    TOOLBOX_WIDTH = 6 + ELEMENT_RENDER_SIZE * ELEMENTS_PER_ROW + BORDER_PADDING,


    _LAST_GUI_CONSTANT_ = 0
  };

} /* namespace MFM */
#endif /*GUICONSTANTS_H*/
