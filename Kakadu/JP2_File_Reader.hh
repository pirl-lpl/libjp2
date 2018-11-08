/*	JP2_File_Reader

HiROC CVS ID: $Id: JP2_File_Reader.hh,v 1.30 2012/04/26 02:55:48 castalia Exp $

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

#ifndef _JP2_File_Reader_
#define _JP2_File_Reader_

#include	"JP2_Reader.hh"

//	PIRL++
#include	"Dimensions.hh"
#include	"Reference_Counted_Pointer.hh"

//	Kakadu
#include	"jp2.h"
#include	"kdu_region_decompressor.h"

#include	<string>
#include	<sstream>
#include	<iomanip>


namespace UA
{
namespace HiRISE
{
namespace Kakadu
{
//	Forward references.
class JP2_Box;

/**	A <i>JP2_File_Reader</i> reads image pixel data from a JPEG2000 JP2
	standard format image file.

	This implementation employs the
	<a href="http://www.kakadusoftware.com/" target="_top">Kakadu
	Software</a> libraries.


	Programmer notes on the use of the Kakadu software:

	There are two possible sources of a JP2 file: A pathname for a local
	file, or a URL for a remote file provided by a JPIP server.

	If the source is a URL for a JPIP server (identified by a jpip:// or
	http:// protocol specifier; a file:// protocol is reverted to a
	pathname for a local file) a kdu_client is instantiated and its
	connect method is used to gain access to the remote file (a.k.a the
	resource). It is possible for the kdu_client to provide multiple
	channels for "compatible" URLs, but this capability has not yet been
	explored here. The kdu_client is a subclass of the kdu_cache which
	manages the data bins delivered by the JPIP server. The kdu_cache is
	a subclass of the kdu_compressed_source which is an abstract class
	providing access specific types of compressed sources.

	If the source is a pathname for a local file a jp2_family_src is
	instantiated and its open method is used to gain direct access to the
	local JP2 file. A kdu_cache can also be used with the open method,
	thus providing a point of commonality for handling either a local or
	remote source.

	The jp2_family_src object (named JP2_Stream here) may be used with
	the open method of a jp2_input_box to confirm that the first box
	contains the required JP2 signature. Then the box object may be used
	to step through all the boxes in the source using the open_next
	method.

	The jp2_family_src object is used to open a jp2_source object (named
	JP2_Source here) which provides full support for interacting with JP2
	files. The jp2_source class is a subclass of jp2_input_box, so the
	JP2_Source may be used to scan all the JP2 boxes instead of using the
	JP2_Stream. The jp2_input_box class is a subclass of the
	kdu_compressed_source class. This would seem to suggest that a
	jp2_input_box opened on a jp2_family_src would be sufficient to
	provide the required kdu_compressed_source interface; however using
	the jp2_source appears to be the recommended approach from the Kakadu
	demo applications. Note that to open a jp2_source object a
	jp2_family_src (or jp2_input_box) object is required.

	The kdu_compressed_source is a key base class. It is required to open
	an input kdu_codestream which constructs the codestream management
	machinery used to render image data.

	There are various image data decompression classes, all of which
	require a kdu_compressed_source: The kdu_stripe_decompressor renders
	image stripes each of which is a contiguous sequence of image lines
	across the full width of the selected image region for all image
	components (bands). The kdu_region_deompressor allows selective
	regions and components of the image data to be rendered. The
	kdu_region_compositor is intended for very flexible rendering for
	image display purposes which limits the rendering to 8-bit per pixel
	RGB plus alpha image data.

	The hierarchy of object creation is:
<dl>
<dt>kdu_client.connect (URL)
<dd>The JPIP_Client for a given source URL is constructed when a
	JP2_JPIP_Reader object is {@link JP2_JPIP_Reader::open(const
	std::string&) opened} for the first time. It is shared amongst
	copies of the JP2_JPIP_Reader object, with each copy obtaining
	a separate connection to the source via the JPIP_Client.

	A Data_Bin_Cache (kdu_cache) is object is attached to the
	JPIP_Client and acts an intermediary between the asynchronous
	network transfer mechanism of JP2 metadata and codestream packets
	from the JPIP server to the JPIP_Client, which deposits the data
	received in the Data_Bin_Cache, and the consumers of the data, which
	extracts data from the Data_Bin_Cache to obtain JP2 metadata and
	codestream packets for decompression and rendering. Each rendering
	engine has its own Data_Bin_Cache object, however each is attached
	to the same underlying JPIP_Client (itself a subclass of kdu_cache)
	where the incoming data is actually held.
	<dl>
	<dt>jp2_family_src.open (pathname or kdu_client)
	<dd>The JP2_Stream object (a jp2_threadsafe_family_src) is opened
		directly on a file containing JP2 data, or on Data_Bin_Cache that
		indirectly provides JP2 data. It manages access to the data from
		the source, whether a file or the data cache of a JPIP client,
		with a consistent interface. Each JP2_File_Reader has its own
		JP2_Stream object.
		<dl>
		<dt>jp2_source.open (jp2_family_src)
		<dd>The JP2_Source is opened on the JP2_Stream object. It is used
			to ensure that the JP2 metadata headers are all available, up to
			and including the main codestream header. At this point the
			JP2_Source can be used to create the codestream decompression
			and rendering machinery. Each JP2_File_Reader has its own
			JP2_Source object.
			<dl>
			<dt>kdu_codestream.create (jp2_source)
			<dd>The JPEG2000_Codestream, created using a JP2_Source
				object and an optional Thread_Group (kdu_thread_env),
				provides the codestream management machinery. Each
				JP2_File_Reader has its own JPEG2000_Codestream object.
				<dl>
				<dt>kdu_region_decompressor.start (kdu_codestream)
				<dd>The Decompressor is the rendering engine that
					gnerates image pixel data from a
					JPEG2000_Codestream using rendering control parameters
					for the
					rendering operation, and the optional Thread_Group
					for multi-threaded processing.	This object is
					resuable.
				</dl>
			</dl>
		</dl>
	</dl>
</dl>

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.30 $
*/
class JP2_File_Reader
:	public UA::HiRISE::JP2_Reader
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Decompression engine error exception signal value.
static const kdu_core::kdu_exception
	READER_ERROR;

