/*	JP2_Utilities

HiROC CVS ID: $Id: JP2_Utilities.hh,v 1.1 2012/09/19 00:40:58 castalia Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
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

#ifndef _JP2_Utilities_
#define _JP2_Utilities_

#include	<string>


namespace UA
{
namespace HiRISE
{
class JP2_Utilities
{
public:
/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Utilities:
*/
/**	Tests if the source names a JP2 formatted file.

	To be a valid JP2 formatted file the pathname must exist, be a
	normal readable file, and the first 12 bytes of the file must contain
	the {@link #JP2_SIGNATURE}.

	This function requires a pathname to a local file; it does not work
	with remote JPIP sources. It is intended to provide a quick check that a
	pathname refers to a JP2 file as might be used by a file selection
	filter.

	@param	pathname	A file pathname.
	@param	report	A pointer to a string to which a report will be
		appended that describes the reason for a false condition, or that
		the file is a JP2 formatted if true is returned. If NULL and the
		pathname can not be accessed or does not appear to be JP2
		formatted, and the throw_exception argument is non-zero an
		exception is thrown that contains the report message.
	@param	throw_exception	If positive, or true, an exception will be
		thrown if the pathname or file is not a valid JP2 file. If zero,
		or false, an exception will not be thrown if the pathname or file
		is invalid. If negative (the default) and the pathname or file is
		invalid an exception will be thrown if report is NULL otherwise
		no exception will be thrown.
	@return	true if the file appears to contain JP2 formatted content;
		false otherwise.
	@throws JP2_IO_Failure	If the pathname does not exist, is not a
		regular file, can not be read or can not be opened and an
		exception is to be thrown if the pathname is invalid.
	@throws JP2_Invalid_Argument	If report is NULL and the pathname
		does not refer to a file that starts with the
		JP2_Reader::JP2_SIGNATURE bytes and an exception is to be thrown
		when the file is invalid.
*/
static bool is_JP2_file (const std::string& pathname,
	std::string* report = NULL, int throw_exception = -1);

/**	Tests if a string provides a valid JP2_Reader URL.

	A valid URL is of the form:

	<i>protocol</i><b>://<i>hostname</i>[<b>:</b><i>port</i>]<b>/</b><i>source</i>

	where the <i>protocol</i> is either "jpip" or "http" (case insensitive).

	The <i>hostname</i>, <i>port</i> and <i>source</i> are not checked
	to have valid values, however both the <i>hostname</i> and
	<i>source</i> fields must be present and the <i>port</i> field if
	present must have a numeric value.

	@param	URL	A URL string.
	@param	report	A pointer to a string to which a report will be
		appended that describes the reason for a false condition or that
		the URL appears to be valid if true is returned. If NULL and the
		URL does not appear to be valid, and the throw_exception argument
		is non-zero an exception is thrown that contains the report
		message.
	@param	throw_exception	If positive, or true, an exception will be
		thrown if the URL is not valid. If zero, or false, an exception
		will not be thrown if the URL is not valid. If negative (the
		default) and the URL is invalid an exception will be thrown if
		report is NULL otherwise no exception will be thrown.
	@return	true if the URL appears to be valid; false otherwise.
	@throws JP2_Invalid_Argument	If report is NULL and the URL does not
		appear to be valid.
*/
static bool is_valid_URL (const std::string& URL,
	std::string* report = NULL, int throw_exception = -1);

/**	Tests if a string provides a valid JPIP URL.

	The string must be a {@link is_valid_URL(const std::string&,
	std::string*, int) valid URL} and begin with the "jpip" protocol
	specification.

	@param	URL	A URL string.
	@return	true if the URL appears to be a valid JPIP URL; false otherwise.
*/
static bool is_JPIP_URL (const std::string& URL);


private:
JP2_Utilities () {}

};	//	Class JP2_Utilities


}	//	namespace HiRISE
}	//	namespace UA
#endif
