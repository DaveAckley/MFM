/*                                              -*- mode:C++ -*-
  AbstractGUIDriver.h Base class for all GUI-based MFM drivers
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
  \file AbstractGUIDriver.h Base class for all GUI-based MFM drivers
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTGUIDRIVER_H
#define ABSTRACTGUIDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include <fcntl.h>     /* for O_WRONLY */
#include <unistd.h>    /* for dup */
#include "Utils.h"     /* for GetDateTimeNow */
#include "Logger.h"
#include "AssetManager.h"
#include "AbstractButton.h"
#include "AbstractCheckbox.h"
#include "AtomViewPanel.h"
#include "StatisticsPanel.h"
#include "Tile.h"
#include "DriverButtonPanel.h"
#include "GridPanel.h"
#include "TextPanel.h"
#include "ToolboxPanel.h"
#include "TeeByteSink.h"
#include "ExternalConfig.h"
#include "ExternalConfigSectionGUI.h"
#include "FileByteSource.h"
#include "Camera.h"
#include "AbstractDriver.h"
#include "VArguments.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "HelpPanel.h"
#include "MovablePanel.h"
#include "AbstractGUIDriverButtons.h"
#include "AbstractGUIDriverTools.h"
#include "GUIConstants.h"
#include "Keyboard.h"
#include <signal.h>   /* for signal, SIGTERM */

namespace MFM
{
  template<class GC>
  class AbstractGUIDriver : public AbstractDriver<GC>
  {
   private:
    typedef AbstractDriver<GC> Super;
    typedef typename Super::OurGrid OurGrid;
    typedef typename GC::EVENT_CONFIG EC;

    OString512 m_startFile;
    bool m_startPaused;
    bool m_thisUpdateIsEpoch;
    bool m_bigText;
    u32 m_thisEpochAEPS;
    bool m_captureScreenshots;
    bool m_pastFirstUpdate;
    u32 m_saveStateIndex;
    u32 m_epochSaveStateIndex;

    bool m_keyboardPaused;   // Toggled by keyboard space, ' ', SDLK_SPACE
    bool m_singleStep;       // Toggled by Step check box, 's', SDLK_SPACE
    bool m_mousePaused;      // Set if any buttons down (with modifiers?), clear if all up
    bool m_gridPaused;       // Set if keyboard || mouse paused, checked by UpdateGrid
    bool m_reinitRequested;

    bool m_renderStats;

    bool m_batchMode;
    s32 m_backupStdout;

    Camera m_camera;
    SDL_Surface* m_screen;
    Panel m_rootPanel;
    Drawing m_rootDrawing;

    u32 m_screenWidth;
    u32 m_screenHeight;

    s32 m_desiredScreenWidth;
    s32 m_desiredScreenHeight;

    bool m_screenResizable;

    ClearButton<GC> m_clearButton;
    ClearGridButton<GC> m_clearGridButton;
    NukeButton<GC> m_nukeButton;
    XRayButton<GC> m_xrayButton;
    ThinButton<GC> m_thinButton;
    GridRunCheckbox<GC> m_gridRunButton, m_gridRunMiniButton;
    GridRenderButton<GC> m_gridRenderButton;
    CacheRenderButton<GC> m_cacheRenderButton;
    GridStepCheckbox<GC> m_gridStepButton;
    FgViewButton<GC>  m_fgViewButton;
    MdViewButton<GC>  m_mdViewButton;
    BgViewButton<GC> m_bgViewButton;
    SaveButton<GC> m_saveButton;
    ScreenshotButton<GC> m_screenshotButton;
    QuitButton<GC> m_quitButton, m_quitMiniButton;
    ReloadButton<GC> m_reloadButton, m_reloadMiniButton;
    PauseTileButton<GC> m_pauseTileButton;
    BGRButton<GC> m_bgrButton;
    FGRButton<GC> m_fgrButton;
    LogButton<GC> m_logButton;
    ShowHelpButton<GC> m_showHelpButton, m_showHelpMiniButton;
    ShowToolboxButton<GC> m_showToolboxButton;
    ShowInfoBoxButton<GC> m_showInfoBoxButton;
    LoadDriverSectionButton<GC> m_loadDriverSectionButton;
    LoadGridSectionButton<GC> m_loadGridSectionButton;
    LoadGUISectionButton<GC> m_loadGUISectionButton;
    EventHistoryStrategyButton<GC> m_eventHistoryStrategyButton;

    SPoint m_grendMove;
    Keyboard m_keyboardMap;

    HelpPanel m_helpPanel;

    TileRenderer<EC> m_tileRenderer;

    GridPanel<GC> m_gridPanel;
    ReplayPanel<GC> m_replayPanel;

    ToolboxPanel<GC> m_toolboxPanel;

    GridToolAtomView<GC> m_gridToolAtomView;
    GridToolTileSelect<GC> m_gridToolTileSelect;
    GridToolPencil<GC> m_gridToolPencil;
    GridToolEraser<GC> m_gridToolEraser;
    GridToolBrush<GC> m_gridToolBrush;
    GridToolAirBrush<GC> m_gridToolAirBrush;
    GridToolXRay<GC> m_gridToolXRay;
    GridToolBucket<GC> m_gridToolBucket;
    GridToolClone<GC> m_gridToolClone;
    GridToolEvent<GC> m_gridToolEvent;

    StatisticsPanel<GC> m_statisticsPanel;
    DisplayAER<GC> m_displayAER;
    IncreaseAEPSPerFrame<GC> m_increaseAEPSPerFrame;
    DecreaseAEPSPerFrame<GC> m_decreaseAEPSPerFrame;

  public:

    const Panel & GetRootPanel() const { return m_rootPanel; }
    Panel & GetRootPanel() { return m_rootPanel; }

