#include "Utils.h"
#include "Fail.h"
#include "Logger.h" /* for LOG */
#include <stdlib.h>
#include <unistd.h> /* for open(), close() */
#include <fcntl.h>  /* for O_RDONLY */
#include <string.h> /* for strerror */
#include <errno.h> /* for errno */
#include <libgen.h> /* for dirname */
#include "OverflowableCharBufferByteSink.h"

namespace MFM {
  namespace Utils {
    u64 GetDateTimeNow()
    {
      time_t t;
      t = time(NULL);
      return GetDateTime(t);
    }

    u64 GetDateTime(time_t t)
    {
      struct tm *tmp;
      tmp = localtime(&t);
      if (tmp == NULL)
        FAIL(ILLEGAL_STATE);
      return
        (((((((u64) tmp->tm_year+1900)
             *100+tmp->tm_mon+1)
            *100+tmp->tm_mday)
           *100+tmp->tm_hour)
          *100+tmp->tm_min)
         *100+tmp->tm_sec);
    }

    u32 GetDateFromDateTime(u64 datetime)
    {
      return (u32) (datetime / (100*100*100));
    }

    u32 GetTimeFromDateTime(u64 datetime)
    {
      return (u32) (datetime % (100*100*100));
    }

    const char * ReadablePath(const char * path)
    {
      s32 fd = open(path, O_RDONLY);
      if (fd >= 0)
      {
        close(fd);
        return 0;
      }
      return strerror(errno);
    }

    bool ResolveProgramRelativePath(ByteSink& result, const char * pathSuffixOrNull)
    {
      const u32 PATH_LEN_UGH = 2000;
      char prog[PATH_LEN_UGH];

      ssize_t got = readlink("/proc/self/exe", prog, sizeof(prog));
      if (got < 0 || got >= PATH_LEN_UGH) {
        LOG.Debug("readlink /proc/self/exe == %d", got);
        return false;
      }
      prog[got] = 0; // readlink doesn't null terminate

      char * dirn = dirname(prog);
      char path[PATH_LEN_UGH];

      if (pathSuffixOrNull) {
        u32 wrote = snprintf(path, PATH_LEN_UGH, "%s/%s", dirn, pathSuffixOrNull);
        if (wrote >= PATH_LEN_UGH) {
          LOG.Debug("readlink path too long (%d)", wrote);
          return false;
        }
      } else 
        strncpy(path, dirn, PATH_LEN_UGH); // dirn gteed to fit since dirname at least deletes '/'

      char * real = realpath(path, 0);
      if (!real) {
        LOG.Debug("realpath '%s' failed: %s", path, strerror(errno));
        return false;
      }

      // Note: Caller Reset()s result before and/or checks for overflow as needed
      result.Print(real); 
      free(real);
      return true;
    }

    bool GetReadableResourceFile(const char * relativePath, ByteSink& result)
    {
      OString1024 buffer;

      const char * (paths[]) = {
        "~/.mfm",                // Possible per-user customizations first
        0,                       // hack meaning 'try /proc/self/exe now'
        "../../MFM",             // AppImage top-level relative path?  Maybe?
        SHARED_DIR,              // Source tree root
        "/usr/lib/" XSTR_MACRO(DEBIAN_PACKAGE_NAME) "/MFM", // Debian install location of mfm
        "/usr/share/mfm",        // Debian install location of mfm (old)
        "."                      // Last desperate hope
      };
      const char * home = getenv("HOME");

      for (u32 i = 0; i < sizeof(paths)/sizeof(paths[0]); ++i) {

        const char * dir = paths[i];
        buffer.Reset();

        if (!dir) {
          if (!ResolveProgramRelativePath(buffer,"..")) // Up one from MFM/bin -> MFM
            continue;
          
          buffer.Printf("/res/%s",relativePath);
          if (buffer.HasOverflowed()) {
            LOG.Debug("Path overflowed '%s'", buffer.GetZString());
            continue;
          }
        } else if (dir[0] == '~' && home)
          buffer.Printf("%s%s/res/%s",home,dir+1,relativePath);
        else
          buffer.Printf("%s/res/%s",dir,relativePath);
        const char * errmsg = ReadablePath(buffer.GetZString());
        if (!errmsg) {
          LOG.Debug("Found resource file '%s'", buffer.GetZString());
          result.Printf("%s",buffer.GetZString());
          return true;
        } else {
          LOG.Debug("Resource file candidate '%s' not readable: %s", buffer.GetZString(), errmsg);
        }
      }
      return false;
    }

    void NormalizePath(const char * path, ByteSink & buffer)
    {
      MFM_API_ASSERT_NONNULL(path);
      const char * home = getenv("HOME");
      if (path[0] == '~' && home)
      {
        buffer.Printf("%s%s", home, path + 1);
      }
      else
      {
        buffer.Printf("%s", path);  // ..just be literal
      }

    }

  }
}
