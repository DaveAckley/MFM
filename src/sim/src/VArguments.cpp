#include "Fail.h"
#include "VArguments.h"

#include <string.h>

namespace MFM
{

  VArguments::VArguments() : m_heldArguments(0)
  {}

  void VArguments::RegisterArgument(const char* description, const char* filter,
				    VArgumentHandleValue func, bool runFunc)
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
	    arg.m_function(arg.m_value = argv[++i]);
	    break;
	  }
	  else if(arg.m_function)
	  {
	    arg.m_function(0);
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
    FAIL(INCOMPLETE_CODE);
  }
}
