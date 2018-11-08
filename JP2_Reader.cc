/*	JP2_Reader

HiROC CVS ID: $Id: JP2_Reader.cc,v 1.52 2012/03/12 22:22:54 castalia Exp $

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

#include	"JP2_Reader.hh"

#if ! defined (THREAD_COUNT) || THREAD_COUNT < 0
#undef THREAD_COUNT
#define THREAD_COUNT processing_units ()
#else
#if THREAD_COUNT == 0
#warning
#warning	>>> Multi-threaded rendering disabled.
#warning
#endif
#endif

#ifdef	INCLUDE_KAKADU
//	Kakadu Software
#include	"kdu_arch.h"
using namespace kdu_core;
#endif

#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
using std::setprecision;
#include	<stdexcept>
using std::bad_alloc;

#if defined (DEBUG)
/*	DEBUG controls

	DEBUG report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_ACCESSORS		(1 << 1)
#define DEBUG_MANIPULATORS	(1 << 2)
#define DEBUG_UTILITIES		(1 << 3)
#define DEBUG_OPEN			(1 << 4)
#define	DEBUG_RENDER		(1 << 5)
#define DEBUG_DISPOSITION	(1 << 6)
#define DEBUG_PIXEL_DATA	(1 << 7)
#define	DEBUG_ONE_LINE		(1 << 8)
#define DEBUG_NOTIFY		(1 << 9)
#define DEBUG_LOCATION		(1 << 10)

#include	<iostream>
using std::clog;
using std::setw;
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
	Constants
*/
const char* const
	JP2_Reader::ID =
		"UA::HiRISE::JP2_Reader ($Revision: 1.52 $ $Date: 2012/03/12 22:22:54 $)";


const char* const
	JP2_Reader::FORMAT_DESCRIPTIONS[] =
		{
		"Ad hoc (user specified)",
		"BSQ (band sequential)",
		"BIP (band interleaved by pixel)",
		"BIL (band interleaved by line)"
		};

const unsigned int
	JP2_Reader::ALL_BANDS					= ((unsigned int)-1);

#ifndef INCREMENTAL_BUFFER_BYTES
#define INCREMENTAL_BUFFER_BYTES			(8 * 1024 * 1024)
#endif
const unsigned long long
	JP2_Reader::DEFAULT_RENDERING_INCREMENT_BYTES = INCREMENTAL_BUFFER_BYTES;

#define BUFFER_SIZE_REDUCTION_DIFFERENTIAL	(1024 * 1024)

#define MAX_ARRAY_ALLOCATION				((unsigned long long)((size_t)-1))


const std::string
	JP2_Reader::Rendering_Monitor::Status_Message[] =
		{
		//	INFO_ONLY
		"Information only.",
		//	LOW_QUALITY_DATA
		"Rendering low quality pixel data.",
		//	TOP_QUALITY_DATA
		"Rendering top quality pixel data.",
		//	RENDERED_DATA_MASK
		"Rendering pixel data.",
		//	CANCELED
		"Data rendering was canceled.",
		//	unused
		"Low quality rendering canceled.",
		"Top quality rendering canceled.",
		"Rendering canceled.",
		//	DONE
		"Data rendering is done."
		};

/*------------------------------------------------------------------------------
	Defaults
*/
#ifndef DEFAULT_IMAGE_DATA_FORMAT
#define DEFAULT_IMAGE_DATA_FORMAT		FORMAT_BIP
#endif
JP2_Reader::Image_Data_Format
	JP2_Reader::Default_Image_Data_Format
		= JP2_Reader::DEFAULT_IMAGE_DATA_FORMAT;

#define STRINGIFIED(name)				#name
#define AS_STRING(name)					STRINGIFIED(name)

#ifndef DEFAULT_JPIP_PROXY
#define _DEFAULT_JPIP_PROXY_			""
#else
#define _DEFAULT_JPIP_PROXY_			AS_STRING(DEFAULT_JPIP_PROXY)
#endif
std::string
	JP2_Reader::Default_JPIP_Proxy =
		_DEFAULT_JPIP_PROXY_;

#ifndef DEFAULT_JPIP_CACHE_DIRECTORY
#define _DEFAULT_JPIP_CACHE_DIRECTORY_	""
#else
#define _DEFAULT_JPIP_CACHE_DIRECTORY_	AS_STRING(DEFAULT_JPIP_CACHE_DIRECTORY)
#endif
std::string
	JP2_Reader::Default_JPIP_Cache_Directory =
		_DEFAULT_JPIP_CACHE_DIRECTORY_;

#ifndef DEFAULT_JPIP_REQUEST_TIMEOUT
#define DEFAULT_JPIP_REQUEST_TIMEOUT	30
#endif
unsigned int
	JP2_Reader::Default_JPIP_Request_Timeout =
		DEFAULT_JPIP_REQUEST_TIMEOUT;

