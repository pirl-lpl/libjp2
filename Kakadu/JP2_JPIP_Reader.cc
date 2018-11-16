/*	JP2_JPIP_Reader

HiROC CVS ID: $Id: JP2_JPIP_Reader.cc,v 1.48 2012/04/26 02:53:31 castalia Exp $

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

#include	"JP2_JPIP_Reader.hh"

//	Kakadu
#include	"jp2.h"
#include	"JP2_Box.hh"
#include	"kdu_client.h"
using namespace kdu_core;
using namespace kdu_supp;
#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
#include	<stdexcept>
using std::exception;

#ifdef _WIN32
#include "Windows.h"	//	For Sleep system function.
#endif

//	Local convenience class.
#include	"KDU_dims.hh"


#if defined (DEBUG)
/*	DEBUG controls

	DEBUG report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_MANIPULATORS		(1 << 2)
#define DEBUG_OPEN				(1 << 4)
#define	DEBUG_RENDER			(1 << 5)
#define	DEBUG_NOTIFIER			(1 << 6)
#define DEBUG_NOTIFY			(1 << 7)
#define DEBUG_DATA_ACQUISITION	(1 << 8)
#define DEBUG_TIMING			(1 << 9)
#define	DEBUG_OVERVIEW			(1 << 10)

#if ((DEBUG) & (DEBUG_TIMING | \
				DEBUG_OPEN | \
				DEBUG_RENDER | \
				DEBUG_DATA_ACQUISITION | \
				DEBUG_OVERVIEW))
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
#endif	//	!_WIN32
#endif
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
	JP2_JPIP_Reader::ID =
		"UA::HiRISE::Kakadu::JP2_JPIP_Reader ($Revision: 1.48 $ $Date: 2012/04/26 02:53:31 $)";


#ifndef DEFAULT_JPIP_TRANSPORT
#define DEFAULT_JPIP_TRANSPORT				http-tcp
#endif
#define JPIP_TRANSPORT						AS_STRING(DEFAULT_JPIP_TRANSPORT)

const int
	JP2_JPIP_Reader::NOT_CONNECTED			= -2;

const std::string
	JP2_JPIP_Reader::RECONNECTING_MESSAGE
		= "Reconnecting to the JPIP server.",
	JP2_JPIP_Reader::WAITING_MESSAGE
		= "Waiting for JPIP server data.",
	JP2_JPIP_Reader::TIMEOUT_MESSAGE
		= "Timeout while waiting for JPIP server data.",
	JP2_JPIP_Reader::ACQUIRED_DATA_MESSAGE
		= "Acquired JPIP server data.";

//!	Server image data window request resolution rounding.
#define	ROUND_DOWN		-1
#define ROUND_UP		1
#define ROUND_NEAREST	0

/*==============================================================================
	JPIP_Client Notifier
*/
struct JPIP_Client_Notifier
:	public kdu_client_notifier
{
//	Readers registered to receive client event notification.
std::vector<JP2_JPIP_Reader*>
	Readers;

kdu_mutex
	Notifier_Lock;


JPIP_Client_Notifier ()
	:
	Readers (),
	Notifier_Lock ()
{}


void
notify ()
{
#if ((DEBUG) & DEBUG_NOTIFIER)
clog << "==> JPIP_Client_Notifier: notify" << endl;
#endif
Notifier_Lock.lock ();
int
	count = Readers.size ();
while (count--)
	{
	#if ((DEBUG) & DEBUG_NOTIFIER)
	clog << "    Setting the Provider_Event for "
			<< Readers[count]->source_name ()
			<< " channel " << Readers[count]->Connection_ID << endl;
	#endif
	if (Readers[count]->Provider_Mutex.exists ())
		{
		Readers[count]->Provider_Mutex.lock ();
		if (Readers[count]->Provider_Event.exists ())
			Readers[count]->Provider_Event.protected_set (); // TODO see kdu_elementary.h's class kdu_event
		#if ((DEBUG) & DEBUG_NOTIFIER)
		else
			clog << "    Provider_Event for reader " << count
					<< " doesn't exist!" << endl;
		#endif
		Readers[count]->Provider_Mutex.unlock ();
		}
	#if ((DEBUG) & DEBUG_NOTIFIER)
	else
		clog << "    Provider_Mutex for reader " << count
				<< " doesn't exist!" << endl;
	#endif
	}
Notifier_Lock.unlock ();
#if ((DEBUG) & DEBUG_NOTIFIER)
clog << "<== JPIP_Client_Notifier: notify" << endl;
#endif
}


void
add
	(
	JP2_JPIP_Reader*	reader
	)
{
Notifier_Lock.lock ();
int
	count = Readers.size ();
while (count--)
	if (Readers[count] == reader)
		break;
if (count < 0)
	Readers.push_back (reader);
Notifier_Lock.unlock ();
}


void
remove
	(
	JP2_JPIP_Reader*	reader
	)
{
Notifier_Lock.lock ();
for (std::vector<JP2_JPIP_Reader*>::iterator
		element = Readers.begin ();
		element != Readers.end ();
	  ++element)
	{
	if (*element == reader)
		{
		Readers.erase (element);
		break;
		}
	}
Notifier_Lock.unlock ();
}
};	//	Class JPIP_Client_Notifier

/*==============================================================================
	Constructors
*/
JP2_JPIP_Reader::JP2_JPIP_Reader ()
	:	JP2_File_Reader (),
	Notifier (NULL),
	JPIP_Client (),
	Connection_ID (NOT_CONNECTED),
	Data_Bin_Cache (),
	Server_Request (),
	Server_Preferences (NULL),
	Provider_Event (),
	Provider_Mutex (),
	Reconnecting (false),
	Connection_Completed (false)
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_JPIP_Reader @ " << (void*)this << endl;
#endif
}


JP2_JPIP_Reader::JP2_JPIP_Reader
	(
	const std::string&	source
	)
	:	JP2_File_Reader (),
	Notifier (NULL),
	JPIP_Client (),
	Connection_ID (NOT_CONNECTED),
	Data_Bin_Cache (),
	Server_Request (),
	Server_Preferences (NULL),
	Provider_Event (),
	Provider_Mutex (),
	Reconnecting (false),
	Connection_Completed (false)
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_JPIP_Reader @ " << (void*)this << endl
	 << "    source = " << source << endl;
#endif
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


JP2_JPIP_Reader::JP2_JPIP_Reader
	(
	const JP2_JPIP_Reader&	JP2_JPIP_reader
	)
	:	JP2_File_Reader (JP2_JPIP_reader),
	Notifier (JP2_JPIP_reader.Notifier),
	JPIP_Client (JP2_JPIP_reader.JPIP_Client),
	Connection_ID (NOT_CONNECTED),
	Data_Bin_Cache (),
	Server_Request (),
	Server_Preferences (NULL),
	Provider_Event (),
	Provider_Mutex (),
	Reconnecting (false),
	Connection_Completed (false)
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_JPIP_Reader @ " << (void*)this << endl
	 << "    Copy @ " << (void*)(&JP2_JPIP_reader) << endl
	 << "    source_name = " << JP2_JPIP_reader.source_name () << endl
	 << "    JPIP_Client.reference_count = "
	 	<< JPIP_Client.reference_count () << endl
	 << "       Notifier.reference_count = "
	 	<< Notifier.reference_count () << endl;