    const GridPanel<GC> & GetGridPanel() const { return m_gridPanel; }
    GridPanel<GC> & GetGridPanel() { return m_gridPanel; }

    void RequestReinit()
    {
      m_reinitRequested = true;
    }

    void InsertAndRegisterButton(HasGUIDriver<GC> & hd)
    {
      AbstractButton & b = hd.GetAbstractButton();
      m_buttonPanel.InsertAndPlace(b);
      RegisterButtonAccelerator(b);
      hd.SetDriver(*this);
    }

    void InsertAndRegisterMiniButton(HasGUIDriver<GC> & hd)
    {
      AbstractButton & b = hd.GetAbstractButton();
      m_miniButtonPanel.InsertAndPlace(b);
      //      RegisterButtonAccelerator(b);
      hd.SetDriver(*this);
    }

    void InsertAndRegisterGridTool(GridToolShapeUpdater<GC> & gt)
    {
      AbstractButton & ab = m_toolboxPanel.RegisterGridTool(gt);
      RegisterButtonAccelerator(ab);
    }

    void OnceOnlyTools()
    {
      m_gridPanel.InitAtomViewPanels(Super::GetGrid(), &m_gridToolAtomView);
      //      m_gridToolAtomView.SetAtomViewPanel(m_gridPanel.GetAtomViewPanel(0));
      InsertAndRegisterGridTool(m_gridToolPencil);
      InsertAndRegisterGridTool(m_gridToolEraser);
      // XXX Consider killing the brush to make room for the spark
      //      InsertAndRegisterGridTool(m_gridToolBrush);
      InsertAndRegisterGridTool(m_gridToolAirBrush);
      InsertAndRegisterGridTool(m_gridToolXRay);
      InsertAndRegisterGridTool(m_gridToolBucket);
      InsertAndRegisterGridTool(m_gridToolClone);
      InsertAndRegisterGridTool(m_gridToolEvent);
      InsertAndRegisterGridTool(m_gridToolAtomView);
      InsertAndRegisterGridTool(m_gridToolTileSelect);
    }

    void OnceOnlyButtons()
    {
      m_statisticsPanel.SetAnchor(ANCHOR_EAST);

      m_miniButtonPanel.SetOtherDriverButtonPanel(m_buttonPanel);
      m_miniButtonPanel.SetAnchor(ANCHOR_EAST);
      m_miniButtonPanel.SetVisible(true);

      InsertAndRegisterMiniButton(m_showHelpMiniButton);
      InsertAndRegisterMiniButton(m_quitMiniButton);
      InsertAndRegisterMiniButton(m_gridRunMiniButton);
      InsertAndRegisterMiniButton(m_reloadMiniButton);

      m_buttonPanel.SetOtherDriverButtonPanel(m_miniButtonPanel);
      m_buttonPanel.SetAnchor(ANCHOR_EAST);
      m_miniButtonPanel.SetVisible(false);

      InsertAndRegisterButton(m_gridRenderButton);
      InsertAndRegisterButton(m_cacheRenderButton);
      InsertAndRegisterButton(m_gridRunButton);
      InsertAndRegisterButton(m_bgrButton);
      InsertAndRegisterButton(m_fgrButton);
      InsertAndRegisterButton(m_logButton);
      InsertAndRegisterButton(m_showHelpButton);
      InsertAndRegisterButton(m_showToolboxButton);
      InsertAndRegisterButton(m_showInfoBoxButton);

      InsertAndRegisterButton(m_loadDriverSectionButton);
      InsertAndRegisterButton(m_loadGridSectionButton);
      InsertAndRegisterButton(m_loadGUISectionButton);
      InsertAndRegisterButton(m_eventHistoryStrategyButton);

      InsertAndRegisterButton(m_gridStepButton);
      InsertAndRegisterButton(m_fgViewButton);
      InsertAndRegisterButton(m_mdViewButton);
      InsertAndRegisterButton(m_bgViewButton);
      InsertAndRegisterButton(m_xrayButton);
      InsertAndRegisterButton(m_thinButton);
      InsertAndRegisterButton(m_nukeButton);
      InsertAndRegisterButton(m_screenshotButton);
      InsertAndRegisterButton(m_saveButton);
      InsertAndRegisterButton(m_reloadButton);
      InsertAndRegisterButton(m_clearButton);
      InsertAndRegisterButton(m_clearGridButton);
      InsertAndRegisterButton(m_pauseTileButton);
      InsertAndRegisterButton(m_quitButton);

      m_screenshotButton.SetScreen(m_screen);
      m_screenshotButton.SetCamera(&m_camera);

      m_buttonPanel.Insert(&m_replayPanel,0);
    }

    void Update(OurGrid& grid)
    {
      // XXX      TileRenderer& tileRenderer = m_grend.GetTileRenderer();

      if (m_singleStep)
      {
        m_keyboardPaused = false;
      }

      m_gridPaused = m_keyboardPaused || m_mousePaused;
      if (!m_gridPaused)
      {
        Super::UpdateGrid(grid);
        if (m_singleStep)
        {
          m_keyboardPaused = true;
          m_singleStep = false;
        }
      }

      // Slew camera
      //XXX      tileRenderer.Move(m_grendMove);

    }

    inline void SetLogVisible(bool vis)
    {
      m_logPanel.SetVisible(vis);
    }

    bool IsLogVisible() const
    {
      return m_logPanel.IsVisible();
    }

    void SetHelpVisible(bool vis)
    {
      m_helpPanel.SetVisible(vis);
    }

    bool IsHelpVisible() const
    {
      return m_helpPanel.IsVisible();
    }

    void SetToolboxVisible(bool vis)
    {
      m_toolboxPanel.SetVisible(vis);
      if (vis)
        m_gridPanel.SetGridTool(m_toolboxPanel.GetCurrentTool());
      else
        m_gridPanel.SetGridTool(0);
    }

