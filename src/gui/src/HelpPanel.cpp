#include "HelpPanel.h"

namespace MFM
{

  const char * HelpPanel::m_helpMessages[] =
  {
    "Keyboard:",
    " [SPACE] Pause / Start simulation",
    " [t] Toggle showing the toolbox",
    "    [Scroll] Brush Size",
    "    [Mid-Mouse Element] Lowlighting",
    " [g] Toggle showing grid",
    " [m] Cycle through memory views",
    " [o] Render atoms as squares / circles",
    " [k] Render Atom heatmap",
    " [h] Toggle showing this help window",
    " [l] Toggle showing the log",
    " [p] Render caches",
    " [esc] Deselect Tile and/or Atom",
    " [r] Toggle recording",
    " [arrow keys] Move grid in window",
    " [,] Decrease AEPS per frame",
    " [.] Increase AEPS per frame",
    " [i] Toggle statistics and settings",
    "    [a] Toggle showing AER statistics",
    "    [b] Toggle showing buttons",
    " [CTRL+q] Quit",
    "",
    "Mouse:",
    " [Ctrl+Drag] Move grid in window",
    " [Scroll] Zoom in and out on grid",
    "",
    "Toolbox Control:",
    " [Left] Select / paint left / clone to",
    " [Right] Select / paint right / clone from",
    " [Scroll] Change brush size",
    0
  };
}
