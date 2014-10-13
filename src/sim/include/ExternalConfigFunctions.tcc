/* -*- C++ -*- */

#include "Element_Dreg.h"
#include "AtomSerializer.h"

namespace MFM
{
  template <class GC>
  class FunctionCallRegisterElement : public ConfigFunctionCall<GC>
  {
   public:
    FunctionCallRegisterElement() : ConfigFunctionCall<GC>("RegisterElement")
    { }

    virtual bool Parse(ExternalConfig<GC> & ec)
    {
      LineCountingByteSource & in = ec.GetByteSource();
      in.SkipWhitespace();
      UUID uuid;
      if (!uuid.Read(in))
        return in.Msg(Logger::ERROR, "Error reading Element UUID");
      s32 ret = this->SkipToNextArg(in);
      if (ret < 0) return false;
      if (ret == 0)
        return in.Msg(Logger::ERROR, "Expected second argument");
      OString16 buf;
      if (!in.ScanIdentifier(buf))
        return in.Msg(Logger::ERROR, "Expected identifier as second argument");
      if (buf.HasOverflowed())
        return in.Msg(Logger::ERROR, "Identifier too long '%s'", buf.GetZString());
      if (!ec.RegisterElement(uuid, buf))
        return in.Msg(Logger::ERROR, "Couldn't register '%@' under name '%s'", &uuid, buf.GetZString());

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply(ExternalConfig<GC> & ec) { /* Work already done */ }

  };

  template <class GC>
  class FunctionCallGA : public ConfigFunctionCall<GC>
  {
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };

   public:
    FunctionCallGA() : ConfigFunctionCall<GC>("GA")
    { }

    virtual bool Parse(ExternalConfig<GC> & ec)
    {
      LineCountingByteSource & in = ec.GetByteSource();
      in.SkipWhitespace();

      OString16 nick;
      if (!in.ScanIdentifier(nick))
      {
        return in.Msg(Logger::ERROR, "Expected identifier as first argument");
      }
      if (nick.HasOverflowed())
      {
        return in.Msg(Logger::ERROR, "Identifier too long '%s'", nick.GetZString());
      }

      const Element<CC> * pelt = ec.LookupElement(nick);
      if (!pelt)
      {
        return in.Msg(Logger::ERROR, "'%@' isn't a registered element nickname", &nick);
      }

      s32 x, y;

      // Umm: P3Atom has 71 non-type bits..
      // OString64 hexData;
      OString128 hexData;

      if (!this->SkipToNextExistingArg(in,"x position"))
      {
        return false;
      }

      if (!in.Scan(x))
      {
        return in.Msg(Logger::ERROR, "Expected x position");
      }

      if (!this->SkipToNextExistingArg(in,"y position"))
      {
        return false;
      }

      if (!in.Scan(y))
      {
        return in.Msg(Logger::ERROR, "Expected y position");
      }

      if (!this->SkipToNextExistingArg(in, "atom body"))
      {
        return false;
      }

      Atom<CC> temp;
      AtomSerializer<CC> as(temp);
      if (in.Scanf("%@",&as) != 1)
      {
        return in.Msg(Logger::ERROR, "Expected hex-encoded Atom body");
      }

      if (!ec.PlaceAtom(*pelt, x, y, as.GetBits()))
      {
        return false;
      }

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply(ExternalConfig<GC> & ec) { /* Work already done */ }

  };

  template <class GC>
  class FunctionCallDisableTile : public ConfigFunctionCall<GC>
  {
   private:
    typedef typename GC::CORE_CONFIG CC;

   public:
    FunctionCallDisableTile() :
      ConfigFunctionCall<GC>("DisableTile")
    { }

    virtual bool Parse(ExternalConfig<GC>& ec)
    {
      LineCountingByteSource& in = ec.GetByteSource();

      in.SkipWhitespace();

      u32 x, y;

      if(!in.Scan(x))
      {
        return in.Msg(Logger::ERROR, "Expected decimal x-coordinate parameter");
      }

      s32 ret;

      ret = this->SkipToNextArg(in);
      if(ret < 0)
      {
        return false;
      }
      if(ret == 0)
      {
        return in.Msg(Logger::ERROR, "Expected second decimal y-coordinate parameter");
      }

      if(!in.Scan(y))
      {
        return in.Msg(Logger::ERROR, "Expected second decimal y-coordinate parameter");
      }

      SPoint tilePt(x, y);

      ec.SetTileToExecuteOnly(tilePt, false);

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply(ExternalConfig<GC> & ec) { /* Work already done */ }
  };

  template <class GC>
  class FunctionCallSetElementParameter : public ConfigFunctionCall<GC>
  {
    typedef typename GC::CORE_CONFIG CC;

   public:
    FunctionCallSetElementParameter() :
      ConfigFunctionCall<GC>("SetElementParameter")
    { }

    virtual bool Parse(ExternalConfig<GC> & ec)
    {
      LineCountingByteSource & in = ec.GetByteSource();
      in.SkipWhitespace();

      OString16 nick;
      if (!in.ScanIdentifier(nick))
      {
        return in.Msg(Logger::ERROR, "Expected element nickname as first argument");
      }

      Element<CC>* elem = ec.LookupElement(nick);
      if (!elem)
      {
        return in.Msg(Logger::ERROR, "'%s' is not a known element nickname", nick.GetZString());
      }
      ElementParameters<CC> & parms = elem->GetElementParameters();

      if (!this->SkipToNextExistingArg(in,"parameter tag"))
      {
        return false;
      }

      OString32 paramTag;
      if (!in.ScanIdentifier(paramTag))
      {
        return in.Msg(Logger::ERROR, "Expected parameter tag as second argument");
      }

      s32 index = parms.GetParameterNumberFromTag(paramTag.GetZString());
      if (index < 0)
      {
        return in.Msg(Logger::ERROR, "'%s' is not a known parameter tag", paramTag.GetZString());
      }

      ElementParameter<CC> * p = parms.GetParameter((u32) index);
      MFM_API_ASSERT_NONNULL(p);

      if (!this->SkipToNextExistingArg(in,"parameter value"))
      {
        return false; // message already issued
      }

      if (!p->ReadValue(in))
      {
        return in.Msg(Logger::ERROR, "Reading value of parameter tag '%s' failed", paramTag.GetZString());
      }

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in)
    {
      FAIL(UNSUPPORTED_OPERATION);
    }

    virtual void Apply(ExternalConfig<GC> & ec)
    { /* Work already done */ }
  };


  template <class GC>
  void RegisterExternalConfigFunctions(ExternalConfig<GC> & ec)
  {
    {
      static FunctionCallRegisterElement<GC> elt;
      ec.RegisterFunction(elt);
    }
    {
      static FunctionCallGA<GC> elt;
      ec.RegisterFunction(elt);
    }
    {
      static FunctionCallSetElementParameter<GC> elt;
      ec.RegisterFunction(elt);
    }
    {
      static FunctionCallDisableTile<GC> elt;
      ec.RegisterFunction(elt);
    }
  }
}
