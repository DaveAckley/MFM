/*                                              -*- mode:C++ -*-
  ExternalConfig.h Support for grid configuratios on the drive
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
  \file ExternalConfig.h Support for grid configuratios on the drive
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIG_H
#define EXTERNALCONFIG_H

#include "Grid.h"

namespace MFM
{

  class FunctionCall; /* Forward Declaration, in ExternalConfig.tcc */

  /**
   * Structure for reading and writing the current grid configuration
   * backed by a filename.
   */
  template<class GC>
  class ExternalConfig
  {
  public:

    /**
     * Construct a new ExternalConfig referencing a specified Grid and
     * backed by a specified file.
     *
     * @param grid The grid to read from or write to.
     *
     * @param filename The name of the file to read from or write to.
     */
    ExternalConfig(Grid<GC>& grid, const char* filename);

    /**
     * Reads from the file at the filename specified at construction
     * and writes the configuration to the grid specified at
     * construction. FAILs if the configuration file is invalid or
     * non-existant.
     */
    void Read();

    /**
     * Writes to the file at the filename specified at construction
     * and reads the configuration to the grid specified at
     * construction.
     */
    void Write();

  private:
    /**
     * The Grid to read from or write to.
     */
    Grid<GC>& m_grid;

    /**
     * The name of the file to read from or write to.
     */
    const char* m_filename;

    void DeactivateTile(FunctionCall& fcall);

    void Dispatch(FunctionCall& fcall, u32 lineNumber);

    void ParseLine(const char* line, u32 lineNumber);
  };
}

#include "ExternalConfig.tcc"

#endif /* EXTERNALCONFIG_H */
