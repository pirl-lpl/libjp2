#	UA HiRISE libPDS_JP2
#
#	gmake syntax
#
#	CVS ID: $Id: Makefile,v 1.12 2013/06/04 20:45:40 guym Exp $

SUBSYSTEM				=	HiView
TITLE					=	$(SUBSYSTEM) - lib$(LIBRARY)
R := R
MAKEFILE_REVISION		:=	$Revision: 1.12 $

PROJECT_ROOT			=	../../..

64_BIT_BUILDS			=	true
64_BIT_ONLY				=	true
NO_CONFIG_DIR			=	true
NO_DATA_DIR				=	true
NO_LOGS_DIR				=	true

#	The library:

LIBRARY					=	Kakadu_JP2

LIBRARY_SOURCES			=	JP2_File_Reader.cc \
							JP2_JPIP_Reader.cc \
							JP2_Box.cc

#	Libraries:

#	JP2 Reader base classes library.
JP2_READER_ROOT			=	..
JP2_READER_INCLUDE		=	-I$(JP2_READER_ROOT)
JP2_READER_LIBRARY		=	-L$(JP2_READER_ROOT) \
								-lJP2_Reader$(64)

#	idaeim PVL libraries.
PVL_LIBRARY				=	-L$(IDAEIM_ROOT)/lib

#	Kakadu JPEG2000 library.
KAKADU_INCLUDE			=	-I$(KAKADU_ROOT)/include
KAKADU_LIBRARY			=	-L$(KAKADU_ROOT)/lib \
								-lkdu_aux$(64) \
								-lkdu$(64) \
								-lpthread

#	PIRL C++ class library.
PIRL_INCLUDE			=	-I$(PIRL_ROOT)/include
PIRL_LIBRARY			=	-L$(PIRL_ROOT)/lib \
								-lPIRL++$(64)

#	Used to link the shared object library.
LIBRARIES				+= 	$(JP2_READER_LIBRARY) \
							$(PIRL_LIBRARY) \
							$(KAKADU_LIBRARY)


#	Include files:
#	>>> WARNING <<< The INCLUDE token is reserved by MSVC on MS/Windows.
INCLUDES				=	$(JP2_READER_INCLUDE) \
							$(PIRL_INCLUDE) \
							$(KAKADU_INCLUDE)
                                  
#		Local include files.
LOCAL_INCLUDE			=	KDU_dims.hh


#	Targets:

all:
.all:				.libraries

#	DEBUG
debug:				.libraries

.libraries:			$(LOCAL_INCLUDE)


#	Installation:

.install:			.headers_install \
					.bin_install

.bin_install:		.libraries_install


#	Produce universal binaries. Comment out for native host binaries.
#	Note: The x86_64 architecture is not supported by the Qt binary distribution.
#FAT_MAC				?=	-arch i386 \
#						-arch ppc


#	Project Makefile configuration.
include				$(PROJECT_ROOT)/Build/Makefile.conf


#	Preprocessor flags:
ifneq ($(strip $(64_BIT_BUILDS)),)
CPPFLAGS 			+=	-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
endif

#	Compiler flags:
ifeq ($(OS),Darwin)

#	Indirect dynamic library linking:
#
#	The OS X linker will resolve all references, including those to
#	indirect libraries, when the library is being built and the
#	-flat_namespace option is being used. So even though the Kakadu_JP2
#	library does not itself directly reference the contents of the PVL
#	libraries the indirect references through the JP2_Metadata module of
#	the JP2_Reader library will need to be resolved. Thus the PVL
#	libraries must be found in this case.
LIBRARIES				+=	$(PVL_LIBRARY)

#	When building fat Mac binaries:
#		The macosx-version-min is needed to link to the generic
#		_pthread_cond_init symbol instead of the $UNIX2003 qualified
#		symbol which is only available from new version of the system library.
#CXXFLAGS			+=	-mmacosx-version-min=10.4
#LDFLAGS				+=	-mmacosx-version-min=10.4
endif
