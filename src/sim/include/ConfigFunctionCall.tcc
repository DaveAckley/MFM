/* -*- C++ -*- */

namespace MFM
{
  template<class GC>
  bool ConfigFunctionCall<GC>::SkipToNextExistingArg(LineCountingByteSource & in, const char * description)
  {
    s32 ret = this->SkipToNextArg(in);
    if (ret < 0)
    {
      return false;  // Message already issued
    }

    if (ret == 0)
    {
      if (description)
      {
        return in.Msg(Logger::ERROR, "Missing argument, expected %s", description);
      }
      else
      {
        return in.Msg(Logger::ERROR, "Missing argument");
      }

    }

    return true;
  }

  template<class GC>
  s32 ConfigFunctionCall<GC>::SkipToNextArg(LineCountingByteSource & in)
  {
    in.SkipWhitespace();
    s32 ch = in.Read();
    if (ch == ')') return 0;
    if (ch == ',') {
      in.SkipWhitespace();
      return 1;
    }
    if (ch < 0) {
      if (ch == -1)
        in.Msg(Logger::ERROR, "Unexpected EOF while looking for ',' or ')'");
      else
        in.Msg(Logger::ERROR, "I/O error (%d)", ch);
    } else
      in.Msg(Logger::ERROR, "Expected ',' or ')', not '%c'",ch);
    return -1;
  }

}
