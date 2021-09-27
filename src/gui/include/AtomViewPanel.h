/*                                              -*- mode:C++ -*-
  AtomViewPanel.h Panel displaying details of selected Atom
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

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
  \file AtomViewPanel.h Panel displaying details of selected Atom
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ATOMVIEWPANEL_H
#define ATOMVIEWPANEL_H

#include "CloseWindowButton.h"
#include "MovablePanel.h"
#include "Grid.h"
#include "TreeViewPanel.h"
#include "AbstractCheckbox.h"
#include "TileRenderer.h"
#include "DrawableSDL.h"


namespace MFM {

  template<class EC>
  struct Ui_Ut_r10111b : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    const u32 read() const { return UlamRef<EC>::Read(); /* entire */ } //gcnl:UlamTypePrimitive.cpp:306
    void write(const u32& targ) { UlamRef<EC>::Write(targ); /* entire */ } //gcnl:UlamTypePrimitive.cpp:338
    Ui_Ut_r10111b(BitStorage<EC>& targ, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 1u, targ, NULL, UlamRef<EC>::PRIMITIVE, uc) { } //gcnl:UlamTypePrimitive.cpp:224
    Ui_Ut_r10111b(const UlamRef<EC>& arg, s32 idx) : UlamRef<EC>(arg, idx, 1u, NULL, UlamRef<EC>::PRIMITIVE) { } //gcnl:UlamTypePrimitive.cpp:236
    Ui_Ut_r10111b(const Ui_Ut_r10111b<EC>& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), NULL, UlamRef<EC>::PRIMITIVE) { MFM_API_ASSERT_ARG(arg.GetLen() == 1); } //gcnl:UlamTypePrimitive.cpp:251
    Ui_Ut_r10111b& operator=(const Ui_Ut_r10111b& rhs); //declare away //gcnl:UlamTypePrimitive.cpp:260
  };

  template<class EC>
  struct Ui_Ut_10111b : public BitVectorBitStorage<EC, BitVector<1u> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitVector<1> BV; //gcnl:UlamTypePrimitive.cpp:466
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypePrimitive.cpp:469

    const u32 read() const { return BVS::Read(0u, 1u); } //gcnl:UlamTypePrimitive.cpp:558
    void write(const u32& v) { BVS::Write(0u, 1u, v); } //gcnl:UlamTypePrimitive.cpp:608
    Ui_Ut_10111b() { } //gcnl:UlamTypePrimitive.cpp:482
    Ui_Ut_10111b(const u32 d) { write(d); } //gcnl:UlamTypePrimitive.cpp:490
    Ui_Ut_10111b(const Ui_Ut_10111b& other) : BVS() { this->write(other.read()); } //gcnl:UlamTypePrimitive.cpp:513
    Ui_Ut_10111b(const Ui_Ut_r10111b<EC>& d) { this->write(d.read()); } //gcnl:UlamTypePrimitive.cpp:522
    virtual const char * GetUlamTypeMangledName() const { return "Ut_10111b"; } //gcnl:UlamType.cpp:963
  };

}


namespace MFM
{
  template <class GC> class GridPanel; // FORWARD

  template <class GC>
  class AtomViewPanel : public MovablePanel
  {
   private:
    typedef MovablePanel Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    SPoint m_gridCoord;
    bool m_inBase;

    bool m_plexViewPreferred;

    Grid<GC>* m_grid;
    GridPanel<GC>* m_gridPanel;

    struct ClearAtomCoordButton : public CloseWindowButton
    {
      typedef CloseWindowButton Super;

      const char * GetDoc() { return "Close atom view panel"; }
      virtual bool GetKey(u32& keysym, u32& mods) { return false; }
      bool ExecuteFunction(u32 keysym, u32 mods) { return false; }

      ClearAtomCoordButton(AtomViewPanel<GC> & avp)
        : Super(&avp)
        , m_avp(avp)
      { }

      AtomViewPanel<GC> & m_avp;

      virtual void OnClick(u8 button)
      {
        m_avp.ClearAtomCoord();
      }
    };

    struct PlexViewCheckbox : public AbstractCheckbox
    {
    private:
      AtomViewPanel<GC> & m_avp;

