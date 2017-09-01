//==============================================================================
//
//  InputHandler.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef INPUTHANDLER_H_INCLUDED
#define INPUTHANDLER_H_INCLUDED

#include "Protected.h"
#include "NbTypes.h"
#include "NwTypes.h"
#include "SysTypes.h"

//------------------------------------------------------------------------------

namespace NodeBase
{
//  An input handler receives messages on an IP port that supports an
//  application protocol.  Its functions are invoked from I/O level,
//  which can be one of the following:
//  o an interrupt service routine (two-stage processing),
//  o the IP stack (three-stage processing), or
//  o a subclass of IoThread that uses recvfrom/recv to interface with the
//    IP stack (four-stage processing, which is the implementation assumed
//    here, since the others are platform specific).
//
//  Each interprocessor protocol supported by a node needs an input handler.
//  The reason is that only protocol-specific software can parse an incoming
//  message to decide which application object should eventually process it.
//
//  All messages that use a SessionBase header are supported by a common
//  input handler that invokes InvokerPool::ReceiveMsg on the invoker pool
//  associated with the IP port's scheduler faction.  This input handler is
//  defined in SbInputHandler.h and must be subclassed for each well-known
//  port that receives or sends messages with SessionBase headers.
//
class InputHandler : public Protected
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~InputHandler();

   //  Returns the port that the handler serves.
   //
   IpPort* Port() const { return port_; }

   //  This function is the first one invoked from I/O level.  It returns a
   //  buffer for receiving a message from SOURCE, which is SIZE bytes long.
   //  It sets DEST to the location where the message is to be placed within
   //  that buffer.  If SOURCE contains bundled messages, they are unbundled
   //  into separate buffers by setting RCVD (which is set to SIZE before
   //  invoking this function) to the number of bytes to be read from SOURCE.
   //  The function is then invoked again, after adjusting SOURCE and SIZE.
   //
   //  The default version allocates an IpBuffer and sets DEST to the top of
   //  that buffer in order to receive all of SOURCE.  It must therefore be
   //  overridden if any of the following is true:
   //  o SOURCE can contain multiple messages (e.g. if using TCP)
   //  o the port receiving the message uses a subclass of IpBuffer
   //  o the port receiving the message needs to build an internal header
   //
   //  An internal header is supported by
   //  o the HEADER argument to IpBuffer's constructor and
   //  o the DEST argument to this function, which allows a message without
   //    a header to be placed at an offset from the top of the buffer so
   //    that ReceiveBuff (see below) can subsequently build the header.
   //  The purpose of the header may be to route the message to the correct
   //  object or provide applications with more than merely a byte-oriented
   //  view of the message.  Such a header must be added to external messages
   //  (that is, messages from other systems).  The header can be preserved,
   //  however, even when sent interprocessor between different nodes in the
   //  same network.  See MsgHeader for an example of such a header.
   //
   virtual IpBuffer* AllocBuff
      (const byte_t* source, MsgSize size, byte_t*& dest, MsgSize& rcvd) const;

   //  This function is invoked after a message of SIZE bytes has been copied
   //  into a BUFFER returned by AllocBuff.  FACTION is the scheduler faction
   //  for the IP port on which the message arrived.  If AllocBuff reserved
   //  space for a header, it can now be filled in.  The message should then
   //  be scheduled for processing, usually by queueing it on an application
   //  object which is, in turn, placed on a work queue.
   //
   //  In rare situations (e.g. a garbled message), ReceiveBuff may need to
   //  discard a message.  When this occurs, it should generate a log.
   //
   //  The default version must be overridden: it generates a log and allows
   //  the buffer to be deleted.
   //
   virtual void ReceiveBuff
      (MsgSize size, IpBufferPtr& buff, Faction faction) const;

   //  Overridden to display member variables.
   //
   virtual void Display(std::ostream& stream,
      const std::string& prefix, const Flags& options) const override;

   //  Overridden for patching.
   //
   virtual void Patch(sel_t selector, void* arguments) override;
protected:
   //  Registers the handler with PORT.  Protected because this class is
   //  virtual.
   //
   explicit InputHandler(IpPort* port);
private:
   //  Overridden to prohibit copying.
   //
   InputHandler(const InputHandler& that);
   void operator=(const InputHandler& that);

   //  The port where the input handler is registered.
   //
   IpPort* port_;
};
}
#endif