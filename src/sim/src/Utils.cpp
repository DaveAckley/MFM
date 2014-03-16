#include "Utils.h"
#include "Fail.h"
#include <time.h>
#include <stdlib.h>

namespace MFM {
  u64 GetDateTimeNow() {
    time_t t;
    t = time(NULL);
    return GetDateTime(t);
  }

  u64 GetDateTime(time_t t) {
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
}
