#	UA HiRISE libPDS_JP2
#
#	gmake syntax
#
#	CVS ID: $Id: Makefile,v 1.12 2013/06/04 20:45:40 guym Exp $

# NOTE: set FAT_MAC for multi-arch OS X compiles, e.g.
# setenv FAT_MAC "-arch i386 -arch x86_64 -arch ppc"
#
SUBSYSTEM				=	HiView
TITLE					=	$(SUBSYSTEM) - lib$(LIBRARY)
R := R
MAKEFILE_REVISION		:=	$Revision: 1.12 $

PROJECT_ROOT			=	../..

SUBDIRECTORIES			=	$(KAKADU_JP2_READER_ROOT)

64_BIT_BUILDS			=	true
64_BIT_ONLY				=	true
NO_CONFIG_DIR			=	true
NO_DATA_DIR				=	true
NO_LOGS_DIR				=	true

#	The library:

LIBRARY					=	JP2

LIBRARY_SOURCES			:=	JP2.cc \
							JP2_Utilities.cc

#	Subdirectory cross dependencies.
#
#	The JP2_Reader implementation in the Kakadu subdirectory depends on
#	the libJP2_Reader library. Thus it is necessary that this library be
#	built first before the subdirectory library is built.
#
#	The libJP2 library depends on the library built in the subdirectory.
#	Thus it is necessary that this library be built after the
#	subdirectory library.
#
#	Building two libraries, with their distinct object modules, in a
#	single Makefile is not supported by the Makefile.conf rules. So the
#	libJP2_Reader library is built with a recursive make invocation using
#	its own JP2_READER_MAKEFILE.
JP2_READER_MAKEFILE			:=	Makefile_JP2_Reader


#	Libraries:

#	Local JP2_Reader library.
JP2_READER_LIBRARY			=	-L. -lJP2_Reader$(64)

#	idaeim PVL libraries.
PVL_INCLUDE				=	-I$(IDAEIM_ROOT)/include
PVL_LIBRARY				=	-L$(IDAEIM_ROOT)/lib \
								-lPVL$(64) \
								-lString$(64) \
								-lidaeim$(64)

#	PIRL C++ class library.
PIRL_INCLUDE				=	-I$(PIRL_ROOT)/include
PIRL_LIBRARY				=	-L$(PIRL_ROOT)/lib \
									-lPIRL++$(64)

#	Subdirectory library.
KAKADU_JP2_READER_ROOT		=	Kakadu
KAKADU_JP2_READER_INCLUDE	=	-I$(KAKADU_JP2_READER_ROOT)
KAKADU_JP2_READER_LIBRARY	=	-L$(KAKADU_JP2_READER_ROOT) \
									-lKakadu_JP2$(64)

LIBRARIES					=	$(JP2_READER_LIBRARY) \
								$(KAKADU_JP2_READER_LIBRARY)

#	Kakadu JPEG2000 library.
KAKADU_INCLUDE				=	-I$(KAKADU_ROOT)/include
KAKADU_LIBRARY				=	-L$(KAKADU_ROOT)/lib \
									-lkdu_aux$(64) \
									-lkdu$(64) \
									-lpthread


LIBRARIES					=	$(JP2_READER_LIBRARY) \
								$(PVL_LIBRARY) \
								$(PIRL_LIBRARY) \
								$(KAKADU_JP2_READER_LIBRARY) \
								$(KAKADU_LIBRARY)

#	Include files:
#	>>> WARNING <<< The INCLUDE token is reserved by MSVC on MS/Windows.
INCLUDES					=	$(PVL_INCLUDE) \
								$(PIRL_INCLUDE) \
								$(KAKADU_INCLUDE) \
								$(KAKADU_JP2_READER_INCLUDE)



#	Targets:

all:
.all:				jp2_reader \
					subdirectories \
					.libraries

#	DEBUG
ifneq ($(strip $(DEBUG)),)
export DEBUG
endif
debug:				TARGET = debug
debug:				all

jp2_reader:
	$(MAKE) -f $(JP2_READER_MAKEFILE) $(TARGET)

#	Subdirectories:

subdirectories:		$(SUBDIRECTORIES)
$(SUBDIRECTORIES):
	$(MAKE) -C $@ $(TARGET)

#	Installation:

.install:			TARGET = .install
.install:			jp2_reader \
					subdirectories \
					.headers_install \
					.bin_install \

.bin_install:		TARGET = .bin_install
.bin_install:		jp2_reader \
					subdirectories \
					.libraries_install

.headers_install:	TARGET = .headers_install
.headers_install:	jp2_reader \
					subdirectories

#	Cleaning:

.clean:				TARGET = .clean
.clean:				jp2_reader \
					subdirectories

.dist_clean:		TARGET = .dist_clean
.dist_clean:		jp2_reader \
					subdirectories


#	Project Makefile configuration.
include				$(PROJECT_ROOT)/Build/Makefile.conf


#	Preprocessor flags:
ifneq ($(strip $(64_BIT_BUILDS)),)
CPPFLAGS 			+=	-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
endif


.PHONY:				subdirectories $(SUBDIRECTORIES) jp2_reader
