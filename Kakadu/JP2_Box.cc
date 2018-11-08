/*	JP2_Box

HiROC CVS ID: $Id: JP2_Box.cc,v 1.1 2012/04/26 00:39:32 castalia Exp $

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

#ifndef DOXYGEN_PROCESSING

#include "jp2.h"
using namespace kdu_supp;
#include	"JP2_Box.hh"

#include	"JP2_Metadata.hh"

#include	<ostream>
using std::hex;
using std::dec;
using std::endl;


namespace UA
{
namespace HiRISE
{
namespace Kakadu
{
std::ostream&
operator<<
	(
	std::ostream&	stream,
	JP2_Box&		Box
	)
{
if (Box.exists ())
	{
	JP2_Box
		box;
	box.open (Box.source (), Box.get_locator ());
   kdu_supp::jp2_locator
		locator (box.get_locator ());
	stream
		<< "    JP2 " << JP2_Metadata::box_name (box.get_box_type ())
			<< "/\"" << JP2_Metadata::type_name (box.get_box_type ())
			<< "\" (0x" << hex << box.get_box_type () << dec << ") box -"
			<< endl;
	jp2_input_box*
		parent (Box.parent ());
	if (parent)
		stream
			<< "      sub-box of "
				<< JP2_Metadata::box_name (parent->get_box_type ())
				<< "/\"" << JP2_Metadata::type_name (parent->get_box_type ())
				<< "\" (0x" << hex << parent->get_box_type () << dec << ')'
				<< endl;
	stream
		<< locator
		<< "       read position = "
			<< box.get_pos () << endl
		<< "       header length = "
			<< box.get_box_header_length () << endl
		<< "     remaining bytes = "
			<< box.get_remaining_bytes () << endl
		<< "     total box bytes = "
			<< box.get_box_bytes () << endl
		<< "         is complete = "
			<< box.is_complete () << endl;
	if (jp2_is_superbox (box.get_box_type ()))
		{
		JP2_Box
			sub_box;
		sub_box.open (&box);
		if (sub_box.exists ())
			{
			stream
				<< "    --> Begin sub-box contents of "
					<< JP2_Metadata::box_name (box.get_box_type ()) << endl;
			while (sub_box.exists ())
				{
				stream << sub_box;
				sub_box.close ();
				sub_box.open_next ();
				}
			stream
			<< "    <-- End sub-box contents of "
				<< JP2_Metadata::box_name (box.get_box_type ()) << endl;
			}
		}
	box.close ();
	}
else
	stream << "    The box is closed." << endl;
return stream;
}


std::ostream&
operator<<
	(
	std::ostream&	stream,
	jp2_locator&	locator
	)
{
stream
	<< "       file position = " << locator.get_file_pos () << endl
	<< "          databin id = " << locator.get_databin_id () << endl
	<< "    databin position = " << locator.get_databin_pos () << endl;
return stream;
}

}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA
#endif
