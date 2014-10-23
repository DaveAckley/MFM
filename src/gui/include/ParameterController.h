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
#include "Parameter.h"

namespace MFM
{
  /**
   * An abstraction used for displaying every Element and Atom
   * Parameter .
   */
  template <class CC>
  class ParameterController : public Panel
  {
   protected:
    typedef typename CC::ATOM_TYPE T;
    Parameter<CC> * m_parameter;
    T * m_patom;
    bool m_bigText;

   public:

    ParameterController() :
      m_parameter(0),
      m_bigText(false)
    { }

    void SetBigText(bool value)
    {
      m_bigText = value;
    }

    virtual ~ParameterController()
    { }

    virtual void SetParameter(ElementParameter<CC>* pb)
    {
      SetParameter(pb, &pb->GetAtom());
    }

    void SetParameter(Parameter<CC>* pb, T * patom)
    {
      if (!pb && !patom)
      {
        m_parameter = 0;
        m_patom = 0;
        return;
      }

      if (!pb || !patom)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      m_parameter = pb;
      m_patom = patom;
      Init();
    }

    virtual void Init() = 0;

    Parameter<CC> * GetParameter()
    {
      return m_parameter;
    }

    const Parameter<CC> * GetParameter() const
    {
      return m_parameter;
    }
  };
}

#endif /* PARAMETERCONTROLLER_H */
