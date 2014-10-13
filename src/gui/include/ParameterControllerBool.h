/*                                              -*- mode:C++ -*-
  ParameterControllerBool.h GUI panel for tweaking Parameter::Bools
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
  \file ParameterControllerBool.h GUI ParameterControllerBool for tweaking Parameter::Bools
  \author David H. Ackley.
  \author Trent R. Small
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef PARAMETERCONTROLLERBOOL_H
#define PARAMETERCONTROLLERBOOL_H

#include "AssetManager.h"
#include "CharBufferByteSink.h"
#include "Drawing.h"
#include "Util.h"
#include "Parameter.h"
#include "ParameterController.h"
#include "AbstractCheckbox.h"

namespace MFM
{
  template <class CC>
  class ParameterControllerBool : public ParameterController<CC>
  {
   private:
    typedef typename CC::ATOM_TYPE T;
    typedef ParameterController<CC> Super;

    enum
    {
      PARAMETERCONTROLLERBOOL_WIDTH = 300,
      PARAMETERCONTROLLERBOOL_HEIGHT = 40,

      PARAMETERCONTROLLERBOOL_WIDTH_BIG = 400,
      PARAMETERCONTROLLERBOOL_HEIGHT_BIG = 50
    };

    inline u32 GetScaledWidth()
    {
      return Super::m_bigText ?
      PARAMETERCONTROLLERBOOL_WIDTH_BIG :
      PARAMETERCONTROLLERBOOL_WIDTH;
    }

    inline u32 GetScaledHeight()
    {
      return Super::m_bigText ?
      PARAMETERCONTROLLERBOOL_HEIGHT_BIG :
      PARAMETERCONTROLLERBOOL_HEIGHT;
    }

    class ParmCheckbox : public AbstractCheckbox
    {
     private:
      typedef AbstractCheckbox Super;
      typedef ParameterControllerBool PCB;

      ParameterControllerBool & m_bc;
      bool m_bigText;

      inline u32 GetScaledWidth()
      {
        return m_bigText ?
        PARAMETERCONTROLLERBOOL_WIDTH_BIG :
        PARAMETERCONTROLLERBOOL_WIDTH;
      }

      inline u32 GetScaledHeight()
      {
        return m_bigText ?
        PARAMETERCONTROLLERBOOL_HEIGHT_BIG :
        PARAMETERCONTROLLERBOOL_HEIGHT;
      }

     public:
      ParmCheckbox(ParameterControllerBool & bc) : m_bc(bc)
      { }

      virtual bool IsChecked() const
      {
        return m_bc.GetParameterValue();
      }

      virtual void SetChecked(bool value)
      {
        m_bc.SetParameterValue(value);
      }

      virtual void OnCheck(bool value)
      {
        SetChecked(value);
      }

      void Init(bool bigText = false)
      {
        m_bigText = bigText;
        this->SetName("ParmCheckBox");
        this->SetDimensions(SPoint(GetScaledWidth(),
                                   GetScaledHeight()));
        this->SetDesiredSize(U32_MAX, GetScaledHeight());
        this->SetRenderPoint(SPoint(5, 5));
        if (m_bc.GetParameter())
        {
          this->SetText(m_bc.GetParameter()->GetName());
        }
      }
    };

    ParmCheckbox m_checkbox;

  public:

    ParameterControllerBool() :
      m_checkbox(*this)
    {
      this->SetName("ParmCtrlBool");
      Panel::Insert(&m_checkbox, NULL);  // This is once-only
      Init();
    }

    void SetParameterValue(bool value)
    {
      if (!this->m_parameter || !this->m_patom)
      {
        FAIL(ILLEGAL_STATE);
      }
      this->m_parameter->SetBitsAsS32(*this->m_patom, value? -1 : 0);
    }

    bool GetParameterValue() const
    {
      if (!this->m_parameter || !this->m_patom)
      {
        FAIL(ILLEGAL_STATE);
      }
      bool ret;
      if (!this->m_parameter->LoadBool(*this->m_patom, ret))
      {
        FAIL(ILLEGAL_STATE);
      }
      return ret;
    }

    void Init()
    {
      Panel::SetDimensions(GetScaledWidth() * 2, GetScaledHeight());
      Panel::SetDesiredSize(10000, GetScaledHeight());
      Panel::SetBackground(Drawing::GREY60);

      m_checkbox.Init(Super::m_bigText);

      LOG.Debug("Innitting %s", Super::m_bigText ? "big" : "normal");
    }
  };
}

#endif /* PARAMETERCONTROLLERBOOL_H */
