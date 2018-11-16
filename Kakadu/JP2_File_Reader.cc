/*	JP2_File_Reader

HiROC CVS ID: $Id: JP2_File_Reader.cc,v 1.51 2012/04/26 03:35:02 castalia Exp $

Copyright (C) 2009-2012  Arizona Board of Regents on behalf of the
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

#include	"JP2_File_Reader.hh"
#include	"JP2_JPIP_Reader.hh"

//	Kakadu
#include	"kdu_arch.h"
#include	"kdu_client.h"
using namespace kdu_core;
using namespace kdu_supp;
#include	"KDU_dims.hh"
#include	"JP2_Box.hh"

#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
#include	<stdexcept>
using std::exception;
#include	<cstring>

#ifdef _WIN32
#include "Windows.h"	//	For Sleep system function.
#endif


#if defined (DEBUG)
/*	DEBUG controls

	DEBUG report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_ACCESSORS		(1 << 1)
#define DEBUG_RES_REGION	(1 << 2)
#define DEBUG_UTILITIES		(1 << 3)
#define DEBUG_OPEN			(1 << 4)
#define DEBUG_METADATA		(1 << 5)
#define	DEBUG_RENDER		(1 << 6)
#define DEBUG_WRITE			(1 << 7)
#define	DEBUG_ONE_LINE		(1 << 8)
#define DEBUG_TIMING		(1 << 9)
#define DEBUG_NOTIFY		(1 << 10)
#define DEBUG_LOCATION		(1 << 11)

#if ((DEBUG) & (DEBUG_TIMING | \
				DEBUG_OPEN | \
				DEBUG_RENDER | \
				DEBUG_LOCATION))

#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
#include	<ctime>
#include	<sys/time.h>

#ifndef DOXYGEN_PROCESSING
namespace
{
double duration (struct timeval* start, struct timeval* end)
{return
	((double (end->tv_usec) / 1000000.0) + end->tv_sec) -
	((double (start->tv_usec) / 1000000.0) + start->tv_sec);}
}
#endif
#endif	//	!_WIN32
#endif

#include	<iostream>
using std::clog;
using std::boolalpha;
using std::hex;
using std::dec;
#endif	//	DEBUG


namespace UA
{
namespace HiRISE
{
namespace Kakadu
{
/*==============================================================================
	Constants
*/
const char* const
	JP2_File_Reader::ID =
		"UA::HiRISE::Kakadu::JP2_File_Reader ($Revision: 1.51 $ $Date: 2012/04/26 03:35:02 $)";


#ifndef READER_ERROR_VALUE
//	KDU_ERROR_EXCEPTION is defined in the included kdu_elementary header.
#define READER_ERROR_VALUE					KDU_ERROR_EXCEPTION
#endif
const kdu_exception
	JP2_File_Reader::READER_ERROR			= READER_ERROR_VALUE;

#ifndef ERROR_MESSAGE_QUEUE_SIZE
#define ERROR_MESSAGE_QUEUE_SIZE			32
#endif

#define	MAX_BOX_AMOUNT						(1 << 16)

/*==============================================================================
	Constructors
*/
JP2_File_Reader::JP2_File_Reader ()
	:
	JP2_Stream (),
	JP2_Source (),
	JPEG2000_Codestream (),
	Decompressor (),
	Channel_Mapping (),
	Expand_Numerator (1, 1),
	Expand_Denominator (1, 1),
	Thread_Group (NULL),
	Error_Message_Queue ()
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_File_Reader: " << (void*)this << endl;
#endif
#if defined DEBUG
clog << boolalpha;
#endif
//	Register the Kakadu error handler.
Error_Message_Queue.configure
	(
	ERROR_MESSAGE_QUEUE_SIZE,
	false,	//	Auto-pop error messages.
	true,	//	Throw exception on error.
	READER_ERROR
	);
kdu_customize_errors (&Error_Message_Queue);
}


JP2_File_Reader::JP2_File_Reader
	(
	const std::string&	source
	)
	:
	JP2_Stream (),
	JP2_Source (),
	JPEG2000_Codestream (),
	Decompressor (),
	Channel_Mapping (),
	Expand_Numerator (1, 1),
	Expand_Denominator (1, 1),
	Thread_Group (NULL),
	Error_Message_Queue ()
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_File_Reader: " << (void*)this << endl
	 << "    source = " << source << endl;
#endif
#if defined DEBUG
clog << boolalpha;
#endif
//	Register the Kakadu error handler.
Error_Message_Queue.configure
	(
	ERROR_MESSAGE_QUEUE_SIZE,
	false,	//	Auto-pop error messages.
	true,	//	Throw exception on error.
	READER_ERROR
	);
kdu_customize_errors (&Error_Message_Queue);

try {open (source);}
catch (exception& except)
	{
	#if (DEBUG & DEBUG_CONSTRUCTORS)
	clog << "!!! Failed to open -" << endl
		 << except.what () <<endl;
	#endif
	}
catch (...)
	{
	#if (DEBUG & DEBUG_CONSTRUCTORS)
	clog << "!!! Failed to open -" << endl
		 << "    Unknown exception" << endl;
	#endif
	}
}


JP2_File_Reader::JP2_File_Reader
	(
	const JP2_File_Reader&	JP2_file_reader
	)
	:	JP2_Reader (JP2_file_reader),
	JP2_Stream (),
	JP2_Source (),
	JPEG2000_Codestream (),
	Decompressor (),
	Channel_Mapping (),
	Expand_Numerator (1, 1),
	Expand_Denominator (1, 1),
	Thread_Group (NULL),
	Error_Message_Queue ()
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_File_Reader @ " << (void*)this << endl
	 << "    Copy @ " << (void*)&JP2_file_reader << endl
	 << "    source_name = " << JP2_file_reader.source_name () << endl;
#endif
#if defined DEBUG
clog << boolalpha;
#endif
//	Register the Kakadu error handler.
Error_Message_Queue.configure
	(
	ERROR_MESSAGE_QUEUE_SIZE,
	false,	//	Auto-pop error messages.
	true,	//	Throw exception on error.
	READER_ERROR
	);
kdu_customize_errors (&Error_Message_Queue);

if (! dynamic_cast<const JP2_JPIP_Reader*>(&JP2_file_reader))
	{
	try {open (JP2_file_reader.source_name ());}
	catch (exception& except)
		{
		#if (DEBUG & DEBUG_CONSTRUCTORS)
		clog << "!!! Failed to open -" << endl
			 << except.what () <<endl;
		#endif
		}
	catch (...)
		{
		#if (DEBUG & DEBUG_CONSTRUCTORS)
		clog << "!!! Failed to open -" << endl
			 << "    Unknown exception" << endl;
		#endif
		}
	}
}


JP2_File_Reader::~JP2_File_Reader () throw()
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">>> ~JP2_File_Reader @ " << (void*)this << endl;
#endif
JP2_File_Reader::reset ();

if (Thread_Group)
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    destroy and delete the Thread_Group @ "
			<< (void*)Thread_Group << endl;
	#endif
	Thread_Group->destroy ();
	delete Thread_Group;
	Thread_Group = NULL;
	}
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << "<<< ~JP2_File_Reader" << endl;
#endif
}


