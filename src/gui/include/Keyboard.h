/*                                              -*- mode:C++ -*-
  Keyboard.h Keyboard function dispatching system
  Copyright (C) 2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file Keyboard.h Keyboard function dispatching system
  \author Dave Ackley
  \date (C) 2015 All rights reserved.
  \lgpl
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <set>
#include "SDL.h"
#include "itype.h"
#include "Fail.h"

namespace MFM {

  typedef bool (*KeyHandlerFunction)(u32 keysym, u32 mods, void * arg, bool isPress);

  class Keyboard
  {
  public:
    static const u32 MAX_BINDINGS = 100;

    struct KeyEntry {
      void Init() {
        m_keysym = 0;
        m_mods = 0;
        m_functionPtr = 0;
        m_functionArg = 0;
        m_onPress = false;
        m_onRelease = false;
        m_inUse = false;
      }
      KeyHandlerFunction m_functionPtr;
      void * m_functionArg;
      bool * m_trackerPtr;
      u32 m_keysym;
      u32 m_mods;
      bool m_inUse;
      bool m_onPress;
      bool m_onRelease;
    };

    void RegisterKey(u32 keysym, u32 mods, KeyHandlerFunction fptr, void * functionArg, bool onPress, bool onRelease) ;

    void RegisterKeyFunction(u32 keysym, u32 mods, KeyHandlerFunction fptr, void * functionArg)
    {
      RegisterKey(keysym, mods, fptr, functionArg, true, false);
    }

    void RegisterKeyTracker(u32 keysym, u32 mods, KeyHandlerFunction fptr, void * functionArg)
    {
      RegisterKey(keysym, mods, fptr, functionArg, true, true);
    }

    bool UnregisterKey(u32 keysym, u32 mods) ;

    Keyboard() ;

    KeyEntry & FindFreeEntry() ;

    KeyEntry * FindMatchingEntry(u32 keysym, u32 mods) ;

    ~Keyboard() { }

    bool HandleEvent(SDL_KeyboardEvent& e) ;

  private:

    bool Release(u32 keysym, u32 mods) ;

    bool Press(u32 keysym, u32 mods) ;

    static u32 MergeMods(u32 mods) ;

    KeyEntry m_keyBindings[MAX_BINDINGS];

  public:

    template <class KEYBOARD, class KEYENTRY>
    class KeyboardAcceleratorIterator
    {
      KEYBOARD & kbd;
      u32 idx;
      bool toValid()
      {
        while (idx < MAX_BINDINGS)
        {
          KEYENTRY & ke = kbd.m_keyBindings[idx];
          if (ke.m_inUse) return true;
          ++idx;
        }
        return false;
      }
    public:
      KeyboardAcceleratorIterator(KEYBOARD& keyboard, u32 startIdx = 0)
        : kbd(keyboard)
        , idx(startIdx)
      {
        toValid();
      }
      bool operator!=(const KeyboardAcceleratorIterator & ak) const
      {
        return &kbd != &ak.kbd || idx != ak.idx;
      }
      void operator++()
      {
        if (idx < MAX_BINDINGS)
        {
          ++idx;
          toValid();
        }
      }

      s32 operator-(const KeyboardAcceleratorIterator &ak) const
      {
        return (s32) idx - (s32) ak.idx;
      }

      KEYENTRY & operator*() const
      {
        if (idx >= Keyboard::MAX_BINDINGS)
          FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
        return kbd.m_keyBindings[idx];
      }

      KEYENTRY * operator->() const
      {
        return &(this->operator*());
      }
    };

    typedef KeyboardAcceleratorIterator<Keyboard, KeyEntry> iterator_type;
    typedef const KeyboardAcceleratorIterator<const Keyboard,const KeyEntry> const_iterator_type;

    iterator_type begin() { return iterator_type(*this); }

    const_iterator_type begin() const { return const_iterator_type(*this); }

    iterator_type end() { return iterator_type(*this, MAX_BINDINGS); }

    const_iterator_type end() const { return const_iterator_type(*this, MAX_BINDINGS); }

  };
} /* namespace MFM */

#endif /*KEYBOARD_H*/
