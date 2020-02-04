#include "T2Utils.h"

namespace MFM {

  bool readWholeFile(const char* path, ByteSink& to) {
    FileByteSource fbs(path);
    if (!fbs.IsOpen()) return false;
    s32 ch;
    while ((ch = fbs.ReadByte()) >= 0) {
      to.WriteByte((u8) ch);
    }
    fbs.Close();
    return true;
  }

  bool readOneLinerFile(const char* path, ByteSink &to) {
    if (!readWholeFile(path,to)) return false;
    if (to.CanChomp()) to.Chomp();
    return true;
  }

  bool readOneDecimalNumberFile(const char* path, s32 & to) {
    OString64 buf;
    if (!readOneLinerFile(path, buf)) return false;
    CharBufferByteSource cbs = buf.AsByteSource();
    if (cbs.Scanf("%d",&to) != 1) return false;
    return true;
  }

  bool readOneBinaryNumberFile(const char* path, s32 & to) {
    OString64 buf;
    if (!readOneLinerFile(path, buf)) return false;
    CharBufferByteSource cbs = buf.AsByteSource();
    if (cbs.Scanf("%b",&to) != 1) return false;
    return true;
  }

  bool readFloatsFromFile(const char* path, double * floats, u32 floatCount) {
    MFM_API_ASSERT_ARG(floats || floatCount == 0);
    FileByteSource fbs(path);
    if (!fbs.IsOpen()) return false;
    bool ret = true;
    for (u32 i = 0; i < floatCount; ++i) {
      if (fbs.Scanf("%f",&floats[i]) != 1) { ret = false; break; }
    }
    fbs.Close();
    return ret;
  }

  bool readWholeProcessOutput(const char* cmd, ByteSink &to) {
    FILE* file = popen(cmd, "r") ;
    if (!file) return false;

    s32 ch;
    while ( (ch = fgetc(file)) >= 0)
      to.WriteByte((u8) ch);

    pclose(file) ;
    return true;
  }


  u32 findPidOfProgram(const char * progname) {
    MFM_API_ASSERT_NONNULL(progname);
    OString128 cmd;
    cmd.Printf("ps -C %s",progname);
    OString128 output;
    if (!readWholeProcessOutput(cmd.GetZString(), output))
      return 0;
    CharBufferByteSource cbs = output.AsByteSource();
    cbs.Scanf("%0[^\n]\n",0);
    u32 pid;
    if (cbs.Scanf("%d",&pid) == 1) return pid;
    return 0;
  }

}