#endif
if (JP2_JPIP_reader.Server_Preferences)
	Server_Preferences =
		new kdu_supp::kdu_window_prefs (*JP2_JPIP_reader.Server_Preferences);

try {open (JP2_JPIP_reader.source_name ());}
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
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << "    Copy - " << image_bands () << " band, " << image_size () << endl;
#endif
}


JP2_JPIP_Reader::~JP2_JPIP_Reader () throw()
{
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << ">>> ~JP2_JPIP_Reader @ " << (void*)this << endl
	 << "    JPIP_Client.reference_count = "
	 	<< JPIP_Client.reference_count () << endl
	 << "       Notifier.reference_count = "
	 	<< Notifier.reference_count () << endl;
#endif
JP2_JPIP_Reader::reset ();

if (Notifier.reference_count () == 1)
	{
	#if (DEBUG & DEBUG_CONSTRUCTORS)
	clog << "    de-register the Notifier from the JPIP_Client." << endl;
	#endif
	//	The notifier will be destroyed; de-register it from the JPIP_Client.
	JPIP_Client->install_notifier (NULL);
	}

if (JPIP_Client.reference_count () == 1)
	{
	#if (DEBUG & DEBUG_CONSTRUCTORS)
	clog << "    shutdown the JPIP client" << endl;
	#endif
	shutdown ();
	if (! is_shutdown ())
		{
		#if (DEBUG & DEBUG_CONSTRUCTORS)
		clog << "    shutdown forced" << endl;
		#endif
		JPIP_request_timeout (0);
		shutdown (false);
		}
	}

if (Server_Preferences)
	delete Server_Preferences;
#if (DEBUG & DEBUG_CONSTRUCTORS)
clog << "<<< ~JP2_JPIP_Reader" << endl;
#endif
}


JP2_JPIP_Reader*
JP2_JPIP_Reader::clone () const
{return new JP2_JPIP_Reader (*this);}

/*==============================================================================
	Source open, configure and close
*/
#ifndef DOXYGEN_PROCESSING
namespace
{
static const string
	open_failed ("Failed to open a JPIP server connection.");
}
#endif


int
JP2_JPIP_Reader::open
	(
	const std::string&	source
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING | DEBUG_OVERVIEW))
clog << ">>> JP2_JPIP_Reader::open: " << source << endl;
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
if (source.empty ())
	{
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "No JPIP source URL specified to open.";
	throw JP2_Invalid_Argument (message.str (), ID);
	}
if (! kdu_client::check_compatible_url (source.c_str (), true))
	{
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "Invalid JPIP source URL: " << source;
	throw JP2_Invalid_Argument (message.str (), ID);
	}

//	Processing threads and mutexes.
deploy_processing_threads ();

if (is_shutdown ())
	{
	//	Establish the initial JPIP_Client connection.

	if (source != source_name ())
		{
		//	Reset source image characterization info.
		reset ();

		//	Set the source name.
		source_name (source);
		}

	if (! JPIP_Client)
		{
		#if ((DEBUG) & DEBUG_OPEN)
		clog << "    construct a JPIP_Client" << endl;
		#endif
		/*	Create and initialize the JPIP_Client.

			N.B.: The JPIP_Client is a Reference_Counted_Pointer to the
			kdu_client that it is assigned. This enables the JPIP_Client
			to be shared by other JP2_JPIP_Reader instantiations that are
			copies of the initial object where the JPIP_Client is
			created and initialized. That is being done here.
		*/
		JPIP_Client = new kdu_client ();
		}

	if (! Notifier)
		{
		/*
			N.B.: The Notifier is a Reference_Counted_Pointer to the
			JPIP_Client_Notifier that it is assigned. This is done
			because there can only be one JPIP_Client_Notifier for
			a JPIP_Client.
		*/
		#if ((DEBUG) & DEBUG_OPEN)
		clog << "    construct a JPIP_Client_Notifier" << endl;
		#endif
		Notifier = new JPIP_Client_Notifier ();
		}

	if (! Server_Preferences)
		{
		//	Create and intialize the JPIP server preferences.
		#if ((DEBUG) & DEBUG_OPEN)
		clog << "    create Server_Preferences with KDU_WINDOW_PREF_FULL"
				<< endl;
		#endif
		Server_Preferences = new kdu_supp::kdu_window_prefs;
		Server_Preferences->set_pref (KDU_WINDOW_PREF_FULL);
		}

	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_OVERVIEW))
	clog << "    Connecting" << endl;
	if (! JPIP_Proxy.empty ())
		clog << "    JPIP_Proxy: " << endl;
	if (! JPIP_Cache_Directory.empty ())
		clog << "    JPIP_Cache_Directory: " << JPIP_Cache_Directory << endl;
	#endif
	try {Connection_ID = JPIP_Client->connect
		(
		NULL,				//	Server hostname; obtained from URL.
		JPIP_Proxy.c_str (),
		NULL,				//	Server resource request; obtained from URL.
		JPIP_TRANSPORT,
		JPIP_Cache_Directory.c_str (),
		KDU_CLIENT_MODE_INTERACTIVE,
		source.c_str ()		//	URL.
		);}
	catch (kdu_exception except)
		{
		ostringstream
			message;
		message
			<< open_failed << endl
			<< Kakadu_error_message (except) << endl
			<< "while opening the JPIP client connection" << endl
			<< "for the " << source_name () << " source";
		if (! JPIP_Proxy.empty ())
			message
				<< endl
				<< "using proxy " << JPIP_Proxy;
		if (! JPIP_Cache_Directory.empty ())
			message
				<< endl
				<< "with data bin cache directory " << JPIP_Cache_Directory;
		message << '.';
		throw JPIP_Exception (message.str (), ID);
		}
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_OVERVIEW))
	clog << "    Connection_ID: " << Connection_ID << endl;
	#endif
	/*
		>>> WARNING <<< The notifier MUST be installed AFTER the
		connection completes!
	*/
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    Installing the JPIP_Client_Notifier" << endl;
	#endif
	JPIP_Client->install_notifier (Notifier);
	}
else
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    JPIP_Client exists and is active" << endl;
	#endif
	if (is_open () &&
		source == source_name ())
		{
		//	Already alive and well.
		#if ((DEBUG) & DEBUG_OPEN)
		clog << "    Already connected" << endl
			 << "<<< JP2_JPIP_Reader::open: " << Connection_ID << endl;
		#endif
		return Connection_ID;
		}

	//	Establish an additional JPIP_Client connection.
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    Checking for compatible connection" << endl;
	#endif
	if (JPIP_Client->check_compatible_connection
			(NULL, NULL, KDU_CLIENT_MODE_INTERACTIVE, source.c_str ()))
		{
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_OVERVIEW))
		clog << "    add a request queue to JPIP_Client" << endl;
		#endif
		if ((Connection_ID = JPIP_Client->add_queue ()) < 0)
			{
			Connection_ID = NOT_CONNECTED;
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_OVERVIEW))
			clog << "    failed to add a request queue" << endl;
			#endif
			ostringstream
				message;
			message
				<< open_failed << endl
				<< "Couldn't add a data request queue to the JPIP client"
				<< "for the " << source_name () << " source.";
			throw JPIP_Exception (message.str (), ID);
			}
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_OVERVIEW))
		clog << "    Connection_ID: " << Connection_ID << endl;
		#endif
		}
	else
		{
		ostringstream
			message;
		message
			<< open_failed << endl
			<< "Incompatible source: " << source << "" << endl
			<< "for existing source: " << source_name () << endl
			<< "The existing connection must be closed"
				" before opening the new connection.";
		throw JP2_Invalid_Argument (message.str (), ID);
		}
	}

