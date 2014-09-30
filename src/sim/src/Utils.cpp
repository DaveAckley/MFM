#include "Utils.h"
#include "Fail.h"
#include <stdlib.h>

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

    bool GetReadableResourceFile(const char * relativePath, char * result, u32 length)
    {
      if (!length) FAIL(ILLEGAL_ARGUMENT);

      const char * (paths[]) = {
        "~/.mfm",                // Possible per-user customizations first
        SHARED_DIR,             // Source tree root
        "/usr/share/mfm",        // Debian install location
        "."                      // Last desperate hope
      };
      const char * home = getenv("HOME");

      for (u32 i = 0; i < sizeof(paths)/sizeof(paths[0]); ++i) {

        const char * dir = paths[i];
        if (dir[0] == '~' && home)
          snprintf(result, length, "%s%s/res/%s",home,dir+1,relativePath);
        else
          snprintf(result, length, "%s/res/%s",dir,relativePath);
        FILE * f = fopen(result,"r");
        if (f) {
          fclose(f);
          return true;
        }
      }
      result[0] = '\0';
      return false;
    }
  }
}
