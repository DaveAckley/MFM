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
#include "Utils.h"     /* for GetDateTimeNow */
#include "Logger.h"
#include "AssetManager.h"
#include "AbstractButton.h"
#include "AbstractCheckbox.h"
#include "AtomViewPanel.h"
#include "Tile.h"
#include "GridRenderer.h"
#include "GridPanel.h"
#include "TextPanel.h"
#include "ToolboxPanel.h"
#include "TeeByteSink.h"
#include "StatsRenderer.h"
#include "ExternalConfig.h"
#include "ExternalConfigSectionMFMS.h"
#include "FileByteSource.h"
#include "Camera.h"
#include "AbstractDriver.h"
#include "VArguments.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "HelpPanel.h"
#include "MovablePanel.h"
#include "AbstractGUIDriverButtons.h"

namespace MFM
{
#define FRAMES_PER_SECOND 100.0

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define STATS_WINDOW_WIDTH 288

#define STATS_START_WINDOW_WIDTH STATS_WINDOW_WIDTH  // Why two constants?
#define STATS_START_WINDOW_HEIGHT 120

  /* super speedy for now */
#define MINIMAL_START_WINDOW_WIDTH 1
#define MINIMAL_START_WINDOW_HEIGHT 1


#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

  template<class GC>
  class AbstractGUIDriver : public AbstractDriver<GC>
  {
   private:
    typedef AbstractDriver<GC> Super;
    typedef typename Super::OurGrid OurGrid;
    typedef typename GC::EVENT_CONFIG EC;

    bool m_startPaused;
    bool m_thisUpdateIsEpoch;
    bool m_bigText;
    u32 m_thisEpochAEPS;
    bool m_captureScreenshots;
    u32 m_saveStateIndex;
    u32 m_epochSaveStateIndex;

    bool m_keyboardPaused;   // Toggled by keyboard space, ' ', SDLK_SPACE
    bool m_singleStep;       // Toggled by Step check box, 's', SDLK_SPACE
    bool m_mousePaused;      // Set if any buttons down, clear if all up
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
    GridRunCheckbox<GC> m_gridRunButton;
    GridRenderButton<GC> m_gridRenderButton;
    HeatmapButton<GC>  m_heatmapButton;
    GridStepCheckbox<GC> m_gridStepButton;
    TileViewButton<GC> m_tileViewButton;
    SaveButton<GC> m_saveButton;
    ScreenshotButton<GC> m_screenshotButton;
    QuitButton<GC> m_quitButton;
    ReloadButton<GC> m_reloadButton;
    PauseTileButton<GC> m_pauseTileButton;
    BGRButton<GC> m_bgrButton;

    HelpPanel m_helpPanel;

    GridRenderer m_grend;
    SPoint m_grendMove;
    StatsRenderer<GC> m_srend;

  public:
    const Panel & GetRootPanel() const { return m_rootPanel; }
    Panel & GetRootPanel() { return m_rootPanel; }

    void RequestReinit()
    {
      m_reinitRequested = true;
    }

    void OnceOnlyButtons()
    {
      m_statisticsPanel.SetAnchor(ANCHOR_EAST);

      m_buttonPanel.InsertCheckbox(&m_gridRenderButton);
      m_buttonPanel.InsertCheckbox(&m_gridRunButton);
      m_buttonPanel.InsertCheckbox(&m_bgrButton);

      m_buttonPanel.InsertButton(&m_heatmapButton);
      m_buttonPanel.InsertButton(&m_gridStepButton);
      m_buttonPanel.InsertButton(&m_clearButton);
      m_buttonPanel.InsertButton(&m_clearGridButton);
      m_buttonPanel.InsertButton(&m_xrayButton);
      m_buttonPanel.InsertButton(&m_nukeButton);
      m_buttonPanel.InsertButton(&m_tileViewButton);
      m_buttonPanel.InsertButton(&m_pauseTileButton);
      m_buttonPanel.InsertButton(&m_saveButton);
      m_buttonPanel.InsertButton(&m_screenshotButton);
      m_buttonPanel.InsertButton(&m_reloadButton);
      m_buttonPanel.InsertButton(&m_quitButton);

      m_screenshotButton.SetDriver(this);
      m_screenshotButton.SetScreen(m_screen);
      m_screenshotButton.SetCamera(&m_camera);

      //m_pauseTileButton.SetGridRenderer(m_grend);

      m_gridRenderButton.SetExternalValue(m_grend.GetGridEnabledPointer());
      m_bgrButton.SetExternalValue(AbstractDriver<GC>::GetGrid().
                                   GetBackgroundRadiationEnabledPointer());
      m_gridRunButton.SetExternalValue(&m_keyboardPaused);

      m_buttonPanel.SetButtonDrivers(*this);
      m_buttonPanel.InsertButtons();

      m_buttonPanel.SetAnchor(ANCHOR_SOUTH);
      m_buttonPanel.SetAnchor(ANCHOR_EAST);
    }