#ifndef DEFAULT_AUTORECONNECT_RETRIES
#define DEFAULT_AUTORECONNECT_RETRIES	2
#endif
int
	JP2_Reader::Default_Autoreconnect_Retries =
		DEFAULT_AUTORECONNECT_RETRIES;

/*==============================================================================
	Constructors
*/
JP2_Reader::JP2_Reader ()
	:	JP2_Metadata (),
	Rendering_Configuration_Initialized (false),
	Image_Data (NULL),
	Buffer_Size (0),
	User_Buffer (false),
	Pixel_Stride (0),
	Line_Stride (0),
	Data_Format (Default_Image_Data_Format),
	Rendered_Bands (),
	Image_Region (),
	Resolution_Level (0),
	Rendered_Region (),
	Swap_Pixel_Bytes (false),
	Rendered_Bits (0),
	Rendering_Increment_Lines (0),
	Thread_Count (THREAD_COUNT),
	JPIP_Request_Timeout (Default_JPIP_Request_Timeout),
	JPIP_Proxy (Default_JPIP_Proxy),
	JPIP_Cache_Directory (Default_JPIP_Cache_Directory),
	Monitor (NULL),
	Autoreconnect_Retries (Default_Autoreconnect_Retries),
	Bytes_Rendered (0)
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_Reader @ " << (void*)this << endl;
#endif
}


JP2_Reader::JP2_Reader
	(
	const JP2_Reader&	JP2_reader
	)
	:	JP2_Metadata (JP2_reader),
	Rendering_Configuration_Initialized (false),
	Image_Data (NULL),
	Buffer_Size (0),
	User_Buffer (false),
	Pixel_Stride (0),
	Line_Stride (0),
	Data_Format (JP2_reader.Data_Format),
	Rendered_Bands (),
	Image_Region (),
	Resolution_Level (0),
	Rendered_Region (),
	Swap_Pixel_Bytes (JP2_reader.Swap_Pixel_Bytes),
	Rendered_Bits (JP2_reader.Rendered_Bits),
	Rendering_Increment_Lines (JP2_reader.Rendering_Increment_Lines),
	Thread_Count (THREAD_COUNT),
	JPIP_Request_Timeout (JP2_reader.JPIP_Request_Timeout),
	JPIP_Proxy (JP2_reader.JPIP_Proxy),
	JPIP_Cache_Directory (JP2_reader.JPIP_Cache_Directory),
	Monitor (NULL),
	Autoreconnect_Retries (JP2_reader.Autoreconnect_Retries),
	Bytes_Rendered (0)
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_Reader @ " << (void*)this << endl
	 << "    Copy @ " << (void*)&JP2_reader << endl
	 << "    source_name = " << JP2_reader.source_name () << endl;
#endif
}



JP2_Reader::~JP2_Reader ()
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">>> ~JP2_Reader @ " << (void*)this << endl;
#endif
JP2_Reader::reset ();	//	Release any local resources.
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << "<<< ~JP2_Reader" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
JP2_Reader&
JP2_Reader::image_data
	(
	void**				pixel_buffers,
	unsigned long long	buffer_size
	)
{
#if ((DEBUG) & DEBUG_ACCESSORS)
clog << ">>> JP2_Reader::image_data:" << endl
	 << "    pixel_buffers @ " << pixel_buffers << endl
	 << "    buffer_size - " << buffer_size << endl;
#endif
unsigned int
	bands = image_bands ();
if (pixel_buffers)
	{
	if (! bands)
		throw JP2_Logic_Error
			("Can't assign image data pixel buffers"
			" until the data source has been opened.", ID);

	//	Delete any local image data buffer.
	delete_local_data_buffer ();

	//	Ensure that an image data buffers array is allocated.
	allocate_data_buffers_array ();

	#if ((DEBUG) & DEBUG_ACCESSORS)
	clog << "    User Image_Data buffers -" << endl;
	#endif
	Rendered_Region.Depth = 0;
	for (unsigned int
			band = 0;
			band < bands;
			band++)
		{
		//	Set Image_Data buffer pointer and Band_Map entries for the band.
		if (Rendered_Bands[band] = (Image_Data[band] = pixel_buffers[band]))
			Rendered_Region.Depth++;	//	Accumulate the rendered bands count.
		#if ((DEBUG) & DEBUG_ACCESSORS)
		clog << "      " << band << " @ " << Image_Data[band] << endl;
		#endif
		}

	//	Keep the Image_Region and Rendered_Region Depth in sync;
	Image_Region.Depth = Rendered_Region.Depth;

	User_Buffer = true;
	Buffer_Size = buffer_size;
	}
else
	{
	if (User_Buffer)
		//	A locally managed image data buffer will need to be allocated.
		Buffer_Size = 0;
	User_Buffer = false;
	}
#if ((DEBUG) & DEBUG_ACCESSORS)
clog << "<<< JP2_Reader::image_data" << endl;
#endif
return *this;
}


