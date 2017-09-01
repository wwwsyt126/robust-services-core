//==============================================================================
//
//  ToneRegistry.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "ToneRegistry.h"
#include <ostream>
#include <string>
#include "Debug.h"
#include "Formatters.h"
#include "Singleton.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace MediaBase
{
fn_name ToneRegistry_ctor = "ToneRegistry.ctor";

ToneRegistry::ToneRegistry()
{
   Debug::ft(ToneRegistry_ctor);

   tones_.Init(Tone::MaxId + 1, Tone::CellDiff(), MemDyn);
}

//------------------------------------------------------------------------------

fn_name ToneRegistry_dtor = "ToneRegistry.dtor";

ToneRegistry::~ToneRegistry()
{
   Debug::ft(ToneRegistry_dtor);
}

//------------------------------------------------------------------------------

fn_name ToneRegistry_BindTone = "ToneRegistry.BindTone";

bool ToneRegistry::BindTone(Tone& tone)
{
   Debug::ft(ToneRegistry_BindTone);

   return tones_.Insert(tone);
}

//------------------------------------------------------------------------------

void ToneRegistry::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Dynamic::Display(stream, prefix, options);

   stream << prefix << "tones [Tone::Id]" << CRLF;
   tones_.Display(stream, prefix + spaces(2), options);
}

//------------------------------------------------------------------------------

Tone* ToneRegistry::GetTone(Tone::Id tid) const
{
   return tones_.At(tid);
}

//------------------------------------------------------------------------------

fn_name ToneRegistry_ToneToPort = "ToneRegistry.ToneToPort";

Switch::PortId ToneRegistry::ToneToPort(Tone::Id tid)
{
   Debug::ft(ToneRegistry_ToneToPort);

   auto tone = Singleton< ToneRegistry >::Instance()->GetTone(tid);
   if(tone != nullptr) return tone->TsPort();
   return NIL_ID;
}

//------------------------------------------------------------------------------

fn_name ToneRegistry_UnbindTone = "ToneRegistry.UnbindTone";

void ToneRegistry::UnbindTone(Tone& tone)
{
   Debug::ft(ToneRegistry_UnbindTone);

   tones_.Erase(tone);
}
}