    void Update(OurGrid& grid)
    {

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
#if 0  // XXX Shouldn't need this; caller is limiting frame rate
      else
      {
        //        SleepMsec(33); // 33 ms ~= 30 fps idle
        SleepMsec(100); // 100 ms ~= 10 fps idle
      }
#endif
      // Slew camera
      m_grend.Move(m_grendMove);

    }

    inline void ToggleStatsView()
    {
      m_statisticsPanel.ToggleVisibility();
      m_grend.SetDimensions(Point<u32>(m_screenWidth - (m_renderStats ? STATS_WINDOW_WIDTH : 0)
                                       , m_screenHeight));
    }

    inline void ToggleLogView()
    {
      m_logPanel.ToggleVisibility();
    }

    inline void ToggleToolbox()
    {
      m_toolboxPanel.ToggleVisibility();
      m_gridPanel.SetPaintingEnabled(m_toolboxPanel.IsVisible());
    }

    void KeyboardUpdate(SDL_KeyboardEvent & key, OurGrid& grid)
    {
      bool isPress = key.type == SDL_KEYDOWN;
      SDLMod mod = key.keysym.mod;
      u32 keysym = key.keysym.sym;
      bool isCtrl = (mod & KMOD_CTRL);
      bool isShift = (mod & KMOD_SHIFT);
      bool isAlt = (mod & KMOD_ALT);
      bool anyMods = isCtrl || isShift || isAlt;

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

      if (!isPress) return;  // From here on only keypresses matter

      if(keysym == SDLK_q && isCtrl)
      {
        exit(0);
      }

      if(keysym == SDLK_a && !anyMods)
      {
        m_srend.SetDisplayAER(1 + m_srend.GetDisplayAER());
      }
      else if(keysym == SDLK_i && !anyMods)
      {
        ToggleStatsView();
      }
      else if(keysym == SDLK_g && !anyMods)
      {
        m_grend.ToggleGrid();
      }
      else if(keysym == SDLK_b && !anyMods)
      {
        m_buttonPanel.ToggleVisibility();
      }
      else if(keysym == SDLK_m && !anyMods)
      {
        m_grend.ToggleMemDraw();
      }
      else if(keysym ==SDLK_k && !anyMods)
      {
        m_grend.ToggleDataHeatmap();
      }
      else if(keysym == SDLK_h && !anyMods)
      {
        m_helpPanel.ToggleVisibility();
      }
      else if(keysym == SDLK_l && !anyMods)
      {
        ToggleLogView();
      }
      else if(keysym == SDLK_p && !anyMods)
      {
        m_grend.ToggleTileSeparation();
      }
      else if(keysym == SDLK_v && !anyMods)
      {
        m_gridPanel.ToggleAtomViewPanel();
      }
      else if(keysym == SDLK_o && !anyMods)
      {
        m_gridPanel.ToggleDrawAtomsAsSquares();
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
      else if(keysym == SDLK_t && !anyMods)
      {
        ToggleToolbox();
      }
      else if(keysym == SDLK_SPACE && !anyMods)
      {
        m_keyboardPaused = !m_keyboardPaused;
      }
      else if(keysym == SDLK_s && !anyMods)
      {
        m_singleStep = true;
        m_keyboardPaused = false;
      }
      else if(keysym == SDLK_COMMA)
      {
        Super::DecrementAEPSPerFrame(isCtrl ? 10 : 1);
      }
      else if(keysym == SDLK_PERIOD)
      {
        Super::IncrementAEPSPerFrame(isCtrl ? 10 : 1);
      }
    }

    GridRenderer & GetGridRenderer()
    {
      return m_grend;
    }

    virtual void PostUpdate()
    {
      /* Update the stats renderer */
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetRecentAER());  // Use backwards averaged value
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());
      m_statisticsPanel.SetCurrentAEPSPerEpoch(this->GetAEPSPerEpoch());
      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
      //      m_statisticsPanel.SetOverheadPercent(Super::GetGrid().GetAverageCacheRedundancy());
    }