//	Attach this object's data bin cache manager object to the JPIP client.
#if ((DEBUG) & DEBUG_OPEN)
clog << "    attach the Data_Bin_Cache to the JPIP_Client @ "
		<< (void*)JPIP_Client << endl;
#endif
Data_Bin_Cache.attach_to (JPIP_Client);

if (! JP2_Stream.exists ())
	{
	/*	Open the JP2_Stream.

		N.B.: The JP2_Stream is part of the upstream JP2 data reading and
		renderinng mechanism located in the JP2_File_Reader. It provides the
		abstraction layer between the metadata acquisition and rendering
		engine and the actual source of the data. It is being opened here
		because, for a JPIP data source, it must be bound to the
		Data_Bin_Cache that will be the data provider for the JP2_Stream.
	*/
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "    open the jp2_threadsafe_family_src on the Data_Bin_Cache"
			<< endl;
	#endif
	JP2_Stream.open (&Data_Bin_Cache);
	}
#if ((DEBUG) & (DEBUG_TIMING | DEBUG_OVERVIEW))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_JPIP_Reader::open: setup time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
start_clock = end_clock;
clog << "                             duration = "
		<< duration (start_time, end_time) << endl;
start_time = end_time;
end_time = &time_values[time_select = time_select ? 0 : 1];
#endif	//	!_WIN32
#endif

//	Add this object to the Notifier's list.
Notifier->add (this);

try
	{
	//	Initiate the JP2 metadata request.
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_OVERVIEW))
	clog << "    Requesting metadata" << endl;
	#endif
	switch (data_request ())
		{
		case DATA_REQUEST_SUBMITTED:
			//	Wait for the JP2 metadata request to be completed.
			data_acquisition ();
		case DATA_REQUEST_SATISFIED:
			//	Already have the data in the cache.
			break;
		case DATA_REQUEST_REJECTED:
			ostringstream
				message;
			message
				<< open_failed << endl
				<< "The data request for metadata was rejected" << endl
				<< "for the " << source_name () << " source.";
			throw JPIP_Exception (message.str (), ID);
		}
	}
catch (JPIP_Exception& except)
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "!!! data_acquisition JPIP_Exception -" << endl
		 << except.what () << endl;
	#endif
	close (Connection_ID);
	ostringstream
		message;
	message
		<< open_failed << endl
		<< except.message () << endl;
	except.message (message.str ());
	throw;
	}
catch (exception& except)
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "!!! data_acquisition exception -" << endl
		 << except.what () << endl;
	#endif
	close (Connection_ID);
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "Unexpected exception while acquiring metadata" << endl
		<< "for the " << source_name () << " source." << endl
		<< except.what ();
	throw JPIP_Exception (message.str (), ID);
	}
catch (kdu_exception except)
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "!!! data_acquisition kdu_exception - " << except << endl;
	#endif
	close (Connection_ID);
	ostringstream
		message;
	message
		<< open_failed << endl
		<< Kakadu_error_message (except) << endl
		<< "while acquiring metadata" << endl
		<< "for the " << source_name () << " source.";
	throw JPIP_Exception (message.str (), ID);
	}
catch (...)
	{
	#if ((DEBUG) & DEBUG_OPEN)
	clog << "!!! data_acquisition unknown exception!" << endl;
	#endif
	close (Connection_ID);
	ostringstream
		message;
	message
		<< open_failed << endl
		<< "Unknown exception while acquiring metadata" << endl
		<< "for the " << source_name () << " source.";
	throw JPIP_Exception (message.str (), ID);
	}
#if ((DEBUG) & DEBUG_OPEN)
clog << "    JP2_JPIP_Reader::open: Data delivery complete" << endl;
#endif
#if ((DEBUG) & (DEBUG_TIMING | DEBUG_OVERVIEW))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_JPIP_Reader::open: medata acquisition time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
start_clock = end_clock;
clog << "                                          duration = "
		<< duration (start_time, end_time) << endl;
start_time = end_time;
end_time = &time_values[time_select = time_select ? 0 : 1];
#endif	//	!_WIN32
#endif

//	Open the JP2 source data stream.
open_source ();

#if ((DEBUG) & (DEBUG_OPEN | DEBUG_TIMING | DEBUG_OVERVIEW))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (end_time, 0);
if (begin_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    JP2_JPIP_Reader::open: total time = "
			<< (double (end_clock - begin_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
clog << "                             duration = "
		<< duration (&begin_time, end_time) << endl;
#endif	//	!_WIN32
clog << "<<< JP2_JPIP_Reader::open: " << Connection_ID << endl;
#endif
Connection_Completed = true;
return Connection_ID;
}


void
JP2_JPIP_Reader::deploy_processing_threads ()
{
#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
clog << ">>> JP2_JPIP_Reader::deploy_processing_threads" << endl;
#endif
JP2_File_Reader::deploy_processing_threads ();

if (! Provider_Mutex.exists () &&
	! Provider_Mutex.create ())
	{
	ostringstream
		message;
	message
		<< "Unable to create a required mutex" << endl
		<< "needed to manage the JPIP server connection" << endl
		<< "for the " << source_name () << " source.";
	throw JP2_Logic_Error (message.str (), ID);
	}
if (! Provider_Event.exists ())
	Provider_Event.create (/* manual reset */ true);

#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
clog << "<<< JP2_JPIP_Reader::deploy_processing_threads" << endl;
#endif
}


bool
JP2_JPIP_Reader::resolution_and_region
	(
	unsigned int		resolution,
	const Rectangle&	region
	)
{
#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_OPEN))
clog << ">>> JP2_JPIP_Reader::resolution_and_region" << endl;
#endif
if (! reconnect ())
	{
	#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_OPEN))
	clog << "    could not reconnect -" << endl
		 << Reconnection_Failure_Message << endl;
	#endif
	ostringstream
		message;
	message
		<< "Can't render." << endl
		<< Reconnection_Failure_Message;
	throw JPIP_Disconnected (message.str (), ID);
	}

bool
	reset = JP2_File_Reader::resolution_and_region (resolution, region);
if (reset &&
	JPIP_Client &&
	JPIP_Client->is_alive (Connection_ID) &&
	! Decompressor.finish ())
	{
	close (Connection_ID);
	ostringstream
		message;
	message << "Couldn't reset the image rendering";
	if (resolution != Resolution_Level)
		message << endl
			<< "resolution from " << Resolution_Level << " to " << resolution;
	if (region != Image_Region)
		{
		message << endl;
		if (resolution != Resolution_Level)
			message << "and ";
		message
			<< "region from " << Image_Region << " to " << region;
		}
	message << '.';
	throw JPIP_Exception (message.str (), ID);
	}
#if ((DEBUG) & (DEBUG_MANIPULATORS | DEBUG_OPEN))
clog << "<<< JP2_JPIP_Reader::resolution_and_region" << endl;
#endif
return reset;
}


bool
JP2_JPIP_Reader::ready
	(
	string*	report
	)
	const
{
bool
	is_ready = JP2_File_Reader::ready (report);

if (! JPIP_Client ||
	! JPIP_Client->is_alive (Connection_ID))
	{
	if (report)
		{
		ostringstream
			reasons;
		if (! is_ready)
			reasons << endl;
		reasons << "No JPIP server connection.";
		*report += reasons.str ();
		}
	is_ready = false;
	}
return is_ready;
}


string
JP2_JPIP_Reader::connection_status () const
{
if (JPIP_Client)
	return JPIP_Client->get_status ();
return "No JPIP client.";
}


void
JP2_JPIP_Reader::close
	(
	bool	force
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << ">>> JP2_JPIP_Reader::close @ " << (void*)this << endl
	 << "    force = " << boolalpha << force << endl
	 << "    Connection_ID = " << Connection_ID << endl;
#endif
if (Notifier)
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    remove from Notifier" << endl;
	#endif
	Notifier->remove (this);
	}

//	Base class close.
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "    close the JP2_File_Reader" << endl;
#endif
JP2_File_Reader::close ();

if (JPIP_Client &&
	Connection_ID != NOT_CONNECTED)
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    disconnect JPIP_Client with timeout of "
			<< JPIP_request_timeout () << " seconds" << endl;
	#endif
	force = ! force;
	JPIP_Client->disconnect
		(
		force,	//	Try to keep the underlying transport open.
		JPIP_request_timeout () * 1000,
		Connection_ID,
		force	//	Wait for completion or timeout.
		);
	}
Connection_Completed = false;
Connection_ID = NOT_CONNECTED;
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "<<< JP2_JPIP_Reader::close" << endl;
#endif
}


