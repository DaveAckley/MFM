/*                                              -*- mode:C++ -*-
  ParameterController.h A Panel that controls a Parameter
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
  \file ParameterController.h A Panel that controls a Parameter
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PARAMETERCONTROLLER_H
#define PARAMETERCONTROLLER_H

#include "Panel.h"
#include "Parameters.h"

namespace MFM
{
  class ParameterController : public Panel
  {
    Parameters::Parameter * m_parameter;

  public:

    ParameterController() :
      m_parameter(0)
    {
    }

    virtual ~ParameterController()
    {
    }

    void SetParameter(Parameters::Parameter * parm)
    {
      m_parameter = parm;
    }

    Parameters::Parameter * GetParameter()
    {
      return m_parameter;
    }

    const Parameters::Parameter * GetParameter() const
    {
      return m_parameter;
    }

  };
}

#endif /* PARAMETERCONTROLLER_H */