    public:
      PlexViewCheckbox(AtomViewPanel<GC> & avp)
        : AbstractCheckbox("Prefer Plex view")
        , m_avp(avp)
      {
        AbstractButton::SetName("PlexView");
        Panel::SetDoc("Prefer Plex view if available");
        Panel::SetFont(FONT_ASSET_BUTTON_SMALL);
        avp.Panel::Insert(this, NULL);

        this->SetEnabledBg(Drawing::ORANGE);//GREY60);
        this->SetEnabledFg(Drawing::BLACK);

        this->SetRenderPoint(SPoint(20, 2));
        this->SetDimensions(SPoint(16, 16));
      }

      virtual bool GetKeyboardAccelerator(u32 & keysym, u32 & mod)
      {
        keysym = SDLK_p;
        mod = KMOD_CTRL;
        return true;
      }

      virtual bool IsChecked() const
      {
        return m_avp.IsPlexViewPreferred();
      }

      virtual void OnCheck(bool checked)
      {
        this->SetChecked(checked);
      }

      virtual void SetChecked(bool checked)
      {
        m_avp.SetPlexViewPreferred(checked);
      }

    };

    ClearAtomCoordButton m_closeWindowButton;
    PlexViewCheckbox m_plexViewCheckbox;
    TreeViewPanel m_treeViewPanel;

    static const u32 ATOM_DRAW_SIZE = 40;

   public:
    AtomViewPanel()
      : MovablePanel(300, 100)
      , m_gridCoord(-1,-1)
      , m_inBase(false)
      , m_plexViewPreferred(true) // true since why not it does nothing if non-Plexer atom
      , m_grid(NULL)
      , m_gridPanel(NULL)
      , m_closeWindowButton(*this) // Inserts itself into us
      , m_plexViewCheckbox(*this) 
      , m_treeViewPanel()
    {
      SetName("AtomViewPanel");
      SetBackground(Drawing::GREY20);
      SetForeground(Drawing::GREY80);

      // Mon Jun  1 03:33:42 2015 NOT READY FOR PRIME TIME
      // this->Panel::Insert(&m_treeViewPanel, NULL);
    }

    bool HasGridCoord() const
    {
      return m_gridCoord.GetX() >= 0 && m_gridCoord.GetY() >= 0;
    }

    const SPoint & GetGridCoord() const
    {
      return m_gridCoord;
    }

    bool IsPlexViewPreferred()
    {
      return m_plexViewPreferred;
    }

    void SetPlexViewPreferred(bool pview)
    {
      m_plexViewPreferred = pview;
    }

    void Init()
    {
      Panel::SetDesiredSize(400, 200);
      m_closeWindowButton.Init();
      m_plexViewCheckbox.Init();
      m_treeViewPanel.Init();
    }

    virtual void PaintUpdateVisibility(Drawing& d)
    {
      this->SetVisible(HasGridCoord());
    }

    void PaintDisplayAtomicControllers(Drawing & d, T& atom,const Element<EC>* elt)
    {
      // XXX DESIGN ME
      // XXX WRITE ME
      // XXX MAKE ME WORK

      const AtomicParameters<EC> & parms = elt->GetAtomicParameters();

      const AtomicParameter<EC> * p;
      for (p = parms.GetFirstParameter(); p; p = p->GetNextParameter())
      {
        switch (p->GetType())
        {
        case VD::U32:
          {
            u32 val = 0;
            bool got = p->LoadU32(atom, val);
            LOG.Debug("u32 %s = %d (%d)", p->GetName(), (s32) val, got);
          }
          break;
        default:
          LOG.Debug("u32 %s unknown type %d", p->GetName(), p->GetType());
        }
      }
    }

    void DrawElementLogo(Drawing & d, FontAsset font, s32 xstart, s32 ystart, u32 drawSize, u32 color, const char * symbol)
    {
      d.SetForeground(Panel::GetForeground());
      d.FillCircle(xstart-1, ystart-1, drawSize + 2, drawSize + 2, (drawSize >> 1) + 1);
      d.SetForeground(color);
      d.FillCircle(xstart, ystart, drawSize, drawSize, drawSize >> 1);
      d.SetFont(font);
      d.SetForeground(Drawing::WHITE);
      d.SetBackground(Drawing::BLACK);

      /* Center on the draw_size x draw_size box */
      d.BlitBackedTextCentered(symbol,
                               SPoint(xstart, ystart), UPoint(drawSize, drawSize));

    }

    void PrintLine(Drawing & d, const u32 baseX, u32 & currentY, const u32 lineHeight, const char * zstr)
    {
      d.BlitBackedText(zstr, SPoint(baseX, currentY),
                       MakeUnsigned(d.GetTextSize(zstr)));
      currentY += lineHeight;
    }

