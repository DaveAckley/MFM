/*                                              -*- mode:C++ -*-
  GridTool.h An interface for tools that affect the grid
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
  \file GridTool.h An interface for tools that affect the grid
  \author Dave Ackley
  \date (C) 2015 All rights reserved.
  \lgpl
*/
#ifndef GRIDTOOL_H
#define GRIDTOOL_H

#include "Panel.h"
#include "Site.h"
#include "AssetManager.h"

namespace MFM
{
  template <class GC> class Grid;         // FORWARD
  template <class GC> class GridPanel;    // FORWARD
  template <class EC> class ToolboxPanel; // FORWARD

  /**
   * An interface for a tool that accepts mouse events and produces
   * changes to the grid via the GridPanel
   */
  template<class GC>
  class GridTool
  {
    // Extract short type names
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    GridPanel<GC> & m_gridPanel;
    ToolboxPanel<GC> & m_toolboxPanel;
    const MasterIconZSheetSlot m_iconNumber;

  public:

    GridTool(GridPanel<GC> & gp, ToolboxPanel<GC> & tbp, MasterIconZSheetSlot iconNumber)
      : m_gridPanel(gp)
      , m_toolboxPanel(tbp)
      , m_iconNumber(iconNumber)
    { }

    virtual ~GridTool() { }

    //    ImageAsset GetImageAsset() const { return m_iconNumber; }

    MasterIconZSheetSlot GetIconSlot() const { return m_iconNumber; }

    GridPanel<GC> & GetGridPanel() { return m_gridPanel; }
    const GridPanel<GC> & GetGridPanel() const { return m_gridPanel; }

    Grid<GC> & GetGrid() { return m_gridPanel.GetGrid(); }
    const Grid<GC> & GetGrid() const { return m_gridPanel.GetGrid(); }

    ToolboxPanel<GC> & GetToolboxPanel() { return m_toolboxPanel; }
    const ToolboxPanel<GC> & GetToolboxPanel() const { return m_toolboxPanel; }

    bool IsSiteEdit()
    {
      return m_toolboxPanel.IsSiteEdit();
    }

    /**
       Called to determine the current radius of this tool, in sites
       around the hit.  Return 0 means this tool does not have a
       configurable radius (so no radius should be displayed).
     */
    virtual u32 GetRadius() = 0;

    /**
       Called to notify this tool that it just has been selected.
     */
    virtual void Selected() = 0;

    /**
       Called to notify this tool that it just has been deselected.
     */
    virtual void Deselected() = 0;

    /**
       Called to notify this tool that a mouse-down has just occurred.
       Note that if GetRadius() returns non-zero, this method will
       also be called on WHEELUP and WHEELDOWN events.
     */
    virtual void Press(MouseButtonEvent & event) = 0;

    /**
       Called to notify this tool that mouse-motion-while-pressed has occurred.
     */
    virtual void Drag(MouseMotionEvent & event) = 0;

    /**
       Called to notify this tool that a mouse-release-after-press has occurred.
     */
    virtual void Release(MouseButtonEvent &) = 0;

    /**
       Called when this tool should draw any tool-related overlay it needs on the grid
     */
    virtual void PaintOverlay(Drawing & drawing) = 0;

  };

  enum ToolShape {
    ROUND_SHAPE,
    DIAMOND_SHAPE,
    SQUARE_SHAPE,
    SHAPE_COUNT
  };

  /**
   * A GridTool that update grid coords it is dragged through.
   * Supports radius maintenance unless the underlying tool rules it
   * out by overloading GetMaxVariableRadius() to return 0.  Provides
   * shape support services for subclasses using variable radius.
   */
  template<class GC>
  class GridToolShapeUpdater : public GridTool<GC>
  {
  public:
    typedef GridTool<GC> Super;

    // Extract short type names
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };

  private:
    OString16 m_tag;
    s32 m_toolRadius; // Used unsigned, <0 implies uninitted
    bool m_mainFunction; // Meaning left button vs right
    bool m_initiateFunction; // Meaning press vs drag
    SPoint m_lastGridCoord; // Tracks last coord pressed or dragged over
    ToolShape m_toolShape;
    OString64 m_doc;