JP2_File_Reader*
JP2_File_Reader::clone () const
{return new JP2_File_Reader (*this);}

/*==============================================================================
	Source open, configure and close
*/
#ifndef DOXYGEN_PROCESSING
namespace
{
static const string
	open_failed ("Failed to open the JP2 source.");
}
#endif


int
JP2_File_Reader::open
	(
	const std::string&	source
	)
{
#if ((DEBUG) & DEBUG_OPEN)
clog << ">>> JP2_File_Reader::open: " << source << endl;
#endif
if (source.empty ())
	{
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "No JP2 source name specified to open.";
	throw JP2_Invalid_Argument (message.str (), ID);
	}

if (is_open ())
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    already open on " << source_name () << endl;
	#endif
	if (source == source_name ())
		{
		#if ((DEBUG) & DEBUG_OPEN)
		clog << "<<< JP2_File_Reader::open" << endl;
		#endif
		return 0;
		}
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    close the existing source to open on the new source" << endl;
	#endif
	close ();
	}

if (source != source_name())
	//	Reset source image characterization info.
	reset ();

//	Set the source name.
source_name (source);

#if ((DEBUG) & DEBUG_OPEN)
clog << "    open the jp2_threadsafe_family_src on \""
		<< source << '"' << endl;
#endif
try {JP2_Stream.open (source.c_str ());}
catch (kdu_exception except)
	{
	ostringstream
		message;
	message
		<< open_failed << endl
		<< Kakadu_error_message (except) << endl
		<< "while opening the source JP2 stream" << endl
		<< "for the " << source_name () << " source";
	throw JP2_Exception (message.str (), ID);
	}

//	Open the JP2 source object.
open_source ();

#if ((DEBUG) & DEBUG_OPEN)
clog << "<<< JP2_File_Reader::open" << endl;
#endif
return 1;
}


void
JP2_File_Reader::open_source ()
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING))
clog << ">>> JP2_File_Reader::open_source: " << endl;
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
clock_t
	begin_clock = clock (),
	start_clock = begin_clock,
	end_clock;
int
	time_select = 0;
struct timeval
	time_values[2],
	begin_time,
	*start_time = &begin_time,
	*end_time = &time_values[time_select];
gettimeofday (&begin_time, 0);
#endif	//	!_WIN32
#endif
if (is_open ())
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING))
	clog << "    Already open" << endl
		 << "<<< JP2_File_Reader::open_source" << endl;
	#endif
	return;
	}

//	Data stream management reset -----------------------------------------------

if (! JP2_Stream)
	{
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "The JP2 stream is closed; it should have been opened"
		<< "for the " << source_name () << " source";
	throw JP2_Logic_Error (message.str (), ID);
	}
if (JPEG2000_Codestream.exists ())
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "     destroy existing JPEG2000_Codestream" << endl;
	#endif
  if (Thread_Group)
    {
			//	Ensure shutdown of all thread processing.
			Thread_Group->cs_terminate (JPEG2000_Codestream);
    }

	JPEG2000_Codestream.destroy ();
	}
if (JP2_Source.exists ())
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "     close existing JP2_Source" << endl;
	#endif
	JP2_Source.close ();
	}

//	Open the JP2 source.
#if ((DEBUG) & DEBUG_OPEN)
clog << "    Open the jp2_source on the JP2_Stream" << endl;
#endif
if (! JP2_Source.open (&JP2_Stream))
	{
	close ();
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "Unable to open the " << source_name () << "\" JP2 source.";
	throw JP2_IO_Failure (message.str (), ID);
	}

//	Metadata -------------------------------------------------------------------

/*	Ensure availability of the JP2 header boxes up to the first codestream box.

	For a caching source it may be necessary to loop on a sleep
	(up to a maximum timeout) until the method returns true.
*/
bool
	headers_read = JP2_Source.read_header ();
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING))
clog << "    Headers read: " << boolalpha << headers_read << endl;
#endif
if (! headers_read &&
	JP2_Stream.uses_cache ())
	{
	//	Caching source (probably a JPIP_JP2_Reader); give it a chance to fill.
	int
		seconds = 10;
	while (seconds--)
		{
		if (headers_read)
			break;
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING))
		clog << "    sleep 1 second waiting for headers" << endl;
		#endif
        #ifdef _WIN32
        Sleep (1000 /* milliseconds */);
        #else
        sleep (1);
        #endif
		headers_read = JP2_Source.read_header ();
		#if ((DEBUG) & DEBUG_OPEN)
		clog << "    Headers read: " << boolalpha << headers_read << endl;
		#endif
		}
	}
if (! headers_read)
	{
	close ();
	ostringstream
		message;
	message
		<< "Unable to read the JP2 headers" << endl
		<< "for the \"" << source_name () << "\" source.";
	throw JP2_IO_Failure (message.str (), ID);
	}

#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_File_Reader::open_source: headers acquisition time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
start_clock = end_clock;
clog << "                                                  duration = "
		<< duration (start_time, end_time) << endl;
start_time = end_time;
end_time = &time_values[time_select = time_select ? 0 : 1];
#endif	//	!_WIN32
clog << "    Metadata ingest ..." << endl;
#endif
if (! ingest_metadata () ||
	! is_complete ())
	{
	ostringstream
		message;
	message
		<< "The required metadata is incomplete" << endl
		<< "for the " << source_name () << " source:" << endl
		<< validity_report ();
	throw JP2_Logic_Error (message.str (), ID);
	}

//	Codestream processing setup ------------------------------------------------

//	Thread-safe processing; creates the Thread_Group.
deploy_processing_threads ();

if (! JPEG2000_Codestream)
	{
	//	Open the JP2000 codestream.
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    create a kdu_codestream on the JP2_source" << endl;
	#endif
	JPEG2000_Codestream.create (&JP2_Source, Thread_Group);
	}

//	A persistent codestream is REQUIRED to enable interactive (re)use.
JPEG2000_Codestream.set_persistent ();

//	Rendering configuration ----------------------------------------------------

if (! Rendering_Configuration_Initialized)
	initialize ();
else
	{
	//	Force resolution and region to be restored.
	unsigned int
		resolution_level = Resolution_Level;
	Resolution_Level = 0;
	resolution_and_region (resolution_level, Image_Region);
	}
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_File_Reader::open_source: headers processing time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
clog << "                                                 duration = "
		<< duration (start_time, end_time) << endl;
