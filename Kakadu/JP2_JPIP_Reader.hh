/*	JP2_JPIP_Reader

HiROC CVS ID: $Id: JP2_JPIP_Reader.hh,v 1.37 2012/04/26 02:49:33 castalia Exp $

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

#ifndef _JP2_JPIP_Reader_
#define _JP2_JPIP_Reader_

#include	"JP2_File_Reader.hh"
#include	"Reference_Counted_Pointer.hh"

//	Kakadu
#include	"kdu_client.h"
//class kdu_window_prefs;

#include	<vector>

namespace UA
{
namespace HiRISE
{
namespace Kakadu
{
//	Forward references.
class JPIP_Client_Notifier;
class JP2_Box;

/**	A <i>JP2_JPIP_Reader</i> reads image pixel data from a JPEG2000 JP2
	image file obtained from a JPIP server.

	This implementation employs the
	<a href="http://www.kakadusoftware.com/" target="_top">Kakadu
	Software</a> libraries.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.37 $
*/
class JP2_JPIP_Reader
:	public JP2_File_Reader
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Connection ID value if no JPIP_Client server connection is available.
static const int
	NOT_CONNECTED;

//!	Rendering_Monitor INFO status messages.
static const std::string
	RECONNECTING_MESSAGE,
	WAITING_MESSAGE,
	TIMEOUT_MESSAGE,
	ACQUIRED_DATA_MESSAGE;

/*==============================================================================
	Constructors
*/
/**	Constructs a JP2_JPIP_Reader.

	The reader must be configured with at least an input JP2 {@link
	source(const std::string&) source} before it can be {@link open()
	opened}. It must also be configured with an output image data {@link
	destination(const std::string&) destination} to be {@link ready()
	ready} to do its job.
*/
JP2_JPIP_Reader ();

/**	Construct a JP2_JPIP_reader for the URL of a JPIP JP2 source.

	@param	source	A jpip or http URL for the source.
*/
explicit JP2_JPIP_Reader (const std::string& source);

/**	Construct a copy a JP2_JPIP_Reader.

	@param	JP2_JPIP_reader	The JP2_JPIP_Reader to copy.
*/
JP2_JPIP_Reader (const JP2_JPIP_Reader& JP2_JPIP_reader);

virtual ~JP2_JPIP_Reader ();

/*==============================================================================
	Reader
*/
/**	Clone the JP2_JPIP_Reader.

	A copy of this JP2_JPIP_Reader is constructed.

	@return	A pointer to a copy of this JP2_JPIP_Reader.
*/
virtual JP2_JPIP_Reader* clone () const;

/**	Open a JP2 source.

	<b>N.B.</b>: Multiple connections may be opened to the same JPIP
	source. To reopen the reader on a different source the reader must
	first be {@link close() closed}. If this reader {@link is_open() is
	open} on the specified source nothing is done.

	If the shared JPIP_Client {@link is_shutdown() is shutdown} a first
	time connection to the JPIP server is attempted. This includes
	resetting the JP2_Metadata if the specified source is different than
	the current {@link source_name() source name}, constructing
	JPIP_Client and JPIP_Client_Notifier (that receives asynchronous
	activity notifications from the client) objects that will be shared
	by all JP2_JPIP_Reader objects connecting to the same source, and
	then intiating the server connection using the {@link jpip_proxy()
	proxy hostname} and {@link jpip_cache_directory() data bin cache
	directory if they have been specified. <b>N.B.</b>: Initiating a
	server connection does not block execution until the connection has
	been completed; connection completion occurs later asychronously.

	When the shared JPIP_Client has any existing connections the
	JP2_JPIP_Reader is checked to see if it is {@link is_open() is open}
	on the specifed source. If so, the currect connection ID is simply
	returned without doing anything else. Otherwise a new data request
	queue for this JP2_JPIP_Reader is added to the shared JPIP_Client.

	The data bin cache manager for this JP2_JPIP_Reader is attached to
	the JPIP_Client, the reader's JP2 stream object is opened on the
	data bin cache manager (unless it has already been bound), and the
	reader is registered with the shared JPIP_Client_Notifier.

	A {@link data_request(Rectangle*, bool) request} for the source
	metadata is now posted to the JPIP server and execution blocks until
	the data request has been {@link data_acquisition(Acquired_Data*)
	acquired} from the server. If the data request fails a JPIP_Exception
	is thrown describing the problem.

	When the acquisition of all the top level source metadata has been
	completed from the JPIP server for the source, the source data is
	{@link JP2_File_Reader::open_source() opened and validated}.

	@param	source	A jpip or http URL string for the source.
	@return	The JPIP server connection ID. <b>N.B.</b>: An exception
		will be thrown if the connection can not be opened for any
		reason.
	@throws	JP2_Invalid_Argument	If no source has been specified or
		the source is not a JPIP compatible URL format, or is for a
		different source when the shared JPIP_Client is already connected
		to a different source.
	@throws JPIP_Exception	If the attempt to connect to the JPIP server
		or obtain the source metadata from the server failed.
	@throws	JP2_Out_of_Range	If any of the image bands (JP2 components)
		do not have identical image structure (image and pixel).
*/
virtual int open (const std::string& source);

