/*	KDU_dims

HiROC CVS ID: $Id: KDU_dims.hh,v 1.5 2012/04/26 00:41:45 castalia Exp $

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

#ifndef _KDU_dims_
#define _KDU_dims_

//	PIRL++
#include	"Dimensions.hh"

//	Kakadu
#include	"kdu_compressed.h"

#include	<ostream>


namespace UA
{
namespace HiRISE
{
namespace Kakadu
{
/*==============================================================================
	<i>KDU_dims</i> is a convenience subclass of the Kakadu kdu_dims.

	The KDU_dims class provides type converters for the Point_2D,
	Size_2D, Rectangle and Cube Dimensions classes and the kdu_coords
	class. Output operators are also provided for KDU_dims, kdu_dims, and
	kdu_coords.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.5 $
*/
struct KDU_dims
:	public kdu_dims
{
KDU_dims () {}

inline KDU_dims (const PIRL::Point_2D& position)
{
pos.x = position.X;
pos.y = position.Y;
}

inline KDU_dims (const PIRL::Size_2D& size_2D)
{
size.x = size_2D.Width;
size.y = size_2D.Height;
}

inline KDU_dims (const PIRL::Rectangle& rectangle)
{
pos.x = rectangle.X;
pos.y = rectangle.Y;
size.x = rectangle.Width;
size.y = rectangle.Height;
}

inline KDU_dims (const PIRL::Cube& cube)
{
pos.x = cube.X;
pos.y = cube.Y;
size.x = cube.Width;
size.y = cube.Height;
}

inline KDU_dims (const kdu_coords& coords)
{
pos.x = coords.x;
pos.y = coords.y;
}
/*
inline KDU_dims( const std::tuple& tuple)
{
   std::tie(pos.x, pos.y, pos.z) = tuple;
}
*/
inline KDU_dims& operator= (const kdu_dims& dimensions)
{
pos.x = dimensions.pos.x;
pos.y = dimensions.pos.y;
size.x = dimensions.size.x;
size.y = dimensions.size.y;
return *this;
}


inline KDU_dims& operator= (const PIRL::Point_2D& position)
{
pos.x = position.X;
pos.y = position.Y;
return *this;
}

inline KDU_dims& operator= (const PIRL::Size_2D& size_2D)
{
size.x = size_2D.Width;
size.y = size_2D.Height;
return *this;
}

inline KDU_dims& operator= (const PIRL::Rectangle& rectangle)
{
pos.x = rectangle.X;
pos.y = rectangle.Y;
size.x = rectangle.Width;
size.y = rectangle.Height;
return *this;
}

inline KDU_dims& operator= (const PIRL::Cube& cube)
{
pos.x = cube.X;
pos.y = cube.Y;
size.x = cube.Width;
size.y = cube.Height;
return *this;
}

inline KDU_dims& operator= (const kdu_coords& coords)
{
pos.x = coords.x;
pos.y = coords.y;
return *this;
}


inline operator PIRL::Point_2D () const
{return PIRL::Point_2D (pos.x, pos.y);}

inline operator PIRL::Size_2D () const
{return PIRL::Size_2D (size.x, size.y);}

inline operator PIRL::Rectangle () const
{return PIRL::Rectangle (pos.x, pos.y, size.x, size.y);}

inline operator PIRL::Cube () const
{return PIRL::Cube (pos.x, pos.y, size.x, size.y, 1);}

inline operator kdu_coords () const
{return kdu_coords (pos.x, pos.y);}

};	//	Class KDU_dims


inline std::ostream& operator<<
	(std::ostream& stream, const KDU_dims& dimensions)
{
stream << dimensions.pos.x << "x, "  << dimensions.pos.y << "y, "
	   << dimensions.size.x << "w, " << dimensions.size.y << "h";
return stream;
}

inline std::ostream& operator<<
	(std::ostream& stream, const kdu_dims& dimensions)
{
stream << dimensions.pos.x << "x, "  << dimensions.pos.y << "y, "
	   << dimensions.size.x << "w, " << dimensions.size.y << "h";
return stream;
}

inline std::ostream& operator<<
	(std::ostream& stream, const kdu_coords& coordinates)
{
stream << coordinates.x << "x, " << coordinates.y << "y";
return stream;
}

}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA

#endif
