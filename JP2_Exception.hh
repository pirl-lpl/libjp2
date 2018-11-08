/*	Jp2_Exception

HiROC CVS ID: $Id: JP2_Exception.hh,v 1.6 2010/10/03 03:21:20 castalia Exp $

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

#ifndef _JP2_Exception_
#define _JP2_Exception_

#include	<exception>
#include	<stdexcept>
#include	<string>
#include	<ios>

namespace UA
{
namespace HiRISE
{
/**	A <i>JP2_Exception</i> is thrown by a JP2 class.

	For all JP2_Exceptions a message string is provided that describes the
	reason for the Exception. The message method will return this message,
	while the what method will prepend the JP2_Exception class ID to the
	message.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.6 $
*/
class JP2_Exception
:	public std::exception
{
public:
/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Constructor
*/
/**	Creates a JP2_Exception containing a message.

	If the message ends with a new-line character ('\\n'), it is removed
	(but only one is removed). If a caller_ID is provided, it preceeds
	the message string separated by a new-line character.

	@param	message	The message string.
	@param	caller_ID	An optional C-string (NULL terminated char*)
		identifying the source of the Exception.
*/
explicit JP2_Exception
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	);

//	Destructor
virtual ~JP2_Exception () throw ()
	{}

/*==============================================================================
	Accessors:
*/
/**	Gets a C-string describing what produced the Exception.

	@return	The C-string (NULL terminated char*) that includes the
		Exception ID as the first line followed by the caller_ID and
		message string.
*/
const char* what () const throw ();

/**	Gets the user-provided caller_ID and message string.

	@return	The message string. The caller_ID followed by the user
		message.
*/
std::string message () const throw ();

/**	Sets a new message string.

	If the message ends with a new-line character ('\\n'), it is removed
	(but only one is removed). If a caller_ID was provided when the
	Exception was created it remains; the new message is appended in
	place of the previous message.

	@param	message	A string to replace the previous message.
*/
void message (const std::string& message);

/*==============================================================================
	Private Data.
*/
private:
std::string
	Message;
std::string::size_type
	User_Message_Index;
};

/*------------------------------------------------------------------------------
	JP2_Invalid_Argument
*/
//!	A JP2_Exception for an invalid argument.
struct JP2_Invalid_Argument
:	public JP2_Exception,
	public std::invalid_argument
{
explicit JP2_Invalid_Argument
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JP2_Exception (std::string ("JP2_Invalid_Argument\n") + message,
			caller_ID),
		std::invalid_argument
			(
			std::string (JP2_Exception::ID)
			+ (message.empty () ?
				"" : (std::string ("\nJP2_Invalid_Argument\n") + message))
			+ (caller_ID ?
				(std::string ("\n") + caller_ID) : "")
			)
	{}
};

/*------------------------------------------------------------------------------
	JP2_Out_of_Range
*/
/**	A JP2_Out_of_Range exception is thrown when a value is found to be out
	of its allowable range.
*/
struct JP2_Out_of_Range
:	public JP2_Exception,
	public std::out_of_range
{
explicit JP2_Out_of_Range
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JP2_Exception (std::string ("JP2_Out_of_Range\n") + message, caller_ID),
		std::out_of_range
			(
			std::string (JP2_Exception::ID)
			+ (message.empty () ?
				"" : (std::string ("\nJP2_Out_of_Range\n") + message))
			+ (caller_ID ?
				(std::string ("\n") + caller_ID) : "")
			)
	{}
};

/*------------------------------------------------------------------------------
	JP2_Logic_Error
*/
/**	A <i>JP2_Logic_Error</i> indicates a logical inconsistency in the object.

	Typically a JP2_Logic_Error is thrown when the object affected is not in
	a state that can carry out the requested operation. For example, this
	exception will be thrown if a JP2_Reader is requested to render from its
	source by the object is not ready due to an incomplete or inconsistent
	configuration.
*/
struct JP2_Logic_Error
:	public JP2_Exception,
	public std::logic_error
{
explicit JP2_Logic_Error
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JP2_Exception (std::string ("JP2_Logic_Error\n") + message, caller_ID),
		std::logic_error
			(
			std::string (JP2_Exception::ID)
			+ (message.empty () ?
				"" : (std::string ("\nJP2_Logic_Error\n") + message))
			+ (caller_ID ?
				(std::string ("\n") + caller_ID) : "")
			)
	{}
};

/*------------------------------------------------------------------------------
	IO_Failure
*/
//!	A JP2_IO_Failure exception is for an I/O failure condtion.
struct JP2_IO_Failure
:	public JP2_Exception,
	public std::ios::failure
{
explicit JP2_IO_Failure
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JP2_Exception (std::string ("JP2_IO_Failure\n") + message, caller_ID),
		std::ios_base::failure
			(
			std::string (JP2_Exception::ID)
			+ (message.empty () ?
				"" : (std::string ("\nJP2_IO_Failure\n") + message))
			+ (caller_ID ?
				(std::string ("\n") + caller_ID) : "")
			)
	{}
};

/*=*****************************************************************************
	JPIP_Exception
*/
//!	A JPIP_Exception is associated with the interaction with a JPIP server.
struct JPIP_Exception
:	public JP2_Exception
{
explicit JPIP_Exception
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JP2_Exception (std::string ("JPIP_Exception\n") + message, caller_ID)
	{}
};

/*------------------------------------------------------------------------------
	JPIP_Timeout
*/
/**	A JPIP_Timeout occurs when a request does not complete within the
	allowed time.
*/
struct JPIP_Timeout
:	public JPIP_Exception,
	public std::overflow_error
{
explicit JPIP_Timeout
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JPIP_Exception (std::string ("JPIP_Timeout\n") + message, caller_ID),
		std::overflow_error
			(
			std::string (JP2_Exception::ID)
			+ (message.empty () ?
				"" : (std::string ("\nJPIP_Timeout\n") + message))
			+ (caller_ID ?
				(std::string ("\n") + caller_ID) : "")
			)
	{}
};

/*------------------------------------------------------------------------------
	JPIP_Disconnected
*/
/**	A JPIP_Disconnected occurs when a JPIP server connection can not be
	accomplished or a JPIP server disconnection occurs.
*/
struct JPIP_Disconnected
:	public JPIP_Exception,
	public std::ios::failure
{
explicit JPIP_Disconnected
	(
	const std::string&	message = "",
	const char*			caller_ID = NULL
	)
	:	JPIP_Exception (std::string ("JPIP_Disconnected\n") + message, caller_ID),
		std::ios::failure
			(
			std::string (JP2_Exception::ID)
			+ (message.empty () ?
				"" : (std::string ("\nJPIP_Disconnected\n") + message))
			+ (caller_ID ?
				(std::string ("\n") + caller_ID) : "")
			)
	{}
};


}	//	namespace HiRISE
}	//	namespace UA
#endif	//	_JP2_Exception_
