//==============================================================================
//
//  MscContext.cpp
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#include "MscContext.h"
#include <ostream>
#include "Algorithms.h"
#include "Debug.h"
#include "Factory.h"
#include "FactoryRegistry.h"
#include "Formatters.h"
#include "Service.h"
#include "ServiceRegistry.h"
#include "Singleton.h"
#include "SysTypes.h"

using std::ostream;
using std::string;

//------------------------------------------------------------------------------

namespace SessionTools
{
fn_name MscContext_ctor = "MscContext.ctor";

MscContext::MscContext(const void* rcvr, ContextType type, uint16_t cid) :
   rcvr_(rcvr),
   type_(type),
   cid_(cid),
   col_(NilMscColumn),
   group_(0)
{
   Debug::ft(MscContext_ctor);
}

//------------------------------------------------------------------------------

fn_name MscContext_dtor = "MscContext.dtor";

MscContext::~MscContext()
{
   Debug::ft(MscContext_dtor);
}

//------------------------------------------------------------------------------

fn_name MscContext_ClearGroup = "MscContext.ClearGroup";

void MscContext::ClearGroup()
{
   Debug::ft(MscContext_ClearGroup);

   if(rcvr_ == nullptr) group_ = 0;
}

//------------------------------------------------------------------------------

void MscContext::Display(ostream& stream,
   const string& prefix, const Flags& options) const
{
   Temporary::Display(stream, prefix, options);

   stream << prefix << "rcvr  : " << rcvr_ << CRLF;
   stream << prefix << "type  : " << type_ << CRLF;
   stream << prefix << "cid   : " << cid_ << CRLF;
   stream << prefix << "col   : " << col_ << CRLF;
   stream << prefix << "group : " << group_ << CRLF;
}

//------------------------------------------------------------------------------

bool MscContext::IsEqualTo(const void* rcvr, uint16_t cid) const
{
   if(rcvr == nullptr) return ((rcvr_ == nullptr) && (cid_ == cid));
   return (rcvr_ == rcvr);
}

//------------------------------------------------------------------------------

ptrdiff_t MscContext::LinkDiff()
{
   int local;
   auto fake = reinterpret_cast< const MscContext* >(&local);
   return ptrdiff(&fake->link_, fake);
}

//------------------------------------------------------------------------------

fn_name MscContext_Names = "MscContext.Names";

void MscContext::Names(string& text1, string& text2) const
{
   Debug::ft(MscContext_Names);

   if((rcvr_ == nullptr) && (cid_ == NIL_ID))
   {
      text1 = "External";
      text2 = "Contexts";
      return;
   }

   if(type_ == MultiPort)
   {
      auto reg = Singleton< ServiceRegistry >::Instance();
      text1 = strClass(reg->GetService(cid_), false);
   }
   else
   {
      auto reg = Singleton< FactoryRegistry >::Instance();
      text1 = strClass(reg->GetFactory(FactoryId(cid_)), false);
   }

   text2 = strContextType(type_);
   text2.push_back(':');

   if(rcvr_ != nullptr)
      text2 += strPtr(rcvr_);
   else
      text2 += "external";
}

//------------------------------------------------------------------------------

fn_name MscContext_SetColumn = "MscContext.SetColumn";

MscColumn MscContext::SetColumn(MscColumn col)
{
   Debug::ft(MscContext_SetColumn);

   col_ = col;
   return col + ColWidth;
}

//------------------------------------------------------------------------------

fn_name MscContext_SetGroup = "MscContext.SetGroup";

bool MscContext::SetGroup(int group)
{
   Debug::ft(MscContext_SetGroup);

   if(rcvr_ == nullptr)
   {
      //  An external context is always assigned to the current group,
      //  but it cannot begin a new group.
      //
      group_ = group;
      return false;
   }

   if(group_ == 0)
   {
      //  An internal context is only assigned to the current group if
      //  it does not yet have a group, and this can begin a new group.
      //
      group_ = group;
      return true;
   }

   //  This internal context is already assigned to a group.
   //
   return false;
}
}