void*
JP2_Reader::image_data
	(
	unsigned int	band,
	unsigned int	line,
	unsigned int	pixel
	)
{
if (! Image_Data)
	return NULL;
if (band  >= Rendered_Region.Depth ||
	line  >= Rendered_Region.Height ||
	pixel >= Rendered_Region.Width)
	{
	ostringstream
		message;
	message
		<< "Invalid image data address request at band " << band
			<< ", line " << line << " and pixel " << pixel << '.';
	if (band >= Rendered_Region.Depth)
		message
			<< endl
			<< "The band is beyond the " << (Rendered_Region.Depth - 1)
				<< " limit.";
	if (line >= Rendered_Region.Height)
		message
			<< endl
			<< "The line is beyond the " << (Rendered_Region.Height - 1)
				<< " limit.";
	if (pixel >= Rendered_Region.Width)
		message
			<< endl
			<< "The pixel is beyond the " << (Rendered_Region.Width - 1)
				<< " limit.";
	throw JP2_Invalid_Argument (message.str (), ID);
	}

return
	(unsigned char*)(Image_Data[band])
		+ (((line * line_stride ()) + pixel_stride ())
			* rendered_pixel_bytes ());
}


JP2_Reader::Image_Data_Format
JP2_Reader::default_image_data_format
	(
	Image_Data_Format	data_format
	)
{
if (! data_format ||
	data_format >= TOTAL_FORMATS)
	{
	ostringstream
		message;
	message << "Unknown image data format (" << data_format << ").";
	throw JP2_Invalid_Argument (message.str (), JP2_Reader::ID);
	}
Image_Data_Format
	previous = Default_Image_Data_Format;
Default_Image_Data_Format = data_format;
return previous;
}


JP2_Reader&
JP2_Reader::image_data_format
	(
	Image_Data_Format data_format
	)
{
#if ((DEBUG) & DEBUG_ACCESSORS)
clog << ">>> JP2_Reader::image_data_format: "
		<< image_data_format_description (data_format) << endl;
#endif
if (! data_format ||
	data_format >= TOTAL_FORMATS)
	{
	ostringstream
		message;
	message << "Unknown image data format (" << data_format << ").";
	throw JP2_Invalid_Argument (message.str (), ID);
	}
Data_Format = data_format;
#if ((DEBUG) & DEBUG_ACCESSORS)
clog << "<<< JP2_Reader::image_data_format" << endl;
#endif
return *this;
}


std::string
JP2_Reader::image_data_format_description
	(
	Image_Data_Format	data_format
	)
	const
{
if (data_format < TOTAL_FORMATS)
	return FORMAT_DESCRIPTIONS[data_format];
ostringstream
	description;
description << "Unknown (" << data_format << ')';
return description.str ();
}


JP2_Reader&
JP2_Reader::image_data_format
	(
	unsigned int	pixel_stride,
	unsigned int	line_stride
	)
{
Pixel_Stride = pixel_stride;
Line_Stride = line_stride;
if (pixel_stride ||
	line_stride)
	Data_Format = FORMAT_AD_HOC;
else
	Data_Format = Default_Image_Data_Format;
return *this;
}


unsigned int
JP2_Reader::pixel_stride () const
{
if (Pixel_Stride)
	return Pixel_Stride;

unsigned int
	bands = rendered_bands ();
if (bands)
	{
	switch (Data_Format)
		{
		case FORMAT_BSQ: return 1;
		case FORMAT_BIP: return bands;
		case FORMAT_BIL: return 1;
		default: break;
		}
	}
return 0;
}


unsigned int
JP2_Reader::line_stride () const
{
if (Line_Stride)
	return Line_Stride;

unsigned int
	bands = rendered_bands ();
if (bands)
	{
	switch (Data_Format)
		{
		case FORMAT_BSQ: return Rendered_Region.Width;
		case FORMAT_BIP: return Rendered_Region.Width * bands;
		case FORMAT_BIL: return Rendered_Region.Width * bands;
		default: break;
		}
	}
return 0;
}