/**	Reconnect to the JPIP server.

	If the reader {@link is_open() is open}, the number of {@link
	autoreconnect_retries() auto-reconnect retries} is zero, the
	Reconnecting flag is raised, or the {@link source_name() source
	name} is empty nothing is done.

	<b>N.B.</b>: It the responsibility of the caller to save any reader
	state - such as a failed Server_Request - before attempting a
	reconnect.

	The Reconnecting flag is raised during the reconnect attempts to
	prevent recursive reconnection attempts. It is reset before the
	method returns; it is also reset if an exception is thrown.

	An immediate {@link open(const std::string&) open} using the current
	{@link source_name() source name} is tried. If this is successful
	true is returned. If a JPIP_Disconnected exception is thrown a delay
	of {@link request_timeout() request timeout} seconds is set. If a
	JPIP_Timeout exception is thrown no delay is set. Any other exception
	is not caught. Execution waits the delay that has been set if any
	retries remain, otherwise false is returned.

	@retrun	true if the reader was reconnected to the JPIP server;
		false if all reconnect tries failed.
*/
virtual bool reconnect ();

/**	Test if the reader is open.

	The reader is open if a shared JPIP_Client has been constructed
	and this JP2_JPIP_Reader has a live connection to the JPIP server
	via the client. In addition, the base JP2_File_Reader must also
	be {@link JP2_File_Reader::is_open() open}.

	<b>N.B.</b>: If this JP2_JPIP_Reader is not open the JPIP_Client may
	still have open connections to the JPIP server source in other
	readers that are sharing the client.

	@return	true if this JP2_JPIP_Reader has been successfully {@link
		open(const std::string&) opened} and not yet closed; false
		otherwise.
	@see	close(bool)
*/
virtual bool is_open () const;

/**	Tests if the reader has the information it needs to render.

	In addition to meeting the rendering requirements of the base
	class, the JPIP connection must be alive.

	@param	report	A pointer to a string to which a report will be
		appended (if non-NULL) describing the reason for a false
		condition. No report will be provided if the condition is true. A
		report may contain several lines.
	@return	true if the reader is ready; false otherwise.
	@see	render()
*/
virtual bool ready (std::string* report = NULL) const;

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
	@throws JPIP_Exception	If there was an error when the decompression
		machinery was told to finish. In this case the reader is closed.
	@throws	JP2_Exception	If a kdu_exception occured.
*/
virtual bool resolution_and_region
	(unsigned int level, const Rectangle& region);

/**	Render the image data.

	Checks that the reader (@link is_open() is open} and if not tries to
	{@link reconnect() reconnect} to the JPIP server.

	The actual work is done by the base class {@link
	JP2_File_Reader::render() renderer}.

	@return	A Cube indicating what was rendered.
	@throws	JP2_Logic_Error	If the reader is not ready().
	@throws	runtime_error	If insufficient memory is available to
		allocate the rendered image data buffers.
*/
virtual Cube render ();

/**	Get the JPIP server connection status description.

	@return	A string describing the current JPIP connection status.
*/
virtual std::string connection_status () const;

/**	Close access to the JP2 source.

	This JP2_JPIP_Reader is deregistered from the JPIP_Client_Notifier.

	The base JP2_File_Reader is {@link JP2_File_Reader::close(bool)
	closed}.

	If this reader has a connection to the JPIP server via the shared
	JPIP client, this is disconnected. If force is false, the JPIP client
	will be asked to try and keep the underlying communications transport
	alive and the method will wait until the disconnection has completed
	or the {@link request_timeout() request timeout} has elapsed;
	otherwise the method will not wait for completion and the underlying
	communications transport, if no longer in use, will be closed.

	<b>N.B.</b>: The reader is not {@link reset() reset}.

	@param	force	Force closing to proceed without delay.
	@see	reset()
	@see	shutdown ()
*/
virtual void close (bool force = false);

