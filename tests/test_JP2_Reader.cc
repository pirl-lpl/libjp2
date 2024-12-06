/*	test_JP2_Reader

HiROC CVS ID: $Id: test_JP2_Reader.cc,v 1.28 2012/06/10 00:31:43 castalia Exp $

Copyright (C) 2009 Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License, version 2, as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include	"JP2.hh"
using UA::HiRISE::JP2;
using UA::HiRISE::JP2_Reader;
using UA::HiRISE::JP2_Exception;
using UA::HiRISE::bytes_of_bits;

//	Kakadu
#include	"kdu_arch.h"

//	PIRL++
#include	"Dimensions.hh"
#include	"Files.hh"
#include	"endian.hh"
using namespace PIRL;

//	idaeim PVL
#include	"PVL.hh"
using idaeim::PVL::Aggregate;

#include	<iostream>
#include	<iomanip>
#include	<cstdlib>
#include	<fstream>
#include	<sstream>
#include	<cctype>
#include	<string>
#include	<cstring>
#include	<vector>
#include	<utility>
#include	<stdexcept>
#include	<ctime>

#ifndef _WIN32
#include	<sys/time.h>
#endif
using namespace std;

/*==============================================================================
	Constants
*/
#ifndef MODULE_VERSION
#define _VERSION_ " "
#else
#define _VERSION_ " v" MODULE_VERSION " "
#endif
//!	Application identification name with source code version and date.
const char* const
	ID =
		"test_JP2_Reader"
		_VERSION_
		"($Revision: 1.28 $ $Date: 2012/06/10 00:31:43 $)";

//!	The runtime command name.
char
	*Program_Name;

//!	Listing format widths.
const int
	LABEL_WIDTH					= 24,
	VALUE_WIDTH					= 9;

//!	Exit status values.
const int
	SUCCESS						= 0,

	//	Command line syntax.
	BAD_SYNTAX					= 1,

	//	Software / data problem.
	INVALID_ARGUMENT			= 11,
	PDS_LABEL_ERROR				= 12,
	NO_IMAGE_DATA				= 13,
	LABEL_INCONSISTENCY			= 14,
	LOGIC_ERROR					= 19,

	//	IO.
	NO_INPUT_FILE				= 20,
	EXISTING_OUTPUT_FILE		= 21,
	IO_FAILURE					= 29,

	//	PVL.
	PVL_ERROR					= 30,

	//	JP2 Reader.
	READER_ERROR				= 40,

	//	Unknown?
	UNKNOWN_ERROR				= -1;

/*==============================================================================
	Usage
*/
void
usage
	(
	int		exit_status = BAD_SYNTAX,
	bool	list_descriptions = false
	)
{
cout
	<< "Usage: " << Program_Name << " [options] [-Input | -Jp2] <pathname>" << endl;
if (list_descriptions)
	cout
	<< endl
	<< "Renders a JP2 file to raw image data." << endl;

cout
	<< "Options -" << endl;

cout
	<< "  -Input | -Jp2 <source>" << endl;
if (list_descriptions)
	cout
	<< "    The source of the JP2 file." << endl
	<< endl;

cout << "  -Area <origin_x>,<origin_y>,<extent_x>,<extent_y>" << endl;
if (list_descriptions)
	cout
	<< "    An area of the image to be selected for decompression. The area" << endl
	<< "    is specified relative to the image at full resolution, before any" << endl
	<< "    resolution level selection size reduction is applied.  If the" << endl
	<< "    area selected extends beyond the width (x) or height (y) of the" << endl
	<< "    image size the area is clipped to corresponding limit. If the" << endl
	<< "    origin of the area falls outside the image boundaries no output" << endl
	<< "    will be generated; an empty image will not be produced." << endl
	<< endl
	<< "    Default: The entire image." << endl
	<< endl;

cout << "  -Resolution <level>" << endl;
if (list_descriptions)
	cout
	<< "    The JPEG2000 codestream may provide access to the image at multiple" << endl
	<< "    resolution levels. The image is always available at full resolution;" << endl
	<< "    level 1. The next level is half resolution, then half of that, etc.;" << endl
	<< "    i.e. the effective image size at resolution level L is the full" << endl
	<< "    resolution size divided by 2^(L-1). If the selected resolution level" << endl
	<< "    is greater than the number of resolution levels available in the JP2" << endl
	<< "    source codestream the image will be rendered at the smallest size." << endl
	<< endl
	<< "    Default: Full resolution level 1." << endl
	<< endl;

cout
	<< "  -[No_]Swap_bytes" << endl;
if (list_descriptions)
	cout
	<< "    Normally multi-byte image pixel values are rendered in MSB order." << endl
	<< "    This option causes the byte order of multi-byte pixels to be swapped." << endl
	<< endl
	<< "    Default: false." << endl
	<< endl;

cout
	<< "  -Description_only" << endl;
if (list_descriptions)
	cout
	<< "    Only list the JP2 data desciption; no rendering is done."
	<< endl
	<< "    Default: The data description is followed by data rendering." << endl
	<< endl;

cout
	<< "  -Prompt" << endl;
if (list_descriptions)
	cout
	<< "    Prompt when ready to begin setup phase, and before rendering the" << endl
	<< "    source data. Just hit enter (return) to proceed from the prompt." << endl
	<< endl
	<< "    Default: no prompting." << endl
	<< endl;

cout
	<< "  -Clone_test" << endl;
if (list_descriptions)
	cout
	<< "    Try to repeate the data rendering on a clone of the initial JP2_Reader." << endl
	<< endl
	<< "    Default: No clone test." << endl;

cout
	<< "  -Help" << endl;
if (list_descriptions)
	cout
	<< "    Prints this usage description." << endl
	<< endl;

exit (exit_status);
}

