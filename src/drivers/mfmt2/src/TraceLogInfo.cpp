#include "TraceLogInfo.h"
#include "T2Utils.h" /* for formatSize */

namespace MFM {
  void TraceLogInfo::reset() {
    mSeq = 0;
    mTag = 0;
    mRange = 0;
    mOffset = BPoint(0,0);
    mBytes = 0;
  }

  bool TraceLogInfo::tryInitFromDir(DIR * dir, struct dirent * ent) {
    MFM_API_ASSERT_NONNULL(dir);
    MFM_API_ASSERT_NONNULL(ent);
    const char * filename = ent->d_name;
    u32 seq, tag, count, range, xoff, yoff;
    u32 matches = sscanf(filename,TRACE_DUMP_FILENAME_FMT "%n",
                         &seq,
                         &tag,
                         &range,
                         &xoff,  // XXX WATCH OUT! xoff/yoff are u32 but 
                         &yoff,  // are meant to be interpreted as s8!
                         &count);
    if (matches != TRACE_DUMP_FILENAME_MATCHES || count != TRACE_DUMP_FILENAME_MATCH_LEN)
      return false;
    mSeq = seq;
    mTag = tag;
    mRange = range;
    mOffset.SetX(xoff);
    mOffset.SetY(yoff);
    {
      int dirFD = dirfd(dir);
      struct stat buf;
      if (fstatat(dirFD, filename, &buf, 0)) {
        LOG.Error("Can't stat %s: %s",filename,strerror(errno));
        mBytes = 0; // ??
      } else {
        mBytes = (u32) buf.st_size; // The fogging disk is only 4GB!
      }
    }
    return true;
  }

  void TraceLogInfo::formatInfo(ByteSink & bs) const {
    s32 x = mOffset.GetX();
    s32 y = mOffset.GetY();
    char sx = '+', sy = '+';
    if (x < 0) { sx = '-'; x = -x; }
    if (y < 0) { sy = '-'; y = -y; }
    bs.Printf("%08x%c%02d%c%02dr%02xs%03x %s",
              mTag,
              sx,x,sy,y,
              mRange,
              mSeq&0xfff,
              formatSize(mBytes,false));
  }

  TraceLogDirManager::TraceLogDirManager() {

  }

  const TraceLogInfo * TraceLogDirManager::getTraceLogInfoFromSeq(u32 seq) const {
    MFM_API_ASSERT_ARG(seq > 0);
    for (u32 i = 0; i < MAX_TRACE_SEQ_TO_KEEP; ++i) {
      if (mFileInfos[i].mSeq == seq) return &mFileInfos[i];
    }
    return 0;
  }

  const TraceLogInfo * TraceLogDirManager::getTraceLogInfoByIndex(u32 index) const {
    MFM_API_ASSERT_ARG(index < MAX_TRACE_SEQ_TO_KEEP);
    return &mFileInfos[index];
  }
  
  u32 TraceLogDirManager::initAndManageTraceDir() {
    const char * traceDir = getTraceDir();

    if (mkdir(traceDir,0755) && errno != EEXIST) {
      LOG.Error("Can't make dir %s: %s",traceDir,strerror(errno));
      return 0;
    }
    DIR *dir = opendir(traceDir);
    if (!dir) {
      LOG.Error("Can't open dir %s: %s",traceDir,strerror(errno));
      return 0;
    }
    struct dirent * ent;
    u32 maxseq = 0, minseq = 0xffffffff - MAX_TRACE_SEQ_TO_KEEP - 1;
    while ((ent = readdir(dir)) != NULL) {
      TraceLogInfo tli;
      if (!tli.tryInitFromDir(dir, ent)) continue;
      if (tli.mSeq < minseq) minseq = tli.mSeq;
      if (tli.mSeq > maxseq) maxseq = tli.mSeq;
    }
    u32 ret = maxseq + 1;
    u32 mintokeep = 0;
    if (maxseq > MAX_TRACE_SEQ_TO_KEEP) 
      mintokeep = maxseq - MAX_TRACE_SEQ_TO_KEEP + 1;

    for (u32 i = 0; i < MAX_TRACE_SEQ_TO_KEEP; ++i)
      mFileInfos[i].reset();

    rewinddir(dir);
    int dirFD = dirfd(dir);
    while ((ent = readdir(dir)) != NULL) {
      TraceLogInfo tli;
      if (!tli.tryInitFromDir(dir, ent)) continue;
      u32 seq = tli.mSeq;
      if (seq + MAX_TRACE_SEQ_TO_KEEP <= maxseq) {
        LOG.Message("Auto-deleting %s", ent->d_name);
        if (unlinkat(dirFD, ent->d_name, 0) != 0) {
          LOG.Message("%s/%s deletion failed: %s",
                      traceDir, ent->d_name, strerror(errno));
          ret = 0;
          break;
        }
      } else {
        u32 index = seq-mintokeep;
        MFM_API_ASSERT_STATE(index < MAX_TRACE_SEQ_TO_KEEP);
        mFileInfos[index] = tli;
      }
    }
    closedir(dir);
    return ret;
  }

  void TraceLogInfo::tryMoveLog() const {
    LOG.Error("IMPLEMENT %s",__PRETTY_FUNCTION__);
  }

  void TraceLogDirManager::tryMoveLog(u32 tag, u32 range, BPoint offset) {
    for (u32 i = 0; i < MAX_TRACE_SEQ_TO_KEEP; ++i) {
      const TraceLogInfo & tli = mFileInfos[i];
      if (tli.mTag == tag) {
        OString32 buf;
        tli.formatInfo(buf);
        LOG.Message("SEND/RECV %s (%d,%d)",
                    buf.GetZString(),
                    offset.GetX(), offset.GetY());
        return;
      }
    }
    LOG.Error("IMPLEMENT %s %08x",__PRETTY_FUNCTION__,tag);
  }
}
