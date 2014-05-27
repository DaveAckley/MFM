#include "DriverArguments.h"
#include "Logger.h"
#include <stdio.h>    /* For fprintf, stderr */
#include <stdlib.h>   /* For atoi, exit */
#include <string.h>   /* For strcmp */
#include <stdarg.h>   /* For va_list, va_args */
#include <time.h>     /* For time */

namespace MFM {

  void DriverArguments::Die(const char * format, ...) {
    fprintf(stderr,"ERROR: ");
    va_list ap;
    va_start(ap,format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
  }

  static char * GetNextArg(int & count, char ** & argv) {
    if (count <= 0)
      return 0;
    --count;
    return *argv++;
  }

  /* This should be written with getopt! */
  int DriverArguments::ProcessArguments(int & argc, char  ** &argv)
  {
    m_programName = GetNextArg(argc,argv);

    if (!m_programName)
      Die("Missing program name?");

    while (argc > 0) {
      const char * arg = GetNextArg(argc,argv);
      if (!arg) Die("Unreachable?");

      if (!strcmp("--",arg)) {
        return argc;
      }

      if (!strcmp("-s",arg) || !strcmp("--seed",arg)) {

        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing seed (integer) argument after %s", arg);

        m_seed = atoi(val);
	LOG.Message("[Seed %d]\n",m_seed);

      } else if (!strcmp("-e",arg) || !strcmp("--events",arg)) {

        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing aeps (integer) argument after %s", arg);

        m_recordEventCountsPerAEPS = atoi(val);
	LOG.Message("[Recording event counts approximately every %d AEPS]\n",
		    m_recordEventCountsPerAEPS);

      } else if (!strcmp("-d",arg) || !strcmp("--dir",arg)) {

        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing aeps (integer) argument after %s", arg);

        m_dataDirPath = val;
	LOG.Message("[Data directory path = %s]\n",m_dataDirPath);

      } else if (!strcmp("-p",arg) || !strcmp("--pictures",arg)) {

        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing aeps (integer) argument after %s", arg);

        m_recordScreenshotPerAEPS = atoi(val);
	LOG.Message("[Recording grid pictures approximately every %d AEPS]\n",
		    m_recordScreenshotPerAEPS);

      }
      else if (!strcmp("-t", arg) || !strcmp("--timebd", arg))
      {
	const char* val = GetNextArg(argc, argv);
	if(!val) Die("Missing aeps (integer) argument after %s", arg);

	m_recordTimeBasedDataPerAEPS = atoi(val);
	LOG.Message("[Recording time based data approximately every %d AEPS]\n",
		    m_recordTimeBasedDataPerAEPS);
      }
      else if (!strcmp("--picturesPerRate",arg)) {

        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing COUNT (integer) argument after %s", arg);

        m_countOfScreenshotsPerRate = atoi(val);
        if (m_countOfScreenshotsPerRate < 1) Die("COUNT must be greater than zero, after %s",arg);
	LOG.Message("[Accelerating in groups of %d pictures]\n",
	            m_countOfScreenshotsPerRate);

      }

      else if (!strcmp("--haltafteraeps", arg))
      {
	const char* val = GetNextArg(argc, argv);
	if(!val) Die("Missing aeps (integer) argument after %s", arg);

	m_haltAfterAEPS = atoi(val);
	if(m_haltAfterAEPS > 0)
	{
	  LOG.Message("[Halting execution after approximately %d AEPS]\n",
		      m_haltAfterAEPS);
	}
      }
      else if (!strcmp("--disabletile", arg))
      {
	char* val = GetNextArg(argc, argv);
	if(!val) Die("Missing tile (upoint, i.e. '(s32,s32)') argument after %s", arg);

	if(m_disabledTileCount >= DRIVERARGUMENTS_MAX_DISABLED_TILES)
	{
	  Die("More than %d tiles may not be disabled", DRIVERARGUMENTS_MAX_DISABLED_TILES);
	}

	m_disabledTiles[m_disabledTileCount++].Parse(val);
	LOG.Message("[Tile @ %s deactivated]\n", val);
      }
      else if (!strcmp("--startwithoutgrid", arg))
      {
	m_startWithoutGridView = true;
	LOG.Message("[Starting in statistics view]\n");
      }
      else if (!strcmp("--startpaused", arg) || !strcmp("--nostartpaused", arg))
      {
	m_startPaused = !strcmp("--startpaused", arg);
        if (m_startPaused)
        {
          LOG.Message("[Starting paused]\n");
        } else {
          LOG.Message("[Starting without initial pause]\n");
        }
      }
      else if(!strcmp("-r", arg) || !strcmp("--rate", arg))
      {
        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing AEPS (integer) argument after %s", arg);

        m_aepsPerFrame = atoi(val);
        if (m_aepsPerFrame < 1) Die("AEPS must be greater than zero, after %s",arg);
        LOG.Message("[Initially rendering at %d AEPS per frame]\n",
		    m_aepsPerFrame);
      }
      else if(!strcmp("-l", arg) || !strcmp("--log", arg))
      {
        const char * val = GetNextArg(argc,argv);
        if (!val) Die("Missing logging level (integer) argument after %s", arg);

        m_initLogLevel = atoi(val);
        if (!Logger::ValidLevel(m_initLogLevel)) Die("Invalid logging level %d", m_initLogLevel);
	LOG.Message("[Initial logging level %s]\n",
                    Logger::StrLevel((Logger::Level) m_initLogLevel));
      }
      else if(!strcmp("--startminimal", arg))
      {
	m_startMinimal = true;
	LOG.Message("[Starting with minimal rendering]\n");
      }

      else if (!strcmp("-h",arg) || !strcmp("--help",arg)) {

        fprintf(stderr,
                "Arguments arg:\n"
                " -h, --help                 Print this help\n"
                " -s NUM, --seed NUM         Set master PRNG seed to NUM (u32)\n"
                " -d DIR, --dir DIR          Store data in per-sim directories under DIR\n"
		"\n"
                " -l NUM, --log NUM          Amount of logging output (0 is none, 1 is errors only)\n"
		"\n"
                " -e AEPS, --events AEPS     Record event counts every AEPS aeps\n"
                " -p AEPS, --pictures AEPS   Record screenshots every AEPS aeps\n"
		" -t AEPS, --timebd AEPS     Records time based data every AEPS aeps\n"
		" -r AEPS, --rate AEPS       The AEPS per frame rendering rate to begin at\n"
		"\n"
		" --disabletile TILEPT       Stops execution of the Tile at TILEPT.\n"
                " --picturesPerRate COUNT    Take COUNT shots per speed from 1 up to -p value\n"
		" --haltafteraeps AEPS       If AEPS > 0, Halts after AEPS elapsed aeps.\n"
		" --startpaused              Start paused to allow display configuration.\n"
		" --nostartpaused            Start running immediately for hands-off operation.\n"
		" --startwithoutgrid         Start with the statistics view on screen.\n"
		" --startminimal             Start with as small a window as possible.\n"
		"\n"
		" --                         Flag end of standard arguments.\n"
                );
        exit(0);
      } else {
        Die("Unrecognized argument '%s', try '%s -h' for help",
            arg, m_programName);
      }
    }
    return 0;
  }
}
