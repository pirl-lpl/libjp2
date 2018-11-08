/*	JP2_Box

HiROC CVS ID: $Id: JP2_Box.hh,v 1.1 2012/04/26 00:39:33 castalia Exp $

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

#ifndef _JP2_Box_
#define _JP2_Box_

#include	"jp2.h"

#include	<iosfwd>


namespace UA
{
namespace HiRISE
{
namespace Kakadu
{
/*==============================================================================
	A <i>JP2_Box</i> is a convenience subclass of the Kakadu jp2_input_box.

	The subclass is used to gain access to the jp2_input_box protected
	data members for the box {@link JP2_Box::source() source} and {@link
	parent() super-box parent}. Access to this information can be very
	useful in a context where the box origination environment is not
	othewise available. For example, when opening another JP2_Box
	instance on the same source with an independent content read context.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.1 $
*/
class JP2_Box
:	public kdu_supp::jp2_input_box
{
public:

/**	Construct a JP2_Box.

	@param	source	A jp2_family_src (or jp2_threadsafe_family_src) pointer
		for the source of JP2 box data. If NULL an unitialized box is
		provided that can be opened later; otherwise the box is immediately
		opened on the source.
	@param	locator	A jp2_locator used to set the inital source data read
		location of the box. Typically a locator obtained from another
		JP2_Box (or jp2_input_box) is used to position the new box at
		the same source data location as the other box, but with an
		independent content read context. <b>N.B.</b>: If the source
		argument is NULL the locator argument is not used.
*/
explicit JP2_Box
	(kdu_supp::jp2_family_src* source = NULL, kdu_supp::jp2_locator locator = kdu_supp::jp2_locator ())
	{if (source) open (source, locator);}

/**	Construct a JP2_Box copy.

	@param	Box	The JP2_Box to be copied. The Box is {@link
		operator=(JP2_Box&) assigned} to this box.
*/
JP2_Box (JP2_Box& Box)
	{*this = Box;}

/**	Assign another JP2_Box to this box.

	This box is first closed. Then, if the other box is open (exists),
	this box is opened on the {@link source() data source} of the other
	box and positioned using the current locator from the other box.

	@param	Box	The JP2_Box to be assigned to this box.
	@return	This JP2_Box.
*/
inline JP2_Box& operator= (JP2_Box& Box)
	{
	close ();
	if (Box.exists ()) open (Box.source (), Box.get_locator ());
	return *this;
	}

/**	Get the data source for this box.

	@return	A pointer to the jp2_family_src that is providing data
		for this JP2 box.
*/
inline kdu_supp::jp2_family_src* source ()
	{return src;}

/**	Get the super-box parent of this box.

	@return	A pointer to the super-box parent jp2_input_box of this box.
		This will be NULL if this box does not have a known parent.
		<b>N.B.</b>: A box may have a parent but not know it; e.g. if the
		box was opened using a locator to a sub-box the parent box will
		not be known.
*/
inline jp2_input_box* parent ()
	{return super_box;}

};	//	Class JP2_Box

/**	Output a description of a JP2_Box to a stream.

	The description is several indented lines of text that identify
	the box type, its location, size, and read context including
	whether it is competely available for reading. <b>N.B.</b>: All
	text lines, including the last line, are terminated with a NL.

	@param	stream	A reference to the ostream where the description
		will be written.
	@param	Box	A reference to the JP2_Box to be described.
*/
std::ostream& operator<< (std::ostream& stream, JP2_Box& Box);

/**	Output a description of a jp2_locater to a stream.

	The description is a few indented lines of text that provide the box
	location information. <b>N.B.</b>: All text lines, including the last
	line, are terminated with a NL.

	@param	stream	A reference to the ostream where the description
		will be written.
	@param	Box	A reference to the jp2_locator to be described.
*/
std::ostream& operator<< (std::ostream& stream, kdu_supp::jp2_locator& locator);

}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA

#endif