if (begin_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_File_Reader::open_source: total time = "
			<< (double (end_clock - begin_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
clog << "                                    duration = "
		<< duration (&begin_time, end_time) << endl;
#endif	//	!_WIN32
clog << "<<< JP2_File_Reader::open_source" << endl;
#endif
}


void
JP2_File_Reader::initialize ()
{
/*
	>>> WARNING << All required default image rendering settings must be
	initialized here in the correct order (there are interdependencies to
	be considered). Essentially, working from the base up initialize
	everything for which there is a reasonable default, unless the user
	has already set the value, and without precluding the user from
	resetting configurable values.
*/
#if ((DEBUG) & DEBUG_OPEN)
clog << ">>> JP2_File_Reader::initialize" << endl;
#endif
//	Image size.
Rendered_Region =
Image_Region = image_size ();
#if ((DEBUG) & DEBUG_OPEN)
clog << "           image size = " << Image_Region << endl;
#endif

//	Image bands (components).
unsigned int
	bands = image_bands ();
#if ((DEBUG) & DEBUG_OPEN)
clog << "          image_bands = " << bands << endl;
#endif
Rendered_Bands.resize (bands, false);	//	Initialize the Band_Map.
render_band (ALL_BANDS, true);

//	Pixel data structure.
int
	pixel_bits = pixel_precision ();
if (! Rendered_Bits)
	Rendered_Bits = pixel_bits;
#if ((DEBUG) & DEBUG_OPEN)
clog << "           pixel_bits = " << pixel_bits << endl
	 << "          pixel_bytes = " << pixel_bytes () << endl;
#endif
bool
	signed_data = pixel_signed ();
#if ((DEBUG) & DEBUG_OPEN)
clog << "          signed_data = " << boolalpha << signed_data << endl;
#endif

//	Total available resolution levels.
#if ((DEBUG) & DEBUG_OPEN)
clog << "    resolution_levels = " << resolution_levels () << endl;
#endif
Resolution_Level = 1;	//	Initial default resolution level.

//	Check all bands for consistency.
#if ((DEBUG) & DEBUG_OPEN)
if (bands > 1)
	clog << "    Image bands consistency check ..." << endl;
#endif
for (unsigned int
		band = 1;
		band < bands;
	  ++band)
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    Band " << band << ": "
			<< pixel_precision (band) << "-bit "
			<< (pixel_signed (band) ? "" : "un") << "signed" << endl;
	#endif
	if (pixel_precision (band) != pixel_bits ||
		pixel_signed (band)    != signed_data)
		{
		ostringstream
			message;
		message
			<< open_failed << endl
			<< "Mismatched image bands:" << endl
			<< "  band 0 - " << pixel_bits << "-bit "
				<< (signed_data ? "" : "un") << "signed pixels" << endl
			<< "  band " << band << " - " << pixel_precision (band) << "-bit "
				<< (pixel_signed (band) ? "" : "un") << "signed pixels" << endl
			<< "for the \"" << source_name () << "\" source.";
		throw JP2_Out_of_Range (message.str (), ID);
		}
	}

//	Channel mapping
#if ((DEBUG) & DEBUG_OPEN)
clog << "    Channel mapping ..." << endl;
#endif
Channel_Mapping.configure (JPEG2000_Codestream);

kdu_coords
	reference_size,
	minimum_size,
	size;
JPEG2000_Codestream.get_subsampling
	(Channel_Mapping.source_components[0], reference_size, true);
minimum_size = reference_size;
for (int
		channel = 0;
		channel < Channel_Mapping.num_channels;
		channel++)
	{
	JPEG2000_Codestream.get_subsampling
		(Channel_Mapping.source_components[channel], size, true);
	#if ((DEBUG) & DEBUG_OPEN)
	clog
		<< "      " << channel << " - component "
		<< Channel_Mapping.source_components[channel]
			<< "; subsampling " << size.x << "w, " << size.y << "h; "
		<< Channel_Mapping.default_rendering_precision[channel]
			<< "-bit "
		<< (Channel_Mapping.default_rendering_signed[channel]
			? "" : "un") << "signed"
		<< (Channel_Mapping.fix16_palette[channel]
			? ", LUT-ed" : "") << endl;
	#endif
	if (minimum_size.x > size.x)
		minimum_size.x = size.x;
	if (minimum_size.y > size.y)
		minimum_size.y = size.y;
	}

if (minimum_size.x < reference_size.x)
	{
	//	Expand horizontally to the most sub-sampled component.
	Expand_Numerator.x = reference_size.x;
	Expand_Denominator.x = minimum_size.x;
	}
if (minimum_size.y < reference_size.y)
	{
	//	Expand vertically to the most sub-sampled component.
	Expand_Numerator.y = reference_size.y;
	Expand_Denominator.y = minimum_size.y;
	}
#if ((DEBUG) & DEBUG_OPEN)
clog << "      Expand_Numerator - " << Expand_Numerator << endl
	 << "    Expand_Denominator - " << Expand_Denominator << endl;
#endif

if (! Rendering_Increment_Lines)
	//	Default rendering increment.
	Rendering_Increment_Lines = effective_rendering_increment_lines ();
#if ((DEBUG) & DEBUG_OPEN)
clog << "<<< JP2_File_Reader::initialize" << endl;
#endif
}


bool
JP2_File_Reader::ingest_metadata ()
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << ">>> JP2_File_Reader::ingest_metadata" << endl;
#endif
bool
	loaded = false;
JP2_Box
	box;
long
	content_amount,
	content_size = 1024;
kdu_byte*
	content = new kdu_byte[content_size];

for (box.open (&JP2_Stream);
	 box.exists ();
	 box.open_next ())
	{
	content_amount = box.get_remaining_bytes ();
	if (content_amount > MAX_BOX_AMOUNT)
		content_amount = 0;
	if (content_amount > content_size)
		{
		delete[] content;
		content = new kdu_byte[content_size = content_amount];
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
		clog << "       content size = " << content_amount << endl;
		#endif
		}

	//	Load the box content.
	if (content_amount > 0 &&
		! (loaded = load_box_content (box, content)))
		break;

	//	Add box to metadata.
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
	clog << "    add_JP2_box: " << type_name (box.get_box_type ()) << endl
		 << "            content @ " << (void*)content << endl
		 << "             amount = " << content_amount << endl
		 << "      file position = " << box.get_locator ().get_file_pos ()
		 	<< endl;
	#endif
	add_JP2_box (box.get_box_type (), box.get_box_header_length (),
		content, content_amount, box.get_locator ().get_file_pos ());

	if (box.get_box_type () == CONTIGUOUS_CODESTREAM_TYPE)
		{
		ingest_codestream_segments (box);
		break;
		}

	box.close ();
	}
delete[] content;
if (box.exists ())
	box.close ();
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << validity_report ()
	 << "<<< JP2_File_Reader::ingest_metadata: " << loaded << endl;
#endif
return loaded;
}


bool
JP2_File_Reader::load_box_content
	(
	JP2_Box&		box,
	unsigned char*	content
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << ">>> JP2_File_Reader::load_box_content:" << endl
	 << box;
#endif
bool
	loaded = true;
long
	content_length = box.get_remaining_bytes ();
if (content_length > 0)
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
	clog << "    reading " << content_length << " byte box content" << endl;
	#endif
	try
		{
		if (box.read (content, (int)content_length) != content_length)
			{
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			clog << "    failed to read entire content!" << endl;
			#endif
			loaded = false;
			}
		}
	catch (...)
		{
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
		clog << "    box.read threw exception!" << endl;
		#endif
		loaded = false;
		}
	}
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << "<<< JP2_File_Reader::load_box_content: " << loaded << endl;
#endif
return loaded;
}


