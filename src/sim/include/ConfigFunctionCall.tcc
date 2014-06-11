/* -*- C++ -*- */

namespace MFM
{
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
