#	UA HiRISE PDS_JP2 applications
#
#	gmake syntax
#
#	CVS ID: $Id: Makefile,v 1.4 2014/04/30 21:37:31 guym Exp $

SUBSYSTEM				=	HiView
TITLE					=	$(SUBSYSTEM) - $(APPLICATIONS)
R := R
MAKEFILE_REVISION		:=	$Revision: 1.4 $

PROJECT_ROOT			=	../../..

#===============================================================================
#	Host machine architecture identification.
include				$(PROJECT_ROOT)/Build/Makefile.host


64_BIT_BUILDS		=	true
64_BIT_ONLY			=	true
NO_CONFIG_DIR		=	true
NO_DATA_DIR			=	true
NO_LOGS_DIR			=	true


#	Application programs:

APPLICATIONS			=	 test_JP2_Reader test_JPIP_Connect


#	Libraries:

#	Kakadu JPEG2000 libraries.
KAKADU_INCLUDE			=	-I$(KAKADU_ROOT)/include
KAKADU_LIBRARY			=	\
	$(KAKADU_ROOT)/lib/libkdu_aux$(64).$(STATIC_LIBRARY_EXT) \
	$(KAKADU_ROOT)/lib/libkdu$(64).$(STATIC_LIBRARY_EXT)

#	PIRL C++ class library.
PIRL_INCLUDE			=	-I$(PIRL_ROOT)/include
PIRL_LIBRARY			=	\
	$(PIRL_ROOT)/lib/libPIRL++$(64).$(STATIC_LIBRARY_EXT)

#	idaeim PVL class libraries.
PVL_INCLUDE				=	-I$(IDAEIM_ROOT)/include
PVL_LIBRARY				=	-L$(IDAEIM_ROOT)/lib \
								-lPVL$(64) \
								-lString$(64) \
								-lidaeim$(64)

#	The subsystem libraries.
SUBSYSTEM_ROOT			=	..
SUBSYSTEM_INCLUDE		=	-I$(SUBSYSTEM_ROOT) -I$(SUBSYSTEM_ROOT)/Kakadu
SUBSYSTEM_LIBRARIES		=	\
	$(SUBSYSTEM_ROOT)/libJP2$(64).$(STATIC_LIBRARY_EXT) \
	$(SUBSYSTEM_ROOT)/libJP2_Reader$(64).$(STATIC_LIBRARY_EXT)

SUBSYSTEM_KAKADU_ROOT	=	../Kakadu
SUBSYSTEM_KAKADU_INCLUDE=	-I$(SUBSYSTEM_KAKADU_ROOT)
SUBSYSTEM_KAKADU_LIBRARY=	\
	$(SUBSYSTEM_KAKADU_ROOT)/libKakadu_JP2$(64).$(STATIC_LIBRARY_EXT)

ifeq ($(OS), WIN)
#	MS/Windows system libraries.
SYSTEM_LIBRARIES		:=	Wsock32.lib \
							Advapi32.lib \
							Userenv.lib \
							Kernel32.lib \
							user32.lib \
							gdi32.lib \
							winspool.lib \
							comdlg32.lib \
							shell32.lib \
							ole32.lib \
							oleaut32.lib \
							uuid.lib \
							odbc32.lib \
							odbccp32.lib \
							Ws2_32.lib
else
SYSTEM_LIBRARIES		:=	-lpthread
endif


LIBRARIES				+= 	$(SUBSYSTEM_LIBRARIES) \
							$(SUBSYSTEM_KAKADU_LIBRARY) \
							$(PVL_LIBRARY) \
							$(KAKADU_LIBRARY) \
							$(PIRL_LIBRARY) \
							$(SYSTEM_LIBRARIES)

#	Include files:
#	>>> WARNING <<< The INCLUDE token is reserved by MSVC on MS?Windows.
INCLUDES				=	$(SUBSYSTEM_INCLUDE) \
							$(PVL_INCLUDE) \
							$(KAKADU_INCLUDE) \
							$(PIRL_INCLUDE)


#	Preprocessor flags:

ifneq ($(strip $(64_BIT_BUILDS)),)
CPPFLAGS 					+=	-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
endif

.applications:	CPPFLAGS	+=	-DMODULE_VERSION='"$(SUBSYSTEM_VERSION)"'


#	Targets:

all:
.all:				.applications

#	DEBUG
debug:				.applications

$(APPLICATIONS):	$(SUBSYSTEM_LIBRARY)


.PHONY:	all .all debug

#	Project Makefile configuration.
include				$(PROJECT_ROOT)/Build/Makefile.conf
