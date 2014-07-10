/*                                              -*- mode:C++ -*-
  AbstractSliderConfig.h Mapping between Elements and Sliders
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
  \file AbstractSliderConfig.h Mapping between Elements and Sliders
  \author Trent R. Small
  \date (C) 2014 All rights reserved.
  \lgpl
*/
#ifndef ABSTRACTSLIDERCONFIG_H
#define ABSTRACTSLIDERCONFIG_H

namespace MFM
{
  template <class CC>
  class AbstractSliderConfig
  {
  private:

    const Element<CC>* m_element;

  public:

    const Element<CC>* GetElement() const
    {
      return m_element;
    }

    void SetElement(const Element<CC>* element)
    {
      m_element = element;
    }

    /**
     * Sets up the Sliders involved with the held element.
     *
     * @param Sliders A pointer to the Sliders which will be set up by
     *                this function.
     *
     * @param maxSliders The maximum number of Sliders that this
     *                   AbstractSliderConfig may set up.
     *
     * @returns The number of Sliders that this AbstractSliderConfig managed
     *          to set up.
     */
    virtual u32 SetupSliders(Slider* sliders, u32 maxSliders) const = 0;
  };
}

#endif /* ABSTRACTSLIDERCONFIG_H */
