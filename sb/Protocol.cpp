//==============================================================================
//
//  Protocol.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "Protocol.h"
#include <ostream>
#include "Algorithms.h"
#include "Debug.h"
#include "Formatters.h"
#include "Parameter.h"
#include "ProtocolRegistry.h"
#include "SbCliParms.h"
#include "Signal.h"
#include "Singleton.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace SessionBase
{
fn_name Protocol_ctor = "Protocol.ctor";

Protocol::Protocol(Id prid, Id base) : base_(base)
{
   Debug::ft(Protocol_ctor);

   signals_.Init(Signal::MaxId + 1, Signal::CellDiff(), MemProt);
   parameters_.Init(Parameter::MaxId + 1, Parameter::CellDiff(), MemProt);

   prid_.SetId(prid);
   Singleton< ProtocolRegistry >::Instance()->BindProtocol(*this);
}

//------------------------------------------------------------------------------

fn_name Protocol_dtor = "Protocol.dtor";

Protocol::~Protocol()
{
   Debug::ft(Protocol_dtor);

   Singleton< ProtocolRegistry >::Instance()->UnbindProtocol(*this);
}

//------------------------------------------------------------------------------

fn_name Protocol_BindParameter = "Protocol.BindParameter";

bool Protocol::BindParameter(Parameter& parameter)
{
   Debug::ft(Protocol_BindParameter);

   return parameters_.Insert(parameter);
}

//------------------------------------------------------------------------------

fn_name Protocol_BindSignal = "Protocol.BindSignal";

bool Protocol::BindSignal(Signal& signal)
{
   Debug::ft(Protocol_BindSignal);

   return signals_.Insert(signal);
}

//------------------------------------------------------------------------------

ptrdiff_t Protocol::CellDiff()
{
   int local;
   auto fake = reinterpret_cast< const Protocol* >(&local);
   return ptrdiff(&fake->prid_, fake);
}

//------------------------------------------------------------------------------

void Protocol::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Protected::Display(stream, prefix, options);

   stream << prefix << "prid : " << prid_.to_str() << CRLF;
   stream << prefix << "base : " << base_ << CRLF;
   stream << prefix << "signals [SignalId]" << CRLF;
   signals_.Display(stream, prefix + spaces(2), options);
   stream << prefix << "parameters [ParameterId]" << CRLF;
   parameters_.Display(stream, prefix + spaces(2), options);
}

//------------------------------------------------------------------------------

void Protocol::DisplayMsg(ostream& stream,
   const string& prefix, const SbIpBuffer& buff) const
{
   stream << prefix << NoProtocolDisplay << CRLF;
}

//------------------------------------------------------------------------------

fn_name Protocol_ExtractSignal = "Protocol.ExtractSignal";

SignalId Protocol::ExtractSignal(const SbIpBuffer& buff) const
{
   Debug::ft(Protocol_ExtractSignal);

   //  This is a pure virtual function.
   //
   Debug::SwErr(Protocol_ExtractSignal, Prid(), 0, ErrorLog);
   return NIL_ID;
}

//------------------------------------------------------------------------------

Parameter* Protocol::FirstParm() const
{
   for(auto pid = 1; pid <= Parameter::MaxId; ++pid)
   {
      auto parm = GetParameter(pid);
      if(parm != nullptr) return parm;
   }

   return nullptr;
}

//------------------------------------------------------------------------------

Signal* Protocol::FirstSignal() const
{
   for(auto sid = 1; sid <= Signal::MaxId; ++sid)
   {
      auto sig = GetSignal(sid);
      if(sig != nullptr) return sig;
   }

   return nullptr;
}

//------------------------------------------------------------------------------

Parameter* Protocol::GetParameter(ParameterId pid) const
{
   auto parm = parameters_.At(pid);
   if(parm != nullptr) return parm;
   if(base_ == NIL_ID) return nullptr;
   auto pro = Singleton< ProtocolRegistry >::Instance()->GetProtocol(base_);
   if(pro == nullptr) return nullptr;
   return pro->GetParameter(pid);
}

//------------------------------------------------------------------------------

Signal* Protocol::GetSignal(SignalId sid) const
{
   auto sig = signals_.At(sid);
   if(sig != nullptr) return sig;
   if(base_ == NIL_ID) return nullptr;
   auto pro = Singleton< ProtocolRegistry >::Instance()->GetProtocol(base_);
   if(pro == nullptr) return nullptr;
   return pro->GetSignal(sid);
}

//------------------------------------------------------------------------------

void Protocol::NextParm(Parameter*& parm) const
{
   if(parm == nullptr) return;

   for(auto pid = parm->Pid() + 1; pid <= Parameter::MaxId; ++pid)
   {
      parm = GetParameter(pid);
      if(parm != nullptr) return;
   }
}

//------------------------------------------------------------------------------

void Protocol::NextSignal(Signal*& sig) const
{
   if(sig == nullptr) return;

   for(auto sid = sig->Sid() + 1; sid <= Signal::MaxId; ++sid)
   {
      sig = GetSignal(sid);
      if(sig != nullptr) return;
   }
}

//------------------------------------------------------------------------------

void Protocol::Patch(sel_t selector, void* arguments)
{
   Protected::Patch(selector, arguments);
}

//------------------------------------------------------------------------------

fn_name Protocol_UnbindParameter = "Protocol.UnbindParameter";

void Protocol::UnbindParameter(Parameter& parameter)
{
   Debug::ft(Protocol_UnbindParameter);

   parameters_.Erase(parameter);
}

//------------------------------------------------------------------------------

fn_name Protocol_UnbindSignal = "Protocol.UnbindSignal";

void Protocol::UnbindSignal(Signal& signal)
{
   Debug::ft(Protocol_UnbindSignal);

   signals_.Erase(signal);
}

//------------------------------------------------------------------------------

fn_name Protocol_Understands = "Protocol.Understands";

bool Protocol::Understands(Id prid1, Id prid2)
{
   Debug::ft(Protocol_Understands);

   if(prid1 == prid2) return true;

   auto reg = Singleton< ProtocolRegistry >::Instance();
   auto pro = reg->GetProtocol(prid1);

   while(pro != nullptr)
   {
      if(pro->base_ == prid2) return true;
      pro = reg->GetProtocol(pro->base_);
   }

   return false;
}
}