bool
JP2_JPIP_Reader::is_open () const
{
if (JPIP_Client &&
	Connection_ID >= 0 &&
	JPIP_Client->is_alive (Connection_ID))
	return JP2_File_Reader::is_open ();
return false;
}


bool
JP2_JPIP_Reader::reconnect ()
{
#if ((DEBUG) & DEBUG_OPEN)
clog << ">>> JP2_JPIP_Reader::reconnect @ " << (void*)this << endl;
#endif
Reconnection_Failure_Message.clear ();
bool
	reconnected = is_open (),
	canceled = false;
int
	retries = autoreconnect_retries (),
	wait_seconds;
if (! reconnected &&
	! Reconnecting &&
	retries &&
	! source_name ().empty ())
	{
	//	Recursion avoidance flag.
	Reconnecting = true;

	//	Where to send notififcations.
	Rendering_Monitor*
		monitor = rendering_monitor ();

	while (! canceled)
		{
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_NOTIFY))
		if (monitor)
			clog << "    JP2_JPIP_Reader::reconnect: notify status "
					<< Rendering_Monitor::INFO_ONLY
					<< " \"" << RECONNECTING_MESSAGE << '"' << endl;
		#endif
		if (monitor &&
			! monitor->notify (*this,
				Rendering_Monitor::INFO_ONLY, RECONNECTING_MESSAGE))
			{
			//	Procedure canceled.
			canceled = true;
			break;
			}

		Reconnection_Failure_Message.clear ();
		try
			{
			//	Try to re-open the source.
			open (source_name ());
			reconnected = true;
			break;
			}
		catch (JPIP_Disconnected& except)
			{
			Reconnection_Failure_Message = except.message ();
			#if ((DEBUG) & DEBUG_OPEN)
			clog << "    " << Reconnection_Failure_Message << endl;
			#endif
			wait_seconds = JPIP_request_timeout ();
			}
		catch (JPIP_Timeout& except)
			{
			Reconnection_Failure_Message = except.message ();
			#if ((DEBUG) & DEBUG_OPEN)
			clog << "    " << Reconnection_Failure_Message << endl;
			#endif
			wait_seconds = 0;
			}
		catch (JP2_Exception& except)
			{
			Reconnection_Failure_Message = except.message ();
			#if ((DEBUG) & DEBUG_OPEN)
			clog << "    " << Reconnection_Failure_Message << endl;
			#endif
			wait_seconds = 0;
			}
		catch (...)
			{
			Reconnecting = false;
			throw;
			}
		if (! --retries)
			break;

		#if ((DEBUG) & DEBUG_OPEN)
		if (wait_seconds)
			clog << "    waiting " << wait_seconds << " seconds" << endl;
		#endif
		while (wait_seconds--)
			{
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_NOTIFY))
			if (monitor)
				clog << "    JP2_JPIP_Reader::reconnect: notify status "
						<< Rendering_Monitor::INFO_ONLY
						<< " \"" << RECONNECTING_MESSAGE << '"' << endl;
			#endif
			if (monitor &&
				! monitor->notify (*this,
					Rendering_Monitor::INFO_ONLY, RECONNECTING_MESSAGE))
				{
				canceled = true;
				break;
				}
            #ifdef _WIN32
            Sleep (1000 /* milliseconds */);
            #else	//	_WIN32
            sleep (1);
            #endif
			}
		}
	Reconnecting = false;
	if (monitor)
		{
		Rendering_Monitor::Status
			status;
		string
			message;
		if (canceled)
			{
			status = Rendering_Monitor::CANCELED;
			message = Rendering_Monitor::Status_Message[status];
			}
		else
			{
			status = Rendering_Monitor::INFO_ONLY;
			message = "Reconnection to the JPIP server ";
			message += reconnected ? "succeeded." : "failed.";
			}
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_NOTIFY))
		clog << "    JP2_JPIP_Reader::reconnect: notify status "
				<< status << " \"" << message << '"' << endl;
		#endif
		monitor->notify (*this, status, message);
		}
	}
#if ((DEBUG) & DEBUG_OPEN)
clog << "<<< JP2_JPIP_Reader::reconnect: " << boolalpha << reconnected << endl;
#endif
return reconnected;
}


void
JP2_JPIP_Reader::reset ()
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << ">>> JP2_JPIP_Reader::reset" << endl;
#endif
JP2_JPIP_Reader::close ();

#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "    close Data_Bin_Cache" << endl;
#endif
Data_Bin_Cache.close ();

JP2_File_Reader::reset ();
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "<<< JP2_JPIP_Reader::reset" << endl;
#endif
}