/**	Reset the reader to it's initial, default, state.

	The
	The reader is {@link close(bool) closed}, any {@link
	deploy_processing_threads() processing threads} are released, and the
	rendering configuration and JP2 metadata are {@link
	JP2_Reader::reset() reset.
*/
virtual void reset ();

/**	The JPIP client is shutdown.

	This reader is {@link reset() reset}.

	All connections that are shared with the shared JPIP_Client are
	disconnected allowing, at most, the {@link request_timeout() request
	timeout} to elapse for completion (or the disconnections will be
	forced). The JPIP_Client is then completely closed which ensures that
	all communication with the server has been closed and its data bin
	cache has been emptied. The Connection_ID is set as {@link
	#NOT_CONNECTED}.

	<b>N.B.</b>: Any JP2_JPIP_Reader objects sharing the JPIP client will
	receive the disconnect notice from the client via the shared
	JPIP_Client_Notifier.

	@param	wait_for_completion	if true wait until the JPIP client
		completes all shared server connections or the
		request timeout expires; if false, don't wait for completion,
		but the disconnects will proceed and the timeout limit
		will be applied to forced disconnection.
*/
void shutdown (bool wait_for_completion = true);

/**	Test if the shared JPIP_Client has been shutdown.

	The JPIP_Client used by the reader may be shared by multiple
	connections to the same JPIP server source. The JPIP_Client,
	and, by extension, all JP2_JPIP_Reader objects sharing a
	connection to the JPIP server source via the client, has been
	shutdown if there are no open connections.

	@return	true if the shared JPIP_Client does not have any
		connections to the server, or the JPIP_Client has not yet
		been constructed; false otherwise.
*/
bool is_shutdown () const;

/*============================================================================
	Helpers
*/
protected:

/**	Load the content of a JP2 box.

	The entire box content, starting with the first byte following the
	box header sequence and including any sub-boxes, is to be read into
	the content buffer. <b>N.B.</b>: The buffer must be large enough to
	hold the entire box content length, which can be determined from the
	get_remaining_bytes method. If the box has no content or has an
	indefinate length (get_remaining_bytes returns -1) that extends to
	the end of the source file then nothing will be done.

	If the box content is not complete (is_complete is false) then
	an attempt will be made to {@link metadata_request(kdu_int32, bool)
	obtain the box metadata. If this succeeds the box is reopened at
	its original location. If the box content could not be obtained or
	the box could not be reopened the data load fails.

	If the box is a super-box each sub-box data is loaded. For each
	sub-box the header sequence for the sub-box is written into the
	content buffer and this method is called using the sub-box and the
	incremented content pointer. If any sub-box fails to load loading
	is aborted.

	If the box is not a super-box its data content is read into the
	content buffer.

	@param	box	A JP2_Box reference. The box must be open and no content
		read (the read pointer positioned immediate following the header
		sequence).
	@param	content	A pointer to a buffer to receive the box content.
		<b>N.B.</b>: The buffer must be large enough to hold the
		entire box content, not including the header sequence.
	@return	true if the box content was loaded into the content buffer;
		false if any problem occured.
*/
virtual bool load_box_content (JP2_Box& box, unsigned char* content);

/**	Deploy the processing threads used by the decompression machinery.
*/
virtual void deploy_processing_threads ();

/**	Initiate a codestream data request to the JPIP server for the
	contents of the region to be rendered.

	The Server_Request and Posted_Server_Request are initialzed to the
	default (null) state. The Server_Request is then set according to the
	region: If the region is NULL a metadata-only request is set. A
	non-empty region is used along with the {@link resolution_level()
	resolution level} to set the image data region request with all
	quality layers. All metadata associated with the window is also requested.
	<b>N.B.</b>: To ensure full coverage the resolution
	round direction for the request window is ROUND_UP. <b>N.B.</b>: If
	the region is empty the method returns immediately with a negative
	value and the Server_Request and Posted_Server_Request remain unset;
	no {@link data_acquisition(Acquired_Data*) data acquisition} wait for
	request completion should be done (because there will be no data
	arrival notification).

	<b>N.B.</b>: The server may return codestream data for a smaller
	region, or lower resolution (higher level), than was requested. If
	this occurs it will not be known until the response is (@link
	data_acquisition(Acquired_Data*) received from the server}.

	The Server_Request is forwarded to the alternate {@link
	data_request(kdu_window&, bool) data request} method.

	@param	region	The region, relative to the rendered resolution grid,
		to be rendered. If NULL a metadata-only request will be posted.
	@param	preemptive	If true any pending requests for the same
		connection are to be preempted.
	@return	{@link #DATA_REQUEST_SUBMITTED} if the data was queued by the
		JPIP client for delivery to the JPIP server; {@link
		#REQUEST_SATISFIED} if the request had no effect because the
		client already has the data for the request (so no {@link
		data_acquisition(Acquired_Data*) data acquisition should be done);
		{@link #DATA_REQUEST_REJECTED} if the request was rejected for
		some reason.
*/
virtual int data_request (Rectangle* region = NULL, bool preemptive = true);

