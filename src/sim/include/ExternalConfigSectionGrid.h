/*                                              -*- mode:C++ -*-
  ExternalConfigSectionGrid.h Support for persisting grid configuration information
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
  \file ExternalConfigSectionGrid.h Support for persisting grid configuration information
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef EXTERNALCONFIGSECTIONGRID_H
#define EXTERNALCONFIGSECTIONGRID_H

#include "ExternalConfig.h"

namespace MFM
{
  template <class GC> class ExternalConfigSectionGrid; // FORWARD

  /** Base class for grid-related configuration function calls used in .MFS files */
  template<class GC>
  class ConfigFunctionCallGrid : public ConfigFunctionCall<GC>
  {
  public:

    ConfigFunctionCallGrid(const char * name, ExternalConfigSectionGrid<GC> & ecsg)
      : ConfigFunctionCall<GC>(name)
      , m_ecsg(ecsg)
    { }
    virtual bool Parse() = 0;

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply() { /* Work already done */ }

  private:
    ExternalConfigSectionGrid<GC> & m_ecsg;

  protected:
    ExternalConfigSectionGrid<GC> & GetECSG() { return m_ecsg; }

  };

  template <class GC>
  struct FunctionCallDefineGridSize : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallDefineGridSize(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallRegisterElement : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallRegisterElement(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallGA : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallGA(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallTile : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallTile(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };

  template <class GC>
  struct FunctionCallSite : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallSite(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };

#if 0
  template <class GC>
  struct FunctionCallDisableTile : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallDisableTile(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };
#endif

  template <class GC>
  struct FunctionCallSetElementParameter : public ConfigFunctionCallGrid<GC>
  {
    FunctionCallSetElementParameter(ExternalConfigSectionGrid<GC> & ec) ;
    virtual bool Parse() ;
  };

  template<class GC>
  class ExternalConfigSectionGrid
    : public ExternalConfigSection<GC>
    , public AtomTypeFormatter<typename GC::EVENT_CONFIG::ATOM_CONFIG>
  {
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    enum { BPA = AC::BITS_PER_ATOM };
    enum { TILE_SIDE = GC::TILE_SIDE };
    enum { EVENT_WINDOW_RADIUS = EC::EVENT_WINDOW_RADIUS };

  public:
    virtual bool ParseAtomType(LineCountingByteSource &in, T& dest) ;
    virtual void PrintAtomType(const T&, ByteSink&) ;

    virtual const char * GetSectionName() const
    {
      return "Grid";
    }

    /*
     * Construct a new ExternalConfigSectionGrid referencing a specified Grid
     *
     * @param grid The grid to read from or write to.
     */
    ExternalConfigSectionGrid(ExternalConfig<GC>& ec, Grid<GC>& grid);

    virtual void Reset() ;

    virtual void WriteSection(ByteSink & byteSink);

    virtual bool ContinueOnErrors() const
    {
      return false;
    }

    virtual bool ReadInit() ;

    virtual bool ReadFinalize() ;

    bool RegisterElement(const UUID & uuid, OString16 & nick) ;

    Element<EC> * LookupElement(const OString16 & nick) const ;

    bool PlaceAtom(const Element<EC> & elt, s32 x, s32 y, const char* dataStr) ;

    bool PlaceAtom(const Element<EC> & elt, s32 x, s32 y, const BitVector<BPA> & bv) ;

    void SetTileToExecuteOnly(const SPoint& tileLoc, bool value);

    const Element<EC> * ParseElementIdentifier(LineCountingByteSource &in) ;

    Grid<GC> & GetGrid()
    {
      return m_grid;
    }

  private:

    /**
     * The Grid to read from or write to.
     */
    Grid<GC>& m_grid;

    static const u32 MAX_REGISTERED_ELEMENTS = 100;
    ByteSink * m_errorsTo;

    struct RegElt {
      UUID m_uuid;
      OString16 m_nick;
      Element<EC> * m_element;
    } m_registeredElements[MAX_REGISTERED_ELEMENTS];

    u32 m_registeredElementCount;

    /**
     * The ElementRegistry to lookup UUIDs in.
     */
    ElementRegistry<EC>& m_elementRegistry;

    FunctionCallDefineGridSize<GC> m_fcDefineGridSize;
    FunctionCallRegisterElement<GC> m_fcRegisterElement;
    FunctionCallTile<GC> m_fcTile;
    FunctionCallGA<GC> m_fcGA;
    FunctionCallSite<GC> m_fcSite;
    FunctionCallSetElementParameter<GC> m_fcSetElementParameter;

  };
}

#include "ExternalConfigSectionGrid.tcc"

#endif /* EXTERNALCONFIGSECTIONGRID_H */