#if defined (DEBUG)
/*******************************************************************************
	DEBUG controls

	DEBUG report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_ALL		-1
#define DEBUG_HELPERS	(1 << 4)
#include	<iostream>
using std::clog;
#endif  // DEBUG

/*==============================================================================
	Rendering Monitor
*/
struct rendering_monitor
:	public JP2_Reader::Rendering_Monitor
{
bool
notify
	(
	JP2_Reader&								reader,
	JP2_Reader::Rendering_Monitor::Status	status,
	const std::string&						message,
	const Cube&								region_rendered,
	const Cube&								image_region_rendered
	)
{
clog << "**> test_JP2_Reader::monitor_rendering::notify:" << endl
	 << "    status " << status << " \"" << message << '"' << endl
	 << "    " << reader.source_name () << endl
	 << "    region rendered " << region_rendered << endl
	 << "                 of " << reader.rendered_region () << endl
	 << "     image rendered " << image_region_rendered << endl
	 << "                 of " << reader.image_region () << endl
	 << flush;
return true;
}
};

/*==============================================================================
	Time duration function
*/
double duration (struct timeval* start, struct timeval* end)
{
return
	((double (end->tv_usec) / 1000000.0) + end->tv_sec) -
	((double (start->tv_usec) / 1000000.0) + start->tv_sec);
}

