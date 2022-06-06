#include "RootPanel.h"
#include "Drawing.h"
#include "Logger.h"


namespace MFM {

  // Offer an easy place to breakpoint on, outside the unwind_protect
  static void rootPanelFailMessage() {
    LOG.Message("Painting failed to the root panel: Drawing incomplete");
  }

  void RootPanel::Paint(Drawing & drawing)
  {
    Rect wdw;
    drawing.GetWindow(wdw);
    u32 bg = drawing.GetBackground();
    u32 fg = drawing.GetForeground();

    unwind_protect(
    {
      LogBacktrace(MFMThrownBacktraceArray, MFMThrownBacktraceSize);
      rootPanelFailMessage();
    },
    {
      this->Super::Paint(drawing); // Dish to regular panel
    });

    drawing.SetWindow(wdw);      // FWIW
    drawing.SetBackground(bg);
    drawing.SetForeground(fg);
  }
  
} /* namespace MFM */
