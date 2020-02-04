#include "LogPanel.h"

namespace MFM {
  void LogPanel::update() {
    if (mFilePathToTrack == 0) return;

    if (mFilePtr == 0) {
      	mFilePtr = fopen(mFilePathToTrack, "r");
        //         if (mFilePtr != 0) fseek(mFilePtr, 0, SEEK_END);
    }

    //bool worked = false;
    if (!ferror(mFilePtr)) {
      u64 pos; 

      if (fseek(mFilePtr, 0, SEEK_END) == 0) {

        // pos will contain no. of chars in 
        // input file. 
        pos = ftell(mFilePtr); 
  
        // Back up a few K or to beginning
        const u32 BACKUP_BYTES = 10000; // XXX using some fnc of TextPanel COLUMNS and LINES
        if (pos > BACKUP_BYTES) pos -= BACKUP_BYTES;
        else pos = 0;

        if (fseek(mFilePtr, pos, SEEK_SET) == 0) { 
          ByteSink & bs = GetByteSink();
          s32 ch;
          
          while ((ch = fgetc(mFilePtr)) >= 0) {
            if (mBytesInLine >= mBytesToSkip)
              bs.WriteByte((u8) ch);
            if (ch == '\n') mBytesInLine = 0; else ++mBytesInLine;
          }

          //          worked = true;
        } 
      }
    }

    if (true/*!worked*/) {  // open every time, we're seeking
      fclose(mFilePtr);
      mFilePtr = 0;
      mBytesInLine = 0;
    }
  }
}