    /**
       Highlight our border if we're the selected one
     */
    virtual void PaintBorder(Drawing & d)
    {
      u32 oldBd = GetBorder();
      if (GetGridPanel().IsSelectedAtomViewPanel(*this))
        SetBorder(Drawing::YELLOW);

      this->Super::PaintBorder(d);
      SetBorder(oldBd);
    }

    virtual void PaintComponent(Drawing& d)
    {
      this->Super::PaintComponent(d);

      //      d.DrawLineDitColor(10000, 20000, 0, 55000, Drawing::BLUE);

      if(!HasGridCoord())
      {
        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        const char* message = "No atom selected.";
        d.BlitText(message, SPoint(32, 32), MakeUnsigned(d.GetTextSize(message)));
        return;
      }

      Grid<GC>& grid = this->GetGrid();
      const Tile<EC>& tile = grid.Get00Tile();
      const ElementTable<EC>& et = tile.GetElementTable();
      const UlamClassRegistry<EC>& ucr = tile.GetUlamClassRegistry();

      const T* catom = grid.GetAtomInSite(m_inBase, m_gridCoord);
      if (!catom)
        FAIL(INCOMPLETE_CODE); // what to do?
      T atom(*catom); // Get a non-const atom so serializer can use it sigh.

      OString512 buff;
      AtomSerializer<AC> serializer(atom);

      const u32 ATOM_X_START = 20;
      const u32 ATOM_Y_START = 25;
      u32 baseX = ATOM_X_START;
      u32 currentY = 2;

      const u32 etype = atom.GetType();
      const Element<EC>* element = m_grid->LookupElement(etype);
      {
        SPoint tileInGrid, siteInTile;
        if (!grid.MapGridToTile(m_gridCoord, tileInGrid, siteInTile))
          FAIL(ILLEGAL_STATE);

        SPointSerializer tsp(tileInGrid);
        SPointSerializer ssp(siteInTile);
        buff.Reset();
        buff.Printf("%s Layer, Site %@, Tile %@",
                    m_inBase?"Base":"Event",
                    &ssp, &tsp);
        const char * str = buff.GetZString();
        d.SetFont(FONT_ASSET_LOGGER);
        d.BlitBackedText(str, SPoint(baseX + 20, currentY), MakeUnsigned(d.GetTextSize(str)));
        buff.Reset();
      }

      currentY = ATOM_Y_START;

      if (!element)
      {
        buff.Printf("Unknown type '0x%04x' in: %@", etype, &serializer);
        const char * str = buff.GetZString();
        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        d.BlitBackedText(str, SPoint(32, 32), MakeUnsigned(d.GetTextSize(str)));
        return;
      }


      bool plexText = false;
      OString1024 plexBuff;
      if (!m_inBase && m_plexViewPreferred) {
        do { // So we can break
          const UlamElement<EC> * ueltptr = element->AsUlamElement();
          if (!ueltptr) break;  // Only ulam elements can be plex

          const UlamClass<EC> * plexerclass = // Search for quark Plexer class
            ucr.GetUlamClassByMangledName("Uq_10106Plexer10"); 
          if (!plexerclass) break; // Not playing with plexer
          //          const u32 plexerclassid = plexerclass->GetRegistrationNumber();

          if (!ueltptr->internalCMethodImplementingIs(plexerclass))
            break;              // cur atom not instance of Plexer
          SPoint tileInGrid, siteInTile;
          if (!grid.MapGridToTile(m_gridCoord, tileInGrid, siteInTile))
            break; // ??

          GridPanel<GC>& gridPanel = GetGridPanel();
          TileRenderer<EC>& tilerenderer = gridPanel.GetTileRenderer();
          Tile<EC>& localtile = grid.GetTile(tileInGrid);
          EventWindow<EC> & ew = localtile.GetEventWindow();
          if (!ew.InitForEvent(siteInTile, false))
            break;
          // OK, as of here, we're thinking ew looks valid

          // Set up to call Plexer.printPlex() via localtile/ew
          DrawableSDL drawable(d);
          EventWindowRendererSDL<EC> ewrs(drawable);
          UlamContextEventSDL<EC> uc(ewrs,localtile); // A uc that has an event window and can draw

          const u32 atomSizeDit = tilerenderer.GetAtomSizeDit();
          Rect screenDitForTile = gridPanel.MapTileInGridToScreenDit(localtile,tileInGrid);
          const SPoint tileDitOrigin = screenDitForTile.GetPosition();
          const SPoint screenDitForSite = tileDitOrigin + siteInTile * atomSizeDit;

          drawable.Reset();
          drawable.SetDitOrigin(screenDitForSite);
          drawable.SetDitsPerSite(atomSizeDit);

          UlamRef<EC> ur(T::ATOM_FIRST_STATE_BIT, ueltptr->GetClassLength(), ew.GetCenterAtomBitStorage(), ueltptr, UlamRef<EC>::ELEMENTAL, uc);

          // Reconfigure logging
          ByteSink * oldlog = LOG.SetByteSink(plexBuff);
          Logger::IncludeFlags oldincludeflags = LOG.GetIncludeFlags();
          Logger::IncludeFlags newincludeflags =
            (Logger::IncludeFlags) (Logger::INCLUDE_TEXT|Logger::INCLUDE_NEWLINE);
          LOG.SetIncludeFlags(newincludeflags); // Default to newline-delimited text only here

          // We have to defend ourselves here.  Even though we know atom
          // IsSane as far as the parity of the atomic header, anything
          // might be wrong down in the user bits, and arbitrary rendering
          // code, even with no eventwindow and thus limited to just this
          // atom, can easily blow up.

          unwind_protect({
              if (oldlog) { LOG.SetByteSink(*oldlog); oldlog = 0; /* restore logging */ }
              LOG.SetIncludeFlags(oldincludeflags);
              const char * failFile = MFMThrownFromFile;
              const unsigned lineno = MFMThrownFromLineNo;
              const char * failMsg = MFMFailCodeReason(MFMThrownFailCode);
              OString256 fbuff;
              SPointSerializer ssp(siteInTile);
              fbuff.Printf("T%s@S%@: printPlex failed: %s (%s:%d)",
                          tile.GetLabel(),
                          &ssp,
                          failMsg,
                          failFile,
                          lineno);
              LOG.Message("%s",fbuff.GetZString());
            },
            {

              u32 sym = ueltptr->GetSymmetry(uc);
              ew.SetSymmetry((PointSymmetry) sym);

              const u32 PLEX_PRINTPLEX_VOWNED_INDEX = 1u; 

              // how to do an ulam virtual function call in c++
              VfuncPtr vfuncptr;
              UlamRef<EC> vfur(ur, PLEX_PRINTPLEX_VOWNED_INDEX, *plexerclass, vfuncptr); 
              typedef Ui_Ut_10111b<EC> (* Uf_919printPlex10) (const UlamContext<EC>&, UlamRef<EC>& );
              Ui_Ut_10111b<EC> plexed = ((Uf_919printPlex10) vfuncptr) (uc, vfur);
              plexText = plexed.read();

            });

          ew.SetFree(); // release the ew (without committing it)
          if (oldlog) { LOG.SetByteSink(*oldlog); oldlog = 0; /* restore logging */ }
          LOG.SetIncludeFlags(oldincludeflags);

        } while (0);
      }
      
      d.SetFont(FONT_ASSET_ELEMENT);

      const char* zstr;
      u32 curX = baseX + ATOM_DRAW_SIZE;
      const u32 LINE_X_START = baseX + 4;
      const u32 LINE_HEIGHT = AssetManager::GetFontLineSkip(d.GetFont()) - 10u;
      const u32 INDENT_AMOUNT = LINE_HEIGHT;

      if (!plexText) { // Default atom/element-level processing
        u32 staticColor = element->GetStaticColor();
        const char * sym = element->GetAtomicSymbol();
        this->DrawElementLogo(d, FONT_ASSET_ELEMENT, baseX, currentY, ATOM_DRAW_SIZE, staticColor, sym);

        const u32 DCOLOR_SIZE = ATOM_DRAW_SIZE - 8;
        u32 dcolor[3];
        dcolor[0] = staticColor;
        dcolor[1] = element->GetDynamicColor(et, ucr, atom, 1);
        dcolor[2] = element->GetDynamicColor(et, ucr, atom, 2);
        bool show1 = false, show2 = false, bothsame = false;
        if (dcolor[1] != dcolor[0]) show1 = true;
        if (dcolor[2] != dcolor[0]) show2 = true;
        if (dcolor[2] == dcolor[1]) {
          bothsame = true;
          show2 = false;
        }
        if (show1)
          {
            const char * l = bothsame ? "" : "1";
            this->DrawElementLogo(d, FONT_ASSET_ELEMENT_SMALL, curX, currentY, DCOLOR_SIZE, dcolor[1], l);
            curX += DCOLOR_SIZE;
          }
        if (show2)
          {
            this->DrawElementLogo(d, FONT_ASSET_ELEMENT_SMALL, curX, currentY, DCOLOR_SIZE, dcolor[2], "2");
            curX += DCOLOR_SIZE;
          }

        UPoint nameSize = MakeUnsigned(d.GetTextSize(element->GetName()));
        d.BlitBackedText(element->GetName(),
                         SPoint(curX,
                                currentY + ATOM_DRAW_SIZE - nameSize.GetY()), // bottom align text
                         nameSize);

        currentY += ATOM_DRAW_SIZE;

        buff.Reset();
        element->AppendDescription(atom, buff);

        zstr = buff.GetZString();

        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);

        this->PrintLine(d, LINE_X_START, currentY, LINE_HEIGHT, zstr);

        buff.Reset();
        buff.Printf("%@", &serializer);
        zstr = buff.GetZString();

        this->PrintLine(d, LINE_X_START, currentY, LINE_HEIGHT, zstr);

        const UlamElement<EC> * uelt = element->AsUlamElement();
        if (!uelt)
        {
          PaintDisplayAtomicControllers(d, atom, element);
          return;
        }

        const u32 printFlags =
          UlamClassPrintFlags::PRINT_MEMBER_NAMES |
          UlamClassPrintFlags::PRINT_MEMBER_VALUES |
          UlamClassPrintFlags::PRINT_RECURSE_QUARKS;

        buff.Reset();
        uelt->Print(ucr, buff, atom, printFlags, T::ATOM_FIRST_STATE_BIT);
      } else {
        buff.Reset();
        CharBufferByteSource cbbs = plexBuff.AsByteSource();
        buff.Copy(cbbs);
      }

