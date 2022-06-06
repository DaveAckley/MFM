#include "T2Utils.h"
#include "FileByteSink.h"
#include "SHA256ish.h"
#include <sys/stat.h>

namespace MFM {

  void printComma(u32 num, ByteSink & to) {
    if (num >= 1000) {
      printComma(num/1000, to);
      to.Printf(",%03d", num%1000);
    } else
      to.Printf("%d", num);
  }

  void printComma(u64 num, ByteSink & to) {
    if (num >= 1000) {
      printComma(num/1000, to);
      to.Printf(",%03d", (u32) (num%1000));
    } else
      to.Printf("%d", (u32) num);
  }

  u32 getModTimeOfFile(const char* path) {
    struct stat buf;
    if (stat(path,&buf) != 0) return 0;
    return (u32) buf.st_mtime; // seconds res is all we want
  }

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

  u64 digestWholeFile64(const char* path) {
    const u64 DIGEST_VERSION = 3; /* Legal values: 1..15 */
    OString16 out;
    if (!digestWholeFile(path, out, false))
      return 0; // Illegal return
    CharBufferByteSource cbbs = out.AsByteSource();
    u64 ret = 0;
    for (u32 i = 0; i < 8; ++i) {  // Take first eight bytes
      s32 ch = cbbs.Read();
      MFM_API_ASSERT_STATE(ch >= 0);
      ret = (ret<<8) | (u8) ch;
    }
    return (ret>>4)|(DIGEST_VERSION<<60); // Toss first four bits
  }

  bool digestWholeFile(const char* path, ByteSink& digestout, bool ashex) {
    FileByteSource fbs(path);
    if (!fbs.IsOpen()) return false;
    SHA256ish hash;
    s32 ch;
    while ((ch = fbs.ReadByte()) >= 0) {
      hash.addByte((u8) ch);
    }
    fbs.Close();
    if (!hash.digest(digestout, ashex)) // This can't be false, right??
      return false;
    return true;
  }

  bool writeWholeFile(const char* path, const char * data) {
    CharBufferByteSource cbbs(data,strlen(data));
    return writeWholeFile(path,cbbs);
  }

  bool writeWholeFile(const char* path, ByteSource &from) {
    FILE * file = fopen(path,"w");
    if (!file) return false;
    FileByteSink fbs(file);
    fbs.Copy(from);
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

  bool endsWith(const std::string& str, const std::string& suffix) {
    return
      str.size() >= suffix.size() &&
      0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
  }

  const char * formatSize(u32 usize, bool trimwhite) {
    const u32 BUFLEN = 4+1;
    static char buf[BUFLEN] = "";
    char * ret = buf;
    if (usize == 0) snprintf(buf,BUFLEN,"  0 ");
    else {
      double size = usize;
      const char * UNITS = " KMGTPE"; // Can't get over G in u32 ..
      for (u32 i = 0; i < strlen(UNITS); ++i) {
        char unit = UNITS[i];
        if (size < 1000) {
          if (size < 1) snprintf(buf,BUFLEN,".%02d%c",(u32)(size*100),unit);
          else if (size >= 9.95 || (u32)(size) == size)
            snprintf(buf,BUFLEN,"%3d%c",(u32)(size+0.5),unit);
          else snprintf(buf,BUFLEN,"%3.1f%c",size,unit);
          break;
        }
        size /= 1000.0;
      }
    }
    if (trimwhite) {
      for (u32 i = BUFLEN; i-- > 0; ) {
        if (buf[i] == ' ') buf[0] = 0;
        else break;
      }
      while (*ret == ' ') ++ret;
    }
    return ret;
  }

}
