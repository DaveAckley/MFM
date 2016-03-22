/*                                              -*- mode:C++ -*-
  AbstractGUIDriverTools.h Drawing tools for the AbstractGUIDriver
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file AbstractGUIDriverTools.h Drawing tools for the AbstractGUIDriver.

  Notes on adding a grid tool to the simulator:

  - Define a class for it -- e.g., GridToolSpreadFoo -- following the
    existing tools as much as possible.  Keep it all reasonably small
    and in this .h file.

  - Update in AssetManager.h and in res/images/SpreadFoo.png as
    necessary to define an IMAGE_ASSET_SPREADFOO_ICON for your tool.

  - Implement the virtual methods, particularly
    GridToolSpreadFoo::UpdateGridCoord(UPoint).  Dish off to other
    methods elsewhere (e.g, in Grid) if the underlying functionality
    is complex or also useful programmatically.

  - Update AbstractGUIDriver.h, making changes in three (3) places:

    = Add a data member 'GridToolSpreadFoo<GC> m_gridToolSpreadFoo;'
      in sequence with the existing tools

    = Add a ctor call for it ', m_gridToolSpreadFoo(m_gridPanel,
      m_toolboxPanel)' in sequence with the existing tools.

    = Add a registration for it
      'm_toolboxPanel.RegisterGridTool(m_gridToolSpreadFoo);' in
      OnceOnlyTools in sequence with the existing tools.

  \author David H. Ackley.
  \author Trent R. Small.
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTGUIDRIVERTOOLS_H
#define ABSTRACTGUIDRIVERTOOLS_H

#include "Grid.h"
#include "GridTool.h"
#include "AssetManager.h"
#include "AtomViewPanel.h"

namespace MFM
{
  template<class GC>
  class GridToolPencil : public GridToolShapeUpdater<GC>
  {
  public:
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolPencil(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("pencil", gp, tbp,
              IMAGE_ASSET_PENCIL_ICON,
              "Use pencil tool (draw atoms)")
    {
      Super::SetToolShape(DIAMOND_SHAPE);
    }

    virtual void UpdateGridCoord(UPoint gridCoord)
    {
      const Element<EC> * elt = this->GetSelectedElement();
      MFM_API_ASSERT_NONNULL(elt);

      Grid<GC> & grid = this->GetGrid();
      grid.PlaceAtomInSite(this->IsSiteEdit(), elt->GetDefaultAtom(), MakeSigned(gridCoord));
    }
  };

  template<class GC>
  class GridToolEraser : public GridToolShapeUpdater<GC>
  {
  public:
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolEraser(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("eraser", gp, tbp,
              IMAGE_ASSET_ERASER_ICON,
              "Use eraser tool (set sites empty)")
    {
      Super::SetToolShape(DIAMOND_SHAPE);
    }

    virtual void UpdateGridCoord(UPoint gridCoord)
    {
      //      ToolboxPanel<GC> & tbp = this->GetToolboxPanel();
      Grid<GC> & grid = this->GetGrid();
      grid.PlaceAtomInSite(this->IsSiteEdit(), Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom(), MakeSigned(gridCoord));
    }
  };

  template<class GC>
  class GridToolBrush : public GridToolShapeUpdater<GC>
  {
  public:
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolBrush(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("brush", gp, tbp,
              IMAGE_ASSET_BRUSH_ICON,
              "Use brush tool (draw atoms)")
    {
      Super::SetToolShape(ROUND_SHAPE);
    }

    virtual void UpdateGridCoord(UPoint gridCoord)
    {
      const Element<EC> * elt = this->GetSelectedElement();
      MFM_API_ASSERT_NONNULL(elt);

      Grid<GC> & grid = this->GetGrid();
      grid.PlaceAtomInSite(this->IsSiteEdit(), elt->GetDefaultAtom(), MakeSigned(gridCoord));
    }
  };

  template<class GC>
  class GridToolAirBrush : public GridToolShapeUpdater<GC>
  {
  public:
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolAirBrush(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("air", gp, tbp,
              IMAGE_ASSET_AIRBRUSH_ICON,
              "Use airbrush tool (draw atoms in random sites)")
    {
      Super::SetToolShape(ROUND_SHAPE);
    }

    virtual void UpdateGridCoord(UPoint gridCoord)
    {
      Grid<GC> & grid = this->GetGrid();
      Random & random = grid.GetRandom();
      if (random.OneIn(100))
      {
        const Element<EC> * elt = this->GetSelectedElement();
        MFM_API_ASSERT_NONNULL(elt);
        grid.PlaceAtomInSite(this->IsSiteEdit(), elt->GetDefaultAtom(), MakeSigned(gridCoord));
      }
    }
  };

  template<class GC>
  class GridToolEvent : public GridToolShapeUpdater<GC>
  {
  public:
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolEvent(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("event", gp, tbp,
              IMAGE_ASSET_EVENT_ICON,
              "Use spark tool (deliver events)")
    {
      Super::SetToolShape(DIAMOND_SHAPE);
    }

    virtual void UpdateGridCoord(UPoint gridCoord)
    {
      const Element<EC> * elt = this->GetSelectedElement();
      MFM_API_ASSERT_NONNULL(elt);

      Grid<GC> & grid = this->GetGrid();
      grid.RunEventIfPausedAt(MakeSigned(gridCoord));
    }
  };

  template<class GC>
  class GridToolXRay : public GridToolShapeUpdater<GC>
  {
  public:
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolXRay(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("xray", gp, tbp,
              IMAGE_ASSET_XRAY_ICON,
              "Use xray tool (flip bits in random sites)")
    {
      Super::SetToolShape(ROUND_SHAPE);
    }

    virtual void UpdateGridCoord(UPoint gridCoord)
    {
      Grid<GC> & grid = this->GetGrid();
      grid.MaybeXRayAtom(MakeSigned(gridCoord));
    }
  };


  template<class GC>
  class GridToolBucket : public GridToolShapeUpdater<GC>
  {
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    enum { R = EC::EVENT_WINDOW_RADIUS};

    bool FitsInMini(const SPoint sp)
    {
      return sp.GetMaximumLength() <= S16_MAX;
    }

    SSPoint MakeMini(const SPoint sp)
    {
      if (!FitsInMini(sp)) FAIL(ILLEGAL_ARGUMENT);
      return SSPoint((s16) sp.GetX(), (s16) sp.GetY());
    }

    SPoint MakeSignedFromMini(const SSPoint mp)
    {
      return SPoint(mp.GetX(), mp.GetY());
    }

    // When flood-filling empty space, our breadth-first open queue
    // grows with the perimeter, not the area.  So with a MAX_OPEN of
    // 2048, costing us 8KB for the queue, we can flood an entire
    // 512x512 grid, and smaller grids or partial floods are easier.

    static const u32 MAX_OPEN = 1<<11;

    SSPoint m_openQueue[MAX_OPEN];
    u32 m_nextOutIndex;
    u32 m_nextInIndex;

    SPoint m_gridCenter;
    u32 m_oldType;
    T m_newAtom;

    bool IsOpenQueueEmpty()
    {
      return m_nextOutIndex == m_nextInIndex;
    }

    bool IsOpenQueueFull()
    {
      return (m_nextInIndex + 1) % MAX_OPEN == m_nextOutIndex;
    }

    void ResetOpenQueue()
    {
      m_nextOutIndex = m_nextInIndex = 0;
    }

    bool AddToOpenQueue(const SPoint sp)
    {
      if (IsOpenQueueFull() || !FitsInMini(sp)) return false;
      m_openQueue[m_nextInIndex] = MakeMini(sp);
      m_nextInIndex = (m_nextInIndex + 1) % MAX_OPEN;
      return true;
    }

    const SPoint RemoveFromOpenQueue()
    {
      if (IsOpenQueueEmpty()) FAIL(ILLEGAL_STATE);
      SPoint sp = MakeSignedFromMini(m_openQueue[m_nextOutIndex]);
      m_nextOutIndex = (m_nextOutIndex + 1) % MAX_OPEN;
      return sp;
    }

    void CheckAndSet(SPoint offset)
    {
      const T* atomp = GetSaneAtom(m_gridCenter + offset);
      if (!atomp) return;

      u32 thisType = atomp->GetType();
      if (thisType == m_oldType)
      {
        Grid<GC> & grid = this->GetGrid();
        grid.PlaceAtomInSite(this->IsSiteEdit(), m_newAtom, m_gridCenter + offset);
        AddToOpenQueue(offset);
      }
    }

    void CheckAdjacent(SPoint offset)
    {
      const MDist<R> & md = MDist<R>::get();
      const u32 loIdx = md.GetFirstIndex(1);
      const u32 hiIdx = md.GetLastIndex(1);

      for (u32 i = loIdx; i <= hiIdx; ++i)
        CheckAndSet(offset + md.GetPoint(i));
    }

    const T* GetSaneAtom(SPoint gridCoord)
    {
      Grid<GC> & grid = this->GetGrid();
      if (!grid.IsGridCoord(gridCoord)) return 0;

      const T* atomp = grid.GetAtomInSite(this->IsSiteEdit(), gridCoord);
      MFM_API_ASSERT_NONNULL(atomp);
      if (!atomp->IsSane()) return 0;
      return atomp;
    }

    void FloodFill(UPoint gridCoord)
    {
      const Element<EC> * elt = this->GetSelectedElement();
      if (!elt) FAIL(ILLEGAL_STATE);

      m_newAtom = elt->GetDefaultAtom();
      m_gridCenter = MakeSigned(gridCoord);

      const T* atomp = GetSaneAtom(m_gridCenter);

      if (!atomp) return;

      m_oldType = atomp->GetType();

      if (m_oldType == m_newAtom.GetType())
        return;                 // Umm no

      ResetOpenQueue();
      CheckAndSet(SPoint(0,0));

      while (!IsOpenQueueEmpty())
      {
        CheckAdjacent(RemoveFromOpenQueue());
      }
    }

  public:

    GridToolBucket(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("bucket", gp, tbp,
              IMAGE_ASSET_BUCKET_ICON,
              "Use bucket tool (flood fill atoms)")
    { }

    /**
       We actually have a radius, but it's kind of random and we don't
       want to confess to it, so we'll pretend we don't
     */
    virtual u32 GetMaxVariableRadius()
    {
      return 0;
    }

    virtual void UpdateGridAround(UPoint point)
    {
      FloodFill(point);
    }

    virtual void UpdateGridCoord(UPoint point)
    {
      // We overrode UpdateGridAround, so 'it should be impossible' to
      // get here
      FAIL(ILLEGAL_STATE);
    }

  };


  template<class GC>
  class GridToolAtomView : public GridToolShapeUpdater<GC>
  {
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    enum { R = EC::EVENT_WINDOW_RADIUS};

    AtomViewPanel<GC> * m_avp;

  public:

    void SetAtomViewPanel(AtomViewPanel<GC> & avp)
    {
      m_avp = &avp;
    }

    AtomViewPanel<GC> & GetAtomViewPanel()
    {
      MFM_API_ASSERT_NONNULL(m_avp);
      return *m_avp;
    }

    void SetAtomCoord(SPoint gridCoord)
    {
      GetAtomViewPanel().SetAtomCoord(gridCoord, this->GetToolboxPanel().IsSiteEdit());
    }

    GridToolAtomView(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("atomview", gp, tbp,
              IMAGE_ASSET_ATOM_SELECTOR_ICON,
              "Use atom select tool (examine atom internals)")
    { }

    virtual u32 GetMaxVariableRadius()
    {
      return 0;
    }

    virtual void UpdateGridAround(UPoint point)
    {
      SetAtomCoord(MakeSigned(point));
    }

    virtual void UpdateGridCoord(UPoint point)
    {
      // We overrode UpdateGridAround, so 'it should be impossible' to
      // get here
      FAIL(ILLEGAL_STATE);
    }

#if 0

    void BoxEdge(Drawing & drawing, const SPoint gridCoord, u32 color)
    {
      if (!this->GetGrid().IsGridCoord(gridCoord)) return;
      UPoint boxCoord = MakeUnsigned(gridCoord);
      Rect screenRectDit;
      GridPanel<GC> & gp = this->GetGridPanel();
      if (!gp.GetScreenRectDitOfGridCoord(boxCoord, screenRectDit))
        return;  // WTH?
      drawing.SetForeground(color);
      drawing.DrawRectDit(screenRectDit);
    }


    virtual void PaintOverlay(Drawing & drawing)
    {
      if (!HasAtomCoord()) return;
      BoxEdge(drawing, m_atomCoord, (Utils::GetDateTimeNow()&1) ? Drawing::WHITE : Drawing::BLACK);
    }
#endif

  };


  template<class GC>
  class GridToolTileSelect : public GridToolShapeUpdater<GC>
  {
    typedef GridToolShapeUpdater<GC> Super;

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    enum { R = EC::EVENT_WINDOW_RADIUS};

  public:

    GridToolTileSelect(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("tileselect", gp, tbp,
              IMAGE_ASSET_SELECTOR_ICON,
              "Use tile select tool (select/deselect tiles)")
    { }

    virtual u32 GetMaxVariableRadius()
    {
      return 0;
    }

    virtual void UpdateGridAround(UPoint point)
    {
      Grid<GC> & grid = this->GetGrid();

      SPoint tileInGrid, siteInTile;
      if (!grid.MapGridToTile(MakeSigned(point), tileInGrid, siteInTile))
        return;

      GridPanel<GC> & gp = this->GetGridPanel();
      gp.SetTileSelected(MakeUnsigned(tileInGrid), this->IsMainFunction());
    }

    virtual void UpdateGridCoord(UPoint point)
    {
      // We overrode UpdateGridAround, so 'it should be impossible' to
      // get here
      FAIL(ILLEGAL_STATE);
    }

  };

  template<class GC>
  class GridToolClone : public GridToolShapeUpdater<GC>
  {
    typedef GridToolShapeUpdater<GC> Super;

    SPoint m_srcCoord;
    SPoint m_destCoord;

    bool HasSrcCoord()
    {
      return m_srcCoord.GetX() >= 0 && m_srcCoord.GetY() >= 0;
    }

    bool HasDestCoord()
    {
      return m_destCoord.GetX() >= 0 && m_destCoord.GetY() >= 0;
    }

    void ClearDestCoord()
    {
      m_destCoord = SPoint(-1,-1);
    }

    void SetDestCoord(SPoint gridCoord)
    {
      m_destCoord = gridCoord;
    }

    SPoint GetDestCoord()
    {
      if (!HasDestCoord())
        FAIL(ILLEGAL_STATE);
      return m_destCoord;
    }

    void SetSrcCoord(SPoint gridCoord)
    {
      m_srcCoord = gridCoord;
      ClearDestCoord();
    }

    bool GetDelta(SPoint & delta)
    {
      if (m_destCoord.GetX() < 0 || m_destCoord.GetY() < 0 ||
          m_srcCoord.GetX() < 0 || m_srcCoord.GetY() < 0)
        return false;
      delta = m_destCoord - m_srcCoord;
      return true;
    }

    void DoClone(SPoint gridCoord)
    {
      // Here we expect to have a src coord, dest coord, and gridCoord
      if (!this->HasSrcCoord() || !this->HasDestCoord())
        return;  // But just fail silently if luser forgot to pick an anchor

      SPoint mouseDelta = gridCoord - GetDestCoord();

      u32 uradius = this->GetRadius();
      if (uradius == 0) return;  // No radius, no clone

      Grid<GC> & grid = this->GetGrid();
      s32 radius = (s32) uradius;
      for (s32 x = -radius; x <= radius; ++x)
        for (s32 y = -radius; y <= radius; ++y)
        {
          const SPoint offset(x,y);

          if (this->IsInShape(offset))
          {
            SPoint asrc = m_srcCoord + mouseDelta + offset;
            SPoint adest = m_destCoord + mouseDelta + offset;
            if (grid.IsGridCoord(asrc) && grid.IsGridCoord(adest))
            {
              const T* from = grid.GetAtom(asrc);
              MFM_API_ASSERT_NONNULL(from);
              grid.PlaceAtomInSite(this->IsSiteEdit(), *from, adest);
            }
          }
        }
    }

  public:

    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridToolClone(GridPanel<GC>& gp, ToolboxPanel<GC>& tbp)
      : Super("clone", gp, tbp,
              IMAGE_ASSET_CLONE_ICON,
              "Use clone tool (copy regions)")
      , m_srcCoord(-1,-1)
      , m_destCoord(-1,-1)
    { }

    void DotGridAround(Drawing & drawing, const SPoint gridCoord, u32 color)
    {
      u32 uradius = this->GetRadius();
      if (uradius == 0) return;  // No radius, no overlay.  Up to you, chumley
      s32 radius = (s32) uradius;
      for (s32 x = -radius; x <= radius; ++x)
        for (s32 y = -radius; y <= radius; ++y)
        {
          const SPoint offset(x,y);

          if (this->IsInShape(offset))
          {
            SPoint absolute = gridCoord + offset;
            if (this->GetGrid().IsGridCoord(absolute))
              DotGridCoord(drawing, MakeUnsigned(absolute), color);
          }
        }
    }

    void DotGridCoord(Drawing & drawing, const UPoint dotCoord, u32 color)
    {
      Rect screenRectDit;
      GridPanel<GC> & gp = this->GetGridPanel();
      if (!gp.GetScreenRectDitOfGridCoord(dotCoord, screenRectDit))
        return;  // WTH?
      u32 dit = Drawing::DIT_PER_PIX;
      screenRectDit.SetPosition(screenRectDit.GetPosition()
                                + MakeSigned(screenRectDit.GetSize()) / 2
                                - SPoint(dit/2, dit/2));
      screenRectDit.SetSize(UPoint(dit,dit));
      drawing.SetForeground(color);
      drawing.DrawRectDit(screenRectDit);
    }

    virtual void Release(MouseButtonEvent & event)
    {
      ClearDestCoord();
    }

    virtual void PaintOverlay(Drawing & drawing)
    {
      if (!HasSrcCoord()) return;

      if (!HasDestCoord())
      {
        DotGridAround(drawing, m_srcCoord, Drawing::GREY);
        return;
      }

      SPoint lastGridCoord = this->GetLastGridCoord();
      if (lastGridCoord.GetX() < 0 || lastGridCoord.GetY() < 0)
        return;

      SPoint mouseDelta = lastGridCoord - GetDestCoord();

      DotGridAround(drawing, m_srcCoord + mouseDelta, Drawing::GREEN);

      DotGridAround(drawing, lastGridCoord, Drawing::YELLOW);

    }

    virtual void UpdateGridAround(UPoint gridCoord)
    {
      SPoint sgridCoord = MakeSigned(gridCoord);
      if (this->IsMainFunction())
      {
        if (this->IsInitiateFunction())
          SetDestCoord(sgridCoord);
        DoClone(sgridCoord);
      }
      else
      {
        SetSrcCoord(sgridCoord); // same action on click or drag
      }
    }

    virtual void UpdateGridCoord(UPoint point)
    {
      // We've overridden the only potential caller of this method, so
      // 'it should be impossible' to get here
      FAIL(ILLEGAL_STATE);
    }

    void SaveDetails(ByteSink & sink) const
    {
      Super::SaveDetails(sink);
      const char * tag = this->GetTag();
      SPoint tmp(m_srcCoord);
      SPointSerializer sp(tmp);
      sink.Printf(" PP(%s_scrd=%@)\n", tag, &sp);
    }

    bool LoadDetails(const char * key, LineCountingByteSource & source)
    {
      if (Super::LoadDetails(key, source)) return true;
      const char * tag = this->GetTag();
      CharBufferByteSource cbbs(key, strlen(key));

      cbbs.Reset();
      if (6 == cbbs.Scanf("%z_scrd",tag))
      {
        SPointSerializer sp(m_srcCoord);
        return 1 == source.Scanf("%@", &sp);
      }

      return false;
    }

  };


} /* namespace MFM */

#endif /* ABSTRACTGUIDRIVERTOOLS_H */
