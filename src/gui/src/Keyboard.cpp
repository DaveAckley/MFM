#include "Keyboard.h"
#include "Fail.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {

  static void PrintCannedHelp(ByteSink & bs) ;

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

  void Keyboard::Register(KeyboardCommandFunction & kcf)
  {
    s32 section = kcf.GetSection();
    u32 order = FindNextInOrder(section);

    KeyEntry * kep = FindMatchingEntry(kcf);
    if (kep)
        FAIL(ILLEGAL_ARGUMENT);

    KeyEntry & ke = FindFreeEntry();
    ke.Init();

    u32 keysym, mods;
    if (kcf.GetKey(keysym, mods))
    {
      mods = MergeMods(mods);
      KeyEntry * kep = FindMatchingEntry(keysym, mods);
      if (kep != 0)
        FAIL(DUPLICATE_ENTRY);
      ke.m_accelerated = true;
      ke.m_keysym = keysym;
      ke.m_mods = mods;
    }

    ke.m_commandPtr = &kcf;
    ke.m_section = section;
    ke.m_order = order;
    ke.m_inUse = true;
  }

  bool Keyboard::Unregister(KeyboardCommandFunction & kcf)
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (!ke.m_inUse) continue;
      if (ke.m_commandPtr == &kcf)
      {
        ke.Init();
        return true;
      }
    }
    return false;
  }

  Keyboard::KeyEntry * Keyboard::FindMatchingEntry(KeyboardCommandFunction & kcf)
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (!ke.m_inUse) continue;
      if (ke.m_commandPtr == &kcf)
        return &ke;
    }
    return 0;
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

  u32 Keyboard::FindNextInOrder(s32 section)
  {
    u32 maxOrder = 0;
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (!ke.m_inUse) continue;
      if (ke.m_section == section && ke.m_order > maxOrder)
        maxOrder = ke.m_order;
    }
    if (maxOrder >= U8_MAX)
      FAIL(OUT_OF_ROOM);
    return maxOrder + 1;
  }

  u32 Keyboard::MergeMods(u32 mods)
  {
    // If either left or right mod, set both such mods
    if (mods & KMOD_CTRL) mods |= KMOD_CTRL;
    if (mods & KMOD_SHIFT) mods |= KMOD_SHIFT;
    if (mods & KMOD_ALT) mods |= KMOD_ALT;
    mods &= KMOD_CTRL|KMOD_SHIFT|KMOD_ALT; // Discard all other modifiers!
    return mods;
  }

  bool Keyboard::Press(u32 keysym, u32 mods)
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (ke.m_inUse && ke.m_accelerated && ke.m_keysym == keysym && ke.m_mods == mods)
      {
        return ke.m_commandPtr->ExecuteFunction(keysym, mods);
      }
    }
    return false;
  }

  bool Keyboard::Release(u32 keysym, u32 mods)
  {
#if 0
    bool anybodyCare = false;
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      KeyEntry & ke = m_keyBindings[i];
      if (ke.m_inUse && ke.m_keysym == keysym && ke.m_onRelease)
      {
        // Call ALL matching keysym functions (regardless of mods) on
        // release!  Mods may have changed while keysym was down!
        anybodyCare |= (*ke.m_functionPtr)(keysym, mods, ke.m_functionArg, false);
      }
    }
    return anybodyCare;