/**	Post a data request to the JPIP server.

	The request is made to the JPIP client queue connection ID associated
	with this JP2_JPIP_Reader. The method returns immediately after the
	request has been issued; it does not wait for request completion.
	<b>N.B.</b>: If the data to fulfill the request is already present in
	the client data bin cache the return value will be negative; i.e. the
	data request can be satsified without requesting data from the JPIP
	server. In this case no {@link data_acquisition(Acquired_Data*) wait}
	for request completion should be done
	(because there will be no data arrival notification).

	If the request is rejected because the connection to the JPIP server
	has been lost an attempt will be made to {@link reconnect()
	reconnect} to the server. If this succeeds the request will be
	reissued.

	On return the Posted_Server_Request will be a copy of the specifed
	server request regardless of other requests issued to accomplish a
	JPIP server reconnect.

	<b>N.B.</b>: If an exception is thrown the reader will have been
	{@link close(bool) closed}.

	@param	server_request	The kdu_window request to be posted to the
		JPIP server via the JPIP_Client.
	@param	preemptive	If true any pending requests for the same
		connection are to be preempted.
	@return	A request status code:
		<ul>
		<li>{@link JP2_File_Reader::DATA_REQUEST_SATISFIED}
			The data request is satisfied by the current contents of
			the data bin cache. <b>No {@link data_acquisition(Acquired_Data*)
			data acquisition should be done.</b>
		<li>{@link JP2_File_Reader::DATA_REQUEST_SUBMITTED}
			The data request was posted to the JPIP server. Data delivery
			is pending. Data delivery notifications will be provided
			by the JPIP client; use {@link data_acquisition(Acquired_Data*)
			data acquisition} calls to wait for data arrival.
		<li>{@link JP2_File_Reader::DATA_REQUEST_REJECTED}
			A problem was detected with the request and it was rejected.
			<b>No {@link data_acquisition(Acquired_Data*)
			data acquisition should be done.</b>
		</ul>
	@throws	JPIP_Disconnected	If the request was not accepted, the
		JPIP client became disonnected from the server, and the attempt
		to reconnect failed.
	@see	data_request_description(int)
*/
int data_request (kdu_supp::kdu_window& server_request, bool preemptive = true);

virtual int metadata_request (kdu_core::kdu_int32 box_type, bool preemptive = false);