/*==============================================================================
	Constructors
*/
/**	Construct a JP2_File_Reader.

	The reader must be {@link open(const std::string&) opened} with a
	source.

	@see	JP2_File_Reader(const std::string&)
*/
JP2_File_Reader ();

/**	Construct a JP2_File_Reader for a source JP2 file.

	<b>N.B.</b>: If the source can not be {@link is_open() successfully
	opened} the JP2_Reader will not be fully functional.

	@param	source	The pathname to a source file.
	@see	open(const std::string&)
*/
explicit JP2_File_Reader (const std::string& source);

/**	Construct a copy of a JP2_File_Reader.

	@param	JP2_file_reader	The JP2_File_Reader to be copied.
*/
JP2_File_Reader (const JP2_File_Reader& JP2_file_reader);

virtual ~JP2_File_Reader () throw();

/*==============================================================================
	Reader
*/
/**	Clone this JP2_File_Reader.

	A copy of this JP2_File_Reader is constructed.

	@return	A pointer to a copy of this JP2_File_Reader.
*/
virtual JP2_File_Reader* clone () const;

/**	Open the JP2_File_Reader on a source file.

	If the reader is not already open it is opened on the source file.

	@param	source	The pathname to a source file.
	@return	The ID of the opened source. This will be zero if the reader
		is already open; otherwise it will be one.
*/
virtual int open (const std::string& source);

/**	Test if the reader is open.

	The reader is open if all three of its data stream management
	components - the JP2_Stream, JP2_Source and JPEG2000_Codestream -
	exist; i.e. the reader components have been opened/created but not
	closed/destroyed.

	@return	true if the JP2_File_Reader has been successfully opened on
		the JP2 source and not yet closed; false otherwise.
*/
virtual bool is_open () const;

/**	Set the effective rendering resolution level and image region.

	The {@link resolution_level(unsigned int) resolution level} and
	{@link image_region(const Rectangle&) image region} interact to
	determine the {@link rendered_region() rendered image size} so they
	are set together.

	<b.N.B.</b>: If the reader has not been {@link open() opened} nothing
	is done.

	The effective resolution level is limited to the range 1 - {@link
	resolution_levels() resolution levels} and applied as an input
	restriction on the codestream rendering machinery.

	If the image region is empty the entire {@link image_size() image
	size} is used as the effective {@link image_region() image region}.
	Otherwise the dimensions of the image region are adjusted by the
	resolution level (divided by 2**(level - 1)), intersected with the
	image size at the rendering resolution and the resulting effective
	image region is applied as an input restriction on the codestream
	rendering machinery. Both the effective image region and {@link
	rendered_region() rendered image region} are set. <b>N.B.</b>: If the
	selected image region does not intersect with the image size an empty
	effective image region will result.

	@param	resolution	The rendering resolution level.
	@param	region	The selected image area to be rendered relative to
		the full resolution image. This will be clipped to the full
		image size. <b>N.B.</b>: If the region is empty the entire image
		will be selected.
	@return	true if there was any change to the resolution or region;
		false otherwise. <b.N.B.</b>: If the data source is not open
		false is returned immediately.
	@throws	JP2_Exception	If a kdu_exception occured.
*/
virtual bool resolution_and_region
	(unsigned int resolution, const PIRL::Rectangle& region);

