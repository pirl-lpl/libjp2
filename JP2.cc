/*	JP2

HiROC CVS ID: $Id: JP2.cc,v 1.11 2012/09/19 00:41:44 castalia Exp $

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

#include	"JP2.hh"

#include	"JP2_Reader.hh"
#include	"JP2_Exception.hh"
#include	"JP2_Utilities.hh"

//	Kakadu implementation.
#include	"Kakadu/JP2_File_Reader.hh"
using UA::HiRISE::Kakadu::JP2_File_Reader;
#include	"Kakadu/JP2_JPIP_Reader.hh"
using UA::HiRISE::Kakadu::JP2_JPIP_Reader;

//	PIRL++
//#include	"Files.hh"
//using namespace PIRL;

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
/*******************************************************************************
	JP2
*/
/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
const char* const
	JP2::ID =
		"UA::HiRISE::JP2 ($Revision: 1.11 $ $Date: 2012/09/19 00:41:44 $)";

/*==============================================================================
	Static methods:
*/
JP2_Reader*
JP2::reader
	(
	const std::string&	source
	)
{
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2::reader: " << source << endl;
#endif
if (source.empty ())
	throw JP2_Invalid_Argument
		("Can't create a JP2_Reader without a source.", ID);

bool
	URL_source = false;
string
	name (source);
if (name.compare (0, 5, "file:") == 0)
	{
	string::size_type
		index = 5;
	if (name[index] == '/')
		while ((index + 1) < name.size () &&
				name[index + 1] == '/')
				++index;
	name.erase (0, index);
	#if ((DEBUG) & DEBUG_CONSTRUCTORS)
	clog << "    The source name references a file - " << name << endl;
	#endif
	}
else
if (name.find ("://") != string::npos)
	{
	#if ((DEBUG) & DEBUG_CONSTRUCTORS)
	clog << "    The source name appears to be a URL" << endl;
	#endif
	URL_source = true;
	}

JP2_Reader
	*reader = NULL;
string
	report;
if (URL_source &&
	JP2_Utilities::is_valid_URL (name, &report))
	reader = new JP2_JPIP_Reader ();
else
if (JP2_Utilities::is_JP2_file (name, &report))
	reader = new JP2_File_Reader ();
else
	{
	ostringstream
		message;
	message
		<< "Failed to create a JP2 " << (URL_source ? "JPIP server " : "")
			<< "reader" << endl
		<< "for the " << name << " source." << endl
		<< report;
	throw JP2_Invalid_Argument (message.str (), ID);
	}

try {reader->open (name);}
catch (JP2_Exception& except)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Failed to create a JP2 " << (URL_source ? "JPIP server " : "")
			<< "reader" << endl
		<< "for the " << name << " source." << endl
		<< except.message ();
	except.message (message.str ());
	throw;
	}
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << "<<< JP2::reader" << endl;
#endif
return reader;
}


JP2_Reader*
JP2::copy
	(
	const JP2_Reader&	JP2_reader
	)
{return copy (&JP2_reader);}


JP2_Reader*
JP2::copy
	(
	const JP2_Reader*	JP2_reader
	)
{
if (! JP2_reader)
	return NULL;

#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2::copy: " << JP2_reader->source_name () << endl;
#endif
JP2_Reader
	*new_reader = NULL;
/*
	>>> WARNING <<< The order of the tests matters
	because a JP2_JPIP_Reader is a JP2_File_Reader, but not vice versa.
*/
if (const JP2_JPIP_Reader* reader =
		dynamic_cast<const JP2_JPIP_Reader*>(JP2_reader))
	new_reader = new JP2_JPIP_Reader (*reader);
else
if (const JP2_File_Reader* reader =
		dynamic_cast<const JP2_File_Reader*>(JP2_reader))
	new_reader = new JP2_File_Reader (*reader);
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2::copy: " << JP2_reader->source_name () << endl;
#endif
return new_reader;
}


}	//	namespace HiRISE
}	//	namespace UA
