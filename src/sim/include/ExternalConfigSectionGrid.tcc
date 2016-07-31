/* -*- C++ -*- */
#include "ConfigFunctionCall.h"
#include "AtomSerializer.h"
#include <string.h>
#include <ctype.h>

namespace MFM
{
  template <class GC>
  FunctionCallDefineGridSize<GC>::FunctionCallDefineGridSize(ExternalConfigSectionGrid<GC> & ec)
    : ConfigFunctionCallGrid<GC>("DefineGridSize", ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallDefineGridSize<GC>::Parse()
  {
    ExternalConfigSectionGrid<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    in.SkipWhitespace();
    const u32 PARM_COUNT = 4;
    u32 parms[PARM_COUNT];
    for (u32 i = 0; i < PARM_COUNT; ++i) {
      if (!in.Scan(parms[i]))
        return in.Msg(Logger::ERROR, "Error reading DefineGridSize parameter #%u", i);
      s32 ret = this->SkipToNextArg(in);
      if (i == PARM_COUNT-1) {
        if (ret != 0)
          return in.Msg(Logger::ERROR, "Expected ')' after DefineGridSize parameters");
      } else {
        if (ret != 1)
          return in.Msg(Logger::ERROR, "Expected ',' after DefineGridSize parameter #%u", i);
      }
    }

    Grid<GC> & grid = ec.GetGrid();
    if (grid.GetWidthSites() != parms[0])
      in.Msg(Logger::WARNING, "DefineGridSize sites wide (%u) not equal to actual sites wide (%u)",
             parms[0], grid.GetWidthSites());
    if (grid.GetHeightSites() != parms[1])
      in.Msg(Logger::WARNING, "DefineGridSize sites high (%u) not equal to actual sites high (%u)",
             parms[1], grid.GetHeightSites());
    if (grid.GetWidth() != parms[2])
      in.Msg(Logger::WARNING, "DefineGridSize tiles wide (%u) not equal to actual tiles wide (%u)",
             parms[2], grid.GetWidth());
    if (grid.GetHeight() != parms[3])
      in.Msg(Logger::WARNING, "DefineGridSize tiles high (%u) not equal to actual tiles high (%u)",
             parms[3], grid.GetHeight());
    return true;
  }

  template <class GC>
  FunctionCallRegisterElement<GC>::FunctionCallRegisterElement(ExternalConfigSectionGrid<GC> & ec)
    : ConfigFunctionCallGrid<GC>("RegisterElement", ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallRegisterElement<GC>::Parse()
  {
    ExternalConfigSectionGrid<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    in.SkipWhitespace();
    UUID uuid;
    if (!uuid.Read(in))
      return in.Msg(Logger::ERROR, "Error reading Element UUID");
    s32 ret = this->SkipToNextArg(in);
    if (ret < 0) return false;
    if (ret == 0)
      return in.Msg(Logger::ERROR, "Expected second argument");
    OString16 buf;
    if (!in.ScanIdentifier(buf))
      return in.Msg(Logger::ERROR, "Expected identifier as second argument");
    if (buf.HasOverflowed())
      return in.Msg(Logger::ERROR, "Identifier too long '%s'", buf.GetZString());
    if (!ec.RegisterElement(uuid, buf))
      return in.Msg(Logger::ERROR, "Couldn't register '%@' under name '%s'", &uuid, buf.GetZString());

    return this->SkipToNextArg(in) == 0;
  }

  template <class GC>
  FunctionCallGA<GC>::FunctionCallGA(ExternalConfigSectionGrid<GC> & ec)
    : ConfigFunctionCallGrid<GC>("GA",ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  const Element<typename GC::EVENT_CONFIG> * ExternalConfigSectionGrid<GC>::ParseElementIdentifier(LineCountingByteSource &in)
  {
    in.SkipWhitespace();

    OString16 nick;
    if (!in.ScanIdentifier(nick))
    {
      in.Msg(Logger::ERROR, "Expected identifier as first argument");
      return 0;
    }

    if (nick.HasOverflowed())
    {
      in.Msg(Logger::ERROR, "Identifier too long '%s'", nick.GetZString());
      return 0;
   }
    const Element<EC> * pelt = this->LookupElement(nick);
    if (!pelt)
    {
      in.Msg(Logger::ERROR, "'%s' isn't a registered element nickname", nick.GetZString());
      return 0;
    }

    return pelt;
  }

  template <class GC>
  bool FunctionCallGA<GC>::Parse()
  {
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    enum { BPA = AC::BITS_PER_ATOM };

    ExternalConfigSectionGrid<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    const Element<EC> * pelt = ec.ParseElementIdentifier(in);

    if (!pelt) return false; // error message already issued

    s32 x, y;

    OString128 hexData;

    if (!this->SkipToNextExistingArg(in,"x position"))
      return false;

    if (!in.Scan(x))
      return in.Msg(Logger::ERROR, "Expected x position");

    if (!this->SkipToNextExistingArg(in,"y position"))
      return false;

    if (!in.Scan(y))
      return in.Msg(Logger::ERROR, "Expected y position");

    if (!this->SkipToNextExistingArg(in, "atom body"))
      return false;

    Atom<AC> temp;
    AtomSerializer<AC> as(temp);
    if (in.Scanf("%@",&as) != 1)
      return in.Msg(Logger::ERROR, "Expected hex-encoded Atom body");

    if (!ec.PlaceAtom(*pelt, x, y, as.GetBits()))
      return false;

    return this->SkipToNextArg(in) == 0;
  }

  template <class GC>
  FunctionCallTile<GC>::FunctionCallTile(ExternalConfigSectionGrid<GC> & ec)
    : ConfigFunctionCallGrid<GC>("Tile",ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallTile<GC>::Parse()
  {
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    enum { BPA = AC::BITS_PER_ATOM };

    ExternalConfigSectionGrid<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    Grid<GC> & grid = ec.GetGrid();
    in.SkipWhitespace();

    u32 x, y;
    if (3 != in.Scanf("%d,%d",&x,&y))
      return false;
    if (x >= grid.GetWidth() || y >= grid.GetHeight())
      return false;
    if (!grid.GetTile(SPoint(x,y)).LoadTile(in))
      return false;
    return in.Scanf(")") == 1;
  }


  template <class GC>
  FunctionCallSite<GC>::FunctionCallSite(ExternalConfigSectionGrid<GC> & ec)
    : ConfigFunctionCallGrid<GC>("Site",ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallSite<GC>::Parse()
  {
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    enum { BPA = AC::BITS_PER_ATOM };

    ExternalConfigSectionGrid<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    in.SkipWhitespace();

    s32 tmp_x, tmp_y;
    if (3 != in.Scanf("%d,%d",&tmp_x,&tmp_y))
      return false;

    if (!ec.GetGrid().LoadSite(SPoint(tmp_x,tmp_y), in, ec))
      return false;

    return in.Scanf(")") == 1;
  }

  template <class GC>
  FunctionCallSetElementParameter<GC>::FunctionCallSetElementParameter(ExternalConfigSectionGrid<GC> & ec)
    : ConfigFunctionCallGrid<GC>("SetElementParameter",ec)
  {
    ec.RegisterFunction(*this);
  }

  template <class GC>
  bool FunctionCallSetElementParameter<GC>::Parse()
  {
    typedef typename GC::EVENT_CONFIG EC;

    ExternalConfigSectionGrid<GC> & ec = this->GetECSG();
    LineCountingByteSource & in = ec.GetByteSource();
    in.SkipWhitespace();

    OString16 nick;
    if (!in.ScanIdentifier(nick))
      return in.Msg(Logger::ERROR, "Expected element nickname as first argument");

    Element<EC>* elem = ec.LookupElement(nick);
    if (!elem)
      return in.Msg(Logger::ERROR, "'%s' is not a known element nickname", nick.GetZString());

    ElementParameters<EC> & parms = elem->GetElementParameters();

    if (!this->SkipToNextExistingArg(in,"parameter tag"))
      return false;

    OString32 paramTag;
    if (!in.ScanIdentifier(paramTag))
      return in.Msg(Logger::ERROR, "Expected parameter tag as second argument");

    s32 index = parms.GetParameterNumberFromTag(paramTag.GetZString());
    if (index < 0)
      return in.Msg(Logger::ERROR, "'%s' is not a known parameter tag", paramTag.GetZString());

    ElementParameter<EC> * p = parms.GetParameter((u32) index);
    MFM_API_ASSERT_NONNULL(p);

    if (!this->SkipToNextExistingArg(in,"parameter value"))
      return false; // message already issued

    if (!p->ReadValue(in))
      return in.Msg(Logger::ERROR, "Reading value of parameter tag '%s' failed", paramTag.GetZString());

    return this->SkipToNextArg(in) == 0;
  }

  template<class GC>
  ExternalConfigSectionGrid<GC>::ExternalConfigSectionGrid(ExternalConfig<GC>& ec, Grid<GC>& grid)
    : ExternalConfigSection<GC>(ec)
    , m_grid(grid)
    , m_errorsTo(0)
    , m_registeredElementCount(0)
    , m_elementRegistry(grid.GetElementRegistry())
    , m_fcDefineGridSize(*this)
    , m_fcRegisterElement(*this)
    , m_fcTile(*this)
    , m_fcGA(*this)
    , m_fcSite(*this)
    , m_fcSetElementParameter(*this)
  { }

  template<class GC>
  void ExternalConfigSectionGrid<GC>::Reset()
  {
    m_registeredElementCount = 0;
  }

  template<class GC>
  bool ExternalConfigSectionGrid<GC>::ReadInit()
  {
    m_grid.Clear();
    return true;
  }

  template<class GC>
  bool ExternalConfigSectionGrid<GC>::ReadFinalize()
  {
    m_grid.RefreshAllCaches();
    m_grid.RecountAtoms();
    return true;
  }

  template<class GC>
  void ExternalConfigSectionGrid<GC>::WriteSection(ByteSink& byteSink)
  {
    /* Zeroth, identify grid size in sites and tiles. */
    byteSink.Printf("DefineGridSize(%u,%u,%u,%u)\n",
                    m_grid.GetWidthSites(),
                    m_grid.GetHeightSites(),
                    m_grid.GetWidth(),
                    m_grid.GetHeight());

    /* First, register all elements. */

    u32 elems = m_elementRegistry.GetEntryCount();

    for(u32 i = 0; i < elems; i++)
    {
      Element<EC> * elt = m_elementRegistry.GetRegisteredElement(i);
      if (!elt) continue;
      OString16 nick;
      this->PrintAtomType(elt->GetDefaultAtom(), nick);

      UUID uuid = m_elementRegistry.GetEntryUUID(i);
      byteSink.Printf("RegisterElement(%@,%s)\n",
                      &uuid,
                      nick.GetZString());

      /* Write configurable element values */

      const Element<EC>* elem = m_elementRegistry.GetEntryElement(i);
      const ElementParameters<EC> & parms = elem->GetElementParameters();

      for(u32 j = 0; j < parms.GetParameterCount(); j++)
      {
        const ElementParameter<EC> * p = parms.GetParameter(j);
        byteSink.Printf(" SetElementParameter(%s,%s,%@)",
                        nick.GetZString(),
                        p->GetTag(),
                        p);
        byteSink.WriteNewline();
      }
    }
    byteSink.WriteNewline();

    /* Then write the tile config */
    for(u32 y = 0; y < m_grid.GetHeight(); y++)
    {
      for(u32 x = 0; x < m_grid.GetWidth(); x++)
      {
        byteSink.Printf("Tile(%d,%d",x,y);
        m_grid.GetTile(SPoint(x,y)).SaveTile(byteSink);
        byteSink.Printf(")\n");
      }
    }

    /* Then, write ALL the damn sites, */
    /* and GA all live atoms. */

    /* The grid size in sites excluding caches */
    const u32 gridWidth = m_grid.GetWidthSites();
    const u32 gridHeight = m_grid.GetHeightSites();

    for(u32 y = 0; y < gridHeight; y++)
    {
      for(u32 x = 0; x < gridWidth; x++)
      {
        SPoint currentPt(x, y);

        byteSink.Printf("Site(%d,%d",x,y);
        m_grid.SaveSite(currentPt,byteSink,*this);
        byteSink.Printf(")\n");

#if 0 // Site(..) includes the event layer atoms

        /* No need to write empties since they are the default */
        if(!Atom<AC>::IsType(*m_grid.GetAtom(currentPt),
                             Element_Empty<EC>::THE_INSTANCE.GetType()))
        {
          byteSink.Printf("GA(");

          /* This wil be a little slow, but meh. Makes me miss hash
           * tables. */
          for(u32 i = 0; i < elems; i++)
          {
            if(Atom<AC>::IsType(*m_grid.GetAtom(currentPt),
                                m_elementRegistry.GetEntryElement(i)->GetType()))
            {
              IntAlphaEncode(i, alphaOutput);
              byteSink.Printf("%s", alphaOutput);
              break;
            }
          }

          T temp = *m_grid.GetAtom(currentPt);
          AtomSerializer<AC> as(temp);
          byteSink.Printf(",%d,%d,%@)\n", x, y, &as);
        }
#endif

      }
    }
    byteSink.WriteNewline();
  }

  template<class GC>
  bool ExternalConfigSectionGrid<GC>::RegisterElement(const UUID & uuid, OString16 & nick)
  {
    LineCountingByteSource & lcbs = this->GetByteSource();
    Element<EC> * elt = m_elementRegistry.Lookup(uuid);
    const UUID * puuid = 0;
    if (!elt) {

      elt =  m_elementRegistry.LookupCompatible(uuid);
      if (!elt)
        return lcbs.Msg(Logger::WARNING, "No alternatives found for unknown/unregistered element '%@'", &uuid);

      puuid = &elt->GetUUID();
      lcbs.Msg(Logger::WARNING, "Using more recent '%@' for '%@'", puuid, &uuid);
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
      {
        lcbs.Msg(Logger::WARNING,"Element '%@' already registered as '%s', skipping", &re.m_uuid, re.m_nick.GetZString());
        return true;
      }
      if (re.m_nick.Equals(nick))
        return lcbs.Msg(Logger::ERROR,"Element nicknamed '%s' already registered for UUID '%@'",
                        nick.GetZString(), &re.m_uuid);
    }
    if (m_registeredElementCount >= sizeof(m_registeredElements)/sizeof(m_registeredElements[0]))
      return lcbs.Msg(Logger::ERROR,"No room for more registered Elements");

    m_registeredElements[m_registeredElementCount].m_uuid = *puuid;
    m_registeredElements[m_registeredElementCount].m_nick = nick;
    m_registeredElements[m_registeredElementCount].m_element = elt;
    ++m_registeredElementCount;

    lcbs.Msg(Logger::MESSAGE,"Registration %d: Nickname '%s' -> UUID '%@'", m_registeredElementCount, nick.GetZString(), puuid);

    return true;
  }

  template<class GC>
  Element<typename GC::EVENT_CONFIG> * ExternalConfigSectionGrid<GC>::LookupElement(const OString16 & nick) const
  {
    for (u32 i = 0; i < m_registeredElementCount; ++i) {
      const RegElt & re = m_registeredElements[i];
      if (re.m_nick.Equals(nick))
        return re.m_element;
    }
    return 0;
  }

  template<class GC>
  void ExternalConfigSectionGrid<GC>::SetTileToExecuteOnly(const SPoint& tileLoc, bool value)
  {
    m_grid.SetTileEnabled(tileLoc, !value);
  }

  template<class GC>
  bool ExternalConfigSectionGrid<GC>::PlaceAtom(const Element<EC> & elt, s32 x, s32 y, const char* hexData)
  {
    SPoint pt(x, y);
    T atom = elt.GetDefaultAtom();

    atom.ReadStateBits(hexData);
    m_grid.PlaceAtom(atom, pt);

    return true;
  }

  template<class GC>
  bool ExternalConfigSectionGrid<GC>::PlaceAtom(const Element<EC> & elt, s32 x, s32 y, const BitVector<BPA> & bv)
  {
    SPoint pt(x, y);
    T atom = elt.GetDefaultAtom();

    atom.ReadStateBits(bv);
    m_grid.PlaceAtom(atom, pt);

    return true;
  }


  template <class GC>
  bool ExternalConfigSectionGrid<GC>::ParseAtomType(LineCountingByteSource &in, T& dest) //typename GC::EVENT_CONFIG::ATOM_CONFIG::ATOM_TYPE
  {
    const Element<EC> * pelt = ParseElementIdentifier(in);
    if (!pelt)
    {
      return false;
    }

    dest = pelt->GetDefaultAtom();
    return true;
  }

  template <class GC>
  void ExternalConfigSectionGrid<GC>::PrintAtomType(const T& atom, ByteSink& bs)
  {
    // We have arranged to use the (to-become) old type as the atom
    // tag..  Very convenient here.
    bs.Printf("T%04x",atom.GetType());
  }

}