  public:
    const char * GetTag() const
    {
      return m_tag.GetZString();
    }

    const char * GetDoc() const
    {
      return m_doc.GetZString();
    }

    GridToolShapeUpdater(const char * tag, GridPanel<GC> & gp, ToolboxPanel<GC> & tbp, MasterIconZSheetSlot iconNumber, const char * doc)
      : Super(gp, tbp, iconNumber)
      , m_tag(tag)
      , m_toolRadius(-1)
      , m_mainFunction(false)
      , m_initiateFunction(false)
      , m_lastGridCoord(-1,-1)
      , m_toolShape(ROUND_SHAPE)  // subclasses may call SetToolShape to have other default
      , m_doc(doc)
    {
      MFM_API_ASSERT_NONNULL(doc);
    }

    void SetToolShape(u32 shape)
    {
      if (shape >= SHAPE_COUNT)
        FAIL(ILLEGAL_ARGUMENT);
      m_toolShape = (ToolShape) shape;
    }

    u32 GetToolShape()
    {
      if (GetMaxVariableRadius() == 0)
        FAIL(ILLEGAL_STATE); // I have no shape
      return m_toolShape;
    }

    virtual ~GridToolShapeUpdater() { }

    virtual u32 GetRadius()
    {
      if (m_toolRadius < 0)
      {
        if (GetMaxVariableRadius() > 0) m_toolRadius = 1;
        else m_toolRadius = 0;
      }
      return m_toolRadius;
    }

    virtual void Selected()
    {
      this->GetGridPanel().SetGridTool(this);
    }

    virtual void Deselected()
    {
      if (this->GetGridPanel().GetGridTool() == this)
        this->GetGridPanel().SetGridTool(0);
    }

    SPoint GetLastGridCoord()
    {
      return m_lastGridCoord;
    }

    bool IsMainFunction()
    {
      return m_mainFunction;
    }

    bool IsInitiateFunction()
    {
      return m_initiateFunction;
    }

    const Element<EC> * GetSelectedElement()
    {
      ToolboxPanel<GC> & tbp = this->GetToolboxPanel();
      const Element<EC> * elt;
      if (this->IsMainFunction())
        elt = tbp.GetPrimaryElement();
      else
        elt = tbp.GetSecondaryElement();
      return elt;
    }

    virtual void Press(MouseButtonEvent & event)
    {
      SDL_MouseButtonEvent & mb = event.m_event.button;
      bool uppity = false;
      if ((uppity = (mb.button == SDL_BUTTON_WHEELUP)) ||
          mb.button == SDL_BUTTON_WHEELDOWN)
      {
        if (this->GetRadius() == 0) return;
        if (uppity) ++m_toolRadius;
        else if (m_toolRadius > 1) --m_toolRadius;
        return;
      }

      m_mainFunction = (mb.button == SDL_BUTTON_LEFT);
      m_initiateFunction = true;

      // Not wheel event
      UPoint gridCoord;
      if (this->GetGridPanel().GetGridCoordAtScreenDit(Drawing::MapPixToDit(event.GetAt()), gridCoord))
      {
        m_lastGridCoord = MakeSigned(gridCoord);
        if (GridPanel<GC>::IsLeftOrRightSetInButtonMask(1<<mb.button))
          this->UpdateGridAround(gridCoord);
      }
    }

    virtual void Drag(MouseMotionEvent & event)
    {
      UPoint gridCoord;
      m_initiateFunction = false;
      if (this->GetGridPanel().GetGridCoordAtScreenDit(Drawing::MapPixToDit(event.GetAt()), gridCoord))
      {
        m_lastGridCoord = MakeSigned(gridCoord);
        if (GridPanel<GC>::IsLeftOrRightSetInButtonMask(event.m_buttonMask))
          this->UpdateGridAround(gridCoord);
      }
    }