bool
JP2_Reader::render_band
	(
	unsigned int	band,
	bool			enable
	)
{
#if ((DEBUG) & DEBUG_ACCESSORS)
clog << ">>> JP2_Reader::render_band: " << ((band == ALL_BANDS) ? -1 : band )
		<< ' ' << boolalpha << enable << endl;
#endif
//	Ensure that an image data buffers array is allocated.
allocate_data_buffers_array ();

bool
	no_conflicts = true;
unsigned int
	bands = image_bands ();
if (bands &&
	band == ALL_BANDS)
	{
	Rendered_Region.Depth = 0;
	for (unsigned int
			band = 0;
			band < bands;
			band++)
		{
		if (enable)
			{
			if (user_buffer () &&
				! Image_Data[band])
				{
				//	Can't enable band with no user buffer.
				#if ((DEBUG) & DEBUG_ACCESSORS)
					clog << "    No user buffer for the band" << endl;
				#endif
				Rendered_Bands[band] = false;
				no_conflicts = false;
				}
			else
				{
				Rendered_Bands[band] = enable;
				++Rendered_Region.Depth;
				}
			}
		else
			Rendered_Bands[band] = enable;
		}
	//	Keep the Image_Region and Rendered_Region Depth in sync;
	Image_Region.Depth = Rendered_Region.Depth;
	}
else
if (band < bands)
	{
	if (enable != Rendered_Bands[band])
		{
		if (enable)
			{
			if (! user_buffer () ||
				Image_Data[band])
				{
				Rendered_Bands[band] = enable;
				Image_Region.Depth = ++Rendered_Region.Depth;
				}
			else
				{
				#if ((DEBUG) & DEBUG_ACCESSORS)
				clog << "    No user buffer for the band" << endl
					 << "<<< JP2_Reader::render_band" << endl;
				#endif
				no_conflicts = false;
				}
			}
		else
			{
			Rendered_Bands[band] = enable;
			Image_Region.Depth = --Rendered_Region.Depth;
			}
		}
	}
else
	{
	ostringstream
		message;
	message << "Can't " << (enable ? "en" : "dis") << "able image band "
				<< band << " of " << bands
				<< " for rendering." << endl;
	throw JP2_Invalid_Argument (message.str (), ID);
	}
#if ((DEBUG) & DEBUG_ACCESSORS)
clog << "<<< JP2_Reader::render_band: " << boolalpha << no_conflicts << endl;
#endif
return no_conflicts;
}


bool
JP2_Reader::is_rendered_band
	(
	unsigned int	band
	)
	const
{
if (band < image_bands ())
	return Rendered_Bands[band];
return false;
}


JP2_Reader&
JP2_Reader::rendered_pixel_bits
	(
	unsigned int	bits
	)
{
if (! bits)
	throw JP2_Invalid_Argument
		("Specifying zero rendered pixel bits is invalid.", ID);
if (bits > 16)
	{
	ostringstream
		message;
	message << "The maximum number of rendered pixel bits is 16, but "
				<< bits << " was requested.";
	throw JP2_Invalid_Argument (message.str (), ID);
	}
Rendered_Bits = bits;
return *this;
}


unsigned int
JP2_Reader::rendered_pixel_bytes () const
{return bytes_of_bits (Rendered_Bits);}


unsigned long long
JP2_Reader::rendered_image_bytes () const
{return (unsigned long long)
	Rendered_Region.area () * image_bands () * rendered_pixel_bytes ();}


unsigned int
JP2_Reader::effective_rendering_increment_lines () const
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">>> JP2_Reader::effective_rendering_increment_lines" << endl;
#endif
unsigned int
	bands = image_bands (),
	increment_lines = 0;
if (bands &&
	Rendered_Region.Width)
	{
	increment_lines = Rendering_Increment_Lines;
	#if ((DEBUG) & DEBUG_RENDER)
	clog << "    suggested increment: " << increment_lines << endl;
	#endif
	if (! increment_lines)
		{
		//	Default incremental rendering.
		unsigned int
			increment_pixels =
				DEFAULT_RENDERING_INCREMENT_BYTES
				/ bands / rendered_pixel_bytes ();
		increment_lines = increment_pixels / Rendered_Region.Width;
		if (increment_pixels % Rendered_Region.Width)
			++increment_lines;
		#if ((DEBUG) & DEBUG_RENDER)
		clog
			<< "    Using " << DEFAULT_RENDERING_INCREMENT_BYTES
				<< " default increment bytes -" << endl
			<< "                   bands: " << bands << endl
			<< "            region width: " << Rendered_Region.Width << endl
			<< "    rendered_pixel_bytes: " << rendered_pixel_bytes () << endl
			<< "        increment_pixels: " << increment_pixels << endl
			<< "         increment_lines: " << increment_lines << endl;
		#endif
		}

	if (! increment_lines)
		increment_lines = 1;
	else
	if (increment_lines > Rendered_Region.Height)
		{
		//	Maximum effective rendering increment.
		increment_lines = Rendered_Region.Height;
		#if ((DEBUG) & DEBUG_RENDER)
		clog << "           region height: " << Rendered_Region.Height << endl;
		#endif
		}
	}
#if ((DEBUG) & DEBUG_RENDER)
clog << "<<< JP2_Reader::effective_rendering_increment_lines: "
		<< increment_lines << endl;
