/*                                              -*- mode:C++ -*-
  V6PanelPool.h An array of V6Panels
  Copyright (C) 2023 Living Computation Foundation

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
  \file V6PanelPool.h An array of V6Panels
  \author David H. Ackley.
  \date (C) 2023 All rights reserved.
  \lgpl
 */
#ifndef V6PANELPOOL_H
#define V6PANELPOOL_H

#include "Panel.h"
#include "V6Panel.h"

namespace MFM
{
  template<unsigned COUNT>
  class V6PanelPool
  {
   private:
    V6Panel m_v6panels[COUNT];
    Panel * m_parent;
    bool m_isVisible;           /** are we being drawn? */
    
    void UpdateVisibility()
    {
      u32 alloccount = 0;
      for (u32 i = 0; i < COUNT; ++i)
      {
        V6Panel & v6p = m_v6panels[i];
        bool alloc = v6p.m_poolIndex >= 0;
        v6p.SetVisible(false); /* assume all invisible */
        if (alloc) {
          ++alloccount;
          if (m_isVisible) v6p.SetVisible(true); /* alloc are visible */
        }
      }
      if (alloccount == 0) {
        // wrong place to do almost surely but
        AllocateV6Panel();
      }
    }

   public:
    void SetVisible(bool vis)
    {
      m_isVisible = vis;
      UpdateVisibility();
    }

    bool IsVisible() const
    {
      return m_isVisible;
    }

    V6Panel * AllocateV6Panel()
    {
      for (unsigned i = 0; i < COUNT; ++i)
      {
        V6Panel & v6p = m_v6panels[i];
        if (v6p.m_poolIndex < 0)
        {
          v6p.m_poolIndex = i;
          return &v6p;
        }
      }
      return NULL;
    }

    bool FreeV6Panel(V6Panel& av6p)
    {
      if (av6p.m_poolIndex >= 0) 
      {
        for (unsigned i = 0; i < COUNT; ++i)
        {
          V6Panel & v6p = m_v6panels[i];
          if (&v6p == &av6p)
          {
            v6p.m_poolIndex = -1;
            return true;
          }
        }
      }
      return false;
    }
    
    V6Panel & GetAllocatedV6Panel(unsigned index)
    {
      MFM_API_ASSERT_ARG(index < COUNT);
      V6Panel & v6p = m_v6panels[index];
      MFM_API_ASSERT_STATE(v6p.m_poolIndex >= 0);
      return v6p;
    }

    V6PanelPool(Panel * parent)
      : m_parent(parent)
    {
      MFM_API_ASSERT_NONNULL(parent);
    }

    void Init()
    {
      for (unsigned i = 0; i < COUNT; ++i)
      {
        V6Panel & v6p = m_v6panels[i];
        v6p.m_poolIndex = -1;
        v6p.Init(m_parent->GetName(), i);
        m_parent->Insert(&v6p, 0);
      }
    }

  };
}

#endif /* V6PANELPOOL_H */