void
JP2_File_Reader::ingest_codestream_segments
	(
	JP2_Box&	codestream_box
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << ">>> JP2_File_Reader::ingest_codestream_segments" << endl;
#endif
/*	>>> WARNING <<<	The codestrean box read pointer is expected to be
	positioned immediately following the box header sequence; i.e. at
	the beginning of the first codestream segement.
*/
JP2_Box
	box (codestream_box.source (), codestream_box.get_locator ());
int
	content_amount,
	content_size = 1024;
kdu_byte*
	content = new kdu_byte[content_size];
long
	file_position =
		box.get_locator ().get_file_pos () +
		box.get_box_header_length ();
int
	marker,
	segment_length;

//	Read the segment marker.
while ((content_amount = box.read (content, 2)) == 2)
	{
	marker = get_short (content);
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
	clog << "            marker = " << segment_name (marker)
			<< " (" << marker_number (marker)
			<< "; 0x" << hex << marker << dec << ')' << endl;
	clog << "    SOT_MARKER = 0x" << hex << SOC_MARKER << dec << endl;
	#endif

	if (marker == SOT_MARKER)
		//	Stop at first tile marker.
		break;

	if (marker == SOC_MARKER ||
		marker == SOD_MARKER ||
		marker == EPH_MARKER ||
		marker == EOC_MARKER ||
	   (marker >= RESERVED_DELIMITER_MARKER_MIN &&
		marker <= RESERVED_DELIMITER_MARKER_MAX))
		//	Delimiting marker; no segment content.
		segment_length = 0;
	else
		{
		//	Read the segment length;
		segment_length = box.read (content, 2);
		if (segment_length < 2)
			//	No more data.
			break;
		segment_length = get_short (content);
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
		clog << "    segment length read = " << segment_length << endl;
		#endif
		}

	if (segment_length)
		{
		//	Read the segment content.
		segment_length -= 2;	//	Exclude the length field.
		if (segment_length > content_size)
			{
			delete[] content;
			content = new kdu_byte[segment_length];
			content_size = segment_length;
			}
		content_amount = box.read (content, segment_length);
		if (content_amount < segment_length)
			//	Insufficient data.
			break;
		segment_length += 2;	//	Add the length field.
		}
	segment_length += 2;		//	Add the marker field.
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
	clog << "    segment_length = " << segment_length << endl
		 << "     file_position = " << file_position << endl;
	#endif

	add_codestream_segment
		(marker, content, segment_length, file_position);

	file_position += segment_length;
	}

#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << "<<< JP2_File_Reader::ingest_codestream_segments" << endl;
#endif
}


bool
JP2_File_Reader::resolution_and_region
	(
	unsigned int		resolution,
	const Rectangle&	selected_region
	)
{
#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
clog << ">>> JP2_File_Reader::resolution_and_region:" << endl
	 << "          resolution = " << resolution << endl
	 << "    Resolution_Level = " << Resolution_Level << endl
	 << "     selected_region = " << selected_region << endl
	 << "        Image_Region = " << Image_Region << endl
	 << "     Rendered_Region = " << Rendered_Region << endl;
#endif
bool
	changed = false;

if (! JPEG2000_Codestream)
	{
	#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
	clog << "<<< JP2_File_Reader::resolution_and_region: not open." << endl;
	#endif
	return false;
	}

//	Limit the rendered resolution level.
if (resolution < 1)
	resolution = 1;
else
if (resolution > resolution_levels ())
	resolution = resolution_levels ();

try
{
if (resolution != Resolution_Level)
	{
	//	Apply the selected resolution.
	Resolution_Level = resolution;
	#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
	clog << "    Resolution_Level-> " << Resolution_Level << endl;
	#endif
	/*
		>>> WARNING The JPEG2000_Codestream must be in persistent mode
		to support repeated resolution and region selections.
	*/
	JPEG2000_Codestream.apply_input_restrictions
		(
		//	First codestream component index.
		0,
		//	Number of codestream components (zero means all from first).
		0,
		//	Discard levels (resolution level).
		resolution - 1,
		//	Max quality layers (zero means all).
		0,
		//	Region of interest on full resolution grid (NULL means all).
		NULL,
		//	All original codestream components remain visible.
		KDU_WANT_OUTPUT_COMPONENTS,

      // TODO veryify conditions in documentation for kdu_region_decompressor
      Thread_Group
		);
	changed = true;
	}

if (changed ||
	selected_region != Image_Region)
	{
	//	Set the region of interest to be rendered.
	KDU_dims
		image_dimensions (image_size ()),
		selection (selected_region);
	#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
	clog << "          image size = " << image_dimensions << endl;
	#endif
	if (selection.is_empty ())
		//	Select the entire image.
		selection = image_dimensions;
	else
		//	Intersection of the entire image with the selected region.
		selection &= image_dimensions;
	#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
	clog << "     selected region = " << selection << endl;
	#endif

	//	Apply the selected resolution level and region.
	JPEG2000_Codestream.apply_input_restrictions
		(0, 0, resolution - 1, 0, &selection, KDU_WANT_OUTPUT_COMPONENTS, Thread_Group);

	//	Set the effective region on the full resolution grid.
	Image_Region = static_cast<const Rectangle&>(selection);

	//	Get and set the effective region on the reduced resolution grid.
	JPEG2000_Codestream.get_dims (0, selection, true);
	Rendered_Region = static_cast<const Rectangle&>(selection);

	changed = true;
	#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
	clog << "        Image_Region-> " << Image_Region << endl
		 << "     Rendered_Region-> " << Rendered_Region << endl;
	#endif
	}
}
catch (kdu_exception except)
	{
	ostringstream
		message;
	message
		<< "Couldn't set rendering resolution " << resolution
			<< " and region " << selected_region << endl
		<< "for the " << source_name () << " source." << endl
		<< Kakadu_error_message (except);
	throw JP2_Exception (message.str (), ID);
	}

#if ((DEBUG) & (DEBUG_RES_REGION | DEBUG_OPEN))
clog << "<<< JP2_File_Reader::resolution_and_region: "
		<< boolalpha << changed << endl;
#endif
return changed;
}


void
JP2_File_Reader::deploy_processing_threads ()
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">>> JP2_File_Reader::deploy_processing_threads" << endl;
#endif
if (! Thread_Group &&
	Thread_Count > 1)
	{
	#if ((DEBUG) & DEBUG_RENDER)
	clog << "    Creating " << Thread_Count << " processing threads." << endl;
	#endif
	Thread_Group = new kdu_thread_env ();
	Thread_Group->create ();	//	Owner thread.

  for (auto
			count = 1;
			count < Thread_Count;
			count++)
		if (! Thread_Group->add_thread ())
			Thread_Count = count;
	#if ((DEBUG) & DEBUG_RENDER)
	clog << "    Created  " << Thread_Count << " processing threads." << endl;
	#endif

  Master_Queue = new kdu_thread_queue ();
try {
  #if ((DEBUG) & DEBUG_RENDER)
	auto attached =
	#endif
  Thread_Group->attach_queue(Master_Queue, NULL, "CHANGEME", 0, KDU_THREAD_QUEUE_SAFE_CONTEXT);
  #if ((DEBUG) & DEBUG_RENDER)
	clog << (attached ? "Successfully created " : "Failed to create ") << "a thread queue" << endl;
	#endif
} catch (kdu_exception except)
  	{
  	ostringstream
  		message;
  	message
  		<< "Couldn't attach a thread queue." << endl
  		<< Kakadu_error_message (except);
  	throw JP2_Exception (message.str (), ID);
  	}

   #if ((DEBUG) & DEBUG_RENDER)
   clog << "    Using default thread queues" << endl;
   #endif
	}
