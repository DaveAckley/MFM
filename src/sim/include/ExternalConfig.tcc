/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "AtomSerializer.h"
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 150
#define MAX_ARG_LEN 64
#define MAX_ARGUMENTS 16

namespace MFM
{
  template<class GC>
  ExternalConfig<GC>::ExternalConfig(Grid<GC>& grid) :
    m_grid(grid), m_elementRegistry(grid.GetElementRegistry()),
    m_registeredFunctionCount(0), m_registeredElementCount(0)
  {
    m_in.SetErrorByteSink(STDERR);
  }

  template<class GC>
  void ExternalConfig<GC>::SetByteSource(ByteSource & byteSource, const char * label)
  {
    m_in.SetByteSource(byteSource);
    m_in.SetLabel(label);
  }

  template<class GC>
  void ExternalConfig<GC>::SetErrorByteSink(ByteSink & errorsTo)
  {
    m_in.SetErrorByteSink(errorsTo);
  }

  template<class GC>
  bool ExternalConfig<GC>::Read()
  {
    m_grid.Clear();

    while (true) {

      m_in.SkipWhitespace();
      s32 ch = m_in.Read();

      if (ch < 0) {
        if (ch == EOF) break;
        return m_in.Msg(Logger::ERROR, "I/O error (%d)", -ch);
      }

      if (ch == '#') {
        // Line comment.  Skip to newline and try again
        m_in.SkipSet("[^\n]");
        continue;
      }

      m_in.Unread();
      OString64 cbbs;
      if (1 != m_in.Scanf("%[_a-zA-Z0-9]",&cbbs))
        return m_in.Msg(Logger::ERROR, "Expected function name");
      if (cbbs.HasOverflowed())
        return m_in.Msg(Logger::ERROR, "Function name too long '%@'", &cbbs);

      m_in.SkipWhitespace();
      if (1 != m_in.Scanf("("))
        return m_in.Msg(Logger::ERROR, "Expected open parenthesis");

      bool handled = false;
      for (u32 i = 0; i < m_registeredFunctionCount; ++i) {
        ConfigFunctionCall<GC> & fc = *m_registeredFunctions[i];
        if (cbbs.Equals(fc.m_functionName)) {
          if (!fc.Parse(*this)) return false;  // Error message already issued
          handled = true;
          break;
        }
      }

      if (!handled)
        return m_in.Msg(Logger::ERROR, "Unknown function '%s'",cbbs.GetZString());
    }
    return true;
  }

  template<class GC>
  void ExternalConfig<GC>::Write(ByteSink& byteSink)
  {
    /* First, register all elements. */

    u32 elems = m_elementRegistry.GetEntryCount();
    char alphaOutput[24];

    for(u32 i = 0; i < elems; i++)
    {
      const UUID& uuid = m_elementRegistry.GetEntryUUID(i);

      IntAlphaEncode(i, alphaOutput);

      byteSink.Printf("RegisterElement(");
      uuid.Print(byteSink);
      byteSink.Printf(",%s)", alphaOutput);
      byteSink.WriteNewline();

      /* Write configurable element values */

      const Element<CC>* elem = m_elementRegistry.GetEntryElement(i);
      const ElementParameters<CC> & parms = elem->GetElementParameters();

      for(u32 j = 0; j < parms.GetParameterCount(); j++)
      {
        const ElementParameter<CC> * p = parms.GetParameter(j);
        byteSink.Printf(" SetElementParameter(%s,%s,%@)",
                        alphaOutput,
                        p->GetTag(),
                        p);
        byteSink.WriteNewline();
      }
    }
    byteSink.WriteNewline();

    /* Then, GA all live atoms. */

    /* The grid size in sites excluding caches */
    const u32 gridWidth = CC::PARAM_CONFIG::TILE_WIDTH *
      GC::GRID_WIDTH -
      CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS *
      GC::GRID_WIDTH * 2;

    const u32 gridHeight = CC::PARAM_CONFIG::TILE_WIDTH *
      GC::GRID_HEIGHT -
      CC::PARAM_CONFIG::EVENT_WINDOW_RADIUS *
      GC::GRID_HEIGHT * 2;

    for(u32 y = 0; y < gridHeight; y++)
    {
      for(u32 x = 0; x < gridWidth; x++)
      {
        SPoint currentPt(x, y);
        /* No need to write empties since they are the default */
        if(!Atom<CC>::IsType(*m_grid.GetAtom(currentPt),
                             Element_Empty<CC>::THE_INSTANCE.GetType()))
        {
          byteSink.Printf("GA(");

          /* This wil be a little slow, but meh. Makes me miss hash
           * tables. */
          for(u32 i = 0; i < elems; i++)
          {
            if(Atom<CC>::IsType(*m_grid.GetAtom(currentPt),
                                m_elementRegistry.GetEntryElement(i)->GetType()))
            {
              IntAlphaEncode(i, alphaOutput);
              byteSink.Printf("%s", alphaOutput);
              break;
            }
          }

          T temp = *m_grid.GetAtom(currentPt);
          AtomSerializer<CC> as(temp);
          byteSink.Printf(",%d,%d,%@)\n", x, y, &as);
        }
      }
    }
    byteSink.WriteNewline();

    /* Set Tile geometry */
    for(u32 y = 0; y < GC::GRID_HEIGHT; y++)
    {
      for(u32 x = 0; x < GC::GRID_WIDTH; x++)
      {
        SPoint currentPt(x, y);
        Tile<CC> & tile = m_grid.GetTile(currentPt);

        if(tile.GetCurrentState() != Tile<CC>::ACTIVE)
        {
          byteSink.Printf("DisableTile(%d,%d)", x, y);
          byteSink.WriteNewline();
        }
      }
    }

    /* Set any additional parameters */
  }