/**	Render the image data.

	The JP2 source is {@link open() opened} if this has not yet
	been done, and the reader is checked that it is {@link ready()
	ready}. Data buffers to hold horizontal stripes of rendered image
	data are allocated.

	The destination file is opened for writing. <b>N.B.</b>: If the file
	already exists the image data will be appended to the current
	content.

	@return	A Cube indicated what was rendered.
	@throws	JP2_Logic_Error	If the reader is not ready().
	@throws	runtime_error	If insufficient memory is available to
		allocate the rendered image data buffers.
*/
virtual Cube render ();

/**	Close access to the JP2 source.

	The JP2 source stream is closed and the rendering machinery resources
	associated with it are released. <b>N.B.</b>: The JP2 metadata
	describing the source and the reader's rendering configuration
	remain unchanged, so the source may be {@link open(const std::string&)
	opened} again for rendering using the previously set configuration.

	@param	force	Not used when closing a JP2_File_Reader.
	@see	reset()
*/
virtual void close (bool force = false);

/**	Reset the reader to it's initial, default, state.

	The reader is {@link close(bool) closed}, any {@link
	deploy_processing_threads() processing threads} are released, and the
	rendering configuration and JP2 metadata are {@link
	JP2_Reader::reset() reset.
*/
virtual void reset ();

/**	Get the next available Kakadu reader error message.

	An error message will be pending delivery if this JP2_File_Reader
	throws a kdu_exception with a value of {@link #READER_ERROR). The
	expected response when this exception is caught is to use this method
	to obtain all available error messages from the Kakadu engine that
	threw the exception.

	The returned message string will contain at least a line identifying
	the exception value. All entries in the Kakadu error message queue,
	if any, are removed from the queue and appended to the message string
	before it is returned.

	@return	A string containing an error message.
*/
std::string Kakadu_error_message (const kdu_core::kdu_exception& except);


protected:

//!	Conceptual {@link data_request(Rectangle*, bool)} return values.
enum
	{
	DATA_REQUEST_SATISFIED	= -1,
	DATA_REQUEST_SUBMITTED	= 1,
	DATA_REQUEST_REJECTED	= 0
	};

/**	Provide a desription for a DATA_REQUEST_XXX return status value.

	@param	status	A {@link data_request(Rectangle*, bool)} return value.
	@return	A string describing the status value.
*/
static std::string data_request_description (int status);

/**	Initiate a codestream data request for the contents of the region
	to be rendered.

	This method is a no-op for a JP2_File_Reader. It is provided for
	interface compatibility with a JP2_JPIP_Reader.

	@param	region		Ignored.
	@param	preemptive	Ignored.
	@return	Always returns {@link #REQUEST_SATISFIED} because the data is
		available.
*/
virtual int data_request (Rectangle* region = NULL, bool preemptive = true);

virtual int metadata_request (kdu_core::kdu_int32 box_type, bool preemptive = false);

/**	<i>Acquired_Data</i> describes the data that has been
	{@link data_aquisition(Acquired_Data*) acquired} from the last
	{@link data_request(Rectangle*, bool) data request}.

	The base Rectangle specifies the region of data that was acquired
	relative to the rendered resolution grid. The Resolution_Level member
	specifies the resolution level at which the data was acquired.
*/
struct Acquired_Data
:	public PIRL::Rectangle
{
//!	The resolution level at which the data was acquired.
int
	Resolution_Level;

inline void acquired (unsigned int resolution_level,
	unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
Resolution_Level = resolution_level;
position (x, y);
size (width, height);
}
};	//	Class Acquired_Data

//!	Bit flags for a {@link data_acquisition(Acquired_Data*)} return values.
enum
	{
	DATA_ACQUISITION_COMPLETE	= (1 << 0),
	DATA_ACQUISITION_INCOMPLETE	= (1 << 1),
	DATA_ACQUISITION_REDUCED	= (1 << 2),
	DATA_ACQUISITION_CANCELED	= (1 << 3)
	};