#if ((DEBUG) & DEBUG_RENDER)
else
	clog << "    Thread_Group "
			<< ((Thread_Count > 1) ? "already" : "not") << " created" << endl;
clog << "<<< JP2_File_Reader::deploy_processing_threads" << endl;
#endif
}


bool
JP2_File_Reader::is_open () const
{
return
	JP2_Stream.exists () &&
	JP2_Source.exists () &&
	JPEG2000_Codestream.exists ();
}


void
JP2_File_Reader::close
	(
	bool	//	force
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << ">>> JP2_File_Reader::close @ " << (void*)this << endl;
#endif
/*
	>>> WARNING <<< The order in which the hierarchy of objects is closed
	matters: failure to close in the correct order can produce a
	segmentation fault.
*/
//	Close the stream bound to the JP2 source.
if (JP2_Stream.exists ())
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    close the JP2_Stream" << endl;
	#endif
	JP2_Stream.close ();
	}

//	Close the JP2 source.
if (JP2_Source.exists ())
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    close the JP2_Source" << endl;
	#endif
	JP2_Source.close ();
	}

//	Release the codestream management machinery.
if (JPEG2000_Codestream.exists ())
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    destroy the JPEG2000_Codestream" << endl;
	#endif
  if (Thread_Group)
    {
			//	Ensure shutdown of all thread processing.
			Thread_Group->cs_terminate (JPEG2000_Codestream);
    }
	JPEG2000_Codestream.destroy ();
	}
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "<<< JP2_File_Reader::close" << endl;
#endif
}


void
JP2_File_Reader::reset ()
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << ">>> JP2_File_Reader::reset" << endl;
#endif
JP2_File_Reader::close ();
JP2_Reader::reset ();
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "<<< JP2_File_Reader::reset" << endl;
#endif
}

/*==============================================================================
	Codestream data acquisition.
*/
int
JP2_File_Reader::data_request
	(
	Rectangle*	region,
	bool		preemptive
	)
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">-< JP2_File_Reader::data_request: " << DATA_REQUEST_SATISFIED << endl;
#endif
return DATA_REQUEST_SATISFIED;
}


int
JP2_File_Reader::metadata_request
	(
	kdu_int32	box_type,
	bool		preemptive
	)
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">-< JP2_File_Reader::data_request: " << DATA_REQUEST_SATISFIED << endl;
#endif
return DATA_REQUEST_SATISFIED;
}


int
JP2_File_Reader::data_acquisition
	(
	Acquired_Data*	acquired_data
	)
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">-< JP2_File_Reader::data_acquisition" << endl;
#endif
if (acquired_data)
	acquired_data->acquired
		(Resolution_Level,
		 Rendered_Region.X, Rendered_Region.Y,
		 Rendered_Region.Width, Rendered_Region.Height);
return DATA_ACQUISITION_COMPLETE;
}


std::string
JP2_File_Reader::data_request_description
	(
	int		status
	)
{
ostringstream
	report;
report << status << " - DATA_REQUEST_";
switch (status)
	{
	case DATA_REQUEST_SATISFIED:	report << "SATISFIED"; break;
	case DATA_REQUEST_SUBMITTED:	report << "SUBMITTED"; break;
	case DATA_REQUEST_REJECTED:		report << "REJECTED";  break;
	default:						report << "unknown";
	}
return report.str ();
}


std::string
JP2_File_Reader::data_acquisition_description
	(
	int		status
	)
{
bool
	add = false;
ostringstream
	report;
report << status << " - DATA_ACQUISITION_";
if (status & DATA_ACQUISITION_COMPLETE)
	{
	report << "COMPLETE";
	add = true;
	}
if (status & DATA_ACQUISITION_INCOMPLETE)
	{
	if (add)
		report << " | ";
	report << "INCOMPLETE";
	add = true;
	}
if (status & DATA_ACQUISITION_REDUCED)
	{
	if (add)
		report << " | ";
	report << "REDUCED";
	add = true;
	}
if (status & DATA_ACQUISITION_CANCELED)
	{
	if (add)
		report << " | ";
	report << "CANCELED";
	}
return report.str ();
}

