#include "AbstractGUIDriver.h"

namespace MFM
{
  template <class GC>
  ClearButton<GC>::ClearButton() : AbstractGridButton<GC>("Clear Tile")
  {
    AbstractButton::SetName("ClearButton");
    Panel::SetDimensions(200,40);
  }

  template <class GC>
  void ClearButton<GC>::OnClick(u8 button)
  {
    GridRenderer & grend = this->GetGridRenderer();

    const SPoint selTile = grend.GetSelectedTile();
    if(selTile.GetX() >= 0 && selTile.GetX() < this->GetGrid().GetWidth() &&
       selTile.GetY() >= 0 && selTile.GetY() < this->GetGrid().GetHeight())
    {
      this->GetGrid().EmptyTile(grend.GetSelectedTile());
    }
  }

  template <class GC>
  ClearGridButton<GC>::ClearGridButton() : AbstractGridButton<GC>("Clear Grid")
  {
    AbstractButton::SetName("ClearGridButton");
    Panel::SetDimensions(200,40);
  }

  template <class GC>
  void ClearGridButton<GC>::OnClick(u8 button)
  {
    this->GetGrid().Clear();
  }

  template <class GC>
  NukeButton<GC>::NukeButton() : AbstractGridButton<GC>("Nuke")
  {
    AbstractButton::SetName("NukeButton");
    Panel::SetDimensions(200,40);
  }

  template <class GC>
  void NukeButton<GC>::OnClick(u8 button)
  {
    this->GetGrid().RandomNuke();
  }

  template <class GC>
  XRayButton<GC>::XRayButton() : AbstractGridButton<GC>("XRay")
  {
    AbstractButton::SetName("XRayButton");
    Panel::SetDimensions(200,40);
  }

  template <class GC>
  void XRayButton<GC>::OnClick(u8 button)
  {
    this->GetGrid().XRay();
  }

  template <class GC>
  GridRunCheckbox<GC>::GridRunCheckbox() : AbstractGridCheckboxExternal<GC>("Pause")
  {
    AbstractButton::SetName("GridRunButton");
    Panel::SetDimensions(200, 25);
  }

  template <class GC>
  void GridRunCheckbox<GC>::OnCheck(bool value)
  { }

  template <class GC>
  GridRenderButton<GC>::GridRenderButton() : AbstractGridCheckboxExternal<GC>("Grid")
  {
    AbstractButton::SetName("GridRenderButton");
    Panel::SetDimensions(200,25);
  }

  template <class GC>
  void GridRenderButton<GC>::OnCheck(bool value)
  { }

  template <class GC>
  FgViewButton<GC>::FgViewButton()
    : AbstractGridButton<GC>("Front: Element")
  {
    AbstractButton::SetName("FgViewButton");
    Panel::SetDimensions(200,25);
  }

  template <class GC>
  void FgViewButton<GC>::UpdateLabel()
  {
    OString32 fgText;
    fgText.Printf("Front: %s",
                  this->GetGridRenderer().GetDrawForegroundTypeName());

     AbstractButton::SetText(fgText.GetZString());
  }

  template <class GC>
  void FgViewButton<GC>::OnClick(u8 button)
  {
    this->GetGridRenderer().NextDrawForegroundType();
    UpdateLabel();
  }

  template <class GC>
  GridStepCheckbox<GC>::GridStepCheckbox() : AbstractGridButton<GC>("Step")
  {
    AbstractButton::SetName("GridStepButton");
    Panel::SetDimensions(200, 40);
  }

  template <class GC>
  void GridStepCheckbox<GC>::OnClick(u8 button)
  {
    this->GetDriver().SetSingleStep(true);
    this->GetDriver().SetKeyboardPaused(false);
  }

  template <class GC>
  BgViewButton<GC>::BgViewButton() : AbstractGridButton<GC>("Back: Light tile")
  {
    AbstractButton::SetName("BgViewButton");
    Panel::SetDimensions(200,40);
  }

  template <class GC>
  void BgViewButton<GC>::UpdateLabel()
  {
    OString32 label;
    label.Printf("Back: %s",
                 this->GetGridRenderer().GetDrawBackgroundTypeName());
    AbstractButton::SetText(label.GetZString());
  }