void
JP2_JPIP_Reader::shutdown
	(
	bool	wait_for_completion
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << ">>> JP2_JPIP_Reader::shutdown @ " << (void*)this << endl
	 << "    wait_for_completion = "
	 	<< boolalpha << wait_for_completion << endl;
#endif
JP2_JPIP_Reader::reset ();

if (JPIP_Client)
	{
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    disconnecting all JPIP_Client server request channels" << endl
		 << "    timeout = " << JPIP_request_timeout () << " seconds" << endl;
	#endif
	JPIP_Client->disconnect
		(
		false,	//	Don't keep the underlying transport open.
		JPIP_request_timeout () * 1000,	//	Milliseconds.
		-1,								//	All request queues.
		wait_for_completion				//	Wait for completion or timeout.
		);
	#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
	clog << "    closing the JPIP_Client" << endl;
	#endif
	JPIP_Client->close ();
	}

#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "    destroy Provider_Mutex and Provider_Event" << endl;
#endif
if (Provider_Mutex.exists ())
	Provider_Mutex.destroy ();
if (Provider_Event.exists ())
	Provider_Event.destroy ();
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_CONSTRUCTORS))
clog << "<<< JP2_JPIP_Reader::shutdown" << endl;
#endif
}


bool
JP2_JPIP_Reader::is_shutdown () const
{
if (JPIP_Client &&
	JPIP_Client->is_alive ())	//	Any live connection.
	return false;
return true;
}

/*==============================================================================
	Data acquisition.
*/
bool
JP2_JPIP_Reader::load_box_content
	(
	JP2_Box&		box,
	unsigned char*	content
	)
{
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << ">>> JP2_JPIP_Reader::load_box_content:" << endl
	 << box;
#endif
bool
	loaded = true;
kdu_int32
	box_type = box.get_box_type ();
long
	content_length = box.get_remaining_bytes ();
if (content_length > 0)
	{
	if (! box.is_complete ())
		{
		//	Request the metadata from the provider.
		jp2_locator
			locator;
		locator.set_file_pos (box.get_locator ().get_file_pos ());
		box.close ();
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
		clog << "    updating box with metadata_request ..." << endl;
		#endif
		int
			status = metadata_request (box_type);
		#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
		clog << "    metadata_request status = "
				<< data_request_description (status) << endl;
		#endif
		if (status != DATA_REQUEST_REJECTED)
			{
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			clog << "    reopen box at -" << endl
				 << locator;
			#endif
			if (! box.open (box.source (), locator))
				{
				#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
				clog << "    reopen box failed!" << endl;
				#endif
				loaded = false;
				}
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			else
				clog << "    reopened box -" << endl
					 << box;
			#endif
			}
		else
			{
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			clog << "    metadata_request failed!" << endl;
			#endif
			loaded = false;
			}
		}

	if (loaded)
		{
		if (jp2_is_superbox (box_type))
			{
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			clog << "    ==> Begin sub-box contents of " << box_name (box_type)
					<< endl;
			#endif
			int
				header_length;
			kdu_int32
				value;
			kdu_long
				box_length (0);
			JP2_Box
				sub_box;
			for (sub_box.open (&box);
				 sub_box.exists () &&
				 	loaded;
				 sub_box.open_next ())
				{
				//	Write the header sequence into the content buffer.
				header_length  = sub_box.get_box_header_length ();
				content_length = sub_box.get_remaining_bytes ();
				if (content_length < 0)
					box_length = content_length = 0;
				else
					box_length = header_length + content_length;

				//	LBox:
				if (header_length > 8)
					value = 1;	//	Use XLBox.
				else
					value = (kdu_int32)box_length;
				put_int (value, content);
				#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
				clog << "        LBox = " << get_int (content) << endl;
				#endif
				content += sizeof (value);

				//	TBox:
				value = sub_box.get_box_type ();
				put_int (value, content);
				#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
				value = get_int (content);
				clog << "        TBox = " << type_name (value)
						<< " (0x" << hex << value << dec << ')' << endl;
				#endif
				content += sizeof (value);

				if (header_length > 8)
					{
					//	XLBox:
					put_long (box_length, content);
					#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
					clog << "        XLBox = " << get_long (content) << endl;
					#endif
					content += sizeof (box_length);
					}

				//	DBox:
				#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
				clog << "        DBox -" << endl;
				#endif
				loaded =  load_box_content (sub_box, content);
				content += content_length;

				sub_box.close ();
				}
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			clog << "    <== End sub-box contents of " << box_name (box_type)
					<< endl;
			#endif
			}
		else
			{
			#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
			clog << "    reading " << content_length
					<< " byte box content" << endl;
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
		}
	}
#if ((DEBUG) & (DEBUG_OPEN | DEBUG_METADATA))
clog << "<<< JP2_JPIP_Reader::load_box_content: " << loaded << endl;
#endif
return loaded;
}


#ifndef DOXYGEN_PROCESSING
std::ostream&
operator<<
	(
	std::ostream&		stream,
	const kdu_window&	window
	)
{
stream
	<< "    kdu_window -" << endl
	<< "             resolution: " << window.resolution << endl
	<< "        round_direction: " << window.round_direction << endl
	<< "                 region: " << window.region << endl
	<< "             max_layers: " << window.max_layers << endl
	<< "          metadata_only: " << window.metadata_only << endl
	<< "       have_metareq_all: " << window.have_metareq_all << endl
	<< "    have_metareq_global: " << window.have_metareq_global << endl
	<< "    have_metareq_stream: " << window.have_metareq_stream << endl
	<< "    have_metareq_window: " << window.have_metareq_window << endl;
kdu_metareq
	*meta = window.metareq;
while (meta)
	{
	stream
		<< "    kdu_metareq @ " << (void*)meta << endl
		<< "         box type: " << JP2_Metadata::box_name (meta->box_type)
			<< " (\"" << JP2_Metadata::type_name (meta->box_type) << "\")"
			<< endl
		<< "        qualifier: " << meta->qualifier << endl
		<< "         priority: " << meta->priority << endl
		<< "       byte_limit: " << meta->byte_limit << endl
		<< "          recurse: " << meta->recurse << endl
		<< "      root_bin_id: " << meta->root_bin_id << endl
		<< "        max_depth: " << meta->max_depth << endl;
	meta = meta->next;
	}
return stream;
}


std::ostream&
operator<<
	(
	std::ostream&			stream,
	const kdu_window_prefs&	preferences
	)
{
stream
	<< "    kdu_window_prefs -" << endl;
int
	flags = 0;
for (int
		choice = 0;
		choice < 3;
		choice++)
	{
	switch (choice)
		{
		case 0:
			flags = preferences.preferred;
			stream << "    preferred: ";
			break;
		case 1:
			flags = preferences.required;
			stream << "     required: ";
			break;
		case 2:
			flags = preferences.denied;
			stream << "       denied: ";
		}
	stream << flags << endl;
	if (flags & KDU_WINDOW_PREF_PROGRESSIVE)
		stream << "      KDU_WINDOW_PREF_PROGRESSIVE" << endl;
	if (flags & KDU_WINDOW_PREF_FULL)
		stream << "      KDU_WINDOW_PREF_FULL" << endl;
	if (flags & KDU_CONCISENESS_PREF_CONCISE)
		stream << "      KDU_CONCISENESS_PREF_CONCISE" << endl;
	if (flags & KDU_CONCISENESS_PREF_LOOSE)
		stream << "      KDU_CONCISENESS_PREF_LOOSE" << endl;
	if (flags & KDU_PLACEHOLDER_PREF_INCR)
		stream << "      KDU_PLACEHOLDER_PREF_INCR" << endl;
	if (flags & KDU_PLACEHOLDER_PREF_EQUIV)
		stream << "      KDU_PLACEHOLDER_PREF_EQUIV" << endl;
	if (flags & KDU_PLACEHOLDER_PREF_ORIG)
		stream << "      KDU_PLACEHOLDER_PREF_ORIG" << endl;
	if (flags & KDU_CODESEQ_PREF_FWD)
		stream << "      KDU_CODESEQ_PREF_FWD" << endl;
	if (flags & KDU_CODESEQ_PREF_BWD)
		stream << "      KDU_CODESEQ_PREF_BWD" << endl;
	if (flags & KDU_CODESEQ_PREF_ILVD)
		stream << "      KDU_CODESEQ_PREF_ILVD" << endl;
	if (flags & KDU_MAX_BANDWIDTH_PREF)
		stream << "      KDU_MAX_BANDWIDTH_PREF = "
					<< preferences.max_bandwidth << endl;
	if (flags & KDU_BANDWIDTH_SLICE_PREF)
		stream << "      KDU_BANDWIDTH_SLICE_PREF = "
					<< preferences.bandwidth_slice << endl;
	if (flags & KDU_COLOUR_METH_PREF)
		stream << "      KDU_COLOUR_METH_PREF = "
					<< preferences.colour_meth_pref_limits[0]
					<< " enumerated, "
					<< preferences.colour_meth_pref_limits[1]
					<< " restricted, "
					<< preferences.colour_meth_pref_limits[2]
					<< " unrestricted, "
					<< preferences.colour_meth_pref_limits[3]
					<< " vendor specific" << endl;
	if (flags & KDU_CONTRAST_SENSITIVITY_PREF)
		stream << "      KDU_CONTRAST_SENSITIVITY_PREF = "
					<< preferences.num_csf_angles << " angles" << endl;
	}
return stream;
}


#ifdef DEBUG
std::string
server_request_status_description
	(
	int		status
	)
{
ostringstream
	report;
report << hex << status << dec;
if (status & KDU_CLIENT_WINDOW_IS_MOST_RECENT)
	report << ", MOST RECENT";
if (status & KDU_CLIENT_WINDOW_RESPONSE_TERMINATED)
	report << ", RESPONSE TERMINATED";
if (status & KDU_CLIENT_WINDOW_IS_COMPLETE)
	report << ", COMPLETE";
return report.str ();
}
#endif
#endif


int
JP2_JPIP_Reader::data_request
	(
	Rectangle*	region,
	bool		preemptive
	)
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">>> JP2_JPIP_Reader::data_request" << endl;
#endif
Server_Request.init ();
Posted_Server_Request.init ();

if (region)
	{
	//	Codestream data.
	#if ((DEBUG) & DEBUG_RENDER)
	clog << "    region = " << region << endl;
	#endif
	if (region->area () == 0)
		{
		#if ((DEBUG) & DEBUG_RENDER)
		clog << "    Empty region area" << endl
			 << "<<< JP2_JPIP_Reader::data_request: "
				<< data_request_description (DATA_REQUEST_SATISFIED) << endl;
		#endif
		return DATA_REQUEST_SATISFIED;
		}

	int
		divisor = resolution_level () - 1;
	Server_Request.round_direction = ROUND_UP;
	//	Image resolution in terms of the size of the entire image.
	Server_Request.resolution.x  = image_width () >> divisor;
	Server_Request.resolution.y  = image_height () >> divisor;
	//	Region of interest on the rendered grid.
	Server_Request.region.pos.x  = region->X;
	Server_Request.region.pos.y  = region->Y;
	Server_Request.region.size.x = region->Width;
	Server_Request.region.size.y = region->Height;
	//	All quality layers.
	Server_Request.max_layers    = 0;

	//	Add region-related metadata requests.
	Server_Request.add_metareq (0, KDU_MRQ_WINDOW | KDU_MRQ_STREAM);
	}
else
	{
	//	Metadata-only.
	#if ((DEBUG) & DEBUG_RENDER)
	clog << "    metadata-only" << endl;
	#endif
	Server_Request.set_metadata_only (true);
	}

int
	status = data_request (Server_Request, preemptive);
#if ((DEBUG) & DEBUG_RENDER)
clog << "<<< JP2_JPIP_Reader::data_request: "
		<< data_request_description (status) << endl;
#endif
return status;
}