/*==============================================================================
	Main
*/
int
main
	(
	int		argument_count,
	char	**arguments
	)
{
//	Application identification.
cout << ID << endl;
Program_Name = *arguments;

string
	JP2_source,
	values;
int
	//	offset x,y and size x,y.
	image_area[]	 = {0, 0, 0, 0},
	resolution_level = 1;

bool
	swap_bytes		= false,
	prompt			= false,
	clone			= false,
	description_only= false;
long
	value;
char
	*character;
unsigned int
	index;

/*------------------------------------------------------------------------------
   Command line arguments
*/
if (argument_count == 1)
    usage ();

for (int
		count = 1;
		count < argument_count;
		count++)
	{
	if (arguments[count][0]== '-')
		{
		switch (toupper (arguments[count][1]))
			{
			case 'R':	//	Resolution.
				if (++count == argument_count ||
					arguments[count][0] == '-')
					{
					cout << "Missing resolution level." << endl
						 << endl;
					usage ();
					}
				value = strtol (arguments[count], &character, 0);
				if (*character ||
					value <= 0)
					{
					cout << "Positive value expected, but "
							<< arguments[count] << " found." << endl;
					usage ();
					}
				resolution_level = value;
				break;

			case 'A':	//	Area.
				if (++count == argument_count)
					{
					cout << "Missing image area values." << endl
						 << endl;
					usage ();
					}
				values = arguments[count];
				index = 0;
				for (char*
						token = strtok (arguments[count], ",xX");
						token;
						token = strtok (NULL, ",xX"))
					{
					if (index == 4)
						goto Invalid_Region_Values;
					value = strtol (token, &character, 0);
					if (*character)
						goto Invalid_Region_Values;
					if (index > 1 &&
						value < 0)
						{
						cout << "Image area sizes must be positive, but "
								<< value << " found in "
								<< values << '.' << endl;
						usage ();
						}
					image_area[index++] = (int)value;
					}
				if (index < 4)
					{
					Invalid_Region_Values:
					cout << "Four image area values expected, but "
							<< values << " found." << endl;
					usage ();
					}
				if (! image_area[2] ||
					! image_area[3])
					{
					cout << "!!! " << arguments[--count] << " " << values
							<< " selects an empty image area." << endl;
					exit (NO_IMAGE_DATA);
					}
				break;

			case 'I':	//	Input pathname.
			case 'J':	//	JP2 pathname.
				if (++count == argument_count ||
					arguments[count][0] == '-')
					{
					cout << "Missing JP2 input file pathname." << endl
						 << endl;
					usage ();
					}
				JP2_Pathname_Argument:
				if (! JP2_source.empty ())
					{
					cout << "Only one JP2 source, please." << endl
						 << "   First filename: " << JP2_source << endl
						 << "  Second filename: " << arguments[count] << endl
						 << endl;
					usage ();
					}
				  JP2_source = arguments[count];
				break;

			case 'S':	//	Swap bytes.
				swap_bytes = true;
				break;

			case 'N':	//	No_Swap_bytes.
				swap_bytes = false;
				break;

			case 'P':	//	Prompting.
				prompt = true;
				break;

			case 'C':	//	Clone test.
				clone = true;
				break;

			case 'D':	//	Description only.
				description_only = true;
				break;

			case 'H':	//	Help.
				usage (SUCCESS, true);
				break;

			default:
				cout << "Unrecognized argument: "  << arguments[count] << endl
					 << endl;
				usage ();
			}
		}
	else
		goto JP2_Pathname_Argument;
	 }

if (JP2_source.empty ())
	{
	cout << "Missing JP2 input file pathname." << endl
		 << endl;
	usage (NO_INPUT_FILE);
    }

char
	input[4];
if (prompt)
	{
	cout << "Ready> ";
	cin.getline (input, 2);
	}

//	Host system.
cout
	<< endl
	<< setw (LABEL_WIDTH) << "Processing host: "
		<< hostname () << endl
	<< setw (LABEL_WIDTH) << "CPU threads: "
		<< kdu_core::kdu_get_num_processors () << endl;

//	Set the JPIP cache directory.
JP2_Reader::default_jpip_cache_directory
	(home_directory_pathname () + FILE_PATHNAME_DELIMITER + ".JP2-test_cache");
cout << setw (LABEL_WIDTH) << "JPIP cache: "
		<< JP2_Reader::default_jpip_cache_directory () << endl;

JP2_Reader
	*JP2_reader[2];
JP2_reader[0] =
JP2_reader[1] = NULL;
int
	reader = 0;
unsigned int
	image_width,
	image_height;
ostringstream
	ratio;
unsigned char
	*producer_ID;

PIRL::Rectangle
	region,
	rendered_region;
Cube
	rendered;
string
	error_report;
int
	exit_status = SUCCESS;
try
{
/*------------------------------------------------------------------------------
	JP2 source
*/
cout
	<< endl
	<< setw (LABEL_WIDTH) << "JP2 source -" << endl
	<< setw (LABEL_WIDTH) << "Source name: " << JP2_source << endl;
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
clock_t
	begin_clock = clock (),
	start_clock = begin_clock,
	end_clock;
struct timeval
	time_a,
	time_b,
	begin_time,
	*start_time = &begin_time,
	*end_time = &time_a;
gettimeofday (&begin_time, 0);
#endif	//	!_WIN32

//	Open the JP2 Reader.
JP2_reader[reader] = JP2::reader (JP2_source);

//	Report the metadata.
cout << "Metadata -" << endl
	 << *JP2_reader[reader]->metadata_parameters () << endl
	 << "JP2 validity -" << endl
	 << JP2_reader[reader]->validity_report () << endl;


//	Configure the JP2 Reader.
if (! image_area[0] &&
	! image_area[1] &&
	! image_area[2] &&
	! image_area[3])
	{
	image_area[2] = JP2_reader[reader]->image_width ();
	image_area[3] = JP2_reader[reader]->image_height ();
	}
region
	.position (image_area[0], image_area[1])
	.size     (image_area[2], image_area[3]);

//	Begin render operations.
Render:
JP2_reader[reader]
	->resolution_and_region (resolution_level, region);

#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "JP2_Reader connect and configure time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC) << " seconds."
			<< flush << endl;
start_clock = end_clock;
	clog << "                             duration = "
			<< duration (start_time, end_time) << flush << endl;
start_time = end_time;
end_time = &time_b;
#endif	//	!_WIN32

if ((producer_ID = JP2_reader[reader]->producer_UUID ()))
	{
	cout << setw (LABEL_WIDTH) << "Producer ID: " << hex << setfill ('0');
	for (index = 0;
		 index < (unsigned int)JP2_Reader::UUID_SIZE;
		 index++)
		{
		if (index)
			cout << ", ";
		cout << "0x" << setw (2) << ((int)producer_ID[index] & 0xFF);
		}
	cout << setfill (' ') << dec << endl;
	}

image_width  = JP2_reader[reader]->image_width (),
image_height = JP2_reader[reader]->image_height ();

cout
	<< setw (LABEL_WIDTH) << "Bytes per pixel: "
		<< setw (VALUE_WIDTH)
		<< JP2_reader[reader]->pixel_bytes () << endl
	<< setw (LABEL_WIDTH) << "Bits per pixel: "
		<< setw (VALUE_WIDTH)
		<< JP2_reader[reader]->pixel_precision () << endl
	<< setw (LABEL_WIDTH) << "Pixels per line: "
		<< setw (VALUE_WIDTH)
		<< image_width << endl
	<< setw (LABEL_WIDTH) << "Lines per band: "
		<< setw (VALUE_WIDTH)
		<< image_height << endl
	<< setw (LABEL_WIDTH) << "Bands per image: "
		<< setw (VALUE_WIDTH)
		<< JP2_reader[reader]->image_bands () << endl
	<< setw (LABEL_WIDTH) << "Signed data: "
		<< setw (VALUE_WIDTH) << boolalpha
		<< JP2_reader[reader]->pixel_signed () << endl
	<< setw (LABEL_WIDTH) << "Resolution levels: "
		<< setw (VALUE_WIDTH)
		<< JP2_reader[reader]->resolution_levels () << endl;

ratio.str ("");
ratio << "1:" << (1 << (JP2_reader[reader]->resolution_level () - 1));
cout
	<< setw (LABEL_WIDTH) << "Resolution: "
		<< setw (VALUE_WIDTH) << ratio.str ()
		<< " level " << JP2_reader[reader]->resolution_level () << endl;

region =
	JP2_reader[reader]->image_region ();

if (image_area[2])
	{
	//	Image area selected.
	if (! region.area ())
		{
		cout
			<< setw (LABEL_WIDTH) << "!!! "
				<< "The selected image area is outside of the image -" << endl
			<< setw (LABEL_WIDTH) << "Selected image area: "
			 	<< image_area[0] << "x, "
				<< image_area[1] << "y, "
				<< image_area[2] << "w, "
				<< image_area[3] << "h" << endl
			<< setw (LABEL_WIDTH) << "Source image size: "
			 	<< image_width << "w, "
				<< image_height << "h" << endl;
		}
	if (image_area[0] != region.X ||
		image_area[1] != region.Y ||
		image_area[2] != (int)region.Width ||
		image_area[3] != (int)region.Height)
		cout
			<< setw (LABEL_WIDTH) << "*** Note: "
				<< "The selected image area - " << PIRL::Rectangle
					(image_area[0], image_area[1], image_area[2], image_area[3])
				<< " - has been adjusted." << endl;
	}

rendered_region = JP2_reader[reader]->rendered_region ();
cout
	<< setw (LABEL_WIDTH) << "Source image area: "
		<< setw (VALUE_WIDTH - 1)
		<< region.X << "x, " << region.Y << "y" << endl
	<< setw (LABEL_WIDTH) << ""
		<< setw (VALUE_WIDTH - 1)
		<< region.Width << "w, " << region.Height << "h" << endl
	<< setw (LABEL_WIDTH) << "Rendered image size: "
		<< setw (VALUE_WIDTH - 1)
		<< rendered_region.Width << "w, "
		<< rendered_region.Height << "h" << endl;


if (JP2_reader[reader]->rendered_pixel_bytes () > 1)
	{
	//	Inform the JP2 Reader of the byte ordering.
	JP2_reader[reader]->swap_pixel_bytes (swap_bytes);

	string
		order (high_endian_host () ? "MSB" : "LSB");
	cout
		<< setw (LABEL_WIDTH) << "Host data order: "
			<< setw (VALUE_WIDTH) << order << endl
		<< setw (LABEL_WIDTH) << "Swap pixel bytes: "
			<< setw (VALUE_WIDTH) << boolalpha << swap_bytes << endl;
	}

if (sizeof (std::streamoff) < sizeof (unsigned long long))
	{
	//	Confirm that the size of the output file can be handled by this system.
	unsigned long long
		output_size =
			((unsigned long long)image_width * image_height
				* JP2_reader[reader]->image_bands ()
				* JP2_reader[reader]->rendered_pixel_bytes ()),
		file_size = (std::streamoff)output_size;
	if (file_size != output_size)
		{
		cout
			<< endl
			<< setw (LABEL_WIDTH) << "!!! "
				<< "The host system is unable to write the expected "
					<< output_size << " byte output file size." << endl;
		exit (IO_FAILURE);
		}
	}
cout << endl;

#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "Data reporting time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC) << " seconds."
			<< flush << endl;
start_clock = end_clock;
clog << "           duration = "
		<< duration (start_time, end_time) << flush << endl;
start_time = end_time;
end_time = &time_a;
#endif	//	!_WIN32

if (description_only)
	exit (SUCCESS);

//..............................................................................
if (prompt)
	{
	cout << "Render> ";
	cin.getline (input, 2);
	}
cout << "Rendering ..." << endl;
JP2_reader[reader]
	->rendering_monitor (new rendering_monitor ());
cout << "*** test_JP2_Reader Rendering_Monitor @ "
		<< (void*)JP2_reader[reader]->rendering_monitor () << endl;

//	Read the JP2 source.
rendered = JP2_reader[reader]->render ();
cout << "Image area rendered = " << rendered << endl;

#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "Data rendering time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC) << " seconds."
			<< flush << endl;
start_clock = end_clock;
clog << "           duration = "
		<< duration (start_time, end_time) << flush << endl;
start_time = end_time;
end_time = &time_b;
#endif	//	!_WIN32

if (clone &&
	reader == 0)
	{
	//	JP2_Reader copy test:
	cout
		<< endl <<
"=============================================================================="
		<< endl
		<< "JP2_reader->clone" << endl
		<< endl;

	JP2_reader[1] = JP2_reader[0]->clone ();

	cout
		<< endl
		<< "Cloning complete" << endl <<
"=============================================================================="
		<< endl;

	if (JP2_reader[1]->is_open ())
		{
		reader = 1;
		goto Render;
		}
	else
		cout << "The cloned reader is not open." << endl;
	}
else
	{
	//	Delete test.
	cout
		<< endl <<
"------------------------------------------------------------------------------"
		<< endl
		<< "delete JP2_reader[0]" << endl
		<< endl;
	delete JP2_reader[0];
	JP2_reader[0] = NULL;

	if (clone)
		{
		cout
			<< endl <<
"------------------------------------------------------------------------------"
			<< endl
			<< "delete JP2_reader[1]" << endl
			<< endl;
		delete JP2_reader[1];
		JP2_reader[1] = NULL;
		}
	}
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "JP2_Reader delete time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC) << " seconds."
			<< endl
		 << "              duration = "
		 	<< duration (start_time, end_time) << endl
		 << " Total processing time = "
			<< (double (end_clock - begin_clock) / CLOCKS_PER_SEC) << " seconds."
			<< endl
		 << "              duration = "
		 	<< duration (&begin_time, end_time) << flush << endl;
#endif	//	!_WIN32
}
//------------------------------------------------------------------------------
catch (JP2_Exception& except)
	{
	error_report += string ("JP2 Reader error: ") + except.message ();
	exit_status = READER_ERROR;
	}
catch (invalid_argument& except)
	{
	error_report += string ("Invalid argument: ") + except.what ();
	exit_status = INVALID_ARGUMENT;
	}
catch (logic_error& except)
	{
	error_report += string ("Logic error: ") + except.what ();
	exit_status = LOGIC_ERROR;
	}
catch (std::ios::failure& except)
	{
	error_report += string ("I/O failure: ") + except.what ();
	exit_status = IO_FAILURE;
	}
catch (exception& except)
	{
	error_report += string ("Exception: ") + except.what ();
	exit_status = UNKNOWN_ERROR;
	}
catch (...)
	{
	error_report += string ("Unknown exception!");
	exit_status = UNKNOWN_ERROR;
	}

if (exit_status != SUCCESS)
	cout << "!!! " << error_report << endl;

if (JP2_reader[0])
	{
	cout << endl
		 << "close JP2_reader[0]" << endl;
	JP2_reader[0]->close (true);
	}
if (JP2_reader[1])
	{
	cout << endl
		 << "close JP2_reader[1]" << endl;
	JP2_reader[1]->close (true);
	}

exit (exit_status);
}