#endif
    return false;
  }

  bool Keyboard::HandleEvent(SDL_KeyboardEvent& e)
  {
    if(e.type == SDL_KEYUP)
    {
      return Release(e.keysym.sym, MergeMods(e.keysym.mod));
    }
    else if(e.type == SDL_KEYDOWN)
    {
      return Press(e.keysym.sym, MergeMods(e.keysym.mod));
    }
    else
      FAIL(ILLEGAL_ARGUMENT);
  }

  void Keyboard::PrintKeyName(ByteSink & bs, u32 keysym, u32 mods)
  {
    if( mods & KMOD_CTRL) bs.Printf("C-");
    if( mods & KMOD_ALT) bs.Printf("M-");
    if( mods & KMOD_SHIFT) bs.Printf("S-");
    bs.Printf("%s", SDL_GetKeyName((SDLKey) keysym));
  }

  const Keyboard::KeyEntry * Keyboard::FindSecOrdEntry(u32 section, u32 order) const
  {
    for (u32 i = 0; i < MAX_BINDINGS; ++i)
    {
      const KeyEntry & ke = m_keyBindings[i];
      if (!ke.m_inUse || ke.m_section != (s32) section || ke.m_order != order)
        continue;
      return &ke;
    }
    return 0;
  }

  void Keyboard::PrintHelp(ByteSink & bs) const
  {
    OString32 kbuff;
    u32 len;
    bs.Printf("Command summary\n");
    for (u32 sec = HELP_SECTION_WINDOWS; sec <= HELP_SECTION_MISC; ++sec)
    {
      bs.Printf("\n");
      kbuff.Reset();
      kbuff.Printf("%s", Keyboard::GetSectionName((FunctionSection) sec));
      len = kbuff.GetLength();
      bs.Printf("%s\n", kbuff.GetZString());
      for (u32 i = 0; i < len; ++i) bs.Printf("-");
      bs.Printf("\n");

      for (u32 order = 1; order < MAX_BINDINGS; ++order)
      {
        // Umm just between us this is non-trivially quadratic?  Such data structure.
        const Keyboard::KeyEntry * kep = FindSecOrdEntry(sec, order);
        if (!kep) continue;

        const Keyboard::KeyEntry & ke = *kep;

        if (!ke.m_accelerated) continue;  // Don't mention stuff without keyboard cmds!

        kbuff.Reset();
        ke.PrintKeyName(kbuff);
        const u32 KEY_COL_WIDTH = 14;
        u32 len = kbuff.GetLength();
        u32 lpad = 0, rpad = KEY_COL_WIDTH;

        if (len < KEY_COL_WIDTH)
        {
          lpad = (KEY_COL_WIDTH - len) / 2;
          rpad = KEY_COL_WIDTH - len - lpad;
        }
        else
        {
          rpad = 0;
        }

        for (u32 i = 0; i < lpad; ++i) bs.Printf(" ");
        bs.Printf("%s", kbuff.GetZString());
        for (u32 i = 0; i < rpad; ++i) bs.Printf(" ");

        const char * doc = ke.m_commandPtr->GetDoc();
        if (!doc) bs.Printf("(no doc found)");
        bs.Printf("%s", doc);

        bs.Printf("\n");
      }
    }

    PrintCannedHelp(bs);
  }

  const char * Keyboard::GetSectionName(const FunctionSection fs)
  {
    switch (fs)
    {
    default: FAIL(ILLEGAL_ARGUMENT);
    case HELP_SECTION_WINDOWS: return "Window management";
    case HELP_SECTION_RUNNING: return "Simulation control";
    case HELP_SECTION_DISPLAY: return "Displaying information";
    case HELP_SECTION_EDITING: return "Grid editing";
    case HELP_SECTION_MISC:    return "Other/Uncategorized";
    }
  }


  void PrintCannedHelp(ByteSink & bs)
  {
    const char * more =
      "\n"
      "Mouse commands\n"
      "----- --------\n"
      "\n"
      "In Grid:\n"
      "   Scroll     Adjust grid zoom \n"
      "    Left      Use tool primary (if toolbox open)\n"
      "    Right     Use tool secondary (if toolbox open)\n"
      "   C-Left     Move grid in window\n"
      "\n"
      "In Toolbox:\n"
      "   Scroll     Adjust tool size\n"
      "    Left      Select tool, shape, or primary element\n"
      "   Middle     Toggle low-lighting (element buttons)\n"
      "    Right     Select secondary element\n"
      "\n"
      "In most windows:\n"
      "   C-Left     Move window\n"
      "   S-Left     Resize window\n"
      "\n"
      "-------------------\n"
      ;
    bs.Printf("%s", more);
  }
} /* namespace MFM */