    virtual void DoEpochEvents(OurGrid& grid, u32 epochs, u32 epochAEPS)
    {
      Super::DoEpochEvents(grid, epochs, epochAEPS);
      m_thisUpdateIsEpoch = true;
      m_thisEpochAEPS = epochAEPS;
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

      /*
      if (m_countOfScreenshotsPerRate > 0) {
        m_maxRecordScreenshotPerAEPS = m_recordScreenshotPerAEPS;
        m_recordScreenshotPerAEPS = 1;
        m_countOfScreenshotsAtThisAEPS = 0;
      }
      */

      if (!getenv("SDL_VIDEO_ALLOW_SCREENSAVER"))          // If user isn't already messing with this
        putenv((char *) "SDL_VIDEO_ALLOW_SCREENSAVER=1");  // Old school sdl 1.2 mechanism

      if (m_batchMode)
      {
        /* Special disgusting hacks to run SDL in ncurses, but then
           suppress the ncurses output, so that we can run what looks
           like a 'GUI' to SDL with no actual display anywhere.  Is
           there not some less-disgusting SDL1.2 way to do this??
        */

        // Step 1: Hack environmental variables to pick driver
        if (!getenv("CACA_DRIVER") && !getenv("SDL_VIDEODRIVER"))
        {
          putenv((char *) "CACA_DRIVER=ncurses");
          putenv((char *) "SDL_VIDEODRIVER=caca");
        }
        else
        {
          fprintf(stderr,"CACA_DRIVER and/or SDL_VIDEODRIVER set in env; could not set batchmode\n");
          exit(-1);
        }

        // Step 2: Temporarily dump stdout

        s32 newdesc;
        fflush(stdout);
        m_backupStdout = dup(1);
        newdesc = open("/dev/null", O_WRONLY);
        dup2(newdesc, 1);
        close(newdesc);

        // Step 3: Initialize SDL
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
        SDL_Init(SDL_INIT_EVERYTHING);
      }

      TTF_Init();

      if (m_desiredScreenWidth > 0) m_screenWidth = (u32) m_desiredScreenWidth;
      if (m_desiredScreenHeight > 0) m_screenHeight = (u32) m_desiredScreenHeight;

      SetScreenSize(m_screenWidth, m_screenHeight);

      m_rootPanel.SetName("Root");
      m_gridPanel.SetBorder(Drawing::BLACK);
      m_gridPanel.SetGridRenderer(&m_grend);
      m_gridPanel.SetToolboxPanel(&m_toolboxPanel);
      m_gridPanel.SetGrid(&Super::GetGrid());

      m_statisticsPanel.SetStatsRenderer(&m_srend);
      m_statisticsPanel.SetGrid(&Super::GetGrid());
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetRecentAER());
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());

  //      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
      m_statisticsPanel.SetOverheadPercent(Super::GetGrid().GetAverageCacheRedundancy());
      m_statisticsPanel.SetVisibility(false);

      m_rootPanel.Insert(&m_gridPanel, NULL);
      m_gridPanel.Insert(&m_statisticsPanel, NULL);
      m_statisticsPanel.Insert(&m_buttonPanel, NULL);
      m_buttonPanel.SetVisibility(true);

      m_gridPanel.Insert(&m_logPanel, NULL);
      m_logPanel.SetName("LogPanel");
      m_logPanel.SetVisibility(false);
      m_logPanel.SetDimensions(m_screenWidth, 160);
      m_logPanel.SetDesiredSize(U32_MAX, 160);
      m_logPanel.SetAnchor(ANCHOR_SOUTH);
      m_logPanel.SetFont(FONT_ASSET_LOGGER);

      m_toolboxPanel.SetName("Toolbox");
      m_toolboxPanel.SetVisibility(false);
      m_toolboxPanel.SetBigText(m_bigText);
      m_toolboxPanel.SetBackground(Drawing::GREY60);
      m_toolboxPanel.SetAnchor(ANCHOR_WEST);
      m_toolboxPanel.SetAnchor(ANCHOR_NORTH);
      m_gridPanel.Insert(&m_toolboxPanel, NULL);
      m_toolboxPanel.RebuildControllers();

      m_helpPanel.SetName("Help");
      m_helpPanel.SetDimensions(m_screenWidth / 3, m_screenHeight);
      m_helpPanel.SetAnchor(ANCHOR_WEST);
      m_gridPanel.Insert(&m_helpPanel, NULL);

      m_rootPanel.Print(STDOUT);

      m_srend.OnceOnly();

      SDL_WM_SetCaption(MFM_VERSION_STRING_LONG, NULL);

      //m_ticksLastStopped = 0;

      m_reinitRequested = false;

      OnceOnlyButtons();

      // Again to 'set' stuff?
      SetScreenSize(m_screenWidth, m_screenHeight);

      if (m_batchMode)
      {
        /* Unhook our secret wires, since hopefully the ncurses
           initialization is done by now, and we won't actually draw
           anything on it later anyway?
        */
        if (m_backupStdout >= 0)
        {
          fflush(stdout);
          dup2(m_backupStdout, 1);
          close(m_backupStdout);
          m_backupStdout = -1;
        }
      }

    }

  public:
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

      m_screenWidth = tmp_m_screenWidth;
      m_screenHeight = tmp_m_screenHeight;
      m_desiredScreenWidth = tmp_m_desiredScreenWidth;
      m_desiredScreenHeight = tmp_m_desiredScreenHeight;
      m_startPaused = tmp_m_startPaused;
      m_thisUpdateIsEpoch = tmp_m_thisUpdateIsEpoch;
      m_bigText = tmp_m_bigText;
      m_thisEpochAEPS = tmp_m_thisEpochAEPS;
      m_captureScreenshots = tmp_m_captureScreenshots;
      //      m_saveStateIndex = tmp_m_saveStateIndex;
      //      m_epochSaveStateIndex = tmp_m_epochSaveStateIndex;
      m_keyboardPaused = tmp_m_keyboardPaused;
      m_singleStep = tmp_m_singleStep;
      m_mousePaused = tmp_m_mousePaused;
      m_gridPaused = tmp_m_gridPaused;
      m_reinitRequested = tmp_m_reinitRequested;
      m_renderStats = tmp_m_renderStats;
      m_batchMode = tmp_m_batchMode;
      m_screenResizable = tmp_m_screenResizable;

      m_tileViewButton.UpdateLabel();
      m_heatmapButton.UpdateLabel();

      return true;
    }

    AbstractGUIDriver(u32 gridWidth, u32 gridHeight)
      : Super(gridWidth, gridHeight)
      , m_startPaused(true)
      , m_thisUpdateIsEpoch(false)
      , m_bigText(false)
      , m_thisEpochAEPS(0)
      , m_captureScreenshots(false)
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
      , m_heatmapButton(m_gridPanel)
      , m_selectedTool(TOOL_SELECTOR)
      , m_toolboxPanel(&m_selectedTool)
      , m_buttonPanel()
      , m_externalConfigSectionMFMS(AbstractDriver<GC>::GetExternalConfig(),*this)
    { }

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
    }

    virtual void PostReinitPhysics()
    {
      for(u32 i = 0; i < AbstractDriver<GC>::m_neededElementCount; i++)
      {
        Element<EC> * elt = AbstractDriver<GC>::m_neededElements[i];
        GetStatsRenderer().DisplayStatsForElement(AbstractDriver<GC>::GetGrid(), *elt);

        const UlamElement<EC> * ue = elt->AsUlamElement();
        if (!ue || ue->GetPlaceable())
          this->RegisterToolboxElement(elt);
      }
    }

    virtual void HandleResize()
    { }

    void ToggleTileView()
    {
      m_grend.ToggleMemDraw();
    }

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
      driver->ToggleStatsView();
      driver->m_srend.SetDisplayAER(driver->m_srend.GetMaxDisplayAER());
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
      driver->m_srend.SetScreenshotTargetFPS(out);
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

      driver.m_helpPanel.SetVisibility(false);
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
      AbstractDriver<GC>::GetExternalConfig().RegisterSection(m_externalConfigSectionMFMS);
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
    }

    EditingTool m_selectedTool;

    GridPanel<GC> m_gridPanel;

    ToolboxPanel<EC> m_toolboxPanel;

    class StatisticsPanel : public MovablePanel
    {
      StatsRenderer<GC>* m_srend;
      OurGrid* m_mainGrid;
      double m_AEPS;
      double m_AER;
      double m_overheadPercent;
      u32 m_aepsPerFrame;
      u32 m_currentAEPSPerEpoch;

     public:
      StatisticsPanel() : m_srend(NULL)
      {
        SetName("StatisticsPanel");
        SetDimensions(STATS_START_WINDOW_WIDTH,
                      SCREEN_INITIAL_HEIGHT);
        SetDesiredSize(STATS_START_WINDOW_WIDTH,
                      U32_MAX);
        SetRenderPoint(SPoint(100000, 0));
        SetForeground(Drawing::WHITE);
        SetBackground(Drawing::DARK_PURPLE);
        m_AEPS = m_AER = 0.0;
        m_currentAEPSPerEpoch = 0;
        m_aepsPerFrame = 0;
        m_overheadPercent = 0.0;
      }

      void SetStatsRenderer(StatsRenderer<GC>* srend)
      {
        m_srend = srend;
      }

      void SetGrid(OurGrid* mainGrid)
      {
        m_mainGrid = mainGrid;
      }

      void SetAEPS(double aeps)
      {
        m_AEPS = aeps;
      }

      void SetCurrentAEPSPerEpoch(u32 aepsPerEpoch)
      {
        m_currentAEPSPerEpoch = aepsPerEpoch;
      }

      void SetAER(double aer)
      {
        m_AER = aer;
      }

      void SetOverheadPercent(double overheadPercent)
      {
        m_overheadPercent = overheadPercent;
      }

      void SetAEPSPerFrame(u32 apf)
      {
        m_aepsPerFrame = apf;
      }

      bool LoadDetails(ByteSource & source)
      {
        u32 tmp_m_AEPS;
        u32 tmp_m_AER;
        u32 tmp_m_overheadPercent;
        u32 tmp_m_aepsPerFrame;
        u32 tmp_m_currentAEPSPerEpoch;
        if (6 != source.Scanf(",%D%D%D%D%D",
                              &tmp_m_AEPS,
                              &tmp_m_AER,
                              &tmp_m_overheadPercent,
                              &tmp_m_aepsPerFrame,
                              &tmp_m_currentAEPSPerEpoch))
          return false;

        MFM_API_ASSERT_NONNULL(m_srend);
        if (!m_srend->LoadDetails(source)) return false;

        m_AEPS = tmp_m_AEPS / 10.0;
        m_AER = tmp_m_AER / 100.0;
        m_overheadPercent = tmp_m_overheadPercent / 1000.0;
        m_aepsPerFrame = tmp_m_aepsPerFrame;
        m_currentAEPSPerEpoch = tmp_m_currentAEPSPerEpoch;
        return true;
      }

      void SaveDetails(ByteSink & sink) const
      {
        sink.Printf(",%D%D%D%D%D",
                    (u32)(10*m_AEPS),
                    (u32)(100*m_AER),
                    (u32)(1000*m_overheadPercent),
                    m_aepsPerFrame,
                    m_currentAEPSPerEpoch);

        MFM_API_ASSERT_NONNULL(m_srend);
        m_srend->SaveDetails(sink);
      }

    protected:
      virtual void PaintComponent(Drawing& drawing)
      {
        this->Panel::PaintComponent(drawing);
        m_srend->RenderGridStatistics(drawing, *m_mainGrid,
                                      m_AEPS, m_AER, m_aepsPerFrame,
                                      m_overheadPercent, false,
                                      m_currentAEPSPerEpoch);
      }

      virtual void PaintBorder(Drawing & config)
      { /* No border please */ }

      virtual bool HandlePostDrag(MouseButtonEvent& mbe)
      {
        return true;  /* Eat the event to keep the grid from taking it */
      }
    }m_statisticsPanel;

    struct ButtonPanel : public Panel
    {
      static const u32 MAX_BUTTONS = 16;
      static const u32 CHECKBOX_SPACING_HEIGHT = 32;
      static const u32 BUTTON_SPACING_HEIGHT = 34;
      static const u32 BUTTON_HEIGHT = 30;
      static const u32 BUTTON_WIDTH = STATS_START_WINDOW_WIDTH;

      virtual void PaintBorder(Drawing & config)
      { /* No border please */ }

      ButtonPanel() :
        m_checkboxCount(0),
        m_buttonCount(0)
      {
        SetName("ButtonPanel");
        SetDimensions(STATS_START_WINDOW_WIDTH,
                      SCREEN_INITIAL_HEIGHT / 2);
        SetDesiredSize(U32_MAX, SCREEN_INITIAL_HEIGHT / 2);
        SetAnchor(ANCHOR_SOUTH);
        /*
        SetRenderPoint(SPoint(0,
                              SCREEN_INITIAL_HEIGHT / 2));
        */
        SetForeground(Drawing::WHITE);
        SetBackground(Drawing::DARK_PURPLE);
        SetFont(FONT_ASSET_ELEMENT);
      }

      void InsertCheckbox(AbstractGridCheckbox<GC>* checkbox)
      {
        if(m_checkboxCount >= MAX_BUTTONS)
        {
          FAIL(OUT_OF_ROOM);
        }

        m_checkboxes[m_checkboxCount++] = checkbox;

        Pack();
      }

      void InsertButton(AbstractGridButton<GC>* button)
      {
        if(m_buttonCount >= MAX_BUTTONS)
        {
          FAIL(OUT_OF_ROOM);
        }

        m_buttons[m_buttonCount++] = button;

        Pack();
      }

      void SetButtonDrivers(AbstractGUIDriver& driver)
      {
        for(u32 i = 0; i < m_checkboxCount; i++)
        {
          m_checkboxes[i]->SetDriver(driver);
        }

        for(u32 i = 0; i < m_buttonCount; i++)
        {
          m_buttons[i]->SetDriver(driver);
        }
      }

      void InsertButtons()
      {
        for(u32 i = 0; i < m_checkboxCount; i++)
        {
          Panel::Insert(m_checkboxes[i], NULL);
        }

        for(u32 i = 0; i < m_buttonCount; i++)
        {
          Panel::Insert(m_buttons[i], NULL);
          m_buttons[i]->SetDimensions(SPoint(BUTTON_WIDTH, BUTTON_HEIGHT));
        }
      }

     private:
      AbstractGridCheckbox<GC>* m_checkboxes[MAX_BUTTONS];
      AbstractGridButton<GC>*   m_buttons[MAX_BUTTONS];

      u32 m_checkboxCount;
      u32 m_buttonCount;

      void Pack()
      {
        for(u32 i = 0; i < m_checkboxCount; i++)
        {
          m_checkboxes[i]->SetRenderPoint(SPoint(2, i * CHECKBOX_SPACING_HEIGHT));
        }

        for(u32 i = 0; i < m_buttonCount; i++)
        {
          m_buttons[i]->SetRenderPoint(SPoint(2, m_checkboxCount * CHECKBOX_SPACING_HEIGHT +
                                                 i * BUTTON_SPACING_HEIGHT));
        }

        Panel::SetDimensions(STATS_WINDOW_WIDTH,
                             m_checkboxCount * CHECKBOX_SPACING_HEIGHT +
                             m_buttonCount * BUTTON_SPACING_HEIGHT);
      }

    } m_buttonPanel;

    TextPanel<200,100> m_logPanel;  // 200 for big timestamps and such..
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

      u32 gotWidth = SDL_GetVideoSurface()->w;
      u32 gotHeight = SDL_GetVideoSurface()->h;
      if (gotWidth != m_screenWidth || gotHeight != m_screenHeight)
        LOG.Message("Screen %dx%d (wanted %dx%d)",
                    gotWidth, gotHeight,
                    m_screenWidth, m_screenHeight);

      AssetManager::Initialize();

      if (m_screen == 0)
      {
        FAIL(ILLEGAL_STATE);
      }

      UPoint newDimensions(width, height);

      m_gridPanel.Init();
      m_rootPanel.SetDimensions(m_screenWidth, m_screenHeight);
      m_gridPanel.SetDimensions(m_screenWidth, m_screenHeight);

      m_rootPanel.SetRenderPoint(SPoint(0, 0));
      m_rootPanel.SetForeground(Drawing::BLUE);
      m_rootPanel.SetBackground(Drawing::RED);
      m_rootPanel.HandleResize(newDimensions);

      m_rootDrawing.Reset(m_screen, FONT_ASSET_ELEMENT);

      if(m_renderStats)
      {
        m_grend.SetDimensions(UPoint(m_screenWidth - STATS_WINDOW_WIDTH,m_screenHeight));
      }
      else
      {
        m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));
      }

      m_srend.SetDrawPoint(SPoint(0,0));
      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));

      printf("Screen resize: %d x %d\n", width, height);
      HandleResize();
    }

    StatsRenderer<GC> & GetStatsRenderer()
    {
      return m_srend;
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

      while(running)
      {
        while(SDL_PollEvent(&event))
        {
          switch(event.type)
          {
          case SDL_VIDEORESIZE:
            SetScreenSize(event.resize.w, event.resize.h);
            break;

          case SDL_QUIT:
            running = false;
            break;

          case SDL_MOUSEBUTTONUP:
            mouseButtonsDown &= ~(1<<(event.button.button));
            dragStartPositions[event.button.button].Set(-1,-1);
            goto mousebuttondispatch;

          case SDL_MOUSEBUTTONDOWN:
            mouseButtonsDown |= 1<<(event.button.button);
            dragStartPositions[event.button.button].Set(event.button.x,event.button.y);
            // FALL THROUGH

          mousebuttondispatch:
            {
              MouseButtonEvent mbe(keyboardModifiers, event, m_selectedTool);
              m_rootPanel.Dispatch(mbe,
                                   Rect(SPoint(),
                                        UPoint(m_screenWidth,m_screenHeight)));
            }
            break;

          case SDL_MOUSEMOTION:
          {
            MouseMotionEvent mme(keyboardModifiers, event,
                                 mouseButtonsDown, dragStartPositions, m_selectedTool);
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
              case SDLK_LSHIFT:  mod = KMOD_LSHIFT; goto mod;
              case SDLK_RSHIFT:  mod = KMOD_RSHIFT; goto mod;
              case SDLK_LCTRL:   mod = KMOD_LCTRL; goto mod;
              case SDLK_RCTRL:   mod = KMOD_RCTRL; goto mod;
              case SDLK_LALT:    mod = KMOD_LALT; goto mod;
              case SDLK_RALT:    mod = KMOD_RALT; goto mod;
              mod:
                if (event.type == SDL_KEYDOWN)
                  keyboardModifiers |= mod;
                else
                  keyboardModifiers &= ~mod;
                break;
              default:
                KeyboardUpdate(event.key, this->GetGrid());
                break;
              }
            }
            break;
          }
        }

        m_mousePaused = mouseButtonsDown != 0;

        /* Limit framerate */
        s32 sleepMS = (s32)
          ((1000.0 / FRAMES_PER_SECOND) -
           (SDL_GetTicks() - lastFrame));
        if(sleepMS > 0)
        {
          SDL_Delay(sleepMS);
        }
        lastFrame = SDL_GetTicks();


        m_thisUpdateIsEpoch = false;  // Assume it's not

        Update(Super::GetGrid());

        m_rootDrawing.Clear();

        m_rootPanel.Paint(m_rootDrawing);

        if (m_thisUpdateIsEpoch)
        {
          if (m_captureScreenshots)
          {
            const char * path = Super::GetSimDirPathTemporary("vid/%010d.png", m_thisEpochAEPS);

            m_camera.DrawSurface(m_screen,path);
          }
        }

        running = this->RunHelperExiter();
        SDL_Flip(m_screen);
      }

      AssetManager::Destroy();
      SDL_FreeSurface(m_screen);
      TTF_Quit();
      SDL_Quit();
    }

    ExternalConfigSectionMFMS<GC>  m_externalConfigSectionMFMS;
  };
} /* namespace MFM */

#endif /* ABSTRACTGUIDRIVER_H */
