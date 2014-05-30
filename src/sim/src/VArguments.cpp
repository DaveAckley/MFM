#include "Fail.h"
#include "VArguments.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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

      filterp = ptr + arglen;
      if(*filterp == '|' || *filterp == 0)
      {
	return true;
      }

      filterp = strchr(filterp, '|');
      if(!filterp)
      {
	return false;
      }
    }
  }

  void VArguments::ProcessArguments(u32 argc, const char** argv)
  {
    for(u32 i = 0; i < argc; i++)
    {
      for(u32 j = 0; j < m_heldArguments; j++)
      {
	VArg& arg = m_argDescriptors[j];
	if(MatchesFilter(argv[i], arg.m_filter))
	{
	  arg.m_appeared = true;
	  if(arg.m_argsNeeded && arg.m_function)
	  {
	    arg.m_function(arg.m_value = argv[++i], arg.m_handlerArg);
	    break;
	  }
	  else if(arg.m_function)
	  {
	    arg.m_function(0, arg.m_handlerArg);
	  }
	  else if(arg.m_argsNeeded)
	  {
	    arg.m_value = argv[++i];
	    break;
	  }
	}
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
	    "Movable Feast Machine Simulator\n"
	    "\n"
	    "Usage:\n"
	    "\n");

    for(u32 i = 0; i < m_heldArguments; i++)
    {
      const VArg& va = m_argDescriptors[i];
      fprintf(stderr, "  %-15s%-20s\n", va.m_filter, va.m_description);
    }

    exit(0);
  }
}