/**	Provide a desription for a DATA_ACQUISITION_XXX return status value.

	@param	status	A {@link data_acquisition(Acquired_Data*)} return value.
	@return	A string describing the status value.
*/
static std::string data_acquisition_description (int status);


/**	Wait for an outstanding {@link data_request(Rectangle*, bool) data
	request} to complete.

	This method is a no-op (other than filling in the acquired data
	structure) for a JP2_File_Reader. It is provided for interface
	compatibility with a JP2_JPIP_Reader.

	@param	acquired_data	If non-null the Acquired_Data members will
		be filled with the description of the data that was acquired.
	@return	Always returns DATA_ACQUISITION_COMPLETE.
	@see	JP2_JPIP_Reader::data_acquistion(Acquired_Data*)
*/
virtual int data_acquisition (Acquired_Data* acquired_data = NULL);

/*============================================================================
	Helpers
*/
protected:

/**	Open and validate the JP2 source data stream.

	If the reader {@link is_open() is open} nothing is done.

	<b>N.B.</b>: The JP2_Stream object must have already been opened by
	the open method of the reader implementation; otherwise a
	JP2_Logic_Error exception will be thrown.

	If the JPEG2000_Codestream has been created (during a previous source
	opening procedure) it is destroyed. Likewise, if the JP2_Source has
	been opened it is closed.

	The JP2_Source is then opened on the JP2_Stream and the source
	headers are read. Failure to open the JP2_Source or read the headers
	- for a cached data source (i.e. a JPIP client for a JPIP server) up
	to ten attempts to read the headers, at one second intervals, are
	attempted - will cause a JP2_IO_Failure exception to be thrown.

	The first two header boxes in the source data stream must be JP2
	Signature followed by a File Type. If these boxes are not found a
	JP2_Invalid_Argument exception is thrown.

	The JPEG2000_Codestream is created using the JP2_Source and any
	{@link deploy_processing_threads() data processing threads} deployed,
	and then set to be persistent for interactive (re)use. The
	JPEG2000_Codestream is then used to obtain the source image data
	characterization including the {@link JP2_Metadata::image_size()
	image size} and number of {@link JP2_Metadata::image_bands() image
	bands}, which are used to initialize the {@link
	JP2_Reader::image_region() image region} and {@link
	JP2_Reader::rendered_region() rendered region}; {@link
	JP2_Metadata::pixel_bits() pixel bits}, which are used to initialize
	the {@link rendered_pixel_bits() rendered pixel bits} if not already
	set; {@link JP2_Metadata::signed_data() signed data} condition;
	number of {@link resolution_levels() resolution levels} in the
	JPEG2000 codestream; {@link JP2_Metadata::producer_UUID() data
	producer UUID} and associated {@link JP2_Metadata::URL() URL} if
	present. If more than one image band is available each is checked to
	confirm that they all have the same dimensions, pixel bits and signed
	data condition; if differening data bands are present a
	JP2_Out_of_Range exception is thrown describing the problem.

	Finally, the basic image rendering configuration is initialized which
	prepares the reader to {@link render() render} the JPEG2000
	codestream to image pixels.
*/
void open_source ();

/**	Initialize the JP2 metadata and rendering configuration.

	The JPEG2000_Codestream is used to obtain the source image data
	characterization including the {@link JP2_Metadata::image_size()
	image size} and number of {@link JP2_Metadata::image_bands() image
	bands}, which are used to initialize the {@link
	JP2_Reader::image_region() image region} and {@link
	JP2_Reader::rendered_region() rendered region}; {@link
	JP2_Metadata::pixel_bits() pixel bits}, which are used to initialize
	the {@link rendered_pixel_bits() rendered pixel bits} if not already
	set; {@link JP2_Metadata::signed_data() signed data} condition;
	number of {@link resolution_levels() resolution levels} in the
	JPEG2000 codestream; {@link JP2_Metadata::producer_UUID() data
	producer UUID} and associated {@link JP2_Metadata::URL() URL} if
	present. If more than one image band is available each is checked to
	confirm that they all have the same dimensions, pixel bits and signed
	data condition; if differening data bands are present a
	JP2_Out_of_Range exception is thrown describing the problem.

	At the same time that the JP2 metadata is initialzed the image
	rendering configuration is initialized from the JP2 metadata. This
	prepares the reader to {@link render() render} the JPEG2000
	codestream to image pixels.

	@throws	JP2_Out_of_Range	If the image data bands do not all have
		the same dimensions.
*/
void initialize ();

