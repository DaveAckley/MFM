/* -*- C++ -*- */
#ifndef LOGPANEL_H
#define LOGPANEL_H

#include "SDL.h"

#include "TextPanel.h"

namespace MFM {

  typedef TextPanel<128,10> TextPanelForLogging;

  struct LogPanel : public TextPanelForLogging {
    const char * mFilePathToTrack;
    FILE * mFilePtr;
    u32 mBytesToSkip;
    u32 mBytesInLine;

    void setPathToTrack(const char * path) {
      mFilePathToTrack = path;
    }

    void setBytesToSkipPerLine(u32 skipbytes) {
      mBytesToSkip = skipbytes;
    }

    LogPanel(const char * path = 0, u32 skipbytes = 0)
      : mFilePathToTrack(0)
      , mFilePtr(0)
      , mBytesToSkip(0)
      , mBytesInLine(0)
    {
      setPathToTrack(path);
      setBytesToSkipPerLine(skipbytes);
    }

    void update() ;

    virtual ~LogPanel() { }
  };
}

#endif /* LOGPANEL_H */