  template<class GC>
  void ExternalConfig<GC>::RegisterFunction(ConfigFunctionCall<GC> & fc)
  {
    for (u32 i = 0; i < m_registeredFunctionCount; ++i) {
      ConfigFunctionCall<GC> & rfc = *m_registeredFunctions[i];
      if (!strcmp(fc.m_functionName, rfc.m_functionName))
        FAIL(DUPLICATE_ENTRY);
    }
    if (m_registeredFunctionCount >= sizeof(m_registeredFunctions)/sizeof(m_registeredFunctions[0]))
      FAIL(OUT_OF_ROOM);

    m_registeredFunctions[m_registeredFunctionCount++] = &fc;
  }

  template<class GC>
  bool ExternalConfig<GC>::RegisterElement(const UUID & uuid, OString16 & nick)
  {
    Element<CC> * elt = m_elementRegistry.Lookup(uuid);
    const UUID * puuid = 0;
    if (!elt) {
      m_in.Msg(Logger::WARNING, "Unknown element '%@', searching for compatible alternatives", &uuid);

      elt =  m_elementRegistry.LookupCompatible(uuid);
      if (!elt)
        return m_in.Msg(Logger::WARNING, "No alternatives found for unknown/unregistered element '%@'", &uuid);

      puuid = &elt->GetUUID();
      m_in.Msg(Logger::WARNING, "Substituting '%@' for '%@'", puuid, &uuid);
    }
    else
    {
      puuid = &elt->GetUUID();
    }

    if (!puuid)
      FAIL(NULL_POINTER);

    for (u32 i = 0; i < m_registeredElementCount; ++i) {
      RegElt & re = m_registeredElements[i];
      if (puuid->Equals(re.m_uuid))
        return m_in.Msg(Logger::ERROR,"Element '%@' already registered as '%s'", &re.m_uuid, re.m_nick.GetZString());
      if (re.m_nick.Equals(nick))
        return m_in.Msg(Logger::ERROR,"Element nicknamed '%s' already registered for UUID '%@'",
                        nick.GetZString(), &re.m_uuid);
    }
    if (m_registeredElementCount >= sizeof(m_registeredElements)/sizeof(m_registeredElements[0]))
      return m_in.Msg(Logger::ERROR,"No room for more registered Elements");

    m_registeredElements[m_registeredElementCount].m_uuid = *puuid;
    m_registeredElements[m_registeredElementCount].m_nick = nick;
    m_registeredElements[m_registeredElementCount].m_element = elt;
    ++m_registeredElementCount;

    m_in.Msg(Logger::MESSAGE,"Registration %d: Nickname '%s' -> UUID '%@'", m_registeredElementCount, nick.GetZString(), puuid);

    return true;
  }

  template<class GC>
  Element<typename GC::CORE_CONFIG> * ExternalConfig<GC>::LookupElement(const OString16 & nick) const
  {
    for (u32 i = 0; i < m_registeredElementCount; ++i) {
      const RegElt & re = m_registeredElements[i];
      if (re.m_nick.Equals(nick))
        return re.m_element;
    }
    return 0;
  }

  template<class GC>
  void ExternalConfig<GC>::SetTileToExecuteOnly(const SPoint& tileLoc, bool value)
  {
    m_grid.SetTileEnabled(tileLoc, !value);
  }

  template<class GC>
  bool ExternalConfig<GC>::PlaceAtom(const Element<CC> & elt, s32 x, s32 y, const char* hexData)
  {
    SPoint pt(x, y);
    T atom = elt.GetDefaultAtom();

    atom.ReadStateBits(hexData);
    m_grid.PlaceAtom(atom, pt);
    return true;
  }

  template<class GC>
  bool ExternalConfig<GC>::PlaceAtom(const Element<CC> & elt, s32 x, s32 y, const BitVector<BPA> & bv)
  {
    SPoint pt(x, y);
    T atom = elt.GetDefaultAtom();

    atom.ReadStateBits(bv);
    m_grid.PlaceAtom(atom, pt);
    return true;
  }
}
