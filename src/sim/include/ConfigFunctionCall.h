/*                                              -*- mode:C++ -*-
  ConfigFunctionCall.h Represent a function call that can appear in a config file
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
  \file ConfigFunctionCall.h Represent a function call that can appear in a config file
  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CONFIGFUNCTIONCALL_H
#define CONFIGFUNCTIONCALL_H

#include "LineCountingByteSource.h"

namespace MFM
{
  template<typename> class ExternalConfig;

  /**
   * An interface representing a function call that can appear in a
   * config file
   */
  template<class GC>
  class ConfigFunctionCall
  {
  public:
    const char * const m_functionName;

    ConfigFunctionCall(const char * name) :
      m_functionName(name), m_valid(false)
    {
    }

    bool IsValid()
    {
      return m_valid;
    }

    const char* GetFunctionName()
    {
      return m_functionName;
    }

    virtual bool Parse(ExternalConfig<GC> & ec) = 0;

    virtual void Print(ByteSink & in) = 0;

    virtual void Apply(ExternalConfig<GC> & ec) = 0;

    virtual ~ConfigFunctionCall() { }

  protected:
    bool m_valid;

    /**
     * Skip w/s, read ',' or ')'.  If ',', skip w/s and return 1.  If
     * ')', return 0.  If anything else, issue error message and
     * return -1.
     */
    s32 SkipToNextArg(LineCountingByteSource & in) ;

    /**
     * Skip w/s, read ','.  If ',', skip w/s and return true.
     * Otherwise, issue error message 'Missing argument, expected ' +
     * description and return false.
     */
    bool SkipToNextExistingArg(LineCountingByteSource & in, const char * description) ;

  };
}

#include "ExternalConfig.h"
#include "ConfigFunctionCall.tcc"

#endif /* CONFIGFUNCTIONCALL_H */