int
JP2_JPIP_Reader::metadata_request
	(
	kdu_int32	box_type,
	bool		preemptive
	)
{
#if ((DEBUG) & DEBUG_DATA_ACQUISITION)
clog << ">>> JP2_JPIP_Reader::metadata_request:" << endl
	 << "      box type = " << box_name (box_type)
		<< "/\"" << type_name (box_type) << "\" (0x" << hex
		<< box_type << dec << ')' << endl
	 << "    preemptive = " << preemptive << endl;
#endif
Server_Request.init ();
Posted_Server_Request.init ();
Server_Request.set_metadata_only (true);
Server_Request.add_metareq (box_type, KDU_MRQ_GLOBAL);
int
	status;
try
	{
	//	Initiate the JP2 metadata request.
	#if ((DEBUG) & (DEBUG_DATA_ACQUISITION))
	clog << "    Requesting metadata" << endl;
	#endif
	status = data_request (Server_Request, preemptive);
	if (status == DATA_REQUEST_SUBMITTED)
		//	Wait for the JP2 metadata request to be completed.
		data_acquisition ();
	}
catch (exception& except)
	{
	#if ((DEBUG) & DEBUG_DATA_ACQUISITION)
	clog << "!!! data_acquisition exception -" << endl
		 << except.what () << endl;
	#endif
	status = DATA_REQUEST_REJECTED;
	}
catch (kdu_exception except)
	{
	#if ((DEBUG) & DEBUG_DATA_ACQUISITION)
	clog << "!!! data_acquisition kdu_exception - " << except << endl
		 << Kakadu_error_message (except) << endl;
	#endif
	status = DATA_REQUEST_REJECTED;
	}
catch (...)
	{
	#if ((DEBUG) & DEBUG_DATA_ACQUISITION)
	clog << "!!! data_acquisition unknown exception!" << endl;
	#endif
	status = DATA_REQUEST_REJECTED;
	}
#if ((DEBUG) & DEBUG_DATA_ACQUISITION)
clog << "<<< JP2_JPIP_Reader::metadata_request: "
		<< data_request_description (status) << endl;
#endif
return status;
}