    virtual void Release(MouseButtonEvent & event) { /* nothing to do */ }

    void HighlightGridCoord(Drawing & drawing, const UPoint gridCoord) {
      Rect screenRectDit;
      GridPanel<GC> & gp = this->GetGridPanel();
      if (!gp.GetScreenRectDitOfGridCoord(gridCoord, screenRectDit))
        return;  // WTH?
      u32 dit = Drawing::DIT_PER_PIX;
      screenRectDit.SetPosition(screenRectDit.GetPosition()
                                + MakeSigned(screenRectDit.GetSize()) / 2
                                - SPoint(dit/2, dit/2));
      screenRectDit.SetSize(UPoint(dit,dit));
      drawing.SetForeground(Drawing::YELLOW);
      drawing.DrawRectDit(screenRectDit);
    }

    virtual void PaintOverlay(Drawing & drawing) {
      SPoint lastGridCoord = this->GetLastGridCoord();
      if (lastGridCoord.GetX() < 0 ||
          lastGridCoord.GetY() < 0)
        return;
      u32 uradius = this->GetRadius();
      if (uradius == 0) return;  // No radius, no overlay.  Up to you, chumley
      s32 radius = (s32) uradius;
      for (s32 x = -radius; x <= radius; ++x)
        for (s32 y = -radius; y <= radius; ++y)
        {
          const SPoint offset(x,y);

          if (IsInShape(offset))
          {
            SPoint absolute = lastGridCoord + offset;
            if (this->GetGrid().IsGridCoord(absolute))
              HighlightGridCoord(drawing, MakeUnsigned(absolute));
          }
        }
    }


    virtual void UpdateGridAround(UPoint gridCoord)
    {
      u32 uradius = this->GetRadius();
      s32 radius = (s32) uradius;
      for (s32 x = -radius; x <= radius; ++x)
        for (s32 y = -radius; y <= radius; ++y)
        {
          const SPoint offset(x,y);

          if (IsInShape(offset))
          {
            SPoint absolute = MakeSigned(gridCoord) + offset;
            if (this->GetGrid().IsGridCoord(absolute))
              UpdateGridCoord(MakeUnsigned(absolute));
          }
        }
    }

    virtual void UpdateGridCoord(UPoint point) = 0;

    /**
       Get the largest legal radius for this tool.  Return 0 to
       indicate variable radius is illegal for this tool, else greater
       than 0 for the max radius size.  Default max size is 100.
     */
    virtual u32 GetMaxVariableRadius()
    {
      return 100;
    }

    static bool IsInShapeSquare(const SPoint & point, u32 radius)
    {
      return point.GetMaximumLength() < radius;
    }

    static bool IsInShapeDiamond(const SPoint & point, u32 radius)
    {
      return point.GetManhattanLength() < radius;
    }

    static bool IsInShapeRound(const SPoint & point, u32 radius)
    {
      if (radius == 0) return false;
      u32 rsq = (radius - 1) * (radius - 1);
      return point.GetEuclideanLengthSquared() <= rsq;
    }

    bool IsInShapeSquare(const SPoint & point) { return IsInShapeSquare(point, this->GetRadius()); }
    bool IsInShapeDiamond(const SPoint & point) { return IsInShapeDiamond(point, this->GetRadius()); }
    bool IsInShapeRound(const SPoint & point) { return IsInShapeRound(point, this->GetRadius()); }

    virtual bool IsInShape(const SPoint & point)
    {
      if (GetMaxVariableRadius() == 0) return false;
      switch (m_toolShape)
      {
      case ROUND_SHAPE: return IsInShapeRound(point);
      case SQUARE_SHAPE: return IsInShapeSquare(point);
      case DIAMOND_SHAPE: return IsInShapeDiamond(point);
      default: FAIL(ILLEGAL_STATE);
      }
    }

    virtual void SaveDetails(ByteSink & sink) const ;

    virtual bool LoadDetails(const char * key, LineCountingByteSource & source) ;

  };
}

#include "GridTool.tcc"

#endif /* GRIDTOOL_H */
