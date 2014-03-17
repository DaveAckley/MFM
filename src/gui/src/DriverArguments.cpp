#include "DriverArguments.h"
#include <stdio.h>    /* For fprintf, stderr */
#include <stdlib.h>   /* For atoi, exit */
#include <time.h>     /* For time */

namespace MFM { 

  int DriverArguments::ProcessArguments(int argc, char **argv) 
  {
    switch (argc) {
    case 1:
      m_seed = time(NULL);
      break;
    case 2:
      m_seed = atoi(argv[1]);
      break;
    default:
      fprintf(stderr,"Too many arguments (%d), 0 or 1 needed\n",argc);
      exit(1);
    }
    return 0;
  }

}