      zstr = buff.GetZString();
      u32 indent = 0;
      u32 lineNum = 1;
      OString64 lineBuff;

      for (u8 ch = *zstr; ch; ch = *++zstr)
      {
        u32 oldIndent = indent;
        u32 oldLineNum = lineNum;
        if (ch == '(')
        {
          ++lineNum;
          ++indent;
        }
        else if (ch == ')')
        {
          if (indent > 0)
            --indent;
        }
        else if (ch == ',')
        {
          ++lineNum;
        }
        if (oldIndent != indent || oldLineNum != lineNum)
        {
          if (lineBuff.GetLength() > 0)
          {
            const char * line = lineBuff.GetZString();
            d.BlitBackedText(line,
                             SPoint(baseX + oldIndent * INDENT_AMOUNT, currentY + oldLineNum * LINE_HEIGHT),
                             MakeUnsigned(d.GetTextSize(line)));
            lineBuff.Reset();
          }
        } else
        {
          if (ch != '\n')
            lineBuff.Printf("%c", ch);
        }
      }
      UPoint ds = Panel::GetDesiredSize();
      u32 dy = MAX(200u,currentY + (lineNum + 2) * LINE_HEIGHT);
      if (dy != ds.GetY())
      {
        ds.SetY(dy);
        Panel::SetDesiredSize(ds.GetX(), ds.GetY());
        Panel * parent = this->GetParent();
        if (parent)
          HandleResize(parent->GetDimensions());
      }
    }

    virtual bool Handle(KeyboardEvent& kbe) ;

    virtual bool Handle(MouseButtonEvent& mbe) ;

    virtual bool Handle(MouseMotionEvent& mme) ;

    void ClearAtomCoord()
    {
      m_gridCoord = SPoint(-1,-1);
    }

    void SetAtomCoord(SPoint gridCoord, bool inBase)
    {
      m_gridCoord = gridCoord;
      m_inBase = inBase;
    }

    void SetGrid(Grid<GC>& grid)
    {
      MFM_API_ASSERT_NULL(m_grid);
      m_grid = &grid;
    }

    Grid<GC> & GetGrid()
    {
      MFM_API_ASSERT_NONNULL(m_grid);
      return *m_grid;
    }

    void SetGridPanel(GridPanel<GC>& gridPanel)
    {
      MFM_API_ASSERT_NULL(m_gridPanel);
      m_gridPanel = &gridPanel;
    }

    GridPanel<GC> & GetGridPanel()
    {
      MFM_API_ASSERT_NONNULL(m_gridPanel);
      return *m_gridPanel;
    }

  };
}

#include "AtomViewPanel.tcc"

#endif /* ATOMVIEWPANEL_H */