#endif
return increment_lines;
}


JP2_Reader&
JP2_Reader::processing_threads
	(
	unsigned int	threads
	)
{
if (! (Thread_Count = threads))
	Thread_Count = processing_units ();
return *this;
}

/*==============================================================================
	Render
*/
bool
JP2_Reader::ready
	(
	string*	report
	)
	const
{
bool
	is_ready = true;
ostringstream
	reason;
if (report)
	reason
		<< "Not ready to render the " << source_name () << " source" << endl
		<< "because:";
if (! is_open ())
	{
	if (report)
		reason << endl
			<< "No source has been opened.";
	is_ready = false;
	}
else
	{
	unsigned long long
		size = minimum_buffer_size (),
		maximum;

	if (user_buffer () &&
		Buffer_Size &&
		Buffer_Size < size)
		{
		//	Undersized user specified pixel buffer.
		if (report)
			{
			reason
				<< endl
				<< "A" << ((Buffer_Size == 8 || Buffer_Size == 18) ? "n " : " ")
					<< magnitude (Buffer_Size);
			if (Buffer_Size >= 1024)
				reason << " (" << Buffer_Size << ')';
			reason
					<< " byte image data buffer size is specified," << endl
				<< "but a " << magnitude (size)
					<< " (" << size << ") byte size is required to render the "
					<< Rendered_Region.Width << " wide, "
					<< Rendered_Region.Height << " high region" << endl
				<< "with a pixel stride of " << pixel_stride ()
					<< " and a line stride of " << line_stride () << '.';
			}
		is_ready = false;
		}
	else
	if (! user_buffer () &&
		(size *= rendered_bands ()) > (maximum = MAX_ARRAY_ALLOCATION))
		{
		//	Can't allocate a buffer for the rendered image region.
		if (report)
			reason << endl
				<< "The required " << magnitude (size) << " (" << size
					<< ") byte image data buffer size" << endl
				<< "for the "
					<< Rendered_Region.Width << " wide, "
					<< Rendered_Region.Height << " high, "
					<< rendered_bands () << " band image region" << endl
				<< "is greater than the maximum " << magnitude (maximum)
					<< " (" << maximum << ") byte allocation.";
		is_ready = false;
		}
	}

if (! is_ready &&
	report)
	*report += reason.str ();
return is_ready;
}


unsigned int
JP2_Reader::default_JPIP_request_timeout
	(
	unsigned int	timeout
	)
{
unsigned int
	previous = Default_JPIP_Request_Timeout;
Default_JPIP_Request_Timeout = timeout;
return previous;
}


int
JP2_Reader::default_autoreconnect_retries
	(
	int		retries
	)
{
int
	previous = Default_Autoreconnect_Retries;
Default_Autoreconnect_Retries = retries;
return previous;
}


bool
JP2_Reader::reconnect ()
{return is_open ();}


void
JP2_Reader::reset ()
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << ">>> JP2_Reader::reset" << endl;
#endif
//	Reset rendering specifications.
Cube
	empty;
Rendered_Bands.clear ();
Image_Region			=
Rendered_Region			= empty;
Resolution_Level		= 0;
Swap_Pixel_Bytes		= false;
Rendered_Bits			= 0;
Rendering_Increment_Lines = 0;
Bytes_Rendered			= 0;
Monitor					= NULL;

//	Delete locally managed image data buffers.
delete_local_data_buffer ();
if (Image_Data)
	{
	delete[] Image_Data;
	Image_Data			= NULL;
	}
Buffer_Size				= 0;
User_Buffer				= false;
Pixel_Stride			=
Line_Stride				= 0;

Data_Format				= Default_Image_Data_Format;

/*	THREAD_COUNT
	A preprocessor macro defined as processing_units() unless
	defined as a non-negative value at compile time.
*/
Thread_Count			= THREAD_COUNT;
JPIP_Proxy				= Default_JPIP_Proxy;
JPIP_Cache_Directory	= Default_JPIP_Cache_Directory;

//	Reset the metadata.
JP2_Metadata::reset ();

Rendering_Configuration_Initialized = false;
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "<<< JP2_Reader::reset" << endl;
#endif
}

/*==============================================================================
	Helpers
*/
unsigned long long
JP2_Reader::minimum_buffer_size () const
{
return (unsigned long long)
	rendered_pixel_bytes () *
	pixel_stride () *
	line_stride () *
	Rendered_Region.height ();
}


bool
JP2_Reader::allocate_data_buffers_array ()
{
if (Image_Data)
	return true;

/*	Hidden local data buffer pointer.

	An additional entry is provided for the pointer to a locally managed
	image data buffer.
*/
unsigned int
	bands = image_bands ();
if (bands)
	{
	Image_Data = new void*[bands + 1];
	for (unsigned int
			band = 0;
			band <= bands;
			band++)
		Image_Data[band] = NULL;
	return true;
	}
return false;
}


void
JP2_Reader::delete_local_data_buffer ()
{
unsigned int
	bands = image_bands ();
if (! Image_Data ||
	! Image_Data[bands])
	return;

#if ((DEBUG) & DEBUG_MANIPULATORS)
clog << ">-< JP2_Reader::delete_local_data_buffer:" << endl
	 << "      Deleting the local image data buffer @ "
		<< Image_Data[bands] << endl;
#endif
delete[] reinterpret_cast<unsigned long long*>(Image_Data[bands]);
Buffer_Size = 0;
for (unsigned int
		band = 0;
		band <= bands;
		band++)
	Image_Data[band] = NULL;
}


void
JP2_Reader::allocate_image_data_buffer ()
{
#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
clog << ">>> JP2_Reader::allocate_image_data_buffer" << endl;
#endif
if (! user_buffer ())
	{
	unsigned long long
		minimum = minimum_buffer_size () * rendered_bands ();
	if (! minimum)
		{
		#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
		clog << "    Zero buffer size." << endl
			 << "<<< JP2_Reader::allocate_image_data_buffer" << endl;
		#endif
		return;
		}

	//	Convert to unsigned long long array size.
	minimum = (minimum / sizeof (long long))
				+ ((minimum % sizeof (long long) ? 1 : 0));
	if (minimum > MAX_ARRAY_ALLOCATION)
		{
		ostringstream
			message;
		message << "The required image data buffer size - "
					<< magnitude (minimum) << " (" << minimum << ") "
					<< (sizeof (long long) << 3) << "-bit words" << endl
				<< "is greater than the maximum size - "
					<< magnitude (MAX_ARRAY_ALLOCATION)
					<< " (" << MAX_ARRAY_ALLOCATION << ")." << endl;
		throw JP2_Out_of_Range (message.str (), ID);
		}

	//	Ensure that an image data buffers array is allocated.
	allocate_data_buffers_array ();

	unsigned int
		bands = image_bands ();
	size_t
		size = (size_t)minimum,
		buffer_size = size / rendered_bands ();
	if (! Image_Data[bands] ||
		 buffer_size > Buffer_Size ||
		(Buffer_Size > BUFFER_SIZE_REDUCTION_DIFFERENTIAL &&
		 buffer_size < (Buffer_Size - BUFFER_SIZE_REDUCTION_DIFFERENTIAL)))
		{
		//	Reallocate the image data buffer.
		delete_local_data_buffer ();

		#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
		clog << "    Allocating " << size
				<< " byte image data buffer" << endl;
		#endif
		try {Image_Data[bands] = new unsigned long long[size];}
		catch (bad_alloc&)
			{
			minimum *= sizeof (long long);
			ostringstream
				message;
			message << "Couldn't allocate a " << magnitude (minimum)
						<< " (" << minimum << ") byte image data buffer.";
			throw JP2_Out_of_Range (message.str (), ID);
			}
		#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
		clog << "      @ " << Image_Data[bands] << endl;
		#endif
		Buffer_Size = buffer_size;

		unsigned char*
			image_data =
				reinterpret_cast<unsigned char*>(Image_Data[bands]);
		switch (Data_Format)
			{
			case FORMAT_BSQ:
				for (unsigned int
						band = 0;
						band < bands;
						band++)
					{
					if (Rendered_Bands[band])
						{
						Image_Data[band] = image_data;
						image_data +=
							Rendered_Region.area () * rendered_pixel_bytes ();
						}
					else
						Image_Data[band] = NULL;
					#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
					clog << "    " << band << " @ " << Image_Data[band] << endl;
					#endif
					}
				break;
			case FORMAT_BIP:
				for (unsigned int
						band = 0;
						band < bands;
						band++)
					{
					if (Rendered_Bands[band])
						{
						Image_Data[band] = image_data;
						image_data += rendered_pixel_bytes ();
						}
					else
						Image_Data[band] = NULL;
					#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
					clog << "    " << band << " @ " << Image_Data[band] << endl;
					#endif
					}
				break;
			case FORMAT_BIL:
				for (unsigned int
						band = 0;
						band < bands;
						band++)
					{
					if (Rendered_Bands[band])
						{
						Image_Data[band] = image_data;
						image_data +=
							Rendered_Region.Width * rendered_pixel_bytes ();
						}
					else
						Image_Data[band] = NULL;
					#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
					clog << "    " << band << " @ " << Image_Data[band] << endl;
					#endif
					}
				break;
			default: break;
			}
		}
	#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
	else
		clog << "    Reusing existing buffer" << endl;
	#endif
	}
#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
else
if (Image_Data)
	{
	clog << "    " << Buffer_Size << " byte user buffers -" << endl;
	for (unsigned int
			band = 0;
			band < image_bands ();
			band++)
		clog << "      " << band << " @ " << Image_Data[band] << endl;
	clog << "    Pixel_Stride - " << pixel_stride () << endl
		 << "     Line_Stride - " << line_stride () << endl;
	}
clog << "<<< JP2_Reader::allocate_image_data_buffer" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Pixel data disposition
*/

#if ((DEBUG) & DEBUG_PIXEL_DATA)
#ifndef DOXYGEN_PROCESSING
namespace
{
void
print_line
	(
	unsigned char*	data,
	unsigned int	count,
	unsigned int	pixel_stride
	)
{
clog << hex << setfill ('0');
for (unsigned int
		index = 0;
		index < count;
		index++,
			data += pixel_stride)
	{
	if (! (index % 16))
		{
		if (index)
			clog << endl;
		clog << dec << setfill (' ')
			 << setw (5) << index << ": "
			 << hex << setfill ('0');
		}
	clog << "  " << setw (2) << ((unsigned int)(*data) & 0xFF);
	}
clog << dec << setfill (' ') << endl;
}


void
print_line
	(
	unsigned short*	data,
	unsigned int	count,
	unsigned int	pixel_stride
	)
{
clog << hex << setfill ('0');
for (unsigned int
		index = 0;
		index < count;
		index++,
			data += pixel_stride)
	{
	if (! (index % 16))
		{
		if (index)
			clog << endl;
		clog << dec << setfill (' ')
			 << setw (5) << index << ": "
			 << hex << setfill ('0');
		}
	clog << "  " << setw (4) << *data;
	}
clog << dec << setfill (' ') << endl;
}


void
print_pixels
	(
	void**			image_data,
	const Cube&		region,
	const Cube&		rendered_region,
	unsigned int	line_stride,
	unsigned int	pixel_stride,
	unsigned int	pixel_sample_bytes
	)
{
clog << ">>> print_pixels:" << endl
	 << "    region " << region << endl
	 << "        of " << rendered_region << endl
	 << "           line stride = " << line_stride << endl
	 << "          pixel stride = " << pixel_stride << endl
	 << "    pixel sample bytes = " << pixel_sample_bytes << endl;
size_t
	offset =
		  ((region.X - rendered_region.X)	//	Horizontal, pixels, offset.
			* pixel_stride)					//	Pixel size in bytes.
		+ ((region.Y - rendered_region.Y)	//	Vertical, lines, offset.
			* pixel_stride * line_stride);	//	Line length in bytes.
clog << "    first pixel offset = " << offset << endl;
int
	last_line = region.Y +
		#if ((DEBUG) & DEBUG_ONE_LINE)
		1;
		#else
		region.Height;
		#endif
unsigned char
	*last_line_data,
	*this_line_data,
	*data_end,
	*buffer_1 = NULL;
unsigned short
	*buffer_2 = NULL;

for (unsigned int
		band = 0;
		band < region.Depth;
		band++)
	{
	if (image_data[band])
		{
		clog << endl
			 << "--> Image_Data[" << band << "] @ "
				<< image_data[band] << endl;
		if (pixel_sample_bytes == 1)
			buffer_1 =
				(unsigned char*)image_data[band] + offset;
		else
			buffer_2 =
				(unsigned short*)image_data[band] + offset;
		int
			matches = 0,
			line = region.Y;
		for (line = region.Y;
			 line < last_line;
			 line++)
			{
			if (line != region.Y)
				{
				if (buffer_1)
					{
					last_line_data = buffer_1 - line_stride;
					this_line_data = buffer_1;
					data_end = buffer_1 + region.Width;
					}
				else
					{
					last_line_data = (unsigned char*)(buffer_2 - line_stride);
					this_line_data = (unsigned char*)buffer_2;
					data_end = (unsigned char*)(buffer_2 + region.Width);
					}
				while (this_line_data < data_end)
					if (*last_line_data++ != *this_line_data++)
						break;
				if (this_line_data == data_end)
					++matches;
				else
					matches = -matches;
				}
			if (matches <= 0)
				{
				if (matches < 0)
					{
					clog << "line " << (line + matches);
					if (matches != -1)
						clog << " - " << (line - 1);
					clog << " matches the previous line." << endl;
					matches = 0;
					}
				clog << "line " << line << " @ ";
				if (buffer_1)
					{
					clog << (void*)buffer_1 << " -" << endl;
					print_line (buffer_1, region.Width, pixel_stride);
					buffer_1 += line_stride;
					}
				else
					{
					clog << (void*)buffer_2 << " -" << endl;
					print_line (buffer_2, region.Width, pixel_stride);
					buffer_2 += line_stride;
					}
				}
			}
		if (matches)
			{
			clog << "line " << (line - matches);
			if (matches != 1)
				clog << " - " << (line - 1);
			clog << " matches the previous line." << endl;
			}
		}
	}
clog << "<<< print_pixels" << endl;
}

}	//	local namespace.
#endif
#endif


/*------------------------------------------------------------------------------
	Decompressed pixel data byte swapping and monitor notification.
*/
bool
JP2_Reader::data_disposition
	(
	Rendering_Monitor::Status	status,
	const std::string&			message,
	const Cube&					region,
	const Cube&					image_region_rendered
	)
{
#if ((DEBUG) & (DEBUG_DISPOSITION | DEBUG_PIXEL_DATA | DEBUG_LOCATION))
clog << ">>> JP2_Reader::data_disposition:" << endl
	 << "    status " << status << " \"" << message << '"' << endl
	 << "    source \"" << source_name () << '"' << endl
	 << "    size " << image_size ()
	 	<< ", resolution level " << resolution_level ()
		<< ", " << rendered_pixel_bytes () << " byte pixel data" << endl
	 << "    region rendered " << region << endl
	 << "                 of " << Rendered_Region << endl
	 << "     image rendered " << image_region_rendered << endl
	 << "                 of " << Image_Region << endl;
#endif
if (Swap_Pixel_Bytes &&
	rendered_pixel_bytes () > 1 &&
	region.area ())
	{
	// Swap byte order.
	#if ((DEBUG) & DEBUG_DISPOSITION)
	clog << "    Swapping pixel bytes ..." << endl;
	#endif
	unsigned int
		pixel_gap = pixel_stride (),
		line_gap = line_stride ();
	size_t
		offset =
			  ((region.X - Rendered_Region.X)	//	Horizontal, pixels, offset.
				* pixel_gap)					//	Pixel stride samples.
			+ ((region.Y - Rendered_Region.Y)	//	Vertical, lines, offset.
				* line_gap);					//	Line stride in samples.
	#if ((DEBUG) & DEBUG_DISPOSITION)
	clog << "            pixel_stride = " << pixel_stride () << endl
		 << "             line_stride = " << line_stride () << endl
		 << "     first sample offset = " << offset << endl;
	#endif
	unsigned char*
		buffer_1;
	unsigned short*
		buffer_2;
	unsigned char
		datum;
	pixel_gap <<= 1;	//	Convert from samples to bytes.
	for (unsigned int
			band = 0;
			band < region.Depth;
			band++)
		{
		if (Image_Data[band])
			{
			buffer_2 =
				(unsigned short*)Image_Data[band] + offset;
			for (unsigned int
					line = 0;
					line < region.Height;
					line++,
					buffer_2 += line_gap)
				{
				buffer_1 = (unsigned char*)buffer_2;
				for (unsigned int
						sample = 0;
						sample < region.Width;
						sample++,
						buffer_1 += pixel_gap)
					{
					datum = *buffer_1;
					*buffer_1 = *(buffer_1 + 1);
					*(buffer_1 + 1) = datum;
					}
				}
			}
		}
	}
#if ((DEBUG) & DEBUG_PIXEL_DATA)
if (region.area ())
	print_pixels (Image_Data, region, Rendered_Region,
		line_stride (), pixel_stride (), rendered_pixel_bytes ());
#endif

bool
	continue_rendering = true;
if (Monitor)
	{
	//	Notify the Rendering_Monitor of the rendering progress.
	#if ((DEBUG) & (DEBUG_DISPOSITION | DEBUG_NOTIFY | DEBUG_LOCATION))
	clog << "*** JP2_Reader::data_disposition: notify status "
			<< status << " \"" << message << '"' << endl
		 << "    region = " << region << endl;
	#endif
	continue_rendering =
		Monitor->notify (*this, status, message, region, image_region_rendered);
	}
#if ((DEBUG) & (DEBUG_DISPOSITION | DEBUG_PIXEL_DATA | DEBUG_LOCATION))
clog << "<<< JP2_Reader::data_disposition: "
		<< boolalpha << continue_rendering << endl;
#endif
return continue_rendering;
}

/*==============================================================================
	Utility
*/
string
magnitude
	(
	unsigned long long	value
	)
{
static const char* const
	MAGNITUDE = " KMGTPEZ";
double
	amount = value;
const char*
	mag;
for (mag = MAGNITUDE;
	*(mag + 1) &&
		amount >= 1024;
	++mag,
		amount /= 1024);
ostringstream
	representation;
representation << setprecision (3) << amount << *mag;
return representation.str ();
}


unsigned int
processing_units ()
{
static unsigned int
	processors =
		#ifdef INCLUDE_KAKADU
		kdu_get_num_processors ();
		#else
		1;
		#endif

return processors;
}


}	//	namespace HiRISE
}	//	namespace UA
