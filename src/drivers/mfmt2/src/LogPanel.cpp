#include "LogPanel.h"

namespace MFM {
  void LogPanel::update() {
    if (mFilePathToTrack == 0) return;
    if (mFilePtr == 0) {
      	mFilePtr = fopen(mFilePathToTrack, "r");
        //         if (mFilePtr != 0) fseek(mFilePtr, 0, SEEK_END);
    }

    if (mFilePtr != 0) {
      ByteSink & bs = GetByteSink();
      s32 ch;

      while ((ch = fgetc(mFilePtr)) >= 0) {
        if (mBytesInLine >= mBytesToSkip)
          bs.WriteByte((u8) ch);
        if (ch == '\n') mBytesInLine = 0; else ++mBytesInLine;
      }

      if (ferror(mFilePtr)) {
        fclose(mFilePtr);
        mFilePtr = 0;
        mBytesInLine = 0;
      }
    }
  }
}
