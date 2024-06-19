/*                                              -*- mode:C++ -*-
  GlobalHooks.h Institutionalized disgusting rat holes
  Copyright (C) 2024 Living Computation Foundation

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
  \file GlobalHooks.h Institutionalized disgusting rat holes
  \author David H. Ackley.
  \date (C) 2024 All rights reserved.
  \lgpl
 */
#ifndef GLOBALHOOKS_H
#define GLOBALHOOKS_H

#include "itype.h"
#include "Fail.h"
#include <cstring> /* for strcmp */

namespace MFM
{
  /** A single disgusting global hook */
  struct GlobalHook {
    GlobalHook(const char * name)
      : m_name(name)
      , m_next(0)
    {
      MFM_API_ASSERT_NONNULL(m_name);
    }
    const char * m_name;
    virtual void * hookHandler(void * arg) = 0;
    struct GlobalHook * m_next;
  };

  /** A singleton list of disgusting global hooks */
  class GlobalHooks {
  public:
    static GlobalHooks& getSingleton();
    void addHook(GlobalHook & hook) {
      MFM_API_ASSERT_NONNULL(hook.m_name);
      MFM_API_ASSERT_NULL(hook.m_next);
      hook.m_next = m_head;
      m_head = &hook;
    }
    
    void * runHook(const char * name, void * arg) {
      MFM_API_ASSERT_NONNULL(name);
      for (GlobalHook * p = m_head; p; p = p->m_next) 
        if (!strcmp(name,p->m_name)) return p->hookHandler(arg);
      return 0;
    }
  private:
    GlobalHooks() : m_head(0) { }
    GlobalHook * m_head;
  };
}

#endif /* GLOBALHOOKS_H */