/*==============================================================================
	Render
*/
Cube
JP2_File_Reader::render ()
{
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
clog << ">>> JP2_File_Reader::render" << endl;
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
clock_t
	begin_clock = clock (),
	start_clock = begin_clock,
	end_clock;
int
	time_select = 0;
struct timeval
	time_values[2],
	begin_time,
	*start_time = &begin_time,
	*end_time = &time_values[time_select];
gettimeofday (&begin_time, 0);
#endif	//	!_WIN32
#endif
Bytes_Rendered = 0;

string
	reasons;
if (! ready (&reasons))
	{
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
	clog << "    Not ready -" << endl
		 << reasons << endl;
	#endif
	ostringstream
		message;
	message
		<< "Couldn't render region " << rendered_region () << endl;
	if (rendered_region () != image_region ())
		message << " of image region " << image_region () << " -" << endl;
	message
		<< "at resolution level " << resolution_level () << '.' << endl
		<< reasons << endl
		<< "for the " << source_name () << " source.";
	throw JP2_Logic_Error (message.str (), ID);
	}

if (! rendered_region ().area ())
	{
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
	clog << "    Empty rendered area." << endl
		 << "<<< JP2_File_Reader::render: (empty)" << endl;
	#endif
	return Cube ();
	}

//	Image data characterization ------------------------------------------------

//	Rendering region management.
Cube
	//	Region to render on the full resolution grid.
	full_res_region_cube (image_region ()),
	//	Region rendered on the full resolution grid.
	full_res_rendered_cube (full_res_region_cube),
	//	Region rendered on the rendering grid.
	rendered_res_cube (rendered_region ());
Rectangle
	//	Region to render on the rendering grid.
	render_region (rendered_region ()),
	//	Current section of the render_region.
	region_section (render_region);
KDU_dims
	//	Current slice of the region_section for decompression.
	region_slice (region_section),
	//	The last part of the region_slice that was decompressed.
	region_rendered;

//	Rendering parameters.
int
    end_line			= render_region.Y + render_region.Height,
    resolution			= resolution_level (),
    pixel_bits			= rendered_pixel_bits (),
    pixel_bytes			= rendered_pixel_bytes (),
    bands				= rendered_bands (),
    pixel_gap			= pixel_stride (),
    row_gap				= line_stride (),
    line_increment		= effective_rendering_increment_lines (),
    suggested_increment;

kdu_coords
	subsampling;
JPEG2000_Codestream.get_subsampling
	(Channel_Mapping.source_components[0], subsampling, true);

#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
clog << "       resolution level = " << resolution << endl
	 << "         pixel precison: "
	 	<< pixel_bits << "-bit (" << pixel_bytes << " byte)" << endl
	 << "            image bands = " << bands << '/' << image_bands () << endl
	 << "           image region = " << full_res_region_cube << endl
	 << "          render_region = " << render_region << endl
	 << "              pixel_gap = " << pixel_gap << endl
	 << "                row_gap = " << row_gap << endl
	 << "         line_increment = " << line_increment << endl
	 << "            subsampling = " << subsampling << endl
	 << "       Expand_Numerator = " << Expand_Numerator << endl
	 << "     Expand_Denominator = " << Expand_Denominator << endl;
#endif

//	Pixel data storage ---------------------------------------------------------

//	Ensure that pixel data storage has been allocated.
allocate_image_data_buffer ();

//	The image data buffers origin on the rendering grid.
kdu_coords
	buffer_origin (render_region.X, render_region.Y);
//	Image data buffers.
unsigned int
	total_bands = image_bands ();
void
	//	The MSVC compiler wont allocate variable length arrays on the stack.
	**image_data = new void*[total_bands];
#if ((DEBUG) & DEBUG_RENDER)
clog << "       image buffers -" << endl
	 << "       buffer_origin: " << buffer_origin << endl;
#endif
for (unsigned int
		band = 0;
		band < total_bands;
		band++)
	{
	image_data[band] = Rendered_Bands[band] ? Image_Data[band] : NULL;
	#if ((DEBUG) & DEBUG_RENDER)
	clog << "                  " << band << " @ "
			<< image_data[band] << endl;
	#endif
	}

//	Codestream acquisition and decompression -----------------------------------
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_File_Reader::render: setup time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
start_clock = end_clock;
clog << "                               duration = "
		<< duration (start_time, end_time) << endl;
start_time = end_time;
end_time = &time_values[time_select = time_select ? 0 : 1];
clock_t
	data_acquisition_clock = 0,
	decompress_clock = 0;
double
	data_acquisition_duration = 0,
	decompress_duration = 0;
#endif	//	!_WIN32
int
	acquisitions = 0,
	decompressions = 0,
	total_decompressions = 0;
JP2_JPIP_Reader
	*JPIP_reader = dynamic_cast<JP2_JPIP_Reader*>(this);
#endif

int
	data_request_status = DATA_REQUEST_SATISFIED,
	//	File source data acquisition is always complete.
	data_acquisition_status = DATA_ACQUISITION_COMPLETE;
Acquired_Data
	acquired_data;

Rendering_Monitor
	*monitor = rendering_monitor ();
//	File source always provides top quality data.
Rendering_Monitor::Status
	status = Rendering_Monitor::TOP_QUALITY_DATA;

kdu_exception
	kdu_exception_value;

bool
	continue_rendering = true;	//	False if rendering canceled.
while (continue_rendering)
	{
	//	Acquire more data when the last request is complete.
	if (data_acquisition_status & DATA_ACQUISITION_COMPLETE)
		{
		//	Initiate a codestream data fetch (only relevant for JPIP).
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
		clog << "==> data_request for region " << region_section << endl;
		#endif
		try {data_request_status = data_request (&region_section);}
		catch (JPIP_Exception except)
			{
			if (Thread_Group)
				Thread_Group->handle_exception (READER_ERROR);
			delete[] image_data;
			ostringstream
				message;
			message
				<< "Couldn't render region " << rendered_region () << endl;
			if (rendered_region () != image_region ())
				message << "- image region " << image_region () << " -" << endl;
			message
				<< "at resolution level " << resolution << endl
				<< "for the " << source_name () << " source." << endl
				<< except.message ();
			except.message (message.str ());
			throw;
			}
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
		clog << "    data_request_status = "
				<< data_request_description (data_request_status) << endl;
		#endif
		if (data_request_status == DATA_REQUEST_REJECTED)
			{
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_LOCATION))
			clog << "<<< JP2_File_Reader::render: (empty)" << endl;
			#endif
			return Cube ();
			}
		}
	if (data_request_status == DATA_REQUEST_SUBMITTED)
		{
		//	JPIP data acquisition pending.
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
		clog << "--> " << ++acquisitions << " - data_acquisition" << endl;
		#endif
		try {data_acquisition_status = data_acquisition (&acquired_data);}
		catch (JPIP_Exception except)
			{
			if (Thread_Group)
				Thread_Group->handle_exception (READER_ERROR);
			delete[] image_data;
			ostringstream
				message;
			message
				<< "Couldn't render region " << rendered_region () << endl;
			if (rendered_region () != image_region ())
				message << "- image region " << image_region () << " -" << endl;
			message
				<< "at resolution level " << resolution << endl
				<< "for the " << source_name () << " source." << endl
				<< except.message ();
			except.message (message.str ());
			throw;
			}
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
		#ifndef _WIN32
		//	Procedure timing is not implemented for MS/Windows.
		end_clock = clock ();
		gettimeofday (end_time, 0);
		#endif	//	!_WIN32
		clog << "<-- " << acquisitions << " - data acquired: "
				<< data_acquisition_description (data_acquisition_status) << endl
			 << "    JPIP client status - "
			 	<< JPIP_reader->connection_status () << endl
			 << "    acquired region " << acquired_data
				<< " at resolution level "
				<< acquired_data.Resolution_Level << endl;
		#ifndef _WIN32
		//	Procedure timing is not implemented for MS/Windows.
		if (start_clock != clock_t (-1) &&
			end_clock != clock_t (-1))
			{
			clog << "    JP2_File_Reader::render: data acquisition time = "
					<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
					<< " seconds" << endl;
			data_acquisition_clock += end_clock - start_clock;
			}
		else
			data_acquisition_clock = clock_t (-1);
		start_clock = end_clock;
		clog << "                                          duration = "
				<< duration (start_time, end_time) << endl;
		data_acquisition_duration += duration (start_time, end_time);
		start_time = end_time;
		end_time = &time_values[time_select = time_select ? 0 : 1];
		#endif	//	!_WIN32
		#endif

		if (data_acquisition_status & DATA_ACQUISITION_CANCELED)
			{
			continue_rendering = false;
			break;
			}

		status = (data_acquisition_status & DATA_ACQUISITION_COMPLETE) ?
					Rendering_Monitor::TOP_QUALITY_DATA :
					Rendering_Monitor::LOW_QUALITY_DATA;

		if (acquired_data.is_empty ())
			{
			//	No data acquired.
			if (data_acquisition_status & DATA_ACQUISITION_INCOMPLETE)
				//	Try again.
				continue;
			else
			if (data_acquisition_status == DATA_ACQUISITION_COMPLETE)
				//	No more data to acquire.
				break;
			}
		}

	//	Codestream decompression -----------------------------------------------

	//	The region slice to decompress is the current region section.
	region_slice = region_section;

	/*	Start the Decompressor.

		After being started the Decompressor will only process codestream
		regions in a sequentially forward manner; the Decompressor will
		not allow decompression regions to be repeated unless it has been
		finished and then restarted. Thus a complete start-process-finish
		sequence must be done whenever new codestream data has been
		acquired.

		<b>N.B.</b>: The Decompressor assumes MSB pixel samples and
		automatically swaps sample bytes on an LSB host.
	*/
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_LOCATION))
	clog
		<< "==> Starting the Decompressor for region " << region_slice << endl;
	decompressions = 0;
	#endif
	try {Decompressor.start
		(
		JPEG2000_Codestream,
		&Channel_Mapping,
		//	Single component (ignored when Channel_Mapping used).
		-1,
		//	Discard levels (resolution).
		resolution - 1,
		//	Max layers (quality).
		INT_MAX,
		//	Selected region on the rendering grid.
		region_slice,
		//	Channel expansion factors used with Channel_Mapping.
		Expand_Numerator,
		Expand_Denominator,
		//	High precision decompression operations.
		true,
		//	Access mode.
		KDU_WANT_OUTPUT_COMPONENTS,
		//	Fastest (ignored when precise is true).
		false,
		//	Multi-threaded processing environment (single threaded if NULL).
		Thread_Group,

    Master_Queue
		);}
	catch (kdu_exception except)
		{
		if (Thread_Group)
			Thread_Group->handle_exception (READER_ERROR);
		delete[] image_data;
		ostringstream
			message;
		message
			<< "Couldn't render region " << rendered_region () << endl;
		if (rendered_region () != image_region ())
			message << "- image region " << image_region () << " -" << endl;
		message
			<< "at resolution level " << resolution << '.' << endl
			<< "Starting the JPEG2000 codestream decompressor failed" << endl
			<< "after rendering " << magnitude (Bytes_Rendered)
				<< " (" << Bytes_Rendered << ") image data bytes" << endl
			<< "for the " << source_name () << " source." << endl
			<< Kakadu_error_message (except);
		throw JP2_Exception (message.str (), ID);
		}

	bool
		continue_decompressing = true;
	while (continue_decompressing)
		{
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
		clog << "..> " << acquisitions << '-'
				<< ++decompressions << '/' << ++total_decompressions
				<< " - decompressing region slice " << region_slice << endl;
		#ifndef _WIN32
		//	Procedure timing is not implemented for MS/Windows.
		start_clock = clock ();
		gettimeofday (start_time, 0);
		#endif	//	!_WIN32
		#endif
		if (line_increment < region_slice.size.y &&
		   (line_increment + (line_increment >> 3)) > region_slice.size.y)
			{
			//	Render the remainder of the slice.
			suggested_increment = region_slice.size.y * region_slice.size.x;
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
			clog << "    increased line increment from " << line_increment
					<< " to " << region_slice.size.y << endl;
			#endif
			}
		else
			suggested_increment = line_increment * region_slice.size.x;
		try
			{
			if (pixel_bytes == 1)
				continue_decompressing = Decompressor.process
					(
					/*	Array of rendered pixel data buffer pointers.

						Each entry is the address of the pixel data buffer
						for the corresponding codestream channel. NULL
						entries indicated channels to be skipped.
					*/
					reinterpret_cast<kdu_byte**>(image_data),

					//	Expand single band to multi-band.
					false,
					//	Pixel sample stride distance in bytes.
					pixel_gap,

					/*	Origin on the rendered grid of the image_data buffers.

						This is set to the render region origin.
						The buffer origin is ignored if the row_gap is zero.
					*/
					buffer_origin,

					//	Line stride distance in pixel samples.
					row_gap,

					/*	Suggested samples, of the first channel, to render.

						<b>N.B.</b>: This is only a suggestion; the method
						may decompress fewer or more lines. Experience shows
						that the method is likely to decompresses one more
						line than suggested. The exception is the last
						increment which will never exceed the incomplete
						region slice.
					*/
					suggested_increment,

					//	Max buffer samples; ignored if row_gap non-zero.
					-1,

					/*	The incomplete region yet to be rendered.

						When the method returns the position of the first
						line (pos.y) will have been moved down (increased)
						by the number of completely decompressed lines.
					*/
					region_slice,

					/*	Reports the region that was rendered.

						The region that was actually decompressed and written
						to the image_data buffers. May be empty on return
						even if there is more data to decompress (i.e. use
						continue_decompressing).
					*/
					region_rendered,

					//	Precision of the rendered pixel samples.
					pixel_bits,
					//	Is row_gap measured in pixels (or samples)?
					false
					);
			else
				continue_decompressing = Decompressor.process
	  				(
					reinterpret_cast<kdu_uint16**>(image_data),
					false,
					pixel_gap,
					buffer_origin,
					row_gap,
					suggested_increment,
					-1,
					region_slice,
					region_rendered,
					pixel_bits,
					false
					);
          /*
          if (Thread_Group)
            {
              #if ((DEBUG) & DEBUG_RENDER)
              clog << "   Waiting for threads to complete..." << endl;
              #endif
                    //	Ensure completion of all thread processing.
            if(!Thread_Group->join (Master_Queue, false, &kdu_exception_value))
            {
            throw JP2_Exception("Thread join failed");
            }
          }*/
			}
		catch (kdu_exception except)
			{
			#if ((DEBUG) & DEBUG_RENDER)
			clog << "<-- Decompression exception!" << endl;
			#endif
			if (Thread_Group)
				Thread_Group->handle_exception (READER_ERROR);
			Decompressor.finish ();
			if (Thread_Group)
				Thread_Group->terminate (NULL, true);
			close ();
			delete[] image_data;
			ostringstream
				message;
			message
				<< "Couldn't render region " << rendered_region () << endl;
			if (rendered_region () != image_region ())
				message << "- image region " << image_region () << " -" << endl;
			message
				<< "at resolution level " << resolution << '.' << endl
				<< "JPEG2000 codestream decompression failed" << endl
				<< "while rendering section " << region_slice << endl
				<< "after rendering " << magnitude (Bytes_Rendered)
					<< " (" << Bytes_Rendered << ") image data bytes" << endl
				<< "for the " << source_name () << " source." << endl
				<< Kakadu_error_message (except);
			throw JP2_Exception (message.str (), ID);
			}



		Bytes_Rendered +=
			region_rendered.area () * bands * pixel_bytes;

		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
		clog << "..> " << acquisitions << '-'
				<< decompressions << '/' << total_decompressions
			<< " - decompression - continue = "
				<< boolalpha << continue_decompressing << endl
			<< "         region_slice: " << region_slice << endl
			<< "      region_rendered: " << region_rendered << endl
			<< "       Bytes_Rendered: " << Bytes_Rendered << endl;
		#ifndef _WIN32
		//	Procedure timing is not implemented for MS/Windows.
		end_clock = clock ();
		gettimeofday (end_time, 0);
		if (start_clock != clock_t (-1) &&
			end_clock != clock_t (-1))
			{
			clog << "    JP2_File_Reader::render: decompress time = "
					<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
					<< " seconds" << endl;
			decompress_clock += end_clock - start_clock;
			}
		else
			decompress_clock = clock_t (-1);
		start_clock = end_clock;
		clog << "                                    duration = "
				<< duration (start_time, end_time) << endl;
		decompress_duration += duration (start_time, end_time);
		start_time = end_time;
		end_time = &time_values[time_select = time_select ? 0 : 1];
		#endif	//	!_WIN32
		#endif

		if (continue_decompressing &&
			region_slice.is_empty ())
			//!!! Work-around for case where process should return false.
			continue_decompressing = false;

		//	Rendered region decompressed for data_disposition.
		rendered_res_cube.Y           = region_rendered.pos.y;
		rendered_res_cube.Height      = region_rendered.size.y;
		//	Reverse map rendered res dimensions to the full res dimensions.
		region_rendered =
			Decompressor.find_codestream_cover_dims (region_rendered,
				subsampling, Expand_Numerator, Expand_Denominator);
		full_res_rendered_cube.Y      = region_rendered.pos.y;
		full_res_rendered_cube.Height = region_rendered.size.y;
		//	Clip to the user specifified dimensions.
		full_res_rendered_cube &= full_res_region_cube;
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_LOCATION))
		clog << "       decompressed region = "
				<< rendered_res_cube << endl
			 << "           rendered region = "
			 	<< render_region << endl
			 << "              image region = "
			 	<< full_res_rendered_cube << endl;
		#endif
		//	Wrangle the data into the correct structure and notify monitor.
		continue_rendering =	//	False if monitor user cancelled.
			data_disposition
				(status, Rendering_Monitor::Status_Message[status],
				rendered_res_cube, full_res_rendered_cube);

		continue_decompressing =
			//	Monitor user cancelled?
			continue_rendering &&
			//	Decompressor not finished with the slice?
			continue_decompressing &&
			//	More region lines to render?
			region_slice.size.y;
		}	//	Decompression.

	//	Stop the decompressor.
  Thread_Group->cs_terminate(JPEG2000_Codestream, &kdu_exception_value);

	if (! Decompressor.finish (&kdu_exception_value, false))
		{
		close ();
		delete[] image_data;
		ostringstream
			message;
			message
				<< "Couldn't render region " << rendered_region () << endl;
			if (rendered_region () != image_region ())
				message << "- image region " << image_region () << " -" << endl;
			message
				<< "at resolution level " << resolution << '.' << endl
			<< "JPEG2000 codestream decompression finish failed" << endl
			<< "after rendering " << magnitude (Bytes_Rendered)
				<< " (" << Bytes_Rendered << ") image data bytes." << endl
			<< "The codestream may be corrupted" << endl
			<< "for the " << source_name () << " source." << endl
			<< Kakadu_error_message (kdu_exception_value);
		throw JP2_Exception (message.str (), ID);
		}

	//	Check if more data needs to be acquired --------------------------------

	if (continue_rendering &&
		data_acquisition_status & DATA_ACQUISITION_COMPLETE)
		{
		if (data_acquisition_status & DATA_ACQUISITION_REDUCED)
			{
			//	All requested data was not acquired; adjust the request region.
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_LOCATION))
			clog << "    adjusting reduced acquired region - "
					<< acquired_data << endl;
			#endif
			acquired_data.Resolution_Level -= resolution;
			if (acquired_data.Resolution_Level < 0)
				acquired_data.Resolution_Level = 0;
			unsigned int
				adjusted_height =
					acquired_data.area ()
					/ (render_region.Width >> acquired_data.Resolution_Level);
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_LOCATION))
			clog << "    adjusted height = " << adjusted_height << endl;
			#endif
			if (adjusted_height < region_section.Height)
				{
				if (adjusted_height == 0)
					{
					if (region_section.Height == 1)
						{
						//	Can't obtain the codestream for a single line!
						ostringstream
							message;
						message
							<< "Couldn't render region "
								<< rendered_region () << endl;
						if (rendered_region () != image_region ())
							message << "- image region "
								<< image_region () << " -" << endl;
						message
							<< "at resolution level "
								<< resolution << '.' << endl
							<< "Unable to obtain a single line"
								" of codestream data" << endl
							<< "for the " << source_name () << " source.";
						throw JPIP_Exception (message.str (), ID);
						}
					adjusted_height = 1;
					}
				region_section.Height = adjusted_height;
				continue;
				}
			}
		else
		if (region_slice.pos.y == end_line)
			//	All data acquired.
			break;
		else
			{
			//	Move to the next section of the region.
			region_section.Y += region_section.Height;
			if ((region_section.Y + region_section.Height)
					> (unsigned int)end_line)
				 region_section.Height = end_line - region_section.Y;
			}
		}
	}	//	Data acquisition.