int
JP2_JPIP_Reader::data_request
	(
	kdu_window&	server_request,
	bool		preemptive
	)
{
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
clog << ">>> JP2_JPIP_Reader::data_request: preemptive "
		<< boolalpha << preemptive << endl
	 << "    Requesting data from Connection_ID " << Connection_ID << endl
	 << server_request;
#endif
int
	status = DATA_REQUEST_SUBMITTED;
kdu_window_prefs
	*request_preferences = Server_Preferences;
if (server_request.get_metadata_only ())
	{
	request_preferences = NULL;
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
	clog << "    metadata only; no Server_Preferences" << endl;
	#endif
	}
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
else
	{
	if (request_preferences)
		clog << *request_preferences;
	else
		clog << "    no Server_Preferences" << endl;
	}
#endif

//	Copy the server request for comparison with what is acquired later.
Posted_Server_Request.copy_from (server_request);

bool
	connected = true,
	requested =
		JPIP_Client->post_window
			(&server_request, Connection_ID, preemptive, request_preferences);
if (! requested)
	{
	connected = JPIP_Client->is_alive (Connection_ID);
	if (! connected)
		{
		//	Server disconnection.
		close (Connection_ID);

		if (! Reconnecting)
			{
			//	Try to reconnect.
			kdu_window
				original_server_request;
			original_server_request.copy_from (server_request);
			if ((connected = reconnect ()))
				{
				//	Reconnected. Try the request again.
				requested =
					JPIP_Client->post_window (&original_server_request,
						Connection_ID, preemptive, request_preferences);
				if (! requested &&
					! (connected = JPIP_Client->is_alive (Connection_ID)))
					close (Connection_ID);
				}
			//	Restore the original request.
			Posted_Server_Request.copy_from (original_server_request);
			}
		}
	if (! connected)
		{
		ostringstream
			message;
		message
			<< "Request for "
				<< (Posted_Server_Request.get_metadata_only () ?
					"codestream " : "meta")
				<< "data from JPIP client failed." << endl;
		if (Posted_Server_Request.get_metadata_only ())
			message
				<< "Could not connect to the JPIP server.";
		else
			message
				<< "Requested region "
					<< Posted_Server_Request.region.pos.x << "x, "
					<< Posted_Server_Request.region.pos.y << "y, "
					<< Posted_Server_Request.region.size.x << "w, "
					<< Posted_Server_Request.region.size.y << 'h'
				<< " at resolution level "
					<< resolution_level () << '.' << endl
				<< "The JPIP server disconnected.";
		throw JPIP_Disconnected (message.str (), ID);
		}
	if (! requested)
		{
		//	Check the status of the request.
		connected = JPIP_Client->get_window_in_progress
				(&Server_Request, Connection_ID, &status);
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    Request rejected - in progress " << connected
				<< ", request_status: "
				<< server_request_status_description (status) << endl
			 << Server_Request;
		#endif
		if (status & KDU_CLIENT_WINDOW_IS_COMPLETE)
			status = DATA_REQUEST_SATISFIED;
		else
			status = DATA_REQUEST_REJECTED;
		}
	}
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
clog << "<<< JP2_JPIP_Reader::data_request: "
		<< data_request_description (status) << endl;
#endif
return status;
}


int
JP2_JPIP_Reader::data_acquisition
	(
	Acquired_Data*	acquired_data
	)
{
#if ((DEBUG) & (DEBUG_RENDER | \
				DEBUG_TIMING | \
				DEBUG_DATA_ACQUISITION | \
				DEBUG_OVERVIEW))
clog << ">>> JP2_JPIP_Reader::data_acquisition" << endl;
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
clock_t
	start_clock = clock (),
	end_clock;
struct timeval
	begin_time,
	end_time;
gettimeofday (&begin_time, 0);
#endif	//	!_WIN32
#endif
int
	status = 0;
unsigned int
	wait;
bool
	notice_received,
	in_progress,
	canceled = false;
Rendering_Monitor*
	monitor = rendering_monitor ();	//	Where to send notififcations.
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
clog << "    Rendering_Monitor @ " << (void*)monitor << endl;
#endif

//	Wait for data to arrive from the server.
wait = JPIP_request_timeout ();
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
clog << "    Waiting " << wait << " seconds for JPIP_Client "
		<< (Posted_Server_Request.get_metadata_only () ? "" : "meta")
		<< "data to arrive" << endl
	 << "    from connection ID " << Connection_ID
	 << " - " << JPIP_Client->get_target_name () << endl;
#endif
Wait_for_Data:
do
	{
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
	clog << "+-+ " << wait << " - begin Provider_Event wait" << endl;
	#endif
	//	Wait one of the wait seconds.
	Provider_Mutex.lock ();
	notice_received = Provider_Event.timed_wait (Provider_Mutex, 1000000);
	Provider_Event.reset ();
	Provider_Mutex.unlock ();
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
	clog << "-+- " << wait << " - end Provider_Event wait" << endl
		 << "          notice_received = "
		 	<< boolalpha << notice_received << endl;
	#endif

	in_progress = JPIP_Client->is_alive (Connection_ID);
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
	clog << "    JPIP client is alive = " << boolalpha << in_progress << endl;
	#endif
	if (notice_received ||
		! in_progress)
		break;

	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
	if (monitor)
		clog << "    sending WAITING_MESSAGE notification" << endl;
	#endif
	if (monitor &&
		//	Give the monitor an opportunity to cancel.
		! monitor->notify (*this,
			Rendering_Monitor::INFO_ONLY, WAITING_MESSAGE))
		{
		//	Waiting canceled.
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    monitor notification responded cancel" << endl;
		#endif
		canceled = true;
		break;
		}
	}
	while (--wait);

if (! canceled)
	{
	if (! in_progress)
		{
		//	Server disconnection.
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    JPIP server disconnection" << endl;
		#endif
		close (Connection_ID);

		if (! Reconnecting)
			{
			//	Try to reconnect.
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
			clog << "    try to reconnect ..." << endl;
			#endif
			kdu_window
				original_server_request;
			original_server_request.copy_from (Posted_Server_Request);
			if ((in_progress = reconnect ()))
				{
				//	Reconnected. Try the request again.
				#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
				clog << "    reconnected; resubmitted the data_request" << endl;
				#endif
				try
					{
					status =
						data_request (original_server_request, true);
					if (status == DATA_REQUEST_SUBMITTED)
						{
						//	Start waiting for data to arrive again.
						wait = JPIP_request_timeout ();
						goto Wait_for_Data;
						}
					if (status == DATA_REQUEST_REJECTED)
						{
						//	DATA_REQUEST_REJECTED:
						ostringstream
							message;
						message
							<< "Acquisition of "
								<< (Connection_Completed ? "" : "meta")
								<< "data from the JPIP server failed." << endl
							<< "The data request was rejected"
								" after reconnecting to the server.";
						throw JPIP_Exception (message.str (), ID);
						}
					}
				catch (JPIP_Disconnected)
					{in_progress = false;}
				}
			}
		if (! in_progress)
			{
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
			clog << "    server connection failed" << endl;
			#endif
			ostringstream
				message;
			message
				<< "Acquistion of " << (Connection_Completed ? "" : "meta")
					<< "data from the JPIP server failed." << endl
				<< (Connection_Completed ?
					"The JPIP server disconnected." :
					"Could not connect to the JPIP server.");
			throw JPIP_Disconnected (message.str (), ID);
			}
		}
	else
	if (! wait)
		{
		//	Server response timeout.
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    JPIP server timeout after " << JPIP_request_timeout ()
				<< " seconds" << endl;
		#endif
		if (monitor)
			{
			#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
			clog << "    sending TIMEOUT_MESSAGE notification" << endl;
			#endif
			monitor->notify (*this,
				Rendering_Monitor::INFO_ONLY, TIMEOUT_MESSAGE);
			}
		close (Connection_ID);
		ostringstream
			message;
		message
			<< "Acquisition of " << (Connection_Completed ? "" : "meta")
				<< "data from the JPIP server failed." << endl
			<< "Timeout after waiting " << JPIP_request_timeout ()
				<< " seconds for data from the JPIP server.";
		throw JPIP_Timeout (message.str (), ID);
		}
	else
	if (monitor &&
		wait != JPIP_request_timeout ())
		{
		//	A WAITING_MESSAGE was sent; send ACQUIRED_DATA_MESSAGE.
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    sending ACQUIRED_DATA_MESSAGE notification" << endl;
		#endif
		canceled = ! monitor->notify (*this,
			Rendering_Monitor::INFO_ONLY, ACQUIRED_DATA_MESSAGE);
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		if (canceled)
			clog << "    monitor notification responded cancel" << endl;
		#endif
		}
	}

