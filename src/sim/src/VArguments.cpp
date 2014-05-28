#include "VArguments.h"

namespace MFM
{

  VArguments::VArguments() : m_heldArguments(0)
  {

  }

  void VArguments::RegisterArgument(char* description, char* filter,
				    VArgumentFunction* func, bool runFunc)
  {
    m_descriptions[m_heldArguments] = description;
    m_filters[m_heldArguments] = filter;
    m_functions[m_heldArguments] = func;
    m_argsNeeded[m_heldArguments] = runFunc;

    m_heldArguments++;
  }

  static bool MatchesFilter(char* str, char* filter)
  {

  }

  void VArguments::Gather(int argc, char** argv)
  {
    for(u32 i = 0; i < argc; i++)
    {
      for(u32 j = 0; j < m_heldArgumenst; j++)
      {
	if(MatchesFilter(argv[i], m_filters[j]))
	{
	  if(m_argsNeeded[j] && m_functions[j])
	  {
	    m_functions[j](m_values[j] = argv[++i]);
	  }
	  else if(m_functions[j])
	  {
	    m_functions[j](0);
	  }
	  else if(m_argsNeeded[j])
	  {
	    m_values[i] = argv[++i];
	  }
	}
      }
    }
  }

  char* VArguments::Get(char* argName)
  {
    return 0;
  }

  void VArguments::Usage()
  {

  }
}