//	Region rendered.
rendered_res_cube.Y = render_region.Y;
rendered_res_cube.Height = region_slice.pos.y - render_region.Y;
region_rendered = rendered_res_cube;
//	Reverse map the rendered res dimensions to the full res dimensions.
region_rendered =
	Decompressor.find_codestream_cover_dims (region_rendered,
		subsampling, Expand_Numerator, Expand_Denominator);
full_res_rendered_cube.Y          = region_rendered.pos.y;
full_res_rendered_cube.Height     = region_rendered.size.y;
//	Clip to the user specifified dimensions.
full_res_rendered_cube &= full_res_region_cube;

#if ((DEBUG) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << "<== Decompression finished" << endl;
#endif
status = continue_rendering ?
	Rendering_Monitor::DONE : Rendering_Monitor::CANCELED;
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
clog << "    JP2_File_Reader::render: notify status "
		<< status << " \""
		<< Rendering_Monitor::Status_Message[status] << '"' << endl
	 << "    rendered region = " << rendered_res_cube << endl
	 << "       image region = " << full_res_rendered_cube << endl;
#endif
if (monitor)
	monitor->notify (*this,
		status, Rendering_Monitor::Status_Message[status],
		rendered_res_cube, full_res_rendered_cube);

//	Cleanup.
delete[] image_data;
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_TIMING | DEBUG_LOCATION))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (begin_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_JPIP_Reader::render: total time = "
			<< (double (end_clock - begin_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl
		 << "                               duration = "
		 	<< duration (&begin_time, end_time) << endl
		 << "    data acquisition time = "
		 	<< (double (data_acquisition_clock) / CLOCKS_PER_SEC) << endl
		 << "                 duration = "
		 	<< data_acquisition_duration << endl
		 << "          decompress time = "
		 	<< (double (decompress_clock) / CLOCKS_PER_SEC) << endl
		 << "                 duration = "
		 	<< decompress_duration << endl
		 << "        data acquisitions = " << acquisitions << endl
		 << "      data decompressions = " << total_decompressions << endl;
#endif	//	!_WIN32
clog << "<<< JP2_File_Reader::render: " << rendered_res_cube << endl;
#endif
return rendered_res_cube;
}


std::string
JP2_File_Reader::Kakadu_error_message
	(
	const kdu_exception&	except
	)
{
ostringstream
	message;
message
	<< "Kakadu exception " << except;
const char*
	report = Error_Message_Queue.pop_message ();
if (report)
	{
	message
		<< " -" << endl
		<< report;
	while ((report = Error_Message_Queue.pop_message ()))
		message << endl << report;
	}
else
	message << '.';
return message.str ();
}

}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA
