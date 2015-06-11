#include "Keyboard.h"
#include "Fail.h"

namespace MFM {

  Keyboard::Keyboard() {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
      m_keyBindings[i].Init();
  }

  Keyboard::KeyEntry & Keyboard::FindFreeEntry()
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      if (!m_keyBindings[i].m_inUse)
        return m_keyBindings[i];
    }
    FAIL(OUT_OF_ROOM);
  }

  void Keyboard::RegisterKey(u32 keysym, u32 mods, KeyHandlerFunction fptr, void * functionArg, KeyTrackerPointer tptr)
  {
    mods = MergeMods(mods);
    KeyEntry * kep = FindMatchingEntry(keysym, mods);
    if (kep)
    {
      if (kep->m_functionPtr != fptr ||
          kep->m_functionArg != functionArg ||
          kep->m_trackerPtr != tptr)
        FAIL(ILLEGAL_STATE);
      return;
    }
    KeyEntry & ke = FindFreeEntry();
    ke.m_keysym = keysym;
    ke.m_mods = mods;
    ke.m_functionPtr = fptr;
    ke.m_functionArg = functionArg;
    ke.m_trackerPtr = tptr;
    ke.m_inUse = true;
  }

  bool Keyboard::UnregisterKey(u32 keysym, u32 mods)
  {
    mods = MergeMods(mods);
    KeyEntry * kep = FindMatchingEntry(keysym, mods);
    if (kep)
    {
      kep->Init();
      return true;
    }
    return false;
  }

  Keyboard::KeyEntry * Keyboard::FindMatchingEntry(u32 keysym, u32 mods)
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (!ke.m_inUse) continue;
      if (ke.m_keysym == keysym && ke.m_mods == mods)
        return &ke;
    }
    return 0;
  }

  u32 Keyboard::MergeMods(u32 mods)
  {
    if (mods & KMOD_CTRL) mods |= KMOD_CTRL;
    if (mods & KMOD_SHIFT) mods |= KMOD_SHIFT;
    if (mods & KMOD_ALT) mods |= KMOD_ALT;
    return mods;
  }

  bool Keyboard::Press(u32 keysym, u32 mods)
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (ke.m_inUse && ke.m_keysym == keysym && ke.m_mods == mods)
      {
        if (ke.m_trackerPtr) *ke.m_trackerPtr = true;
        if (ke.m_functionPtr) (*ke.m_functionPtr)(keysym, mods, ke.m_functionArg);
        return true;
      }
    }
    return false;
  }

  bool Keyboard::Release(u32 keysym)
  {
    bool anybodyCare = false;
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (ke.m_inUse && ke.m_keysym == keysym && ke.m_trackerPtr)
      {
        *ke.m_trackerPtr = false;
        anybodyCare = true;
      }
    }
    return anybodyCare;
  }

  bool Keyboard::HandleEvent(SDL_KeyboardEvent& e)
  {
    if(e.type == SDL_KEYUP)
    {
      return Release(e.keysym.sym);
    }
    else if(e.type == SDL_KEYDOWN)
    {
      return Press(e.keysym.sym, MergeMods(e.keysym.mod));
    }
    else
      FAIL(ILLEGAL_ARGUMENT);
  }
} /* namespace MFM */
