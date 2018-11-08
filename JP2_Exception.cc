/*	JP2_Exception

HiROC CVS ID: $Id: JP2_Exception.cc,v 1.3 2010/01/30 22:24:09 castalia Exp $

Copyright (C) 2009-2010  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/

#include	"JP2_Exception.hh"

#include	<exception>
#include	<string>
using std::string;
#include	<cstring>


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants:
*/
const char* const
	JP2_Exception::ID =
		"UA::HiRISE::JP2_Exception ($Revision: 1.3 $ $Date: 2010/01/30 22:24:09 $)";

/*==============================================================================
	Constructor
*/
JP2_Exception::JP2_Exception
	(
	const std::string&	message,
	const char*			caller_ID
	)
		//	Prepend the Exception ID to the Message.
	:	Message
			(
			string (ID)
			+ (caller_ID ? (string ("\n") + caller_ID) : "")
			+ '\n' + message
			)
{
if (! Message.empty () && Message[Message.size () - 1] == '\n')
	//	Remove the trailing new-line.
	Message.erase (Message.size () - 1);
User_Message_Index = strlen (ID) + (caller_ID ? (strlen (caller_ID) + 2) : 1);
}

/*==============================================================================
	Accessors:
*/
const char*
JP2_Exception::what ()
	const throw ()
{return Message.c_str ();}


std::string
JP2_Exception::message ()
	const throw ()
{
// Exclude the JP2_Exception ID; include any caller_ID.
string::size_type
	index = strlen (ID) + 1;
if (index > Message.size ())
	index = 0;				// No Exception ID!?!
return Message.substr (index);
}


void
JP2_Exception::message
	(
	const std::string&	new_message
	)
{
if (User_Message_Index >= Message.length ())
	//	No user message to replace; append on the next line.
	(Message += '\n') += new_message;
else
	//	Replace the trailing user message portion of the Message.
	Message.replace (User_Message_Index, Message.length () - User_Message_Index,
		new_message);
if (! Message.empty () && Message[Message.size () - 1] == '\n')
	//	Remove the trailing new-line.
	Message.erase (Message.size () - 1);
}


}	//	namespace HiRISE
}	//	namespace UA