    bool IsToolboxVisible() const
    {
      return m_toolboxPanel.IsVisible();
    }

    void SetInfoBoxVisible(bool vis)
    {
      m_statisticsPanel.SetVisible(vis);
    }

    bool IsInfoBoxVisible() const
    {
      return m_statisticsPanel.IsVisible();
    }

    void TogglePauseOnSelectedTiles()
    {
      m_gridPanel.TogglePauseOnSelectedTiles();
    }

    void ClearSelectedTiles()
    {
      m_gridPanel.ClearSelectedTiles();
    }

    /*
    static bool KeyHandlerToggleButtons(u32, u32, void* arg, bool)
    {
      AbstractGUIDriver & d = *(AbstractGUIDriver*) arg;
      d.m_buttonPanel.SetVisible(!d.m_buttonPanel.IsVisible());
      return true;
    }
    */

    static bool KeyHandlerQuit(u32, u32, void*)
    {
      exit(0);
    }

    void RegisterButtonAccelerator(AbstractButton & ab)
    {
      m_keyboardMap.Register(ab);
    }

    void RegisterKeyboardFunctions()
    {
      m_keyboardMap.Register(m_displayAER);
      m_keyboardMap.Register(m_buttonPanel);
      m_keyboardMap.Register(m_increaseAEPSPerFrame);
      m_keyboardMap.Register(m_decreaseAEPSPerFrame);
    }

    void KeyboardUpdate(SDL_KeyboardEvent & key, OurGrid& grid, const SPoint where)
    {
      {
        KeyboardEvent kbe(key, where);
        if (m_rootPanel.Dispatch(kbe,
                                 Rect(SPoint(),
                                      UPoint(m_screenWidth,m_screenHeight))))
          return;
      }
      m_keyboardMap.HandleEvent(key);

#if 0

      u32 speed = isShift ? CAMERA_FAST_SPEED : CAMERA_SLOW_SPEED;

      // handle semi-auto keys first, here

      /* Camera Movement*/
      if(keysym == SDLK_LEFT)
      {
        if (isPress) m_grendMove.SetX(-speed);
        else m_grendMove.SetX(0);
      }
      else if(keysym == SDLK_DOWN)
      {
        if (isPress) m_grendMove.SetY(speed);
        else m_grendMove.SetY(0);
      }
      else if(keysym == SDLK_UP)
      {
        if (isPress) m_grendMove.SetY(-speed);
        else m_grendMove.SetY(0);
      }
      else if(keysym == SDLK_RIGHT)
      {
        if (isPress) m_grendMove.SetX(speed);
        else m_grendMove.SetX(0);
      }
      else if(keysym == SDLK_ESCAPE && !anyMods)
      {
        m_gridPanel.DeselectAtomAndTile();
      }
      else if(keysym == SDLK_r && !anyMods)
      {
        m_captureScreenshots = !m_captureScreenshots;
        if (m_captureScreenshots)
          LOG.Message("Capturing screenshots every %d AEPS", this->GetAEPSPerEpoch());
        else
          LOG.Message("Not capturing screenshots");
      }
      else if(keysym == SDLK_COMMA)
      {
        Super::DecrementAEPSPerFrame(isCtrl ? 10 : 1);
      }
      else if(keysym == SDLK_PERIOD)
      {
        Super::IncrementAEPSPerFrame(isCtrl ? 10 : 1);
      }
#endif
    }

    virtual void PostUpdate()
    {
    }

    u32 GetThisEpochAEPS() const
    {
      return m_thisEpochAEPS;
    }

    virtual void DoEpochEvents(OurGrid& grid, u32 epochs, u32 epochAEPS)
    {
      Super::DoEpochEvents(grid, epochs, epochAEPS);
      m_thisUpdateIsEpoch = true;
      m_thisEpochAEPS = epochAEPS;
    }

    void LoadStartFile()
    {
      if (m_startFile.GetLength() == 0)
      {
        LOG.Debug("Start file loading suppressed");
        return;
      }
      if (!this->LoadMFS(m_startFile.GetZString()))
      {
        LOG.Error("Start file (%s) loading failed", m_startFile.GetZString());
        exit(-3);
      }
    }

