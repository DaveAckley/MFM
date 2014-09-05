/*                                              -*- mode:C++ -*-
  EditingTool.h Enumeration of all editing tool types
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
  \file EditingTool.h Enumeration of all editing tool types
  \author Trent R. Small
  \date (C) 2014 All rights reserved.
  \lgpl
*/

#ifndef EDITINGTOOL_H
#define EDITINGTOOL_H

namespace MFM
{
  enum EditingTool
  {
    TOOL_SELECTOR,
    TOOL_ATOM_SELECTOR,
    TOOL_PENCIL,
    TOOL_BUCKET,
    TOOL_ERASER,
    TOOL_BRUSH,
    TOOL_XRAY,
    TOOL_CLONE,
    TOOL_AIRBRUSH
  };
}

#endif /* EDITINGTOOL_H */