  template <class GC>
  void BgViewButton<GC>::OnClick(u8 button)
  {
    this->GetGridRenderer().NextDrawBackgroundType();
    UpdateLabel();
  }

  template <class GC>
  SaveButton<GC>::SaveButton() : AbstractGridButton<GC>("Save")
  {
    AbstractButton::SetName("SaveButton");
    Panel::SetDimensions(200, 40);
  }

  template <class GC>
  void SaveButton<GC>::OnClick(u8 button)
  {
    this->GetDriver().SaveGridWithNextFilename();
  }

  template <class GC>
  ScreenshotButton<GC>::ScreenshotButton()
    : AbstractGridButton<GC>("Screenshot")
    , m_currentScreenshot(0)
    , m_screen(0)
    , m_camera(0)
  {
    AbstractButton::SetName("Screenshot");
    Panel::SetDimensions(200, 40);
  }

  template <class GC>
  void ScreenshotButton<GC>::SetScreen(SDL_Surface* screen)
  {
    m_screen = screen;
  }

  template <class GC>
  void ScreenshotButton<GC>::SetCamera(Camera* camera)
  {
    m_camera = camera;
  }

  template <class GC>
  void ScreenshotButton<GC>::SetDriver(AbstractDriver<GC>* driver)
  {
    m_driver = driver;
  }

  template <class GC>
  void ScreenshotButton<GC>::OnClick(u8 button)
  {
    if(m_driver && m_screen && m_camera)
    {
      const char * path = m_driver->GetSimDirPathTemporary("screenshot/%010d.png",
                                                           ++m_currentScreenshot);
      m_camera->DrawSurface(m_screen, path);

      LOG.Message("Screenshot saved at %s", path);
    }
    else
    {
      LOG.Debug("Screenshot not saved; screen is null. Use SetScreen() first.");
    }
  }

  template<class GC>
  QuitButton<GC>::QuitButton() : AbstractGridButton<GC>("Quit")
  {
    AbstractButton::SetName("QuitButton");
    Panel::SetDimensions(200,40);
  }

  template<class GC>
  void QuitButton<GC>::OnClick(u8 button)
  {
    exit(0);
  }

  template<class GC>
  ReloadButton<GC>::ReloadButton() : AbstractGridButton<GC>("Reload")
  {
    AbstractButton::SetName("ReloadButton");
    Panel::SetDimensions(200,40);
  }

  template<class GC>
  void ReloadButton<GC>::OnClick(u8 button)
  {
    this->GetDriver().LoadFromConfigurationPath();
  }

  template<class GC>
  PauseTileButton<GC>::PauseTileButton() : AbstractGridButton<GC>("Pause Tile")
  {
    AbstractButton::SetName("PauseTileButton");
    Panel::SetDimensions(200, 40);
  }

  template<class GC>
  void PauseTileButton<GC>::OnClick(u8 button)
  {
    SPoint selectedTile = this->GetGridRenderer().GetSelectedTile();

    if(selectedTile.GetX() >= 0 && selectedTile.GetY() >= 0)
    {
      bool isEnabled = this->GetDriver().GetGrid().IsTileEnabled(selectedTile);
      this->GetDriver().GetGrid().SetTileEnabled(selectedTile, !isEnabled);
    }
  }

  template<class GC>
  BGRButton<GC>::BGRButton() : AbstractGridCheckboxExternal<GC>("Writes fault")
  {
    AbstractButton::SetName("BGRButton");
    Panel::SetDimensions(250,25);
    Panel::SetVisibility(true);
  }

  template<class GC>
  void BGRButton<GC>::OnCheck(bool value)
  {
    this->GetDriver().GetGrid().SetBackgroundRadiation(this->IsChecked());
  }

  template<class GC>
  LogButton<GC>::LogButton() : AbstractGridCheckbox<GC>("Show log")
  {
    AbstractButton::SetName("LogButton");
    Panel::SetDimensions(250,25);
    Panel::SetVisibility(true);
  }

  template<class GC>
  void LogButton<GC>::OnCheck(bool value)
  {
    this->GetDriver().m_logPanel.SetVisibility(this->IsChecked());
  }

} /* namespace MFM */