    virtual void OnceOnly(VArguments& args)
    {
      /// Mux our screen logger into the LOGing path, before calling parent!
      {

        ByteSink * old = LOG.SetByteSink(m_logSplitter);
        m_logSplitter.SetSink1(old);
        m_logSplitter.SetSink2(&m_logPanel.GetByteSink());
      }

      // Let the parent 'go first'!
      Super::OnceOnly(args);

      /// Default the start file
      {
        if (m_startFile.GetLength() > 0)
        {
          if (!strcmp(m_startFile.GetZString(),"-"))  // erase '-' -- so no start file
          {
            m_startFile.Reset();
          }
        }
        else
        {

          // Accept prior rev start file if needed
          OString128 buff;
          s32 revFound = -1;
          for (s32 rev = MFM_VERSION_REV; rev >= 0; --rev)
          {
            buff.Reset();
            buff.Printf("mfs/start-%d.%d.%d.mfs", 
                        MFM_VERSION_MAJOR,
                        MFM_VERSION_MINOR,
                        rev);
            if (Utils::GetReadableResourceFile(buff.GetZString(), m_startFile))
            {
              revFound = rev;
              break;
            }
          }

          if (revFound != MFM_VERSION_REV)
          {
            if (revFound < 0)
            {
              LOG.Error("No v%d.%d.%d-compatible start file found (maybe supply --start-file ?)",
                        MFM_VERSION_MAJOR, MFM_VERSION_MINOR, MFM_VERSION_REV);
              exit(-2);
            }
            else
            {
              LOG.Warning("Using prior revision start file (%d.%d.%d, we are %d.%d.%d)",
                          MFM_VERSION_MAJOR, MFM_VERSION_MINOR, revFound,
                          MFM_VERSION_MAJOR, MFM_VERSION_MINOR, MFM_VERSION_REV);
            }
          }
          else
          {
            LOG.Message("Found start file %s", m_startFile.GetZString());
          }
        }
      }

      if (!getenv("SDL_VIDEO_ALLOW_SCREENSAVER"))          // If user isn't already messing with this
        putenv((char *) "SDL_VIDEO_ALLOW_SCREENSAVER=1");  // Old school sdl 1.2 mechanism

      if (m_batchMode)
      {
        /* Is there not some less-disgusting SDL1.2 way to do this??
        */

        if (!getenv("SDL_VIDEODRIVER"))
        {
          putenv((char *) "SDL_VIDEODRIVER=dummy");
        }
        else
        {
          fprintf(stderr,"SDL_VIDEODRIVER set in env; could not set batchmode\n");
          exit(-1);
        }

        if ( SDL_Init(0) == -1)
        {
          fprintf(stderr,"Could not initialize SDL: %s.\n", SDL_GetError());
          exit(-1);
        }

        // This done later in setscreensize..
        // SDL_Surface * s = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE|SDL_NOFRAME);

        LOG.Message("SDL initialized for batch mode");
      }
      else
      {
	u32 flags =SDL_INIT_TIMER|SDL_INIT_VIDEO;
        int ret = SDL_Init(flags);
	if (ret)
	{
	  LOG.Error("SDL_Init(0x%x) failed: %s",
		    flags,
		    SDL_GetError());
	  FAIL(ILLEGAL_STATE);
	}
      }

      TTF_Init();

      if (m_desiredScreenWidth > 0) m_screenWidth = (u32) m_desiredScreenWidth;
      if (m_desiredScreenHeight > 0) m_screenHeight = (u32) m_desiredScreenHeight;

      SetScreenSize(m_screenWidth, m_screenHeight);

      m_rootPanel.SetName("Root");
      m_gridPanel.SetBorder(Drawing::BLACK);
      m_gridPanel.SetGrid(&Super::GetGrid());
      m_gridPanel.SetTileRenderer(&m_tileRenderer);

#if 0
      m_statisticsPanel.SetGrid(&Super::GetGrid());
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetRecentAER());
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());

      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
      //      m_statisticsPanel.SetOverheadPercent(Super::GetGrid().GetAverageCacheRedundancy());
#endif
      m_statisticsPanel.SetVisible(false);

      m_rootPanel.Insert(&m_gridPanel, NULL);

      m_helpPanel.SetDimensions(1000, 500);
      m_helpPanel.SetDesiredSize(1000, 500);
      m_helpPanel.SetFont(FONT_ASSET_HELPPANEL_SMALL);
      m_helpPanel.SetAnchor(ANCHOR_EAST);
      m_helpPanel.SetAnchor(ANCHOR_SOUTH);

      m_gridPanel.Insert(&m_helpPanel, NULL);

      m_toolboxPanel.SetName("Toolbox");
      m_toolboxPanel.SetVisible(true);
      m_toolboxPanel.SetBigText(m_bigText);
      m_toolboxPanel.SetBackground(Drawing::GREY60);
      m_toolboxPanel.SetAnchor(ANCHOR_WEST);
      m_toolboxPanel.SetAnchor(ANCHOR_NORTH);

      // Put toolbox below help but above most..
      m_gridPanel.Insert(&m_toolboxPanel, NULL);
      m_toolboxPanel.AddButtons();
      m_toolboxPanel.RebuildControllers();

      m_gridPanel.Insert(&m_buttonPanel, NULL);
      m_gridPanel.Insert(&m_miniButtonPanel, NULL);

      m_gridPanel.Insert(&m_logPanel, NULL);
      m_logPanel.SetName("LogPanel");
      m_logPanel.SetVisible(false);
      m_logPanel.SetDimensions(m_screenWidth, 160);
      m_logPanel.SetDesiredSize(U32_MAX, 160);
      m_logPanel.SetAnchor(ANCHOR_SOUTH);
      m_logPanel.SetFont(FONT_ASSET_LOGGER);

      m_gridPanel.Insert(&m_statisticsPanel, NULL);

      //Getting pretty long..      m_rootPanel.Print(STDOUT);

      SDL_WM_SetCaption(MFM_VERSION_STRING_LONG, NULL);

      //m_ticksLastStopped = 0;

      m_reinitRequested = false;

      OnceOnlyButtons();

      OnceOnlyTools();

