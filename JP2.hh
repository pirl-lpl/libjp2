/*	JP2

HiROC CVS ID: $Id: JP2.hh,v 1.11 2012/09/19 00:41:44 castalia Exp $

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

#ifndef _JP2_
#define _JP2_

/*	<b.N.B.</b>: JP2.hh is a "super-header".

	This single header can be included by applications when the JP2
	module is to be used. All other top-level interface definitions
	will automatically be included.
*/
#include	"JP2_Reader.hh"
#include	"JP2_Exception.hh"
#include	"JP2_Utilities.hh"

#include	<string>


namespace UA
{
namespace HiRISE
{
//	Forward reference.
class JP2_Reader;

/**	The <i>JP2</i> class provides a simple interface to the JP2 module.

	Factory functions are provided for constructing and copying an
	abstract JP2_Reader. All the interfaces needed to use a JP2_Reader
	and its JP2_Metadata, the exceptions that might be thrown
	(JP2_Exception), and the JP2_Utilities are included.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.11 $
*/
class JP2
{
public:
/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/**	Construct and open a suitable JP2_Reader for the source.

	If the source name appears to be a {@link is_valid_URL(const
	std::string, std::string*, int) valid URL} a JP2_JPIP_Reader is
	constructed. For a URL with a "file" protocol the source name (after
	the "://" delimiter) is taken to be a local filesystem pathname. If
	the source source name is for a {@link is_JP2_file(const std::string,
	std::string*, int) JP2 file} a JP2_File_Reader is constructed.
	Otherwise a JP2_Invalid_Argument exception is thrown.

	The JP2_Reader that is constructed is used to open the source.
	
	@param	source	The pathname to a JP2 file obtained from the local
		filesystem, or a URL reference to a JP2 file obtained from a JPIP
		server. An empty source name is invalid.
	@return	A pointer to a JP2_Reader for the source.
	@throw	JP2_Invalild_Argument	If the source name does not provide a
		valid reference to a JP2 source file.
	@throw	JP2_Exception	If the source could not be opened with a
		JP2_Reader.
*/
static JP2_Reader* reader (const std::string& source);

/**	Construct a copy of a JP2_Reader.

	The new JP2_Reader will use the same source being used by the
	JP2_Reader that was copied. For a JP2_JPIP_Reader the same JPIP
	server connection will be shared.

	@param	JP2_reader	A reference to a JP2_Reader.
	@return	A new JP2_Reader object that is a copy of the specified
		JP2_Reader.
	@throws JP2_Exception	If the specifed JP2_Reader could not be
		copied.
*/
static JP2_Reader* copy (const JP2_Reader& JP2_reader);

/**	Construct a copy of a JP2_Reader.

	The new JP2_Reader will use the same source being used by the
	JP2_Reader that was copied. For a JP2_JPIP_Reader the same JPIP
	server connection will be shared.

	@param	JP2_reader	A pointer to a JP2_Reader.
	@return	A new JP2_Reader object that is a copy of the specified
		JP2_Reader. This will be NULL if the specified JP2_Reader
		pointer is NULL.
	@throws JP2_Exception	If the specifed JP2_Reader could not be
		copied.
*/
static JP2_Reader* copy (const JP2_Reader* JP2_reader);

private:
JP2 () {}

};	//	Class JP2


}	//	namespace HiRISE
}	//	namespace UA
#endif
