#include "HelpPanel.h"

namespace MFM
{

  const char * HelpPanel::m_helpMessages[] = {
    "Keyboard:",
    " [SPACE] Pause / Start simulation",
    " [t] Show toolbox",
    "    [Scroll] Brush Size",
    " [g] Show grid",
    " [m] Toggle memory view",
    " [k] Render Atom heatmap",
    " [h] Show this help window",
    " [l] Show log",
    " [p] Render caches",
    " [esc] Deselect Tile",
    " [r] Toggle recording",
    " [arrow keys] Move tiles",
    " [,] Decrease AEPS per frame",
    " [.] Increase AEPS per frame",
    " [i] Show statistics and settings",
    "    [a] Show AER statistics",
    "    [b] Hide buttons",
    " [CTRL+q] Quit",
    "",
    "Mouse:",
    " [Ctrl+Drag] Move tiles",
    " [Scroll] Zoom",
    "",
    "Toolbox Control:",
    " [Left] Select / paint left / clone to",
    " [Right] Select / paint right / clone from",
    0
  };
}