      // Again to 'set' stuff?
      SetScreenSize(m_screenWidth, m_screenHeight);

    }

  public:
    TileRenderer<EC> & GetTileRenderer() { return m_tileRenderer; }
    const TileRenderer<EC> & GetTileRenderer() const { return m_tileRenderer; }

    void SetSingleStep(bool single) { m_singleStep = single; }
    bool IsSingleStep() const { return m_singleStep; }

    void SetKeyboardPaused(bool pause) { m_keyboardPaused = pause; }
    bool IsKeyboardPaused() const { return m_keyboardPaused; }

    void SaveGridWithNextFilename()
    {
        const char* filename =
          Super::GetSimDirPathTemporary("save/%D.mfs", m_saveStateIndex++);
        Super::SaveGrid(filename);
    }

    void SaveScreenConfig(ByteSink& bs)
    {
      bs.Printf("%D%D%D%D",
                m_screenWidth, m_screenHeight,
                m_desiredScreenWidth, m_desiredScreenHeight);
      bs.Printf("%D%D%D%D",
                m_startPaused, m_thisUpdateIsEpoch,
                m_bigText, m_thisEpochAEPS);
      bs.Printf("%D%D%D",
                m_captureScreenshots, m_saveStateIndex,
                m_epochSaveStateIndex);
      bs.Printf("%D%D%D%D",
                m_keyboardPaused, m_singleStep,
                m_mousePaused, m_gridPaused);
      bs.Printf("%D%D%D%D",
                m_reinitRequested, m_renderStats,
                m_batchMode,m_screenResizable);

      bs.Printf(",%D%D%D%D",
                (u32) (1000 * this->GetAER()),
                (u32) (1000 * this->GetRecentAER()),
                (u32) (1000 * this->GetOverheadPercent()),
                this->GetAEPSPerFrame());
      bs.Print(this->GetMsSpentRunning(),Format::LXX64);
    }

    bool LoadScreenConfig(LineCountingByteSource& bs)
    {
      u32 tmp_m_screenWidth;
      u32 tmp_m_screenHeight;
      u32 tmp_m_desiredScreenWidth;
      u32 tmp_m_desiredScreenHeight;

      if (4 != bs.Scanf("%D%D%D%D",
                        &tmp_m_screenWidth,
                        &tmp_m_screenHeight,
                        &tmp_m_desiredScreenWidth,
                        &tmp_m_desiredScreenHeight))
        return false;

      u32 tmp_m_startPaused;
      u32 tmp_m_thisUpdateIsEpoch;
      u32 tmp_m_bigText;
      u32 tmp_m_thisEpochAEPS;
      if (4 != bs.Scanf("%D%D%D%D",
                        &tmp_m_startPaused,
                        &tmp_m_thisUpdateIsEpoch,
                        &tmp_m_bigText,
                        &tmp_m_thisEpochAEPS))
        return false;

      u32 tmp_m_captureScreenshots;
      u32 tmp_m_saveStateIndex;
      u32 tmp_m_epochSaveStateIndex;
      if (3 != bs.Scanf("%D%D%D",
                        &tmp_m_captureScreenshots,
                        &tmp_m_saveStateIndex,
                        &tmp_m_epochSaveStateIndex))
        return false;

      u32 tmp_m_keyboardPaused;
      u32 tmp_m_singleStep;
      u32 tmp_m_mousePaused;
      u32 tmp_m_gridPaused;
      if (4 != bs.Scanf("%D%D%D%D",
                        &tmp_m_keyboardPaused,
                        &tmp_m_singleStep,
                        &tmp_m_mousePaused,
                        &tmp_m_gridPaused))
        return false;

      u32 tmp_m_reinitRequested;
      u32 tmp_m_renderStats;
      u32 tmp_m_batchMode;
      u32 tmp_m_screenResizable;
      if (4 != bs.Scanf("%D%D%D%D",
                        &tmp_m_reinitRequested,
                        &tmp_m_renderStats,
                        &tmp_m_batchMode,
                        &tmp_m_screenResizable))
        return false;

      u32 tmp_GetAER;
      u32 tmp_GetRecentAER;
      u32 tmp_GetOverheadPercent;
      u32 tmp_GetAEPSPerFrame;
      if (5 != bs.Scanf(",%D%D%D%D",
                        &tmp_GetAER,
                        &tmp_GetRecentAER,
                        &tmp_GetOverheadPercent,
                        &tmp_GetAEPSPerFrame))
        return false;

      u64 tmp_GetMsSpentRunning;
      if (!bs.Scan(tmp_GetMsSpentRunning,Format::LXX64))
        return false;

      m_screenWidth = tmp_m_screenWidth;
      m_screenHeight = tmp_m_screenHeight;
      m_desiredScreenWidth = tmp_m_desiredScreenWidth;
      m_desiredScreenHeight = tmp_m_desiredScreenHeight;

      // Mon Jun 8 11:48:17 2015 Restoring m_startPaused seems to mean
      // we can't ever start running unless the original run that led
      // the the original save had --run
      //      m_startPaused = tmp_m_startPaused;
      m_thisUpdateIsEpoch = tmp_m_thisUpdateIsEpoch;
      m_bigText = tmp_m_bigText;
      m_thisEpochAEPS = tmp_m_thisEpochAEPS;

      // Sat Jun 13 17:14:47 2015 Restoring screenshots means we
      // ignore the -p command line argument -- at least with the
      // startup file.  I think we'd like to give priority to the
      // command line, for this kind of thing, especially since an
      // .mfz file contains a command line.
      //      m_captureScreenshots = tmp_m_captureScreenshots;
      // Sun Jun 7 18:34:44 2015 Restoring m_saveStateIndex means we
      // fail to start at 10.mfs in the new simulation save/ directory
      //      m_saveStateIndex = tmp_m_saveStateIndex;
      //      m_epochSaveStateIndex = tmp_m_epochSaveStateIndex;
      m_keyboardPaused = tmp_m_keyboardPaused;
      m_singleStep = tmp_m_singleStep;
      // Sun Jun  7 18:34:11 2015 Restoring mousePaused means EVERY use of
      // the 'Save' button leads to a paused restart!
      //      m_mousePaused = tmp_m_mousePaused;
      //      m_gridPaused = tmp_m_gridPaused;
      m_reinitRequested = tmp_m_reinitRequested;
      m_renderStats = tmp_m_renderStats;
      m_batchMode = tmp_m_batchMode;
      m_screenResizable = tmp_m_screenResizable;

      this->SetAER(tmp_GetAER/1000.0);
      this->SetRecentAER(tmp_GetRecentAER/1000.0);
      this->SetOverheadPercent(tmp_GetOverheadPercent/1000.0);
      this->SetAEPSPerFrame(tmp_GetAEPSPerFrame);
      this->SetMsSpentRunning(tmp_GetMsSpentRunning);

      m_fgViewButton.UpdateLabel();
      m_mdViewButton.UpdateLabel();
      m_bgViewButton.UpdateLabel();

      return true;
    }

    AbstractGUIDriver(u32 gridWidth, u32 gridHeight)
      : Super(gridWidth, gridHeight)
      , m_startPaused(true)
      , m_thisUpdateIsEpoch(false)
      , m_bigText(false)
      , m_thisEpochAEPS(0)
      , m_captureScreenshots(false)
      , m_pastFirstUpdate(false)
      , m_saveStateIndex(0)
      , m_epochSaveStateIndex(0)
      , m_keyboardPaused(false)
      , m_singleStep(false)
      , m_mousePaused(false)
      , m_gridPaused(false)
      , m_reinitRequested(false)
      , m_renderStats(false)
      , m_batchMode(false)
      , m_backupStdout(-1)
      , m_screen(0)
      , m_screenWidth(SCREEN_INITIAL_WIDTH)
      , m_screenHeight(SCREEN_INITIAL_HEIGHT)
      , m_desiredScreenWidth(-1)
      , m_desiredScreenHeight(-1)
      , m_screenResizable(true)
      , m_clearButton()
      , m_clearGridButton()
      , m_nukeButton()
      , m_xrayButton()
      , m_thinButton()
      , m_gridRunButton()
      , m_gridRenderButton()
      , m_cacheRenderButton()
      , m_gridStepButton()
      , m_fgViewButton()
      , m_mdViewButton()
      , m_bgViewButton()
      , m_saveButton()
      , m_screenshotButton()
      , m_quitButton()
      , m_reloadButton()
      , m_reloadMiniButton()
      , m_pauseTileButton()
      , m_bgrButton()
      , m_fgrButton()
      , m_logButton()
      , m_showHelpButton()
      , m_showToolboxButton()
      , m_showInfoBoxButton()
      , m_loadDriverSectionButton()
      , m_loadGridSectionButton()
      , m_loadGUISectionButton()
      , m_eventHistoryStrategyButton()
      , m_grendMove()
      , m_keyboardMap()
      , m_helpPanel(m_keyboardMap)
      , m_tileRenderer()
      , m_gridPanel()
      , m_replayPanel(m_gridPanel)
      , m_toolboxPanel()
      , m_gridToolAtomView(m_gridPanel, m_toolboxPanel)
      , m_gridToolTileSelect(m_gridPanel, m_toolboxPanel)
      , m_gridToolPencil(m_gridPanel, m_toolboxPanel)
      , m_gridToolEraser(m_gridPanel, m_toolboxPanel)
      , m_gridToolBrush(m_gridPanel, m_toolboxPanel)
      , m_gridToolAirBrush(m_gridPanel, m_toolboxPanel)
      , m_gridToolXRay(m_gridPanel, m_toolboxPanel)
      , m_gridToolBucket(m_gridPanel, m_toolboxPanel)
      , m_gridToolClone(m_gridPanel, m_toolboxPanel)
      , m_gridToolEvent(m_gridPanel, m_toolboxPanel)
      , m_statisticsPanel(*this)
      , m_displayAER(m_statisticsPanel)
      , m_increaseAEPSPerFrame(*this)
      , m_decreaseAEPSPerFrame(*this)
      , m_buttonPanel("ButtonPanel",false)
      , m_miniButtonPanel("miniButtonPanel",true)
      , m_externalConfigSectionGUI(AbstractDriver<GC>::GetExternalConfig(),*this)
    {
      m_startFile.Reset();
    }

    ~AbstractGUIDriver()
    { }

    virtual void ReinitUs()
    {
      /*
      m_nextEventCountsAEPS = 0;
      m_nextScreenshotAEPS = 0;
      m_nextTimeBasedDataAEPS = 0;
      */
    }

    virtual void PostReinit(VArguments& args)
    {
      HandleResize();

      m_renderStats = false;

      m_toolboxPanel.AddButtons();

      LoadStartFile();  // This had better be late enough in bootseq..

    }

    virtual void PostReinitPhysics()
    {
      for(u32 i = 0; i < AbstractDriver<GC>::m_neededElementCount; i++)
      {
        Element<EC> * elt = AbstractDriver<GC>::m_neededElements[i];
        m_statisticsPanel.DisplayStatsForElement(AbstractDriver<GC>::GetGrid(), *elt);

        const UlamElement<EC> * ue = elt->AsUlamElement();
        if (!ue || ue->GetPlaceable())
          this->RegisterToolboxElement(elt);
      }
      RegisterKeyboardFunctions();

      // Need to re-add after RegisterToolboxElement to get tooltips
      m_toolboxPanel.AddButtons();

    }

    virtual void HandleResize()
    { }

    void RegisterToolboxElement(Element<EC>* element)
    {
      m_toolboxPanel.RegisterElement(element);
    }

    /*********************************
     * GUI SPECIFIC DRIVER ARGUMENTS *
     *********************************/

    static void ConfigStatsOnlyView(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;


      driver->m_screenWidth = STATS_START_WINDOW_WIDTH;
      driver->m_screenHeight = STATS_START_WINDOW_HEIGHT;
      driver->m_statisticsPanel.SetVisible(true);
      driver->m_statisticsPanel.SetDisplayAER(driver->m_statisticsPanel.GetMaxDisplayAER());
    }

    static void ConfigBatchMode(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_batchMode = true;
    }

    static void ConfigMinimalView(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_screenWidth = MINIMAL_START_WINDOW_HEIGHT;
      driver->m_screenHeight = MINIMAL_START_WINDOW_WIDTH;
    }

    static void SetRecordScreenshotPerAEPSFromArgs(const char* fpsstr, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;
      VArguments& args = driver->m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(fpsstr, out, 0, 1000);
      if (errmsg)
      {
        args.Die("Bad FPS '%s': %s", fpsstr, errmsg);
      }

      driver->m_captureScreenshots = true;
      driver->m_statisticsPanel.SetScreenshotTargetFPS(out);
    }

    static void SetRunLabelFromArgs(const char* label, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->SetRunLabel(label);
    }

    static void SetStartFileFromArgs(const char* path, void* driverptr)
    {
      AbstractGUIDriver& driver = *((AbstractGUIDriver<GC>*)driverptr);
      VArguments& args = driver.m_varguments;
      if (driver.m_startFile.GetLength() > 0)
        args.Die("Start file specified twice (was '%s', here '%s')",
                 driver.m_startFile.GetZString(),
                 path);
      driver.m_startFile.Printf("%s",path);
      if (driver.m_startFile.HasOverflowed())
        args.Die("Start file path too long '%s'", path);
    }

    const char * GetRunLabel() const
    {
      return m_statisticsPanel.GetRunLabel().GetZString();
    }

    void SetRunLabel(const char * label)
    {
      m_statisticsPanel.SetRunLabel(label);
    }

    static void SetScreenWidthFromArgs(const char* str, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;
      VArguments& args = driver->m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(str, out, 0, 10000);
      if (errmsg)
      {
        args.Die("Bad screen width '%s': %s", str, errmsg);
      }

      driver->m_desiredScreenWidth = out;
    }

    static void SetScreenHeightFromArgs(const char* str, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;
      VArguments& args = driver->m_varguments;

      s32 out;
      const char * errmsg = AbstractDriver<GC>::GetNumberFromString(str, out, 0, 10000);
      if (errmsg)
      {
        args.Die("Bad screen height '%s': %s", str, errmsg);
      }

      driver->m_desiredScreenHeight = out;
    }

    static void SetScreenSizeFixed(const char* str, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;
      driver->m_screenResizable = false;
    }

    static void SetStartPausedFromArgs(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver& driver = *((AbstractGUIDriver*)driverptr);

      driver.m_startPaused = false;
    }

    static void DontShowHelpPanelOnStart(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver& driver = *((AbstractGUIDriver*)driverptr);

      driver.m_helpPanel.SetVisible(false);
    }

    static void SetIncreaseTextSizeFlag(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver& driver = *((AbstractGUIDriver*)driverptr);

      LOG.Debug("Increase text size.\n");

      driver.m_bigText = true;
    }

    virtual void RegisterExternalConfigSections()
    {
      Super::RegisterExternalConfigSections();
      AbstractDriver<GC>::GetExternalConfig().RegisterSection(m_externalConfigSectionGUI);
    }

    void AddDriverArguments()
    {
      Super::AddDriverArguments();

      this->RegisterArgumentSection("GUI switches");

      this->RegisterArgument("Provide no GUI at all (batch mode).",
                             "--no-gui", &ConfigBatchMode, this, false);

      this->RegisterArgument("Start with only the statistics view on the screen.",
                             "--startwithoutgrid", &ConfigStatsOnlyView, this, false);

      this->RegisterArgument("Start with a minimal-sized window.",
                             "--startminimal", &ConfigMinimalView, this, false);

      this->RegisterArgument("Request a starting screen width of ARG pixels.",
                             "--screenwidth|--sw", &SetScreenWidthFromArgs, this, true);

      this->RegisterArgument("Request a starting screen height of ARG pixels.",
                             "--screenheight|--sh", &SetScreenHeightFromArgs, this, true);

      this->RegisterArgument("Request a fixed-size (non-resizable) window.",
                             "--screenfixed|--sf", &SetScreenSizeFixed, this, false);

      this->RegisterArgument("Record a png per epoch for playback at ARG fps",
                             "-p|--pngs", &SetRecordScreenshotPerAEPSFromArgs, this, true);

      this->RegisterArgument("Simulation begins upon program startup.",
                             "--run", &SetStartPausedFromArgs, this, false);

      this->RegisterArgument("Help panel is not shown upon startup.",
                             "-n| --nohelp", &DontShowHelpPanelOnStart, this, false);

      this->RegisterArgument("Increase button and text size.",
                             "--bigtext", &SetIncreaseTextSizeFlag, this, false);

      this->RegisterArgument("Place label at top of statistics.",
                             "--label", &SetRunLabelFromArgs, this, true);

      this->RegisterArgument("Specify alternate start-up file (use - for none).",
                             "--start-file", &SetStartFileFromArgs, this, true);

    }

    DriverButtonPanel m_buttonPanel, m_miniButtonPanel;

    TextPanel<256,100> m_logPanel;  // 256 for big timestamps and FAIL paths..
    TeeByteSink m_logSplitter;

    void ResetScreenSize()
    {
      SetScreenSize(m_screenWidth, m_screenHeight);
    }

    void SetScreenSize(u32 width, u32 height)
    {
      m_screenWidth = width;
      m_screenHeight = height;
      u32 flags = SDL_SWSURFACE;
      if (m_screenResizable) flags |= SDL_RESIZABLE;
      m_screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, 32, flags);

      if (m_screen == 0)
      {
        LOG.Error("SDL_SetVideoMode(%d,%d,32,0x%x) failed: %s",
		  m_screenWidth, m_screenHeight, flags,
		  SDL_GetError());
        FAIL(ILLEGAL_STATE);
      }


      u32 gotWidth = SDL_GetVideoSurface()->w;
      u32 gotHeight = SDL_GetVideoSurface()->h;
      if (gotWidth != m_screenWidth || gotHeight != m_screenHeight)
        LOG.Message("Screen %dx%d (wanted %dx%d)",
                    gotWidth, gotHeight,
                    m_screenWidth, m_screenHeight);

      AssetManager::Initialize();

      UPoint newDimensions(width, height);

      m_gridPanel.Init();
      m_rootPanel.SetDimensions(m_screenWidth, m_screenHeight);
      m_gridPanel.SetDimensions(m_screenWidth, m_screenHeight);

      m_rootPanel.SetRenderPoint(SPoint(0, 0));
      m_rootPanel.SetForeground(Drawing::BLUE);
      m_rootPanel.SetBackground(Drawing::RED);
      m_rootPanel.HandleResize(newDimensions);

      m_rootDrawing.Reset(m_screen, FONT_ASSET_ELEMENT);

