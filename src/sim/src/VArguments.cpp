#include "Fail.h"
#include "VArguments.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "Version.h"

namespace MFM
{

  VArguments::VArguments() : m_heldArguments(0)
  {}

  void VArguments::Die(const char * format, ...) {
    fprintf(stderr,"ERROR: ");
    va_list ap;
    va_start(ap,format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
  }

  void VArguments::RegisterArgument(const char* description, const char* filter,
				    VArgumentHandleValue func, void* handlerArg,
				    bool runFunc)
  {
    if(m_heldArguments >= VARGUMENTS_MAX_SIZE)
    {
      FAIL(OUT_OF_ROOM);
    }

    VArg& arg = m_argDescriptors[m_heldArguments++];

    arg.m_description = description;
    arg.m_filter = filter;
    arg.m_function = func;
    arg.m_argsNeeded = runFunc;
    arg.m_handlerArg = handlerArg;
  }

  void VArguments::RegisterArgumentSection(const char* sectionLabel)
  {
    if(m_heldArguments >= VARGUMENTS_MAX_SIZE)
    {
      FAIL(OUT_OF_ROOM);
    }

    VArg& arg = m_argDescriptors[m_heldArguments++];

    arg.m_description = sectionLabel;
    arg.m_filter = 0;
    arg.m_function = 0;
    arg.m_argsNeeded = false;
    arg.m_handlerArg = 0;
  }

  static bool MatchesFilter(const char* str, const char* filter)
  {
    u32 arglen = strlen(str);
    const char* filterp = filter;

    if(arglen == 0)
    {
      FAIL(ILLEGAL_ARGUMENT);
    }

    while(true)
    {
      const char* ptr = strstr(filterp, str);
      if(!ptr)
      {
	return false;
      }

      // Bump filterp past our match
      filterp = ptr + arglen;

      // Ensure we matched from the beginning of an arg, so that
      // str=='-std' WON't match filter=='--no-std'
      if (ptr==filter || ptr[-1] == '|')
      {

        // Ensure we matched a whole filter alternative
        if(*filterp == '|' || *filterp == 0)
          {
            return true;
          }
      } // else bad match, check for other alternatives 

      filterp = strchr(filterp, '|');
      if(!filterp)
      {
        return false;
      }
    }
  }

  void VArguments::ProcessArguments(u32 argc, const char** argv)
  {
    m_programName = argv[0];

    u32 initIndex = 1;          // 1 to skip program name
    if (argc > 1 && *argv[1] == '{')  // Also skip geometry arg if present
    {
      ++initIndex;
    }

    for(u32 i = initIndex; i < argc; i++)
    {
      bool handled = false;
      for(u32 j = 0; j < m_heldArguments; j++)
      {
	VArg& arg = m_argDescriptors[j];

        if (!arg.m_filter) continue;  // Skip sections

	if(MatchesFilter(argv[i], arg.m_filter))
	{
	  arg.m_appeared = true;
          handled = true;

	  if(arg.m_argsNeeded && i >= argc - 1)
            Die("'%s' requires an argument", argv[i]);

	  if(arg.m_argsNeeded && arg.m_function)
	  {
	    arg.m_function(arg.m_value = argv[++i], arg.m_handlerArg);
	  }
	  else if(arg.m_function)
	  {
	    arg.m_function(0, arg.m_handlerArg);
	  }
	  else if(arg.m_argsNeeded)
	  {
	    arg.m_value = argv[++i];
	  }
	}
      }

      if (!handled)
      {
        Die("Unrecognized argument '%s'", argv[i]);
      }

    }
  }

  u32 VArguments::GetInt(const char* argName) const
  {
    return atoi(Get(argName));
  }

  const char* VArguments::Get(const char* argName) const
  {
    for(u32 i = 0; i < m_heldArguments; i++)
    {
      if (!m_argDescriptors[i].m_filter) continue;  // Skip sections

      if(MatchesFilter(argName, m_argDescriptors[i].m_filter))
      {
	return m_argDescriptors[i].m_value;
      }
    }

    FAIL(ILLEGAL_ARGUMENT);
  }

  bool VArguments::Appeared(const char* argName) const
  {
    for(u32 i = 0; i < m_heldArguments; i++)
    {
      if (!m_argDescriptors[i].m_filter) continue;  // Skip sections

      if(MatchesFilter(argName, m_argDescriptors[i].m_filter))
      {
	return m_argDescriptors[i].m_appeared;
      }
    }

    FAIL(ILLEGAL_ARGUMENT);
  }

  void VArguments::Usage() const
  {
    fprintf(stderr,
	    "Movable Feast Machine (%s)\n"
	    "\n"
	    "Usage:\n"
	    "  %s [GEOMETRY] [SWITCHES]\n"
            "\n"
            " GEOMETRY is a 'tile type' placed between a grid width and a grid height,\n"
            " and surrounded by '{}'s.  For example, geometry '{5C3}' denotes a grid\n"
            " 5 wide by 3 high of tiles of type 'C' (1K sites/tile), checkerboard layout;\n"
	    " A staggered layout has double braces, like {{5C3}}\n.",
            MFM_VERSION_STRING_SHORT,
            m_programName?m_programName:""
            );

    for(u32 i = 0; i < m_heldArguments; i++)
    {
      const VArg& va = m_argDescriptors[i];
      if (va.m_filter)
      {
        fprintf(stderr, "  %-20s%-20s%s\n",
                va.m_filter,
                va.m_description,
                va.m_argsNeeded ? " [ARG]" : "");
      }
      else
        fprintf(stderr, "\n %s\n", va.m_description);  // Section label

    }

    exit(0);
  }
}