/**	The JP2_Metadata is initialized.

	All JP2 boxes up to, but not including, the first codestream box are
	{@link load_box_content(JP2_Box&, unsigned char*) loaded} and each
	{@link JP2_Metadata::add_JP2_box (Type_Code, int, unsigned char*,
	long, long) box is added to the JP2_Metadata}. When the first
	codestream box is encountered its main header {@link
	ingest_codestream_segments(JP2_Box&) segments are added to the
	JP2_Metadata}.

	<b>N.B.</b>: After ingesting all the metadata its validity can be
	tested for {@link JP2_Metadata::is_complete() completeness} to ensure
	that all required information has been obtained from the data source.

	@return	true if all metadata was successfully ingested; false if
		there was any problem loading JP2 box content.
	@throws	JP2_Logic_Error	If the metadata structure was found to be
		invalid for any reason.
	@throws	invalid_argument	If an invalid box value was encountered,
		probably because of data stream corruption.
*/
bool ingest_metadata ();

/**	Load the content of a JP2 box.

	The entire box content, starting with the first byte following the
	box header sequence and including any sub-boxes, is to be read into
	the content buffer. <b>N.B.</b>: The buffer must be large enough to
	hold the entire box content length, which can be determined from the
	get_remaining_bytes method. If the box has no content or has an
	indefinate length (get_remaining_bytes returns -1) that extends to
	the end of the source file then nothing will be done.

	@param	box	A JP2_Box reference. The box must be open and no content
		read (the read pointer positioned immediately following the
		header sequence).
	@param	content	A pointer to a buffer to receive the box content.
		<b>N.B.</b>: The buffer must be large enough to hold the
		entire box content, not including the header sequence.
	@return	true if the box content was loaded into the content buffer;
		false if any problem occured.
*/
virtual bool load_box_content (JP2_Box& box, unsigned char* content);

/**	Codestream segments are added to the JP2_Metadata.

	The codestream main header segments up to, but not including, the
	first start-of-tile (SOT) segment are {@link
	JP2_Metadata::add_codestream_segment(Marker_Code, const unsigned
	char*, int, long) added to the JP2_Metadata}.

	<b>N.B.</b>: The codestream main header segments are presumed to
	be read accessible from the codestream box.

	@param	codestream_box	A reference to a JP2_Box for the image
		JPEG2000 codestream. The box must be open and no content read
		(the read pointer positioned immediately following the header
		sequence).
	@throws	JP2_Logic_Error	If the codestream structure was found to be
		invalid for any reason.
	@throws	invalid_argument	If an invalid segment value was
		encountered, probably because of data stream corruption.
*/
void ingest_codestream_segments (JP2_Box& codestream_box);

/**	Deploy JP2 codestream processing threads.

	If the Thread_Group for this reader has already been created or the
	number of {@link JP2_Reader::processing_threads() processing threads}
	to be used is less than two, nothing is done.

	A new Thread_Group is created and threads are added to it until the
	desired number of processing threads are present or the system
	refuses to provide more threads (in which case the number of reported
	processing threads is reduced accordingly).
*/
virtual void deploy_processing_threads ();

/*==============================================================================
	Data
*/
protected:
//------------------------------------------------------------------------------
//	Kakadu classes used by both File and JPIP readers.

//!	JP2 file input stream.
mutable kdu_supp::jp2_threadsafe_family_src
	JP2_Stream;

//!	JP2 content source.
mutable kdu_supp::jp2_source
	JP2_Source;

//!	JPEG2000 codestream.
mutable kdu_core::kdu_codestream
	JPEG2000_Codestream;

//!	Reusable codestream decompression machinery.
mutable kdu_supp::kdu_region_decompressor
	Decompressor;

//	N.B.: Declared in kdu_region_decompressor.h
   kdu_supp::kdu_channel_mapping
	Channel_Mapping;
   kdu_core::kdu_coords
	Expand_Numerator,
	Expand_Denominator;

/*	Decompression processing threads.

	The Thread_Group is used by the rendering engine.
*/
kdu_core::kdu_thread_env
	*Thread_Group;

/*	Kakadu error message queue.

	Note: The message queue object is thread safe.
*/
   kdu_core::kdu_message_queue
	Error_Message_Queue;
};	//	Class JP2_File_Reader

}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA
#endif