#if 0
      TileRenderer& tileRenderer = m_grend.GetTileRenderer();

      if(m_renderStats)
      {
        tileRenderer.SetDimensions(UPoint(m_screenWidth - STATS_WINDOW_WIDTH,m_screenHeight));
      }
      else
      {
        tileRenderer.SetDimensions(UPoint(m_screenWidth,m_screenHeight));
      }
#endif

      //      m_srend.SetDrawPoint(SPoint(0,0));
      //      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));

      printf("Screen resize: %d x %d\n", width, height);
      HandleResize();
    }

    void RunHelper()
    {
      m_keyboardPaused = m_startPaused;
      m_singleStep = false;

      bool running = true;

      SDL_Event event;
      s32 lastFrame = SDL_GetTicks();

      u32 mouseButtonsDown = 0;
      u32 keyboardModifiers = 0;
      ButtonPositionArray dragStartPositions;
      SPoint lastKnownMousePosition;

      while(running)
      {
        while(SDL_PollEvent(&event))
        {
          switch(event.type)
          {
          default:
            LOG.Debug("Unhandled SDL event type %d", event.type);
            break;

          case SDL_VIDEORESIZE:
            SetScreenSize(event.resize.w, event.resize.h);
            break;

          case SDL_QUIT:
            running = false;
            break;

          case SDL_MOUSEBUTTONUP:
            lastKnownMousePosition.Set(event.button.x, event.button.y);
            mouseButtonsDown &= ~(1<<(event.button.button));
            dragStartPositions[event.button.button].Set(-1,-1);
            goto mousebuttondispatch;

          case SDL_MOUSEBUTTONDOWN:
            lastKnownMousePosition.Set(event.button.x, event.button.y);
            mouseButtonsDown |= 1<<(event.button.button);
            dragStartPositions[event.button.button].Set(event.button.x,event.button.y);
            // FALL THROUGH

          mousebuttondispatch:
            {
              MouseButtonEvent mbe(keyboardModifiers, event);
              m_rootPanel.Dispatch(mbe,
                                   Rect(SPoint(),
                                        UPoint(m_screenWidth,m_screenHeight)));
            }
            break;

          case SDL_MOUSEMOTION:
          {
            lastKnownMousePosition.Set(event.motion.x, event.motion.y);
            MouseMotionEvent mme(keyboardModifiers, event,
                                 mouseButtonsDown, dragStartPositions);
            m_rootPanel.Dispatch(mme,
                                 Rect(SPoint(),
                                      UPoint(m_screenWidth,m_screenHeight)));
          }
          break;

          case SDL_KEYDOWN:
          case SDL_KEYUP:
            {
              u32 mod = 0;
              switch (event.key.keysym.sym) {
              case SDLK_LSHIFT:  mod = KMOD_SHIFT; goto mod;
              case SDLK_RSHIFT:  mod = KMOD_SHIFT; goto mod;
              case SDLK_LCTRL:   mod = KMOD_CTRL; goto mod;
              case SDLK_RCTRL:   mod = KMOD_CTRL; goto mod;
              case SDLK_LALT:    mod = KMOD_ALT; goto mod;
              case SDLK_RALT:    mod = KMOD_ALT; goto mod;
              mod:
                if (event.type == SDL_KEYDOWN)
                  keyboardModifiers |= mod;
                else
                  keyboardModifiers &= ~mod;
                break;
              default:
                KeyboardUpdate(event.key, this->GetGrid(), lastKnownMousePosition);
                break;
              }
            }
            break;
          }
        }

        m_mousePaused = mouseButtonsDown != 0 && keyboardModifiers != 0;

        /* Limit framerate */
        s32 sleepMS = (s32)
          ((1000.0 / FRAMES_PER_SECOND_IDLE) -
           (SDL_GetTicks() - lastFrame));
        if(m_gridPaused && sleepMS > 0)
        {
          SDL_Delay(sleepMS);
        }
        lastFrame = SDL_GetTicks();


        m_thisUpdateIsEpoch = false;  // Assume it's not

        if (m_captureScreenshots && !m_pastFirstUpdate)
        {
          m_rootDrawing.Clear();
          m_rootPanel.Paint(m_rootDrawing);
          const char * path = Super::GetSimDirPathTemporary("screenshot/%D-%D.png",
                                                            m_thisEpochAEPS,
                                                            0);
          m_camera.DrawSurface(m_screen,path);
        }
        Update(Super::GetGrid());

        m_pastFirstUpdate = true;

        m_rootDrawing.Clear();

        m_rootPanel.Paint(m_rootDrawing);

        if (m_thisUpdateIsEpoch)
        {
          if (m_captureScreenshots)
          {
            const char * path = Super::GetSimDirPathTemporary("vid/%D.png", m_thisEpochAEPS);

            m_camera.DrawSurface(m_screen,path);
          }
        }

        bool wantOut = this->RunHelperExiter();
        running &= wantOut;  // Don't reset running if it was already false
        SDL_Flip(m_screen);
      }

      AssetManager::Destroy();
      SDL_FreeSurface(m_screen);
      TTF_Quit();
      SDL_Quit();
    }

    ExternalConfigSectionGUI<GC>  m_externalConfigSectionGUI;

  public:
    bool IsLoadGUISection() const { return m_externalConfigSectionGUI.IsEnabled(); }
    void SetLoadGUISection(bool val) { m_externalConfigSectionGUI.SetEnabled(val); }

  };
} /* namespace MFM */

#endif /* ABSTRACTGUIDRIVER_H */