/**	Wait for an outstanding {@link data_request(Rectangle*, bool) data
	request} to complete.

	The thread of execution is blocked unless or until the JPIP client
	Notifier has posted a notify event from the JPIP client. However,
	execution will not be blocked longer than the {@link
	JPIP_request_timeout(unsigned int) request timeout}, and at one second
	intervals an {@link JP2_Reader::Rendering_Monitor::Status::INFO_ONLY}
	notfication with a {@link WAITING_MESSAGE} will be sent to the {@link
	rendering_monitor() rendering monitor}, if available, which may
	cancel waiting for data to arrive. If the JPIP server disconnects
	while waiting, waiting stops.

	When the procedure continues, if waiting was canceled a {@link
	JP2_Reader::Rendering_Monitor::Status::CANCELED} notification will be
	sent to a registered rendering monitor. Then the procedure will
	proceed with data request checking.

	If the JPIP server has disconnected this reader is {@link close(bool)
	closed}. An attempt is then made to {@link reconnect() reconnect} to
	the JPIP server (unless the reconnection procedure is in progress).
	If the reconnection is successful the previous {@link
	data_request(kdu_window&, bool) data request} is resubmitted. If the
	data request is {@link JP2_File_Reader::DATA_REQUEST_SUBMITTED} this
	method goes back to waiting for the data arrival notify event from
	the JPIP client. If the data request is {@link
	JP2_File_Reader::DATA_REQUEST_SATISFIED} the procedure will proceed
	with data request checking. If the request is {@link
	JP2_File_Reader::DATA_REQUEST_REJECTED} a JPIP_Exception is thrown
	describing the problem. If the reconnection fails (or was already in
	progress) a JPIP_Disconnected exception is thrown.

	If the JPIP server is connected but the data arrival notify event did
	not occur within the request timeout limit then an INFO_ONLY
	notification with a TIMEOUT_MESSAGE is sent to a registered rendering
	monitor. Then a JPIP_Timeout exception is thrown.

	When the data arrival notify event is received the JPIP_Client is
	queried for the status of the request. If the JPIP server connection
	is idle or the request status indicates that it is complete the
	return value is set to DATA_ACQUISITION_COMPLETE. Otherwise, if
	waiting for data arrival was canceled the return value is set to
	DATA_ACQUISITION_CANCELED; if not canceled and the data request was
	for metadata only, the procedure goes back to waiting for data
	arrival; or when codestream data has arrived but the data request is
	not complete\ the return value is set to
	DATA_ACQUISITION_INCOMPLETE.

	If the state of the data request can be obtained from the JPIP client
	and the resolution or region of the data that has arrived does not
	equal what was initially {@link data_request(kdu_window&, bool)
	requested} then the DATA_ACQUISITION_REDUCED bit is set in the return
	value.

	If the acquired data pointer argument is non-NULL and the state of
	the server request is known the resolution and image region provided
	by the JPIP server in response to the request are set in the
	Acquired_Data structure.

	Note: The Connection_Completed flag is set to true if it was false
	and any data was acquired.

	@param	acquired_data	A pointer to an Acquired_Data structure
		which, if non-NULL and the state of the data request is known,
		will be filled with the description of the data that was
		acquired.
	@return	A data acquisition code with one or more of the following
		bit flags set:
		<dl>
		<dt>{@link JP2_File_Reader::DATA_ACQUISITION_INCOMPLETE}
		<dd>More data for the last request is expected.

		<dt>{@link JP2_File_Reader::DATA_ACQUISITION_COMPLETE}
		<dd>The data request has been completed.

		<dt>{@link JP2_File_Reader::DATA_ACQUISITION_REDUCED}
		<dd>The state of the data request is known and it indicates that
			a lower resolution or smaller image region than requested was
			returned by the server.

		<dt>{@link JP2_File_Reader::DATA_ACQUISITION_CANCELED}
		<dd>The response to a monitor notification was false which
			cancels the data acquisition.
		</dl>
	@throws JPIP_Disconnected	If the JPIP client request queue
		associated with the connection ID for this JP2_JPIP_Reader
		disconnected and could not be reconnected.
	@throws JPIP_Exception	If the resubmitted data request after a
		successful reconnection was rejected.
	@throws	JPIP_Timeout	If the a data arrival notify event did not
		occur within the request timeout limit.
*/
virtual int data_acquisition (Acquired_Data* acquired_data = NULL);

/*==============================================================================
	Data
*/
protected:

//!	The asynchronous JPIP_Client event notifier.
friend class JPIP_Client_Notifier;
//std::shared_ptr<JPIP_Client_Notifier>
PIRL::Reference_Counted_Pointer<JPIP_Client_Notifier>
	Notifier;

//!	JPIP client.
//std::shared_ptr<kdu_supp::kdu_client>
PIRL::Reference_Counted_Pointer<kdu_supp::kdu_client>
	JPIP_Client;

//!	JPIP connection channel request queue ID.
int
	Connection_ID;

/*	The JP2 data bin cache manager attached to the JPIP_Client.

	Each rendering engine must have its own data bin cache manager even
	though the object may be attached to the same shared JPIP client
	(kdu_client, itself a subcass of kdu_cache) where the underlying
	data actually resides.
*/
kdu_supp::kdu_cache
	Data_Bin_Cache;

//	Reusable JPIP server window request informmation object.
kdu_supp::kdu_window
	Server_Request,
	Posted_Server_Request;
kdu_supp::kdu_window_prefs
	*Server_Preferences;

//!	Data acquisition provider blocking event semaphore.
kdu_core::kdu_event
	Provider_Event;
//!	Data acquisition provider synchronization lock used with the Provider_Event.
kdu_core::kdu_mutex
	Provider_Mutex;

//!	Flag that a reconnection attempt is in progress.
bool
	Reconnecting;
std::string
	Reconnection_Failure_Message;

//!	Flag that the JPIP server connection is complete.
bool
	Connection_Completed;

};	//	Class JP2_JPIP_Reader


}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA
#endif
