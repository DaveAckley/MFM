/* -*- C++ -*- */
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 150
#define MAX_FUNC_NAME_LEN 64
#define MAX_ARG_LEN 64
#define MAX_ARGUMENTS 16

namespace MFM
{

  class FunctionCall
  {
  private:
    char m_functionName[MAX_FUNC_NAME_LEN + 1];
    char m_rawArguments[MAX_ARG_LEN + 1]; /* hold this */
    char* m_arguments[MAX_ARGUMENTS]; /* so these can point at it */
    u32  m_argumentCount;
    bool m_valid;

  public:

    FunctionCall(const char* callStr) :
      m_argumentCount(0), m_valid(false)
    {
      Parse(callStr);
    }

    bool IsValid()
    {
      return m_valid;
    }

    const char* GetFunctionName()
    {
      return m_functionName;
    }

    const char* GetArgument(u32 idx)
    {
      if(idx > m_argumentCount)
      {
	FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
      }

      return m_arguments[idx];
    }

    const u32 GetArgumentCount()
    {
      return m_argumentCount;
    }

  private:

    bool ParseFuncName(const char* callStr)
    {
      u32 len = strlen(callStr);
      char* fname = m_functionName;

      if(len <= MAX_FUNC_NAME_LEN)
      {
	while(*callStr && *callStr != '(')
	{
	  *(fname++) = *(callStr++);
	}
	*fname = 0;
	return true;
      }
      return false;
    }

    bool ParseArguments(const char* callStr)
    {
      u32 len = strlen(callStr);
      char* rawPtr = m_rawArguments;

      if(len <= MAX_FUNC_NAME_LEN)
      {
	/* Seek to arguments */
	while(*callStr && *callStr != '(')
	{
	  callStr++;
	}

	if(*callStr == '(')
	{
	  callStr++;

	  /* We're at the arguments. Eat until ')' .*/
	  while(*callStr && *callStr != ')')
	  {
	    *(rawPtr++) = *(callStr++);
	  }

	  if(*callStr == ')' && !(*(callStr + 1)))
	  {
	    *rawPtr = 0;
	    return true;
	  }
	}
      }
      return false;
    }

    bool TranslateArguments()
    {
      char* argPtr = m_rawArguments;
      u32 argIdx = 0;

      m_arguments[argIdx++] = argPtr;

      while(*argPtr)
      {
	if(*argPtr == ',' || *argPtr == ')')
	{
	  if(*argPtr == ',')
	  {
	    m_arguments[argIdx++] = argPtr + 1;
	    *argPtr = 0;
	  }
	  else if(*argPtr == ')')
	  {
	    *argPtr = 0;
	    break;
	  }
	}
	argPtr++;
      }

      m_argumentCount = argIdx;
      return true;
    }

    bool TrimArguments()
    {
      u32 i;
      for(i = 0; i < m_argumentCount; i++)
      {
	while(isspace(*m_arguments[i]))
	{
	  m_arguments[i]++;
	}
      }
      return true;
    }

    void Parse(const char* callStr)
    {
      m_valid =
	ParseFuncName(callStr) &&
	ParseArguments(callStr) &&
	TranslateArguments() &&
	TrimArguments();
    }
  };

  template<class GC>
  ExternalConfig<GC>::ExternalConfig(Grid<GC>& grid, const char* filename) :
    m_grid(grid), m_filename(filename)
  { }

  template<class GC>
  void ExternalConfig<GC>::Read()
  {
    FILE* fp = fopen(m_filename, "r");
    char lineBuffer[MAX_LINE_LEN];
    char readC;
    u32 buffIdx = 0,
      lineNum = 0;

    m_grid.Clear();

    if(!fp)
    {
      FAIL(IO_ERROR);
    }

    while((readC = fgetc(fp)) != EOF)
    {
      if(readC == '\n')
      {
	lineBuffer[buffIdx] = 0;
	buffIdx = 0;
	if(lineBuffer[0] != '#' && lineBuffer[0]) /* Allow for comments, empty lines */
	{
	  ParseLine(lineBuffer, lineNum);
	}
	if(buffIdx >= MAX_LINE_LEN)
	{
	  /* Line is too long. Abort! */
	  FAIL(OUT_OF_ROOM);
	}
      }
      else
      {
	lineBuffer[buffIdx++] = readC;
      }
    }

    fclose(fp);
  }

  template<class GC>
  void ExternalConfig<GC>::Write()
  {

  }

  static void RegisterAtom(FunctionCall& call)
  {

  }

  static void PlaceAtom(FunctionCall& call)
  {

  }

  static void SetParameter(FunctionCall& call)
  {

  }

  static void Dispatch(FunctionCall& fcall, u32 lineNumber, const char* filename)
  {
    if(!strcmp(fcall.GetFunctionName(), "RegisterAtom"))
    {
      RegisterAtom(fcall);
    }
    else if(!strcmp(fcall.GetFunctionName(), "PlaceAtom"))
    {
      PlaceAtom(fcall);
    }
    else if(!strcmp(fcall.GetFunctionName(), "SetParameter"))
    {
      SetParameter(fcall);
    }
    else
    {
      LOG.Error("%s:%d:0 error: No function named \"%s\"",
		filename, lineNumber, fcall.GetFunctionName());
    }
  }

  template<class GC>
  void ExternalConfig<GC>::ParseLine(const char* line, u32 lineNumber)
  {
    FunctionCall fcall(line);

    if(!fcall.IsValid())
    {
      LOG.Error("%s:%d:0 error: Invalid line: %s", m_filename, lineNumber, line);
    }
    else
    {
      Dispatch(fcall, lineNumber, m_filename);
    }
  }
}
