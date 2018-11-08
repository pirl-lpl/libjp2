/*	JP2_Utilities

HiROC CVS ID: $Id: JP2_Utilities.cc,v 1.1 2012/09/19 00:40:58 castalia Exp $

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

#include	"JP2_Utilities.hh"

#include	"JP2_Exception.hh"

//	PIRL++
#include	"Files.hh"
using namespace PIRL;

#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
#include	<fstream>
using std::ifstream;
#include	<iomanip>
using std::endl;


#if defined (DEBUG)
/*	DEBUG controls

	DEBUG report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_UTILITIES		(1 << 1)

#include	<iostream>
using std::clog;
using std::boolalpha;
using std::hex;
using std::dec;
using std::setfill;
#endif	//	DEBUG


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
const char* const
	JP2_Utilities::ID =
		"UA::HiRISE::JP2_Utilities ($Revision: 1.1 $ $Date: 2012/09/19 00:40:58 $)";

namespace
{
//!	The JP2 file format signature in the first 12 bytes of a file.
const unsigned char
	JP2_SIGNATURE[12] =
		{
		0x00, 0x00, 0x00, 0x0C,		//	Box size: 12 bytes
		0x6A, 0x50, 0x20, 0x20,		//	Signature: "jP  "
		0x0D, 0x0A, 0x87, 0x0A		//	Content: <CR><LF>0x87<LF>
		};
}

/*==============================================================================
	Utilities:
*/
bool
JP2_Utilities::is_JP2_file
	(
	const std::string&	pathname,
	std::string*		report,
	int					throw_exception
	)
{
#if ((DEBUG) & DEBUG_UTILITIES)
clog << ">>> JP2::is_JP2_file: " << pathname << endl;
#endif
if (report &&
	throw_exception < 0)
	throw_exception = 0;
ostringstream
	message;
message << "The " << pathname << " pathname" << endl;

if (! file_exists (pathname))
	{
	message << "does not exist.";

	IOS_Failure:
	if (throw_exception)
		throw JP2_IO_Failure (message.str (), JP2_Utilities::ID);

	Invalid:
	if (report)
		*report += message.str ();
	#if ((DEBUG) & DEBUG_UTILITIES)
	clog << message.str () << endl
		 << "<<< is_JP2_file: false" << endl;
	#endif
	return false;
	}
if (! file_is_normal (pathname))
	{
	message << "does not refer to a normal file.";
	goto IOS_Failure;
	}
if (! file_is_readable (pathname))
	{
	message << "does not refer to a readable file.";
	goto IOS_Failure;
	}
ifstream
	file (pathname.c_str (),
		//	Avoid CR-LF filtering by MS/Windows.
		std::ios::in | std::ios::binary);
if (! file)
	{
	message << "does not refer to a file that can be opened.";
	goto IOS_Failure;
	}
unsigned char
	characters[sizeof (JP2_SIGNATURE)];
if (! file.read ((char*)characters, sizeof (characters)))
	{
	file.close ();
	message << "does not refer to a file from which "
			<< sizeof (characters) << " bytes can be read.";
	Invalid_Argument:
	if (throw_exception)
		throw JP2_Invalid_Argument (message.str (), JP2_Utilities::ID);
	goto Invalid;
	}
file.close ();
#if ((DEBUG) & DEBUG_UTILITIES)
clog << "    JP2_SIGNATURE check -" << endl;
#endif
for (unsigned int
		index = 0;
		index < sizeof (characters);
		index++)
	{
	#if ((DEBUG) & DEBUG_UTILITIES)
	clog << "    " << ((unsigned int)characters[index] & 0xFF)
			<< " - " << ((unsigned int)JP2_SIGNATURE[index] & 0xFF) << endl;
	#endif
	if (characters[index] != JP2_SIGNATURE[index])
		{
		message << "does not refer to a file beginning with the JP2 signature.";
		goto Invalid_Argument;
		}
	}

if (report)
	{
	message << "appears to be a valid JP2 formatted file.";
	*report += message.str ();
	}
#if ((DEBUG) & DEBUG_UTILITIES)
clog << message.str () << endl
	 << "<<< JP2::is_JP2_file: true" << endl;
#endif
return true;
}


bool
JP2_Utilities::is_valid_URL
	(
	const std::string&	URL,
	std::string*		report,
	int					throw_exception
	)
{
#if ((DEBUG) & DEBUG_UTILITIES)
clog << ">>> JP2::is_valid_URL: " << URL << endl;
#endif
if (report &&
	throw_exception < 0)
	throw_exception = 0;
ostringstream
	message;
message << "The " << URL << " URL" << endl;

string::size_type
	first = URL.find ("://");
if (first == string::npos)
	{
	message << "does not contain a \"://\" protocol field delimiter.";
	Invalid:
	if (throw_exception)
		throw JP2_Invalid_Argument (message.str (), JP2_Utilities::ID);
	if (report)
		*report += message.str ();
	#if ((DEBUG) & DEBUG_UTILITIES)
	clog << message.str () << endl
		 << "<<< JP2::is_valid_URL: false" << endl;
	#endif
	return false;
	}
string
	part = URL.substr (0, first);
for (string::iterator
		character = part.begin ();
		character != part.end ();
		++character)
    *character = toupper (*character);
if (part != "JPIP" &&
	part != "HTTP")
	{
	message << "protocol must be either \"jpip\" or \"http\".";
	goto Invalid;
	}

if (URL.length () == 7)
	{
	message << "does not have a hostname field.";
	goto Invalid;
	}

string::size_type
	last = URL.find ('/', first += 3);
if (last == first)
	{
	message << "hostname field is empty.";
	goto Invalid;
	}
if (last == string::npos ||
	last == (URL.length () - 1))
	{
	message << "does not have a file source field.";
	goto Invalid;
	}

first = URL.rfind (':', last) + 1;
if (first != 5)
	{
	if (first == last)
		{
		message << "port number field is empty.";
		goto Invalid;
		}
	if (URL.substr (first, last - first).find_first_not_of ("0123456789")
		!= string::npos)
		{
		message << "port number field is non-numeric.";
		goto Invalid;
		}
	}

if (report)
	{
	message << "appears to be valid.";
	*report += message.str ();
	}
#if ((DEBUG) & DEBUG_UTILITIES)
clog << message.str () << endl
	 << "<<< JP2::is_valid_URL: true" << endl;
#endif
return true;
}


bool
JP2_Utilities::is_JPIP_URL
	(
	const std::string&	URL
	)
{
if (is_valid_URL (URL, 0, 0))
	{
	string::size_type
		first = URL.find (':');
	if (first != string::npos)
		{
		string
			part = URL.substr (0, first);
		for (string::iterator
				character = part.begin ();
				character != part.end ();
				++character)
    		*character = toupper (*character);
		if (part == "JPIP")
			return true;
		}
	}
return false;
}

}	//	namespace HiRISE
}	//	namespace UA