if (canceled &&
	monitor)
	{
	#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
	clog << "    sending CANCELED notification" << endl;
	#endif
	monitor->notify (*this,
		Rendering_Monitor::CANCELED,
		Rendering_Monitor::Status_Message[Rendering_Monitor::CANCELED]);
	}

//	Get the status of the request.
in_progress =
	JPIP_Client->get_window_in_progress
		(&Server_Request, Connection_ID, &status);
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
clog << "      received_bytes: "
		<< JPIP_Client->get_received_bytes (Connection_ID) << endl
	 << "      request in progress - " << boolalpha << in_progress
		<< ", request_status: "
		<< server_request_status_description (status) << endl
	 << "       client status: "
		<< JPIP_Client->get_status (Connection_ID) << endl;
#if ((DEBUG) & DEBUG_RENDER)
clog << "               alive: "
		<< JPIP_Client->is_alive (Connection_ID) << endl
	 << "              active: "
		<< JPIP_Client->is_active () << endl
	 << "                idle: "
		<< JPIP_Client->is_idle (Connection_ID) << endl
	 << "      posted server request -" << endl
	 << Posted_Server_Request
	 << "      active server request -" << endl
	 << Server_Request;
#endif
#endif

//	Set the return status.
if (JPIP_Client->is_idle (Connection_ID) ||
	((status & KDU_CLIENT_WINDOW_IS_MOST_RECENT) &&
	 (status & KDU_CLIENT_WINDOW_IS_COMPLETE)))
	status = DATA_ACQUISITION_COMPLETE;
else
	{
	status = DATA_ACQUISITION_INCOMPLETE;
	if (! canceled &&
		Posted_Server_Request.get_metadata_only ())
		{
		//	Continue waiting for metadata.
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    incomplete metadata only request" << endl;
		#endif
		goto Wait_for_Data;
		}
	}

if (canceled)
	{
	status |= DATA_ACQUISITION_CANCELED;

	if (monitor)
		{
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    sending CANCELED notification" << endl;
		#endif
		monitor->notify (*this,
			Rendering_Monitor::CANCELED,
			Rendering_Monitor::Status_Message[Rendering_Monitor::CANCELED]);
		}

	try
		{
		status = data_request (NULL, true);
		if (status == DATA_REQUEST_SUBMITTED)
			{
			//	Wait for cancellation to complete.
			wait = JPIP_request_timeout ();
			goto Wait_for_Data;
			}
		if (status == DATA_REQUEST_REJECTED)
			{
			//	DATA_REQUEST_REJECTED:
			ostringstream
				message;
			message
				<< "Cancellation of a JPIP server "
					<< (Posted_Server_Request.get_metadata_only () ?
						"" : "meta")
					<< "data request failed." << endl
				<< "The canellation request was rejected.";
			throw JPIP_Exception (message.str (), ID);
			}
		}
	catch (JPIP_Disconnected)
		{in_progress = false;}

	if (! in_progress)
		{
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    server connection failed" << endl;
		#endif
		ostringstream
			message;
		message
			<< "Acquistion of " << (Connection_Completed ? "" : "meta")
				<< "data from the JPIP server failed." << endl
			<< (Connection_Completed ?
				"The JPIP server disconnected." :
				"Could not connect to the JPIP server.");
		throw JPIP_Disconnected (message.str (), ID);
		}
	}
#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
clog << "    JP2_JPIP_Reader::data_acquisition: "
		<< data_acquisition_description (status) << endl
	 << "    Server request region -" << endl
	 << "        posted = " << Posted_Server_Request.region << endl
	 << "      received = " << Server_Request.region << endl
	 << "    Server request resolution -" << endl
	 << "        posted = " << Posted_Server_Request.resolution << endl
	 << "      received = " << Server_Request.resolution << endl;
#endif
if (! Server_Request.is_empty ())
	{
	//	Acquired data description.
	if (Server_Request.region     != Posted_Server_Request.region ||
		Server_Request.resolution != Posted_Server_Request.resolution)
		status |= DATA_ACQUISITION_REDUCED;

	if (acquired_data)
		{
		unsigned int
			resolution = 0;
		if (Server_Request.resolution.x)
			{
			int
				level = image_width () / Server_Request.resolution.x;
			while (level)
				{
				++resolution;
				level >>= 1;
				}
			}
		acquired_data->acquired (resolution,
			Server_Request.region.pos.x,
			Server_Request.region.pos.y,
			Server_Request.region.size.x,
			Server_Request.region.size.y);
		#if ((DEBUG) & (DEBUG_RENDER | DEBUG_DATA_ACQUISITION))
		clog << "    acquired_data = region " << *acquired_data
			 << " at resolution level " << acquired_data->Resolution_Level
			 	<< " (" << image_width ()
				<< '/' << Server_Request.resolution.x << ')' << endl;
		#endif
		}
	}

if (! Connection_Completed &&
	JPIP_Client->get_received_bytes (Connection_ID) > 0)
	Connection_Completed = true;
#if ((DEBUG) & (DEBUG_RENDER | \
				DEBUG_TIMING | \
				DEBUG_DATA_ACQUISITION | \
				DEBUG_OVERVIEW))
#ifndef _WIN32
//	Procedure timing is not implemented for MS/Windows.
end_clock = clock ();
gettimeofday (&end_time, 0);
if (start_clock != clock_t (-1) &&
	end_clock != clock_t (-1))
	clog << "    acquisition time = "
			<< (double (end_clock - start_clock) / CLOCKS_PER_SEC)
			<< " seconds" << endl;
clog << "            duration = "
		<< duration (&begin_time, &end_time) << endl;
#endif	//	!_WIN32
clog << "<<< JP2_JPIP_Reader::data_acquisition: "
		<< data_acquisition_description (status) << endl;
#endif
return status;
}

/*==============================================================================
	Render
*/
Cube
JP2_JPIP_Reader::render ()
{
#if ((DEBUG) & DEBUG_RENDER)
clog << ">-< JP2_JPIP_Reader::render" << endl;
#endif
//	Reconnect if needed.
if (! reconnect ())
	{
	ostringstream
		message;
	message
		<< "Can't render." << endl
		<< Reconnection_Failure_Message;
	throw JPIP_Disconnected (message.str (), ID);
	}
return JP2_File_Reader::render ();
}


}	//	namespace Kakadu
}	//	namespace HiRISE
}	//	namespace UA
