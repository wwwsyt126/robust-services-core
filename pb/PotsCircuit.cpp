//==============================================================================
//
//  PotsCircuit.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "PotsCircuit.h"
#include <iomanip>
#include <sstream>
#include "Debug.h"
#include "Log.h"
#include "MsgHeader.h"
#include "NbAppIds.h"
#include "PotsProfile.h"
#include "Switch.h"

using std::ostream;
using std::setw;
using std::string;
using namespace SessionBase;
using namespace MediaBase;

//------------------------------------------------------------------------------

namespace PotsBase
{
const PotsCircuit::SignalEntry PotsCircuit::NilSignalEntry = { };

int PotsCircuit::StateCount_[] = { };

//------------------------------------------------------------------------------

fn_name PotsCircuit_ctor = "PotsCircuit.ctor";

PotsCircuit::PotsCircuit(PotsProfile& profile) :
   state_(Idle),
   offhook_(false),
   ringing_(false),
   digits_(false),
   flash_(false),
   cause_(Cause::NilInd),
   profile_(&profile),
   trafficId_(0),
   buffIndex_(0)
{
   Debug::ft(PotsCircuit_ctor);

   for(auto i = 0; i < TraceSize; ++i) trace_[i] = NilSignalEntry;

   StateCount_[state_]++;
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_dtor = "PotsCircuit.dtor";

PotsCircuit::~PotsCircuit()
{
   Debug::ft(PotsCircuit_dtor);

   StateCount_[state_]--;
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_ClearTrafficId = "PotsCircuit.ClearTrafficId";

void PotsCircuit::ClearTrafficId(size_t tid)
{
   Debug::ft(PotsCircuit_ClearTrafficId);

   if(trafficId_ == tid) trafficId_ = 0;
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_CreateMsg = "PotsCircuit.CreateMsg";

Pots_UN_Message* PotsCircuit::CreateMsg(PotsSignal::Id sid) const
{
   Debug::ft(PotsCircuit_CreateMsg);

   auto msg = new Pots_UN_Message(nullptr, 12);
   if(msg == nullptr) return nullptr;

   msg->Header()->injected = true;

   PotsHeaderInfo phi;
   phi.signal = sid;
   phi.port = TsPort();
   msg->AddHeader(phi);

   return msg;
}

//------------------------------------------------------------------------------

void PotsCircuit::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Circuit::Display(stream, prefix, options);

   stream << prefix << "state     : " << state_ << CRLF;
   stream << prefix << "offhook   : " << offhook_ << CRLF;
   stream << prefix << "ringing   : " << ringing_ << CRLF;
   stream << prefix << "digits    : " << digits_ << CRLF;
   stream << prefix << "flash     : " << flash_ << CRLF;
   stream << prefix << "cause     : " << cause_ << CRLF;
   stream << prefix << "trafficId : " << trafficId_ << CRLF;
   stream << prefix << "trace     : " << strTrace() << CRLF;

   if(profile_ != nullptr)
      stream << prefix << "profile DN : " << profile_->GetDN();
   else
      stream << prefix << "profile : undefined";
   stream << CRLF;
}

//------------------------------------------------------------------------------

fixed_string CircuitStateStr[PotsCircuit::State_N] =
{
   "Idle", "Actv", "Orig", "Term", "Lock"
};

void PotsCircuit::DisplayStateCounts(ostream& stream, const string& prefix)
{
   stream << prefix;
   for(auto i = 0; i < State_N; ++i) stream << setw(6) << CircuitStateStr[i];
   stream << CRLF;

   stream << prefix;
   for(auto i = 0; i < State_N; ++i) stream << setw(6) << StateCount_[i];
   stream << CRLF;
}

//------------------------------------------------------------------------------

string PotsCircuit::Name() const
{
   std::ostringstream name;

   name << "POTS ";

   if(profile_ != nullptr)
      name << profile_->GetDN();
   else
      name << "unassigned";

   return name.str();
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_ReceiveMsg = "PotsCircuit.ReceiveMsg";

void PotsCircuit::ReceiveMsg(const Pots_NU_Message& msg)
{
   Debug::ft(PotsCircuit_ReceiveMsg);

   PotsRingInfo* pri;
   PotsScanInfo* psi;
   CauseInfo* pci;

   auto phi = msg.FindType< PotsHeaderInfo >(PotsParameter::Header);

   SignalEntry entry = NilSignalEntry;
   entry.signal = phi->signal & 0x0f;

   switch(phi->signal)
   {
   case PotsSignal::Supervise:
      //
      //  Look for all possible parameters.
      //
      if(state_ == Idle) SetState(Terminator);
      pri = msg.FindType< PotsRingInfo >(PotsParameter::Ring);
      psi = msg.FindType< PotsScanInfo >(PotsParameter::Scan);
      pci = msg.FindType< CauseInfo >(PotsParameter::Cause);

      if(pri != nullptr)
      {
         if(pri->on)
         {
            entry.ringOn = true;
            Trace(entry);
            ringing_ = true;

            if(state_ == Active) SetState(Terminator);

            //  If onhook, send an Alerting unless an alerting timeout is
            //  wanted.  If offhook, send an Offhook because the previous
            //  offhook might have arrived on the ingress queue and been
            //  rejected by overload controls.
            //
            if(!offhook_)
            {
               if(!Debug::SwFlagOn(CipAlertingTimeoutFlag))
               {
                  SendMsg(PotsSignal::Alerting);
               }
            }
            else
            {
               SendMsg(PotsSignal::Offhook);
            }
         }
         else
         {
            entry.ringOff = true;
            Trace(entry);
            ringing_ = false;
         }
      }

      //  Update the events to be reported.
      //
      if(psi != nullptr)
      {
         digits_ = psi->digits;
         flash_ = psi->flash;

         if(digits_)
            entry.digsOn = true;
         else
            entry.digsOff = true;

         if((state_ == Active) && digits_) SetState(Originator);
      }

      if(pci != nullptr) cause_ = pci->cause;
      if(pri == nullptr) Trace(entry);
      return;

   case PotsSignal::Lockout:
      //
      //  Connect silence.  Wait for an onhook.  Report nothing else.
      //
      Trace(entry);
      SetState(LockedOut);
      digits_ = false;
      flash_ = false;
      MakeConn(Switch::SilentPort);
      return;

   case PotsSignal::Release:
      //
      //  Idle the circuit.  If it is offhook, send an offhook immediately.
      //
      Trace(entry);
      pci = msg.FindType< CauseInfo >(PotsParameter::Cause);

      if((pci != nullptr) && (pci->cause == Cause::ResetCircuit))
      {
         ResetCircuit();
         return;
      }

      SetState(offhook_ ? Active : Idle);
      MakeConn(Switch::SilentPort);
      ringing_ = false;
      digits_ = false;
      flash_ = false;
      cause_ = Cause::NilInd;
      if(state_ == Active) SendMsg(PotsSignal::Offhook);
      return;

   default:
      auto log = Log::Create("POTS SHELF INVALID INCOMING SIGNAL");
      if(log == nullptr) return;
      *log << "sig=" << phi->signal << SPACE << strState() << CRLF;
      Log::Spool(log);
      return;
   }
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_ResetCircuit = "PotsCircuit.ResetCircuit";

void PotsCircuit::ResetCircuit()
{
   Debug::ft(PotsCircuit_ResetCircuit);

   //  If the circuit is not in its initial state, reset it and
   //  generate a log.
   //
   bool err;
   string info;
   auto rx = RxFrom();

   err = ((rx != Switch::SilentPort) || (state_ != Idle) || offhook_);
   err = (err || ringing_ || digits_ || flash_ || (cause_ != Cause::NilInd));

   if(err)
   {
      info = strState();
      MakeConn(Switch::SilentPort);
      SetState(Idle);
      offhook_ = false;
      ringing_ = false;
      digits_ = false;
      flash_ = false;
      cause_ = Cause::NilInd;

      auto log = Log::Create("POTS SHELF CIRCUIT RESET");
      if(log == nullptr) return;
      *log << info << CRLF;
      Log::Spool(log);
   }
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_ResetStateCounts = "PotsCircuit.ResetStateCounts";

void PotsCircuit::ResetStateCounts(RestartLevel level)
{
   Debug::ft(PotsCircuit_ResetStateCounts);

   if(level < RestartCold) return;

   for(auto i = 0; i < State_N; ++i) StateCount_[i] = 0;
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_SendMsg1 = "PotsCircuit.SendMsg";

bool PotsCircuit::SendMsg(Pots_UN_Message& msg)
{
   Debug::ft(PotsCircuit_SendMsg1);

   bool ok = false;
   auto sid = msg.GetSignal();

   switch(sid)
   {
   case PotsSignal::Offhook:
      //
      //  Send this even if already offhook. There are two reasons for this.
      //
      //  Overload controls discard origination attempts.  Under the "dial
      //  tone at all costs" approach, an Offhook is therefore periodically
      //  retransmitted so that users who wait patiently (instead of rattling
      //  the switchhook) eventually get dial tone.  POTS call software takes
      //  this into account by discarding a retransmitted offhook.
      //
      //e Race conditions can cause lost messages.  For example, the suspend
      //  timer can expire just as a user goes back offhook.  The call gets
      //  released, the offhook message (queued on the context) gets discarded,
      //  and the circuit receives a Release.  When the circuit processes the
      //  Release, it must retransmit the Offhook.  It can be argued that when
      //  a context is deleted, messages still queued against it (such as the
      //  Offhook) should be reinjected.  This would cause the creation of a
      //  context to process the message, but this has not been implemented.
      //
      offhook_ = true;
      ok = true;
      if(state_ == Idle) SetState(Active);
      break;
   case PotsSignal::Digits:
      ok = digits_;
      break;
   case PotsSignal::Flash:
      ok = flash_;
      break;
   case PotsSignal::Alerting:
      ok = ringing_;
      break;
   case PotsSignal::Onhook:
      if(offhook_)
      {
         offhook_ = false;
         digits_ = false;
         ok = true;
         if(state_ == Active) SetState(Idle);
         break;
      }
   }

   if(ok)
   {
      auto entry = NilSignalEntry;
      entry.signal = sid & 0xf;
      Trace(entry);
      return msg.Send(Message::External);
   }

   //  When msg.Send is invoked, the message is deleted, even on failure.
   //  We should therefore do the same.
   //
   delete &msg;

   auto log = Log::Create("POTS SHELF INVALID OUTGOING SIGNAL");
   if(log == nullptr) return false;
   *log << "sig=" << sid << SPACE << strState() << CRLF;
   Log::Spool(log);
   return false;
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_SendMsg2 = "PotsCircuit.SendMsg(signal)";

bool PotsCircuit::SendMsg(PotsSignal::Id sid)
{
   Debug::ft(PotsCircuit_SendMsg2);

   switch(sid)
   {
   case PotsSignal::Offhook:
   case PotsSignal::Alerting:
   case PotsSignal::Flash:
   case PotsSignal::Onhook:
      break;
   default:
      //
      //  This includes PotsSignal::Digits, which needs a parameter.
      //
      Debug::SwErr(PotsCircuit_SendMsg2, TsPort(), sid);
      return false;
   }

   auto msg = CreateMsg(sid);
   if(msg == nullptr) return false;
   return SendMsg(*msg);
}

//------------------------------------------------------------------------------

fn_name PotsCircuit_SetState = "PotsCircuit.SetState";

void PotsCircuit::SetState(State state)
{
   Debug::ft(PotsCircuit_SetState);

   StateCount_[state_]--;
   state_ = state;
   StateCount_[state_]++;
}

//------------------------------------------------------------------------------

string PotsCircuit::strState() const
{
   std::ostringstream stream;

   stream << "cct=" << Name();
   stream << " p=" << TsPort();
   stream << " rx=" << RxFrom();
   stream << " s=" << state_;
   stream << " h=" << offhook_;
   stream << " r=" << ringing_;
   stream << " d=" << digits_;
   stream << " f=" << flash_;
   stream << " c=" << int(cause_);
   stream << " t=" << trafficId_;
   stream << " m=" << strTrace();

   return stream.str();
}

//------------------------------------------------------------------------------

fixed_string SigChars = "01BDA5E78SLRcdef";

string PotsCircuit::strTrace() const
{
   string s;
   char c;
   size_t i = buffIndex_;

   while(true)
   {
      auto entry = trace_[i];

      if(entry.signal != 0)
      {
         c = SigChars[entry.signal];

         if(entry.signal == PotsSignal::Supervise)
         {
            if(entry.digsOn) c = '@';
            else if(entry.digsOff) c = '#';
            else if(entry.ringOn) c = '*';
            else if(entry.ringOff) c = '.';
         }

         s += c;
      }

      if(++i == TraceSize) i = 0;
      if(i == buffIndex_) break;
   }

   return s;
}

//------------------------------------------------------------------------------

void PotsCircuit::Trace(const SignalEntry& entry)
{
   trace_[buffIndex_] = entry;
   if(++buffIndex_ >= TraceSize) buffIndex_ = 0;
}
}