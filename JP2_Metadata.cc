/*	JP2_Metadata

HiROC CVS ID: $Id: JP2_Metadata.cc,v 2.3 2012/09/28 14:01:20 castalia Exp $

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

#include	"JP2_Metadata.hh"

#include	"JP2_Exception.hh"

#include	"PVL.hh"
using idaeim::PVL::Parameter;
using idaeim::PVL::Aggregate;
using idaeim::PVL::Assignment;
using idaeim::PVL::Value;
using idaeim::PVL::Integer;
using idaeim::PVL::Real;
using idaeim::PVL::String;
using idaeim::PVL::Array;
using idaeim::PVL::Parser;
using idaeim::PVL::Lister;

#include	"endian.hh"
using PIRL::MSB_native;

#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::hex;
using std::setfill;
using std::setw;
using std::uppercase;
using std::endl;
#include	<cmath>
#include    <stdexcept>
using std::invalid_argument;
using std::out_of_range;
#include	<cstring>


#if defined (DEBUG)
/*	DEBUG controls

	DEBUG report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_ACCESSORS		(1 << 1)
#define DEBUG_MANIPULATORS	(1 << 2)
#define DEBUG_HELPERS		(1 << 3)
#define DEBUG_PVL			(1 << 4)
#define	DEBUG_GET_VALUE		(1 << 5)

#include	<iostream>
using std::clog;
using std::boolalpha;
using std::dec;
#endif  // DEBUG


namespace UA
{
namespace HiRISE
{
/*******************************************************************************
	JP2_Metadata
*/
/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
const char* const
	JP2_Metadata::ID =
		"UA::HiRISE::JP2_Metadata ($Revision: 2.3 $ $Date: 2012/09/28 14:01:20 $)";

/*------------------------------------------------------------------------------
	JP2 Boxes
*/
const JP2_Metadata::Type_Code
	JP2_Metadata::SIGNATURE_TYPE						= 0x6A502020,
	JP2_Metadata::FILE_TYPE_TYPE						= 0x66747970,
	JP2_Metadata::JP2_HEADER_TYPE						= 0x6A703268,
		JP2_Metadata::IMAGE_HEADER_TYPE					= 0x69686472,
		JP2_Metadata::BITS_PER_COMPONENT_TYPE				= 0x62706363,
		JP2_Metadata::COLOUR_SPECIFICATION_TYPE				= 0x636F6C72,
		JP2_Metadata::PALETTE_TYPE							= 0x70636C72,
		JP2_Metadata::COMPONENT_MAPPING_TYPE				= 0x636D6170,
		JP2_Metadata::CHANNEL_DEFINITION_TYPE				= 0x63646566,
		JP2_Metadata::RESOLUTION_TYPE						= 0x72657320,
			JP2_Metadata::CAPTURE_RESOLUTION_TYPE				= 0x72657363,
			JP2_Metadata::DEFAULT_DISPLAY_RESOLUTION_TYPE		= 0x72657364,
	JP2_Metadata::CONTIGUOUS_CODESTREAM_TYPE			= 0x6A703263,
	JP2_Metadata::INTELLECTUAL_PROPERTY_TYPE			= 0x6A703269,
	JP2_Metadata::XML_TYPE								= 0x786D6C20,
	JP2_Metadata::UUID_TYPE								= 0x75756964,
	JP2_Metadata::UUID_INFO_TYPE						= 0x75696E66,
		JP2_Metadata::UUID_LIST_TYPE						= 0x756C7374,
		JP2_Metadata::URL_TYPE								= 0x75726C20,
	JP2_Metadata::ASSOCIATION_TYPE						= 0x61736F63,
	JP2_Metadata::LABEL_TYPE							= 0X6C626C20,
	JP2_Metadata::PLACEHOLDER_TYPE						= 0x70686C64,

	JP2_Metadata::INVALID_CODE
		= (JP2_Metadata::Type_Code)-1;

namespace
{
struct Code_Name
	{
	//!	Box type code.
	JP2_Metadata::Type_Code		code;
	//!	Box description name.
	JP2_Metadata::Name_String	name;
	};

static const Code_Name
	CODE_NAMES[] =
	{
	{JP2_Metadata::SIGNATURE_TYPE,					"Signature"},
	{JP2_Metadata::FILE_TYPE_TYPE,					"File_Type"},
	{JP2_Metadata::JP2_HEADER_TYPE,					"JP2_Header"},
	{JP2_Metadata::IMAGE_HEADER_TYPE,				"Image_Header"},
	{JP2_Metadata::BITS_PER_COMPONENT_TYPE,			"Bits_Per_Component"},
	{JP2_Metadata::COLOUR_SPECIFICATION_TYPE,		"Colour_Specification"},
	{JP2_Metadata::PALETTE_TYPE,					"Palette"},
	{JP2_Metadata::COMPONENT_MAPPING_TYPE,			"Component_Mapping"},
	{JP2_Metadata::CHANNEL_DEFINITION_TYPE,			"Channel_Definition"},
	{JP2_Metadata::RESOLUTION_TYPE,					"Resolution"},
	{JP2_Metadata::CAPTURE_RESOLUTION_TYPE,			"Capture_Resolution"},
	{JP2_Metadata::DEFAULT_DISPLAY_RESOLUTION_TYPE,	"Default_Display_Resolution"},
	{JP2_Metadata::CONTIGUOUS_CODESTREAM_TYPE,		"Contiguous_Codestream"},
	{JP2_Metadata::INTELLECTUAL_PROPERTY_TYPE,		"Intellectual_Property"},
	{JP2_Metadata::XML_TYPE,						"XML"},
	{JP2_Metadata::UUID_TYPE,						"UUID"},
	{JP2_Metadata::UUID_INFO_TYPE,					"UUID_Info"},
	{JP2_Metadata::UUID_LIST_TYPE,					"UUID_List"},
	{JP2_Metadata::URL_TYPE,						"URL"},
	{JP2_Metadata::ASSOCIATION_TYPE,				"Association"},
	{JP2_Metadata::LABEL_TYPE,						"Label"},
	{JP2_Metadata::PLACEHOLDER_TYPE,				"Placeholder"},

	{JP2_Metadata::INVALID_CODE,					"Unknown"},
	{0, NULL}
	};
}

JP2_Metadata::Name_String
	JP2_Metadata::UNKNOWN_NAME					= "Unknown";

const int
	JP2_Metadata::INTEGER_FIELD_SIZE			= 4,
	JP2_Metadata::LONG_INTEGER_FIELD_SIZE		= 8;

JP2_Metadata::Name_String
	JP2_Metadata::NAME_PARAMETER				= "Name",
	JP2_Metadata::TYPE_PARAMETER				= "Type",
	JP2_Metadata::LENGTH_PARAMETER				= "Length",
	JP2_Metadata::POSITION_PARAMETER			= "^Position";

JP2_Metadata::Name_String
	JP2_Metadata::SIGNATURE_PARAMETER			= "Signature";
const unsigned char
	JP2_Metadata::JP2_SIGNATURE[12] =
		{
		0x00, 0x00, 0x00, 0x0C,		//	Box size: 12 bytes
		0x6A, 0x50, 0x20, 0x20,		//	Signature: "jP  "
		0x0D, 0x0A, 0x87, 0x0A		//	Content: <CR><LF>0x87<LF>
		};
const unsigned int
	JP2_Metadata::SIGNATURE_CONTENT				= 0x0D0A870A;

JP2_Metadata::Name_String
	JP2_Metadata::DATA_POSITION_PARAMETER		= "^Data_Position",
	JP2_Metadata::DATA_LENGTH_PARAMETER			= "Data_Length",
	JP2_Metadata::VALUE_BITS_PARAMETER			= "Value_Bits";

JP2_Metadata::Name_String
	JP2_Metadata::BRAND_PARAMETER				= "Brand",
	JP2_Metadata::MINOR_VERSION_PARAMETER		= "Minor_Version",
	JP2_Metadata::COMPATIBILITY_LIST_PARAMETER	= "Compatibility",
	JP2_Metadata::JP2_COMPATIBLE				= "jp2 ";

JP2_Metadata::Name_String
	JP2_Metadata::HEIGHT_PARAMETER				= "Height",
	JP2_Metadata::WIDTH_PARAMETER				= "Width",
	JP2_Metadata::IMAGE_BANDS_PARAMETER			= "Image_Bands",
	JP2_Metadata::COMPRESSION_TYPE_PARAMETER	= "Compression_Type",
	JP2_Metadata::COLORSPACE_UNKNOWN_PARAMETER	= "Colorspace_Unknown",
	JP2_Metadata::INTELLECTUAL_PROPERTY_PARAMETER
		= "Intellectual_Property_Rights";

JP2_Metadata::Name_String
	JP2_Metadata::SPECIFICATION_METHOD_PARAMETER
		= "Specification_Method",
	JP2_Metadata::PRECEDENCE_PARAMETER
		= "Precedence",
	JP2_Metadata::COLOURSPACE_APPROXIMATION_PARAMETER
		= "Colourspace_Approximation",
	JP2_Metadata::ENUMERATED_COLOURSPACE_PARAMETER
		= "Enumerated_Colourspace";
const int
	JP2_Metadata::ENUMERATED_COLOURSPACE		= 1,
	JP2_Metadata::RESTRICTED_ICC_PROFILE		= 2;

JP2_Metadata::Name_String
	JP2_Metadata::ENTRIES_PARAMETER				= "Entries",
	JP2_Metadata::COLUMNS_PARAMETER				= "Columns",
	JP2_Metadata::VALUES_PARAMETER				= "Values";

JP2_Metadata::Name_String
	JP2_Metadata::MAP_PARAMETER					= "Map";
const int
	JP2_Metadata::COMPONENT_INDEX				= 0,
	JP2_Metadata::MAP_TYPE_INDEX				= 1,
	JP2_Metadata::PALETTE_INDEX					= 2,
	JP2_Metadata::DIRECT_USE					= 0,
	JP2_Metadata::PALETTE_MAPPING				= 1,
	JP2_Metadata::CHANNEL_INDEX					= 0,
	JP2_Metadata::CHANNEL_TYPE_INDEX			= 1,
	JP2_Metadata::CHANNEL_ASSOCIATION_INDEX		= 2,
	JP2_Metadata::COLOUR_IMAGE_DATA				= 0,
	JP2_Metadata::OPACITY						= 1,
	JP2_Metadata::PREMULTIPLIED_OPACITY			= 2,
	JP2_Metadata::IMAGE_ASSOCIATION				= 0,
	JP2_Metadata::NO_ASSOCIATION				= 0xFFFF;

JP2_Metadata::Name_String
	JP2_Metadata::VERTICAL_NUMERATOR_PARAMETER		= "Vertical_Numerator",
	JP2_Metadata::VERTICAL_DENOMINATOR_PARAMETER	= "Vertical_Denominator",
	JP2_Metadata::VERTICAL_EXPONENT_PARAMETER		= "Vertical_Exponent",
	JP2_Metadata::HORIZONTAL_NUMERATOR_PARAMETER	= "Horizontal_Numerator",
	JP2_Metadata::HORIZONTAL_DENOMINATOR_PARAMETER	= "Horizontal_Denominator",
	JP2_Metadata::HORIZONTAL_EXPONENT_PARAMETER		= "Horizontal_Exponent";

JP2_Metadata::Name_String
	JP2_Metadata::CODESTREAM_PARAMETER			= "Codestream";

JP2_Metadata::Name_String
	JP2_Metadata::UUID_PARAMETER				= "UUID";
const int
	JP2_Metadata::UUID_SIZE						= 16;

JP2_Metadata::Name_String
	JP2_Metadata::VERSION_PARAMETER				= "Version",
	JP2_Metadata::FLAGS_PARAMETER				= "Flags",
	JP2_Metadata::URL_PARAMETER					= "URL";

JP2_Metadata::Name_String
	JP2_Metadata::TEXT_PARAMETER				= "Text";

JP2_Metadata::Name_String
	JP2_Metadata::ORIGINAL_BOX_PARAMETER		= "Original_Box",
	JP2_Metadata::BIN_ID_PARAMETER				= "Bin_ID",
	JP2_Metadata::EQUIVALENT_BOX_PARAMETER		= "Equivalent_Box",
	JP2_Metadata::CODESTREAM_ID_PARAMETER		= "Codestream_ID",
	JP2_Metadata::TOTAL_CODESTREAMS_PARAMETER	= "Total_Codestreams",
	JP2_Metadata::EXTENDED_BOX_LIST_PARAMETER	= "Extended_Box_List";
const int
	JP2_Metadata::PLACEHOLDER_FLAGS_ORIGINAL_MASK				= (1 << 0),
	JP2_Metadata::PLACEHOLDER_FLAGS_EQUIVALENT_MASK				= (1 << 1),
	JP2_Metadata::PLACEHOLDER_FLAGS_CODESTREAM_MASK				= (1 << 2),
	JP2_Metadata::PLACEHOLDER_FLAGS_MULTIPLE_CODESTREAM_MASK	= (1 << 3);

/*------------------------------------------------------------------------------
	Codestream Segments
*/
const JP2_Metadata::Marker_Code

	//	Delimiters:
	JP2_Metadata::SOC_MARKER						= 0xFF4F,
	JP2_Metadata::SOD_MARKER						= 0xFF93,
	JP2_Metadata::EPH_MARKER						= 0xFF92,
	JP2_Metadata::EOC_MARKER						= 0xFFD9,

	//	Fixed information.
	JP2_Metadata::SIZ_MARKER						= 0xFF51,

	//	Functional.
	JP2_Metadata::COD_MARKER						= 0xFF52,
	JP2_Metadata::COC_MARKER						= 0xFF53,
	JP2_Metadata::RGN_MARKER						= 0xFF5E,
	JP2_Metadata::QCD_MARKER						= 0xFF5C,
	JP2_Metadata::QCC_MARKER						= 0xFF5D,
	JP2_Metadata::POC_MARKER						= 0xFF5F,
	JP2_Metadata::SOT_MARKER						= 0xFF90,

	//	Pointer.
	JP2_Metadata::TLM_MARKER						= 0xFF55,
	JP2_Metadata::PLM_MARKER						= 0xFF57,
	JP2_Metadata::PLT_MARKER						= 0xFF58,
	JP2_Metadata::PPM_MARKER						= 0xFF60,
	JP2_Metadata::PPT_MARKER						= 0xFF61,

	//	Bitstream.
	JP2_Metadata::SOP_MARKER						= 0xFF91,

	//	Informational.
	JP2_Metadata::CRG_MARKER						= 0xFF63,
	JP2_Metadata::COM_MARKER						= 0xFF64,

	//	Reserved.
	JP2_Metadata::RESERVED_DELIMITER_MARKER_MIN		= 0xFF30,
	JP2_Metadata::RESERVED_DELIMITER_MARKER_MAX		= 0xFF3F;

namespace
{
struct Marker_Name
	{
	//!	Segment marker code.
	JP2_Metadata::Marker_Code	marker;
	//!	Segment description name.
	JP2_Metadata::Name_String	name;
	};

static const Marker_Name
	MARKER_NAMES[] =
	{
	//	Delimiting markers and marker segments.
	{JP2_Metadata::SOC_MARKER, "Start_of_Codestream"},
	{JP2_Metadata::SOD_MARKER, "Start_of_Data"},
	{JP2_Metadata::EPH_MARKER, "End_of_Packet_Header"},
	{JP2_Metadata::EOC_MARKER, "End_of_Codestream"},

	//	Fixed information marker segments.
	{JP2_Metadata::SIZ_MARKER, "Size"},

	//	Functional marker segments.
	{JP2_Metadata::COD_MARKER, "Coding_Style_Default"},
	{JP2_Metadata::COC_MARKER, "Coding_Style_Component"},
	{JP2_Metadata::RGN_MARKER, "Region_of_Interest"},
	{JP2_Metadata::QCD_MARKER, "Quantization_Default"},
	{JP2_Metadata::QCC_MARKER, "Quantization_Component"},
	{JP2_Metadata::POC_MARKER, "Progression_Order_Change"},
	{JP2_Metadata::SOT_MARKER, "Start_of_Tile_Part"},

	//	Pointer marker segments.
	{JP2_Metadata::TLM_MARKER, "Tile_Lengths"},
	{JP2_Metadata::PLM_MARKER, "Packet_Length_Main"},
	{JP2_Metadata::PLT_MARKER, "Packet_Length_Tile"},
	{JP2_Metadata::PPM_MARKER, "Packed_Packet_Main"},
	{JP2_Metadata::PPT_MARKER, "Packed_Packet_Tile"},

	//	In-bit-stream markers and marker segments.
	{JP2_Metadata::SOP_MARKER, "Start_of_Packet"},

	//	Information marker segments.
	{JP2_Metadata::CRG_MARKER, "Component_Registration"},
	{JP2_Metadata::COM_MARKER, "Comment"},
	{0, NULL}
	};
}

//!	Marker code parameter.
JP2_Metadata::Name_String
	JP2_Metadata::MARKER_PARAMETER					= "Marker";

//!	SOT parameters.
JP2_Metadata::Name_String
	JP2_Metadata::TILE_INDEX_PARAMETER				= "Tile_Index",
	JP2_Metadata::TILE_PART_LENGTH_PARAMETER		= "Tile_Part_Length",
	JP2_Metadata::TILE_PART_INDEX_PARAMETER			= "Tile_Part_Index",
	JP2_Metadata::TOTAL_TILE_PARTS_PARAMETER		= "Total_Tile_Parts";

//!	SIZ parameters.
JP2_Metadata::Name_String
	JP2_Metadata::CAPABILITY_PARAMETER
		= "Capability",
	JP2_Metadata::REFERENCE_GRID_WIDTH_PARAMETER
		= "Reference_Grid_Width",
	JP2_Metadata::REFERENCE_GRID_HEIGHT_PARAMETER
		= "Reference_Grid_Height",
	JP2_Metadata::HORIZONTAL_IMAGE_OFFSET_PARAMETER
		= "Horizontal_Image_Offset",
	JP2_Metadata::VERTICAL_IMAGE_OFFSET_PARAMETER
		= "Vertical_Image_Offset",
	JP2_Metadata::TILE_WIDTH_PARAMETER
		= "Tile_Width",
	JP2_Metadata::TILE_HEIGHT_PARAMETER
		= "Tile_Height",
	JP2_Metadata::HORIZONTAL_TILE_OFFSET_PARAMETER
		= "Horizontal_Tile_Offset",
	JP2_Metadata::VERTICAL_TILE_OFFSET_PARAMETER
		= "Vertical_Tile_Offset",
	JP2_Metadata::HORIZONTAL_SAMPLE_SPACING_PARAMETER
		= "Horizontal_Sample_Spacing",
	JP2_Metadata::VERTICAL_SAMPLE_SPACING_PARAMETER
		= "Vertical_Sample_Spacing";

//!	COD parameters.
JP2_Metadata::Name_String
	JP2_Metadata::CODING_STYLE_PARAMETER
		= "Coding_Style",
	JP2_Metadata::PROGRESSION_ORDER_PARAMETER
		= "Progression_Order",
	JP2_Metadata::QUALITY_LAYERS_PARAMETER
		= "Quality_Layers",
	JP2_Metadata::MULTIPLE_COMPONENT_TRANSFORM_PARAMETER
		= "Multiple_Component_Transform",
	JP2_Metadata::RESOLUTION_LEVELS_PARAMETER
		= "Resolution_Levels",
	JP2_Metadata::CODE_BLOCK_WIDTH_PARAMETER
		= "Code_Block_Width",
	JP2_Metadata::CODE_BLOCK_HEIGHT_PARAMETER
		= "Code_Block_Height",
	JP2_Metadata::CODE_BLOCK_STYLE_PARAMETER
		= "Code_Block_Style",
	JP2_Metadata::TRANSFORM_PARAMETER
		= "Transform",
	JP2_Metadata::PRECINCT_SIZE_PARAMETER
		= "Precinct_Size";
//!	Coding style bit flag masks.
const int
	JP2_Metadata::ENTROPY_CODER_STYLE				= 1 << 0,
	JP2_Metadata::SOP_STYLE							= 1 << 1,
	JP2_Metadata::EPH_STYLE							= 1 << 2;
//!	Progression order values.
const int
	JP2_Metadata::LRCP_PROGRESSION_ORDER			= 0,
	JP2_Metadata::RLCP_PROGRESSION_ORDER			= 1,
	JP2_Metadata::RPCL_PROGRESSION_ORDER			= 2,
	JP2_Metadata::PCRL_PROGRESSION_ORDER			= 3,
	JP2_Metadata::CPRL_PROGRESSION_ORDER			= 4;
JP2_Metadata::Name_String
	JP2_Metadata::PROGRESSION_ORDERS[] =
		{
		"Layer-Resolution-Component-Position",
		"Resolution-Layer-Component-Position",
		"Resolution-Position-Component-Layer",
		"Position-Component-Resolution-Layer",
		"Component-Position-Resolution-Layer"
		};
//!	Code block style bit flag masks.
const int
	JP2_Metadata::SELECTIVE_ARITHMETIC_BYPASS_FLAG	= 1 << 0,
	JP2_Metadata::RESET_CONTEXT_PROBABILITIES		= 1 << 1,
	JP2_Metadata::TERMINATION_FLAG					= 1 << 2,
	JP2_Metadata::VERTICALLY_CAUSAL_CONTEXT_FLAG	= 1 << 3,
	JP2_Metadata::PREDICTABLE_TERMINATION_FLAG		= 1 << 4,
	JP2_Metadata::SEGMENTATION_SYMBOLS_FLAG			= 1 << 5;
//!	Transform values.
const int
	JP2_Metadata::TRANSFORM_IRREVERSIBLE			= 0,
	JP2_Metadata::TRANSFORM_REVERSIBLE				= 1;
JP2_Metadata::Name_String
	JP2_Metadata::TRANSFORMS[] =
		{
		"9-7 irreversible filter",
		"5-3 reversible filter"
		};

//!	COC parameters.
JP2_Metadata::Name_String
	JP2_Metadata::COMPONENT_INDEX_PARAMETER			= "Component_Index";

//!	RGN parameters.
JP2_Metadata::Name_String
	JP2_Metadata::ROI_STYLE_PARAMETER				= "ROI_Style",
	JP2_Metadata::IMPLICIT_SHIFT_PARAMETER			= "Implicit_Shift";

//!	QCD parameters.
JP2_Metadata::Name_String
	JP2_Metadata::QUANTIZATION_STYLE_PARAMETER		= "Quantization_Style",
	JP2_Metadata::TOTAL_GUARD_BITS_PARAMETER		= "Total_Guard_Bits",
	JP2_Metadata::STEP_SIZE_PARAMETER				= "Step_Size";
//!	Quantization style bit field masks.
const int
	JP2_Metadata::NO_QUANTIZATION					= 0,
	JP2_Metadata::QUANTIZATION_SCALAR_DERIVED		= 1,
	JP2_Metadata::QUANTIZATION_SCALAR_EXPOUNDED		= 2;

//!	POC parameters.
JP2_Metadata::Name_String
	JP2_Metadata::LEVEL_INDEX_PARAMETER				= "Level_Index",
	JP2_Metadata::LAYER_INDEX_PARAMETER				= "Layer_Index";

//!	TLM parameters.
JP2_Metadata::Name_String
	JP2_Metadata::INDEX_PARAMETER					= "Index",
	JP2_Metadata::TILE_INDEX_SIZE_PARAMETER			= "Tile_Index_Size",
	JP2_Metadata::TILE_PART_LENGTH_SIZE_PARAMETER	= "Tile_Part_Length_Size";

//!	PLM and PLT parameters.
JP2_Metadata::Name_String
	JP2_Metadata::PACKET_LENGTH_PARAMETER			= "Packet_Length",
	JP2_Metadata::CONTINUATION_PARAMETER			= "Continuation";

//!	CRG parameters.
JP2_Metadata::Name_String
	JP2_Metadata::HORIZONTAL_COMPONENT_OFFSET_PARAMETER
		= "Horizontal_Component_Offset",
	JP2_Metadata::VERTICAL_COMPONENT_OFFSET_PARAMETER
		= "Vertical_Component_Offset";

//!	COM parameters.
JP2_Metadata::Name_String
	JP2_Metadata::DATA_TYPE_PARAMETER				= "Data_Type",
	JP2_Metadata::BINARY_DATA_PARAMETER				= "Binary_Data",
	JP2_Metadata::TEXT_DATA_PARAMETER				= "Text_Data";
//!	Data type values.
const int
	JP2_Metadata::DATA_TYPE_BINARY					= 0,
	JP2_Metadata::DATA_TYPE_TEXT					= 1;

/*==============================================================================
	Constructors:
*/
JP2_Metadata::JP2_Metadata ()
	:
	//	JP2 box values.
	Image_Bands (0),
	Pixel_Precision (NULL),
	Producer_UUID (NULL),
	Label_URL (),

	//	Codestream segment values.
	Pixel_Width (NULL),
	Pixel_Height (NULL),
	Total_Tiles (0),
	Quality_Layers (0),
	Resolution_Levels (0),
	Progression_Order (-1),
	Transform (-1),

	//	PVL.
	Parameters (new Aggregate (Parser::CONTAINER_NAME)),
	Codestream_Parameters (NULL),
	PLM_Packet_Length_Array (NULL),
	PLT_Packet_Length_Array (NULL),
	PLM_Packet_Length (0),
	PLT_Packet_Length (0),
	PLM_Packet_Length_Bytes_Remaining (0),
	PLT_Packet_Length_Bytes_Remaining (0),
	Data_Buffer (NULL),
	Data_Amount (-1),
	JP2_Validity (0),
	Codestream_Validity (0)
{
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_Metadata" << endl;
#endif
}


JP2_Metadata::JP2_Metadata
	(
	const JP2_Metadata&	JP2_metadata
	)
	:
	Source_Name (JP2_metadata.Source_Name),
	Image_Size (JP2_metadata.Image_Size),
	Image_Bands (JP2_metadata.Image_Bands),
	Pixel_Precision (NULL),
	Producer_UUID (NULL),
	Label_URL (JP2_metadata.Label_URL),

	Reference_Grid_Size (JP2_metadata.Reference_Grid_Size),
	Tile_Size (JP2_metadata.Tile_Size),
	Image_Offsets (JP2_metadata.Image_Offsets),
	Tile_Offsets (JP2_metadata.Tile_Offsets),
	Pixel_Width (NULL),
	Pixel_Height (NULL),
	Total_Tiles (JP2_metadata.Total_Tiles),
	Quality_Layers (JP2_metadata.Quality_Layers),
	Resolution_Levels (JP2_metadata.Resolution_Levels),
	Progression_Order (JP2_metadata.Progression_Order),
	Transform (JP2_metadata.Transform),

	Parameters (new Aggregate (*JP2_metadata.Parameters)),
	Codestream_Parameters (NULL),
	PLM_Packet_Length_Array (NULL),
	PLT_Packet_Length_Array (NULL),
	PLM_Packet_Length (0),
	PLT_Packet_Length (0),
	PLM_Packet_Length_Bytes_Remaining (0),
	PLT_Packet_Length_Bytes_Remaining (0),
	Data_Buffer (NULL),
	Data_Amount (-1),
	JP2_Validity (JP2_metadata.JP2_Validity)
{
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">-< JP2_Metadata: Copy " << JP2_metadata.Source_Name << endl;
#endif
if (JP2_metadata.Pixel_Precision)
	{
	Pixel_Precision = new int[Image_Bands];
	memcpy (Pixel_Precision, JP2_metadata.Pixel_Precision,
		Image_Bands * sizeof (int));
	}
if (JP2_metadata.Producer_UUID)
	{
	Producer_UUID = new unsigned char[UUID_SIZE];
	memcpy (Producer_UUID, JP2_metadata.Producer_UUID, UUID_SIZE);
	}
if (JP2_metadata.Pixel_Width)
	{
	Pixel_Width = new unsigned int[Image_Bands];
	memcpy (Pixel_Width, JP2_metadata.Pixel_Width,
		Image_Bands * sizeof (int));
	}
if (JP2_metadata.Pixel_Height)
	{
	Pixel_Height = new unsigned int[Image_Bands];
	memcpy (Pixel_Height, JP2_metadata.Pixel_Height,
		Image_Bands * sizeof (int));
	}

Codestream_Parameters = dynamic_cast<Aggregate*>
	(Parameters->find (box_name (CONTIGUOUS_CODESTREAM_TYPE)));
if (Codestream_Parameters)
	{
	Parameter
		*parameter;
	parameter = Codestream_Parameters->find (segment_name (PLM_MARKER) +
		Parameters->path_delimiter () + PACKET_LENGTH_PARAMETER);
	if (parameter &&
		parameter->value ().is_Array ())
		{
		PLM_Packet_Length_Array = dynamic_cast<Array*>(&(parameter->value ()));
		PLM_Packet_Length =
			JP2_metadata.PLM_Packet_Length;
		PLM_Packet_Length_Bytes_Remaining =
			JP2_metadata.PLM_Packet_Length_Bytes_Remaining;
		}
	parameter = Codestream_Parameters->find (segment_name (PLT_MARKER) +
		Parameters->path_delimiter () + PACKET_LENGTH_PARAMETER);
	if (parameter &&
		parameter->value ().is_Array ())
		{
		PLT_Packet_Length_Array = dynamic_cast<Array*>(&(parameter->value ()));
		PLT_Packet_Length =
			JP2_metadata.PLT_Packet_Length;
		PLT_Packet_Length_Bytes_Remaining =
			JP2_metadata.PLT_Packet_Length_Bytes_Remaining;
		}
	}
}


JP2_Metadata::~JP2_Metadata ()
{
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">>> ~JP2_Metadata @ " << (void*)this << endl;
#endif
if (Parameters)
	delete Parameters;
if (Pixel_Precision)
	{
	#if ((DEBUG) & DEBUG_CONSTRUCTORS)
	clog << "    delete Pixel_Precision array" << endl;
	#endif
	delete[] Pixel_Precision;
	}
if (Producer_UUID)
	{
	#if ((DEBUG) & DEBUG_CONSTRUCTORS)
	clog << "    delete Producer_UUID array" << endl;
	#endif
	delete[] Producer_UUID;
	}
if (Pixel_Width)
	{
	#if ((DEBUG) & DEBUG_CONSTRUCTORS)
	clog << "    delete Pixel_Width array" << endl;
	#endif
	delete[] Pixel_Width;
	}
if (Pixel_Height)
	{
	#if ((DEBUG) & DEBUG_CONSTRUCTORS)
	clog << "    delete Pixel_Height array" << endl;
	#endif
	delete[] Pixel_Height;
	}
#if ((DEBUG) & DEBUG_CONSTRUCTORS)
clog << ">>> ~JP2_Metadata" << endl;
#endif
}

/*==============================================================================
	Accessors:
*/
int
JP2_Metadata::pixel_bytes
	(
	int		band
	) const
{
int
	bytes = pixel_precision (band);
if (bytes > 0)
	bytes = bytes_of_bits (bytes);
return bytes;
}


int
JP2_Metadata::pixel_precision
	(
	int		band
	) const
{
int
	precision = -1;
if (Pixel_Precision &&
	band >= 0 &&
	band < (int)Image_Bands)
	{
	precision = Pixel_Precision[band];
	if (precision < 0)
		precision = -precision;
	}
return precision;
}


bool
JP2_Metadata::pixel_signed
	(
	int		band
	) const
{
bool
	has_sign = false;
if (Pixel_Precision &&
	band >= 0 &&
	band < (int)Image_Bands)
	has_sign = (Pixel_Precision[band] < 0);
return has_sign;
}


Size_2D
JP2_Metadata::pixel_size
	(
	int		band
	) const
{
Size_2D
	size;
if (Pixel_Width &&
	Pixel_Height &&
	band >= 0 &&
	band < (int)Image_Bands)
	size.size (Pixel_Width[band], Pixel_Height[band]);
return size;
}

/*==============================================================================
	Accessors
*/
JP2_Metadata&
JP2_Metadata::reset ()
{
Image_Size.Width  =
Image_Size.Height = 0;
Image_Bands       = 0;
if (Pixel_Precision)
	delete[] Pixel_Precision;
Pixel_Precision = NULL;
if (Pixel_Height)
	delete[] Pixel_Height;
Pixel_Height = NULL;
if (Pixel_Width)
	delete[] Pixel_Width;
Pixel_Width = NULL;
if (Producer_UUID)
	delete[] Producer_UUID;
Producer_UUID = NULL;
Label_URL.clear ();

Reference_Grid_Size.Width 	=
Reference_Grid_Size.Height	= 0;
Tile_Size.Width				=
Tile_Size.Height			= 0;
Image_Offsets.X				=
Image_Offsets.Y				= 0;
Tile_Offsets.X				=
Tile_Offsets.Y				= 0;

Total_Tiles					= 0;
Quality_Layers				= 0;
Progression_Order			= -1;
Resolution_Levels			= 0;
Transform					= -1;

//	Clear the Aggregate hierarchy.
Parameters->clear ();
Parameters->name (Parser::CONTAINER_NAME);
Codestream_Parameters = NULL;

JP2_Validity        = 0;
Codestream_Validity = 0;

PLM_Packet_Length_Array = NULL;
PLM_Packet_Length = 0;
PLM_Packet_Length_Bytes_Remaining = 0;
PLT_Packet_Length_Array = NULL;
PLT_Packet_Length = 0;
PLT_Packet_Length_Bytes_Remaining = 0;

Data_Buffer = NULL;
Data_Amount = -1;

return *this;
}

/*------------------------------------------------------------------------------
	JP2 Boxes
*/
void
JP2_Metadata::add_JP2_box
	(
	Type_Code				type_code,
	int						header_length,
	const unsigned char*	content,
	long long				amount,
	long long				data_position
	)
{
if (content)
	{
	Data_Buffer = content;
	if (amount >= 0)
		{
		Data_Amount = amount;
		amount += header_length;	//	Total box length.
		}
	else
		//	Negative box length indicates indefinite (to EOF) length.
		Data_Amount = 0;

	Parameters->add (JP2_box
		(type_code, header_length, amount, data_position));
	}
}


void
JP2_Metadata::add_JP2_box
	(
	const unsigned char*	data,
	long long				amount,
	long long				data_position
	)
{
if (data &&
	amount > 0)
	{
	Data_Buffer = data;
	Data_Amount = amount;
	if (amount >= box_length (data, amount))
		Parameters->add (JP2_box (data_position));
	}
}


void
JP2_Metadata::add_JP2_boxes
	(
	const unsigned char*	data,
	long long				amount,
	long long				data_position
	)
{
if (data &&
	amount > 0)
	{
	Data_Buffer = data;
	Data_Amount = amount;
	add_JP2_boxes (Parameters, data_position);
	}
}


void
JP2_Metadata::add_JP2_boxes
	(
	Aggregate*	container,
	long long	data_position
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::add_JP2_boxes:" << endl
	 << "        container @ " << (void*)container << endl
	 << "    data_position = " << data_position << endl
	 << "      Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "      Data_Amount = " << Data_Amount << endl
	 << "       box_length = "
	 	<< box_length (Data_Buffer, Data_Amount) << endl;
#endif
const unsigned char*
	box_start = Data_Buffer;
while (Data_Amount &&
	   Data_Amount >= box_length (Data_Buffer, Data_Amount))
	{
	container->add (JP2_box (data_position));
	if (data_position >= 0)
		{
		data_position += (Data_Buffer - box_start);
		box_start = Data_Buffer;
		}
	#if ((DEBUG) & DEBUG_PVL)
	clog << "    next box:" << endl
		 << "    data_position = " << data_position << endl
		 << "      Data_Buffer @ " << (void*)Data_Buffer << endl
		 << "      Data_Amount = " << Data_Amount << endl
		 << "       box_length = "
	 		<< box_length (Data_Buffer, Data_Amount) << endl;
	#endif
	}
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::add_JP2_boxes" << endl;
#endif
}


Aggregate*
JP2_Metadata::JP2_box
	(
	long long	data_position
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::JP2_box:" << endl
	 << "    data_position = " << data_position << endl
	 << "      Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "      Data_Amount = " << Data_Amount << endl;
#endif
if (! Data_Amount ||
	Data_Amount < box_length (Data_Buffer, Data_Amount))
	{
	#if ((DEBUG) & DEBUG_PVL)
	clog << "    Insufficient data for "
			<< box_length (Data_Buffer, Data_Amount) << " box length" << endl;
	#endif
	return NULL;
	}
long
	content_size = Data_Amount;
if (content_size < 8)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid JP2 box size specified: " << content_size << endl
		<< "  Box header must be at least 8 bytes.";
	throw invalid_argument (message.str ());
	}

const unsigned char*
	box_start = Data_Buffer;
long long
	box_length = get_unsigned_integer (string ("JP2 box ") + LENGTH_PARAMETER);
Type_Code
	type_code  = get_unsigned_integer (string ("JP2 box ") + TYPE_PARAMETER);
#if ((DEBUG) & DEBUG_PVL)
clog << "          type_code = \"" << type_name (type_code)
		<< "\" (0x" << hex << type_code << ')' << dec << endl
	 << "         box_length = " << box_length << endl;
#endif

if (box_length == 1)
	{
	//	Use the Box Extended Length field.
	if (content_size < 16)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Invalid JP2 box size specified: " << content_size << endl
			<< "  Box header must be 16 bytes.";
		throw invalid_argument (message.str ());
		}
	box_length = get_long_integer
		(type_name (type_code) + " Extended " + LENGTH_PARAMETER);
	#if ((DEBUG) & DEBUG_PVL)
	clog << "         box_length = " << box_length << endl;
	#endif
	}

#if ((DEBUG) & DEBUG_PVL)
clog << "    header length = " << (Data_Buffer - box_start) << endl
	 << "      Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "      Data_Amount = " << Data_Amount << endl;
Aggregate*
	parameters =
#else
return
#endif
JP2_box (type_code, Data_Buffer - box_start, box_length, data_position);
#if ((DEBUG) & DEBUG_PVL)
clog << "<< JP2_Metadata::JP2_box" << endl;
return parameters;
#endif
}


std::string
JP2_Metadata::type_name
	(
	Type_Code	type_code,
	bool		throw_exception
	)
{
string
	name;
char
	character;
for (Type_Code
		mask = 0xFF000000,
		shift = 24;
	 	mask;
	 	mask >>= 8,
		shift -= 8)
	{
	//	Pull out the next byte as a char.
	character = (char)((type_code & mask) >> shift);
	if (character < 32 ||
		character > 126)
		{
		if (throw_exception)
			{
			ostringstream
				message;
			message
				<< "Byte " << (shift >> 3)
					<< " of box type " << type_code
					<< " (0x" << hex << setfill ('0') << setw (8) << type_code
					<< ") is not a printable type name character.";
			throw JP2_Invalid_Argument (message.str (), ID);
			}
		return box_name (INVALID_CODE);
		}
	else
		name += character;
	}
return name;
}


JP2_Metadata::Type_Code
JP2_Metadata::type_code
	(
	const std::string&	name,
	bool				throw_exception
	)
{
if (name.length () != (unsigned int)INTEGER_FIELD_SIZE)
	{
	if (throw_exception)
		{
		ostringstream
			message;
		message
			<< "The type name used to produce a type code"
				" must have " << INTEGER_FIELD_SIZE << " characters," << endl
			<< "but specified string has "
				<< name.length () << " characters:" << endl
			<< '"' << name << '"';
		throw JP2_Invalid_Argument (message.str (), ID);
		}
	return INVALID_CODE;
	}
int
	code = 0,
	character;
for (unsigned int
		index = 0;
		index < name.length ();
		index++)
	{
	character = (int)name[index] & 0xFF;
	if (character < 32 ||
		character > 126)
		{
		if (throw_exception)
			{
			ostringstream
				message;
			message
				<< "Character " << index
					<< " is not a printable character" << endl
				<< "for convering to a type code value from string" << endl
				<< '"' << name << '"';
			throw JP2_Invalid_Argument (message.str (), ID);
			}
		return INVALID_CODE;
		}
	code <<= 8;
	code |= character;
	}
return code;
}


std::string
JP2_Metadata::box_name
	(
	Type_Code	type_code
	)
{
for (const Code_Name*
		table = CODE_NAMES;
		table->code;
	  ++table)
	if (table->code == type_code)
		return table->name;
return UNKNOWN_NAME;
}


JP2_Metadata::Type_Code
JP2_Metadata::box_type
	(
	const unsigned char*	data,
	long					amount
	)
{
if (amount < (INTEGER_FIELD_SIZE << 1))
	return INVALID_CODE;
return get_int (data + INTEGER_FIELD_SIZE);
}


long long
JP2_Metadata::box_length
	(
	const unsigned char*	data,
	long					amount
	)
{
long long
	length = 0;
if (amount >= INTEGER_FIELD_SIZE)
	{
	length = get_int (data);
	if (length == 1)
		{
		if (amount >= ((INTEGER_FIELD_SIZE << 1) + LONG_INTEGER_FIELD_SIZE))
			length = get_long (data + (INTEGER_FIELD_SIZE << 1));
		else
			length = 0;
		}
	}
return length;
}


namespace
{
string
data_position_report
	(
	long	data_position,
	char	end_punctuation
	)
{
ostringstream
	report;
if (data_position >= 0)
	report
		<< endl
		<< "at data source offset position " << data_position;
report << end_punctuation;
return report.str ();
}
}


Aggregate*
JP2_Metadata::JP2_box
	(
	Type_Code	type_code,
	int			header_length,
	long long	box_length,
	long long	data_position
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::JP2_box: \"" << type_name (type_code)
		<< "\" (0x" << hex << type_code << ')' << dec << endl
	 << "    header_length = " << header_length << endl
	 << "       box_length = " << box_length << endl
	 << "    data_position = " << data_position << endl
	 << "      Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "      Data_Amount = " << Data_Amount << endl;
#endif
if (header_length < 8)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid JP2 box header size specified: " << header_length << endl
		<< "for box type \"" << type_name (type_code) << '"'
		<< data_position_report (data_position, '.');
		throw invalid_argument (message.str ());
	}
if (box_length >= 0)
	{
	if (header_length > box_length)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Invalid box length of " << box_length << " byte"
				<< ((box_length == 1) ? "" : "s") << endl
			<< "with a box header size of " << header_length << " bytes" << endl
			<< "for box type \"" << type_name (type_code) << '"'
			<< data_position_report (data_position, '.');
		throw invalid_argument (message.str ());
		}
	if (box_length > (header_length + Data_Amount))
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Insufficient box data content provided: " << Data_Amount << endl
			<< "Expected a box length of " << box_length << " bytes including the "
				<< header_length << " byte header" << endl
			<< "for box type \"" << type_name (type_code) << '"'
			<< data_position_report (data_position, '.');
		throw invalid_argument (message.str ());
		}
	}

//	Box header parameters:

Aggregate
	*box = new Aggregate (type_name (type_code));
box_header_parameters (box, type_code, box_length, data_position);

if (box_length < 0)
	box_length = header_length;
box_length -= header_length;		//	Box content amount.
long long
	position = data_position;
if (data_position >= 0)
	position += header_length;	//	Possible sub-box position.

//	Validity check:

if (JP2_Validity == 0)
	{
	if (type_code != SIGNATURE_TYPE)
		{
		//	The first box must be the Signature.
		ostringstream
			message;
		message
			<< "The first JP2 box must be the "
				<< box_name (SIGNATURE_TYPE) << " (\""
				<< type_name (SIGNATURE_TYPE) << "\") box." << endl
			<< "The first JP2 box found: \"" <<  type_name (type_code) << '"'
			<< data_position_report (data_position, '.');
		throw JP2_Logic_Error (message.str (), ID);
		}
	else
	if ((header_length + box_length) != sizeof (JP2_SIGNATURE))
		{
		ostringstream
	    	message;
		message
	    	<< "A " << box_name (SIGNATURE_TYPE) << " (\""
			<< type_name (SIGNATURE_TYPE)<< "\") box must contain "
			<< sizeof (JP2_SIGNATURE) << " bytes" << endl
	    	<< "but the box length is "
				<< (header_length + box_length) << " bytes"
	    	<< data_position_report (data_position, '.');
		throw JP2_Logic_Error (message.str (), ID);
		}
	}
else
if (JP2_Validity == SIGNATURE_FLAG &&
	type_code != FILE_TYPE_TYPE)
	{
	//	The second box must be the File Type.
	ostringstream
		message;
	message
		<< "The second JP2 box must be the "
			<< box_name (FILE_TYPE_TYPE) << " (\""
			<< type_name (FILE_TYPE_TYPE) << "\") box." << endl
		<< "The second JP2 box found: \"" <<  type_name (type_code) << '"'
		<< data_position_report (data_position, '.');
	throw JP2_Logic_Error (message.str (), ID);
	}
int
	type_flag (type_flag_from_code (type_code));
if ((type_flag & (REQUIRED_BOXES | OPTIONAL_ONCE_BOXES)) &&
	(type_flag & JP2_Validity))
	{
	ostringstream
		message;
	message
		<< "A second " << JP2_Metadata::box_name (type_code) << " (\""
			<< JP2_Metadata::type_name (type_code) << "\") box was provided"
		<< data_position_report (data_position, ',') << endl
		<< "but this box type should only occur once in a JP2 file.";
	throw JP2_Logic_Error (message.str (), JP2_Metadata::ID);
	}

//	Box content parameters:

switch (type_code)
	{
	//	Super boxes that contain additional boxes:

	case JP2_HEADER_TYPE:
	case RESOLUTION_TYPE:
	case UUID_INFO_TYPE:
	case ASSOCIATION_TYPE:
		JP2_Validity |= type_flag;
		add_JP2_boxes (box, position);

		if (type_code == UUID_INFO_TYPE)
			//	Extract cached values.
			check_UUID_info_parameters (box);
		break;

	//	Basic required boxes:

	case SIGNATURE_TYPE:
		signature_parameters (box);
		break;
	case FILE_TYPE_TYPE:
		file_type_parameters (box, box_length);
		break;

		//	JP2 Header sub-boxes:

		case IMAGE_HEADER_TYPE:
			image_header_parameters (box);
			break;
		case BITS_PER_COMPONENT_TYPE:
			bits_per_component_parameters (box, box_length);
			break;
		case COLOUR_SPECIFICATION_TYPE:
			colour_specification_parameters (box, box_length, position);
			break;
		case PALETTE_TYPE:
			palette_parameters (box);
			break;
		case COMPONENT_MAPPING_TYPE:
			component_mapping_parameters (box, box_length);
			break;
		case CHANNEL_DEFINITION_TYPE:
			channel_definition_parameters (box);
			break;

			//	JP2 Header/Resolution sub-boxes:
			case CAPTURE_RESOLUTION_TYPE:
			case DEFAULT_DISPLAY_RESOLUTION_TYPE:
				resolution_parameters (box);
				break;

	//	Basic optional boxes:

	case LABEL_TYPE:
	case XML_TYPE:
		text_parameter (box, box_length);
		break;
	case UUID_TYPE:
		UUID_parameters (box, box_length, position);
		break;

		//	UUID Info sub-boxes:
		case UUID_LIST_TYPE:
			UUID_list_parameters (box);
			break;
		case URL_TYPE:
			URL_parameters (box, box_length);
			break;

	//	JPEG2000 codestream box.
	case CONTIGUOUS_CODESTREAM_TYPE:
		contiguous_codestream_parameters (box, box_length, position);
		break;

	//	JPIP placeholder box.
	case PLACEHOLDER_TYPE:
		placeholder_parameters (box, box_length, position);
		break;

	//	Opaque boxes.
	case INTELLECTUAL_PROPERTY_TYPE:
	default:
		data_position_parameters (box, box_length, position);
	}
JP2_Validity |= type_flag;
#if ((DEBUG) & DEBUG_PVL)
clog << "    box parameters -" << endl
	 << *box
	 << "<<< JP2_Metadata::JP2_box" << endl;
#endif
return box;
}

/*..............................................................................
	JP2 box parameters
*/
void
JP2_Metadata::signature_parameters
	(
	Aggregate*	box
	)
{
unsigned int
	signature = get_unsigned_integer (box->name () + ' ' + SIGNATURE_PARAMETER);
box->add (Assignment (SIGNATURE_PARAMETER)
	= Integer (signature, 16));
if (signature != SIGNATURE_CONTENT)
	{
	ostringstream
		message;
	message
		<< "Invalid JP2 Signature: 0x" << hex
			<< SIGNATURE_CONTENT << " expected, 0x" << signature << " found.";
	throw JP2_Invalid_Argument (message.str (), ID);
	}
}


void
JP2_Metadata::file_type_parameters
	(
	Aggregate*	box,
	long long	content_size
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::file_type_parameters:" << endl
	 << "             box @ " << (void*)box << endl
	 << "    content_size = " << content_size << endl
	 << "     Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "     Data_Amount = " << Data_Amount << endl;
unsigned int
	datum;
#endif
long
	box_content_size = Data_Amount;
bool
	compatible = false;
string
	compatibility (to_string (get_unsigned_integer
		(box->name () + ' ' + BRAND_PARAMETER)));
#if ((DEBUG) & DEBUG_PVL)
clog << "    " << BRAND_PARAMETER << " = " << compatibility << endl;
#endif
if (compatibility == JP2_COMPATIBLE)
	compatible = true;
box->add (Assignment (BRAND_PARAMETER)
	= String (compatibility));
box->add (Assignment (MINOR_VERSION_PARAMETER)
	= Integer (
		#if ((DEBUG) & DEBUG_PVL)
		datum =
		#endif
		get_unsigned_integer (box->name () + ' ' + MINOR_VERSION_PARAMETER),
		Value::UNSIGNED));
#if ((DEBUG) & DEBUG_PVL)
clog << "    " << MINOR_VERSION_PARAMETER << " = " << datum << endl;
#endif

Array
	list (Value::SET);
content_size -= box_content_size - Data_Amount;
while (content_size >= INTEGER_FIELD_SIZE)
	{
	#if ((DEBUG) & DEBUG_PVL)
	clog << "     Data_Buffer @ " << (void*)Data_Buffer << endl
		 << "    content_size = " << content_size << endl;
	#endif
	compatibility = to_string (get_unsigned_integer ());
	content_size -= INTEGER_FIELD_SIZE;
	#if ((DEBUG) & DEBUG_PVL)
	clog << "    " << COMPATIBILITY_LIST_PARAMETER << " = "
	 		<< compatibility << endl;
	#endif
	if (compatibility == JP2_COMPATIBLE)
		compatible = true;
	list.add (String (compatibility));
	}
box->add (Assignment (COMPATIBILITY_LIST_PARAMETER)
	= list);

if (! compatible)
	{
	ostringstream
		message;
	message
		<< "File Type " << COMPATIBILITY_LIST_PARAMETER
		<< " \"" << JP2_COMPATIBLE << "\" not found.";
	throw JP2_Logic_Error (message.str (), ID);
	}
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::file_type_parameters" << endl;
#endif
}


void
JP2_Metadata::image_header_parameters
	(
	Aggregate*	box
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::image_header_parameters:" << endl
	 << "            box @ " << (void*)box << endl
	 << "    Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "    Data_Amount = " << Data_Amount << endl;
#endif
auto height = get_unsigned_integer (box->name () + ' ' + HEIGHT_PARAMETER);
auto width = get_unsigned_integer (box->name () + ' ' + WIDTH_PARAMETER);
Size_2D image_size (width, height);
unsigned int
	image_bands =
		get_unsigned_short_integer
	 		(box->name () + ' ' + IMAGE_BANDS_PARAMETER);
Parameter*
	value_bits = value_bits_parameter (1, box->name ());
unsigned int
	compression_type =
		get_unsigned_byte (box->name () + ' ' + COMPRESSION_TYPE_PARAMETER);
bool
	colorspace_unknown =
		get_unsigned_byte (box->name () + ' ' + COLORSPACE_UNKNOWN_PARAMETER),
	intellectual_property =
		get_unsigned_byte (box->name () + ' ' + INTELLECTUAL_PROPERTY_PARAMETER);
#if ((DEBUG) & DEBUG_PVL)
clog << "     image_size = " << image_size << endl
	 << "    image_bands = " << image_bands << endl
	 << "     value_bits -" << endl
	 << *value_bits
	 << "    compression_type = " << compression_type << endl
	 << "       colorspace_unknown = " << boolalpha << colorspace_unknown << endl
	 << "    intellectual_property = " << intellectual_property << endl;
#endif
(*box)
	.add (Assignment (HEIGHT_PARAMETER)
		= Integer (image_size.height (), Value::UNSIGNED).units ("rows"))
	.add (Assignment (WIDTH_PARAMETER)
		= Integer (image_size.width (), Value::UNSIGNED).units ("columns"))
	.add (Assignment (IMAGE_BANDS_PARAMETER)
		= Integer (image_bands, Value::UNSIGNED))
	.add (value_bits)
	.add (Assignment (COMPRESSION_TYPE_PARAMETER)
		= Integer (compression_type, Value::UNSIGNED))
	.add (Assignment (COLORSPACE_UNKNOWN_PARAMETER)
		= String (colorspace_unknown ? "true" : "false"))
	.add (Assignment (INTELLECTUAL_PROPERTY_PARAMETER)
		= String (intellectual_property ? "true" : "false"));

//	Ignore all but the first Image Header box.
if (! (JP2_Validity & IMAGE_HEADER_FLAG))
	{
	//	Takes precedence over SIZ segment.
	Image_Size  = image_size;
	Image_Bands = image_bands;

	//	Don't use pixel_precision here: only one value for all bands.
	if (Pixel_Precision)
		delete[] Pixel_Precision;
	Pixel_Precision = new int[Image_Bands];
	int
		pixel_bits = value_bits->value ()[0];
	while (image_bands--)
		Pixel_Precision[image_bands] = pixel_bits;
	}
#if ((DEBUG) & DEBUG_PVL)
else
	clog << "    redundant box does not update data cache" << endl;
clog << "    Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "    Data_Amount = " << Data_Amount << endl
	 << "<<< JP2_Metadata::image_header_parameters" << endl;
#endif
}


void
JP2_Metadata::bits_per_component_parameters
	(
	Aggregate*	box,
	long long	content_size
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::bits_per_component_parameters:" << endl
	 << "             box @ " << (void*)box << endl
	 << "    content_size = " << content_size << endl;
#endif
Parameter*
	value_bits = value_bits_parameter ((int)content_size, box->name ());
box->add (value_bits);
#if ((DEBUG) & DEBUG_PVL)
clog << "    value_bits -" << endl
	 << *value_bits;
#endif
pixel_precision
	(dynamic_cast<Array*>(&(value_bits->value ())),
	box_name (BITS_PER_COMPONENT_TYPE) +
		" (\"" + type_name (BITS_PER_COMPONENT_TYPE) + "\") JP2 box");
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::bits_per_component_parameters" << endl;
#endif
}


namespace
{
void
no_Image_Bands_exception
	(
	const string&	name
	)
{
/*
	The required Image_Bands value has not been set. The value is
	obtained from the either the image header box or the SIZ segment. The
	image header box is required to occur before the codestream and the
	SIZ segment is required to be the first segment after the SOC
	delimiter, so as long as this validity check has already been made
	the error condition should never occur.
*/
ostringstream
	message;
message
	<< "Can't add the " << name << " codestream segment" << endl
	<< "because the " << JP2_Metadata::IMAGE_BANDS_PARAMETER
		<< " value from either the "
		<< JP2_Metadata::box_name (JP2_Metadata::IMAGE_HEADER_TYPE) << " (\""
		<< JP2_Metadata::type_name (JP2_Metadata::IMAGE_HEADER_TYPE)
		<< "\") JP2 box or the "
		<< JP2_Metadata::segment_name (JP2_Metadata::SIZ_MARKER)
		<< " codestream segment has not been provided.";
throw JP2_Logic_Error (message.str (), JP2_Metadata::ID);
}
}


void
JP2_Metadata::pixel_precision
	(
	Array*			values,
	const string&	description
	)
{
if (values)
	{
	if (! Image_Bands)
		no_Image_Bands_exception (description);
	if (Image_Bands != values->size ())
		{
		ostringstream
			message;
		message
			<< "Invalid " << description << ':' << endl
			<< values->size () << " pixel precision value"
				<< ((values->size () == 1) ? "" : "s") << " found but the "
				<< IMAGE_BANDS_PARAMETER << " value is " << Image_Bands << '.';
		throw JP2_Logic_Error (message.str (), ID);
		}

	int
		index = values->size ();
	if (! Pixel_Precision)
		Pixel_Precision = new int[index];
	else
	if (Pixel_Precision[0] != 255)
		{
		//	The pixel precision for all bands has already been set.
		while (--index >= 0)
			{
			if (! Pixel_Precision[index])
				//	Reset zero (variable) precision.
				Pixel_Precision[index] = (int)(values->at (index));
			else
			if (Pixel_Precision[index] != (int)(values->at (index)))
				{
				ostringstream
					message;
				message
					<< "Invalid " << description << ':' << endl
					<< "The " << VALUE_BITS_PARAMETER
						<< " of " << (int)(values->at (index))
						<< " for band " << index << endl
					<< "is inconsitent with the previously set "
						<< Pixel_Precision[index] << " value.";
				throw JP2_Logic_Error (message.str (), ID);
				}
			}
		}
	else
		while (--index >= 0)
			Pixel_Precision[index] = values->at (index);
	}
}


void
JP2_Metadata::colour_specification_parameters
	(
	Aggregate*	box,
	long long	content_size,
	long long	data_position
	)
{
JP2_Validity |= COLOUR_SPECIFICATION_FLAG;
long
	amount = Data_Amount;
int
	method = get_unsigned_byte
		(box->name () + ' ' + SPECIFICATION_METHOD_PARAMETER);
box->add (Assignment (SPECIFICATION_METHOD_PARAMETER)
	= Integer (method, Value::UNSIGNED));
box->add (Assignment (PRECEDENCE_PARAMETER)
	= Integer (get_unsigned_byte
		(box->name () + ' ' + PRECEDENCE_PARAMETER),
		Value::UNSIGNED));
box->add (Assignment (COLOURSPACE_APPROXIMATION_PARAMETER)
	= Integer (get_unsigned_byte
		(box->name () + ' ' + COLOURSPACE_APPROXIMATION_PARAMETER),
		Value::UNSIGNED));
if (method == ENUMERATED_COLOURSPACE)
	box->add (Assignment (ENUMERATED_COLOURSPACE_PARAMETER)
		= Integer (get_unsigned_integer
			(box->name () + ' ' + ENUMERATED_COLOURSPACE_PARAMETER),
			Value::UNSIGNED));
else
	{
	amount -= Data_Amount;
	content_size -= amount;
	if (data_position >= 0)
		data_position += amount;
	data_position_parameters (box, content_size, data_position);
	}
}


void
JP2_Metadata::palette_parameters
	(
	Aggregate*	box
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::palette_parameters:" << endl
	 << "            box @ " << (void*)box << endl
	 << "    Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "    Data_Amount = " << Data_Amount << endl;
#endif
int
	entries = get_unsigned_short_integer
		(box->name () + ' ' + ENTRIES_PARAMETER),
	columns = get_unsigned_byte
		(box->name () + ' ' + COLUMNS_PARAMETER);
Parameter*
	value_bits = value_bits_parameter (columns, box->name ());
#if ((DEBUG) & DEBUG_PVL)
clog << "    entries = " << entries << endl
	 << "    columns = " << columns << endl
	 << "    value_bits -" << endl
	 << *value_bits;
#endif
box->add (Assignment (ENTRIES_PARAMETER)
	= Integer (entries, Value::UNSIGNED));
box->add (Assignment (COLUMNS_PARAMETER)
	= Integer (columns, Value::UNSIGNED));
box->add (value_bits);

string
	description (box->name ());
description += ' ';
description += VALUES_PARAMETER;
Array
	palette (Value::SEQUENCE),
	*palette_row,
	*values = dynamic_cast<Array*>(&(value_bits->value ()));
while (entries--)
	{
	palette_row = new Array (Value::SEQUENCE);
	for (int column = 0;
			 column < columns;
		   ++column)
		palette_row->add (new Integer
			(get_value ((int)values->at (column), description)));
	palette.add (palette_row);
	}
box->add (Assignment (VALUES_PARAMETER) = palette);
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::palette_parameters" << endl;
#endif
}


void
JP2_Metadata::component_mapping_parameters
	(
	Aggregate*	box,
	long long	content_size
	)
{
string
	description (box->name ());
description += ' ';
description += MAP_PARAMETER;
Array
	map (Value::SEQUENCE),
	*set;
while (content_size > 4)
	{
	set = new Array (Value::SEQUENCE);
	set->add (new Integer (get_unsigned_short_integer
		(description + " component index"), Value::UNSIGNED));
	set->add (new Integer (get_unsigned_byte
		(description + " map type"), Value::UNSIGNED));
	set->add (new Integer (get_unsigned_byte
		(description + " palette index"), Value::UNSIGNED));
	map.add (set);
	content_size -= 4;
	}
Parameter*
	parameter = new Assignment (MAP_PARAMETER);
parameter->comment
	("\nMap entries:\n"
	 "  Component index,\n"
	 "  Map type,\n"
	 "  Palette index");
*parameter = map;
box->add (parameter);
}


void
JP2_Metadata::channel_definition_parameters
	(
	Aggregate*	box
	)
{
int
	entries = get_unsigned_short_integer
		(box->name () + ' ' + ENTRIES_PARAMETER);
box->add (Assignment (ENTRIES_PARAMETER)
	= Integer (entries, Value::UNSIGNED));
string
	description (box->name ());
description += ' ';
description += MAP_PARAMETER;
Array
	map (Value::SEQUENCE),
	*set;
while (entries--)
	{
	set = new Array (Value::SEQUENCE);
	set->add (new Integer (get_unsigned_short_integer
		(description + " channel index"), Value::UNSIGNED));
	set->add (new Integer (get_unsigned_short_integer
		(description + " channel type"), Value::UNSIGNED));
	set->add (new Integer (get_unsigned_short_integer
		(description + " channel association"), Value::UNSIGNED));
	map.add (set);
	}
Parameter
	*parameter = new Assignment (MAP_PARAMETER);
parameter->comment
	("\nChannel definition entries:\n"
	 "  Channel index,\n"
	 "  Channel type,\n"
	 "  Channel association");
*parameter = map;
box->add (parameter);
}


void
JP2_Metadata::resolution_parameters
	(
	Aggregate*	box
	)
{
box->add (Assignment (VERTICAL_NUMERATOR_PARAMETER)
	= Integer (get_unsigned_short_integer
		(box->name () + ' ' + VERTICAL_NUMERATOR_PARAMETER),
		Value::UNSIGNED));
box->add (Assignment (VERTICAL_DENOMINATOR_PARAMETER)
	= Integer (get_unsigned_short_integer
		(box->name () + ' ' + VERTICAL_DENOMINATOR_PARAMETER),
		Value::UNSIGNED));
int
	numerator   = get_unsigned_short_integer
		(box->name () + ' ' + HORIZONTAL_NUMERATOR_PARAMETER),
	denominator = get_unsigned_short_integer
		(box->name () + ' ' + HORIZONTAL_DENOMINATOR_PARAMETER);
box->add (Assignment (VERTICAL_EXPONENT_PARAMETER)
	= Integer (get_byte
		(box->name () + ' ' + VERTICAL_EXPONENT_PARAMETER)));
box->add (Assignment (HORIZONTAL_NUMERATOR_PARAMETER)
	= Integer (numerator, Value::UNSIGNED));
box->add (Assignment (HORIZONTAL_DENOMINATOR_PARAMETER)
	= Integer (denominator, Value::UNSIGNED));
box->add (Assignment (HORIZONTAL_EXPONENT_PARAMETER)
	= Integer (get_byte
		(box->name () + ' ' + HORIZONTAL_EXPONENT_PARAMETER)));
}


void
JP2_Metadata::UUID_parameters
	(
	Aggregate*	box,
	long long	content_size,
	long long	data_position
	)
{
box->add (UUID_parameter (box->name ()));
data_position_parameters (box, content_size, data_position, false);
}


void
JP2_Metadata::check_UUID_info_parameters
	(
	Aggregate*	box
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::check_UUID_info_parameters" << endl;
#endif
if (! Producer_UUID)
	{
	//	Confirm one UUID and URL acquired.
	#if ((DEBUG) & DEBUG_PVL)
	clog << "    check_UUID_info_parameters: UUID and URL check" << endl;
	#endif
	unsigned int
		flags = 0;
	Aggregate
		*group;
	Parameter
		*parameter;
	Array
		*UUID_value;
	for (unsigned int
			index = 0;
			index < box->size ();
		  ++index)
		{
		#if ((DEBUG) & DEBUG_PVL)
		clog << "    " << index << " - " << box->at (index).name () << endl;
		#endif
		if (box->at (index).is_Aggregate ())
			{
			group = dynamic_cast<Aggregate*>(&(box->at (index)));
			if (group->name () == type_name (UUID_LIST_TYPE))
				{
				#if ((DEBUG) & DEBUG_PVL)
				clog << "    UUID_LIST_TYPE -" << endl;
				#endif
				if (flags & 1)
					{
					//	More than one UUID list.
					flags = 0;
					break;
					}
				for (unsigned int
						entry = 0;
						entry < group->size ();
					  ++entry)
					{
					parameter = &(group->at (entry));
					#if ((DEBUG) & DEBUG_PVL)
					clog << "      " << *parameter;;
					#endif
					if (parameter->name () == ENTRIES_PARAMETER)
						{
						if ((int)parameter->value () != 1)
							{
							//	More than one UUID.
							flags = 0;
							break;
							}
						}
					else
					if (parameter->name () == UUID_PARAMETER)
						{
						if (! parameter->is_Assignment () ||
							! parameter->value ().is_Array () ||
							(flags & 1))
							{
							flags = 0;
							break;
							}
						UUID_value =
							dynamic_cast<Array*>(&(parameter->value ()));
						if (UUID_value->size () != (int)UUID_SIZE)
							{
							//	Invalid UUID size.
							flags = 0;
							break;
							}
						flags |= 1;
						Producer_UUID = new unsigned char[UUID_SIZE];
						for (int
								element = 0;
								element < UUID_SIZE;
							  ++element)
							Producer_UUID[element] =
								(unsigned int)(UUID_value->at (element));
						#if ((DEBUG) & DEBUG_PVL)
						clog << "      Producer_UUID set" << endl;
						#endif
						}
					}
				if (! (flags & 1))
					//	Invalid UUID list.
					break;
				}
			else
			if (group->name () == type_name (URL_TYPE))
				{
				#if ((DEBUG) & DEBUG_PVL)
				clog << "    URL_TYPE -" << endl;
				#endif
				if (flags & 2)
					{
					//	More than one URL.
					flags = 0;
					break;
					}
				for (unsigned int
						entry = 0;
						entry < group->size ();
					  ++entry)
					{
					parameter = &(group->at (entry));
					if (parameter->name () == URL_PARAMETER)
						{
						#if ((DEBUG) & DEBUG_PVL)
						clog << "      " << *parameter;
						#endif
						if (! parameter->is_Assignment () ||
							! parameter->value ().is_String () ||
							(flags & 2))
							{
							flags = 0;
							break;
							}
						Label_URL = (string)parameter->value ();
						#if ((DEBUG) & DEBUG_PVL)
						clog << "      Label_URL set" << endl;
						#endif
						}
					}
				if (! (flags & 2))
					//	Invalid URL.
					break;
				}
			}	//	Aggregate entry.
		}
	if (flags != 3)
		{
		#if ((DEBUG) & DEBUG_PVL)
		clog << "    Invalid" << endl;
		#endif
		if (Producer_UUID)
			delete[] Producer_UUID;
		Producer_UUID = NULL;
		Label_URL.clear ();
		}
	#if ((DEBUG) & DEBUG_PVL)
	else
		clog << "    Valid" << endl;
	#endif
	}
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::check_UUID_info_parameters" << endl;
#endif
}


void
JP2_Metadata::UUID_list_parameters
	(
	Aggregate*	box
	)
{
int
	entries = get_unsigned_short_integer
		(box->name () + ' ' + ENTRIES_PARAMETER);
box->add (Assignment (ENTRIES_PARAMETER)
	= Integer (entries, Value::UNSIGNED));
while (entries--)
	box->add (UUID_parameter (box->name ()));
}


Parameter*
JP2_Metadata::UUID_parameter
	(
	const string&	description
	)
{
string
	describe (description);
describe += ' ';
describe += UUID_PARAMETER;
Array
	uuid (Value::SEQUENCE);
ostringstream
	hex_encoded;
hex_encoded << "\nHex encoded: " << hex << setfill ('0') << uppercase;
unsigned char
	datum;
for (int
		count = 0;
		count < UUID_SIZE;
	  ++count)
	{
	datum = get_unsigned_byte (describe);
	hex_encoded << setw (2) << (unsigned int)datum;
	uuid.add (new Integer (datum, Value::UNSIGNED, 16));
	}
Parameter*
	parameter = new Assignment (UUID_PARAMETER);
*parameter = uuid;
parameter->comment (hex_encoded.str ());
return parameter;
}


void
JP2_Metadata::URL_parameters
	(
	Aggregate*	box,
	long long	content_size
	)
{
box->add (Assignment (VERSION_PARAMETER)
	= Integer (get_unsigned_byte
		(box->name () + ' ' + VERSION_PARAMETER),
		Value::UNSIGNED));
box->add (Assignment (FLAGS_PARAMETER)
	= Integer (get_value
		(24, box->name () + ' ' + FLAGS_PARAMETER),
		Value::UNSIGNED, 16));
content_size -= 4;
box->add (Assignment (URL_PARAMETER)
	= String (get_string ((long)content_size, box->name ())));
}


void
JP2_Metadata::text_parameter
	(
	Aggregate*	box,
	long long	content_size
	)
{
box->add (Assignment (TEXT_PARAMETER)
	= String (get_string ((long)content_size, box->name ()), String::TEXT));
}


void
JP2_Metadata::placeholder_parameters
	(
	Aggregate*	box,
	long long	content_size,
	long long	data_position
	)
{
long
	amount = Data_Amount;
unsigned int
	flags = get_unsigned_integer (box->name () + ' ' + FLAGS_PARAMETER);
box->add ((Assignment (FLAGS_PARAMETER)
	= Integer (flags, Value::UNSIGNED, 16))
	.comment (string ("\n")
		+ (((flags & PLACEHOLDER_FLAGS_ORIGINAL_MASK) == 0) ?
			"No o" : "O") + "riginal box provided.\n"
		+ (((flags & PLACEHOLDER_FLAGS_EQUIVALENT_MASK) == 0) ?
			"No e" : "E") + "quivalent box provided.\n"
		+ (((flags & PLACEHOLDER_FLAGS_CODESTREAM_MASK) == 0) ?
			"No c" : "C") + "odestream provided."));
Aggregate
	*group;
if ((flags & PLACEHOLDER_FLAGS_ORIGINAL_MASK) != 0)
	{
	group = new Aggregate (ORIGINAL_BOX_PARAMETER);
	group->add (Assignment (BIN_ID_PARAMETER)
		= Integer (get_long_integer (box->name () + ' '
			+ ORIGINAL_BOX_PARAMETER + ' ' + BIN_ID_PARAMETER),
			Value::UNSIGNED));
	box_header_parameters (group);
	box->add (group);
	}
if ((flags & PLACEHOLDER_FLAGS_EQUIVALENT_MASK) != 0)
	{
	group = new Aggregate (EQUIVALENT_BOX_PARAMETER);
	group->add (Assignment (BIN_ID_PARAMETER)
		= Integer (get_long_integer (box->name () + ' '
			+ EQUIVALENT_BOX_PARAMETER + ' ' + BIN_ID_PARAMETER),
			Value::UNSIGNED));
	box_header_parameters (group);
	box->add (group);
	}
if ((flags & PLACEHOLDER_FLAGS_CODESTREAM_MASK) != 0)
	{
	box->add (Assignment (CODESTREAM_ID_PARAMETER)
		= Integer (get_long_integer
			(box->name () + ' ' + CODESTREAM_ID_PARAMETER),
			Value::UNSIGNED));
	unsigned int
		total = get_unsigned_integer (box->name () + ' ' +
			TOTAL_CODESTREAMS_PARAMETER);
	if ((flags & PLACEHOLDER_FLAGS_MULTIPLE_CODESTREAM_MASK) == 0)
		total = 1;
	box->add (Assignment (TOTAL_CODESTREAMS_PARAMETER)
		= Integer (total, Value::UNSIGNED));
	}

amount -= Data_Amount;
content_size -= amount;
if (content_size > 0)
	{
	group = new Aggregate (EXTENDED_BOX_LIST_PARAMETER);
	if (data_position >= 0)
		data_position += amount;
	data_position_parameters (box, content_size, data_position);
	box->add (group);
	}
}


void
JP2_Metadata::box_header_parameters
	(
	Aggregate*	box,
	Type_Code	type_code,
	long long	box_length,
	long long	data_position
	)
{
if (! type_code)
	{
	//	Get the box header values.
	box_length = get_unsigned_integer (box->name () + ' ' + LENGTH_PARAMETER);
	type_code  = get_unsigned_integer (box->name () + ' ' + TYPE_PARAMETER);
	if (box_length == 1)
		box_length = get_long_integer
			(box->name () + " Extended " + LENGTH_PARAMETER);
	}
box->add (Assignment (NAME_PARAMETER)
	= String (box_name (type_code), Value::IDENTIFIER));
box->add (Assignment (TYPE_PARAMETER)
	= Integer (type_code, Value::UNSIGNED, 16));
data_position_parameters (box, box_length, data_position, false);
}


void
JP2_Metadata::data_position_parameters
	(
	Aggregate*	group,
	long long	content_size,
	long long	data_position,
	bool		consume_data
	)
{
if (data_position >= 0)
	group->add (Assignment (DATA_POSITION_PARAMETER)
		= Integer (data_position).units ("byte offset"));
Parameter
	*parameter = new Assignment (DATA_LENGTH_PARAMETER);
*parameter = Integer (content_size). units ("bytes");
if (content_size < 0)
	parameter->comment
		("\nIndefinite length to end of source.");
group->add (parameter);

if (consume_data &&
	content_size > 0)
	{
	//	Consume the data.
	if (content_size > Data_Amount)
		content_size = Data_Amount;
	Data_Amount -= content_size;
	Data_Buffer += content_size;
	}
}


Parameter*
JP2_Metadata::value_bits_parameter
	(
	int				count,
	const string&	description
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::value_bits_parameter:" << endl
	 << "          count = " << count << endl
	 << "    description = " << description << endl
	 << "    Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "    Data_Amount = " << Data_Amount << endl;
#endif
string
	describe (description);
describe += ' ';
describe =+ VALUE_BITS_PARAMETER;
Array
	value_bits (Value::SEQUENCE);
int
	datum;
while (count--)
	{
	if ((datum = get_byte (describe) + 1) < 0)
		 datum = -(datum & 0x7F);
	value_bits.add (new Integer (datum));
	}
#if ((DEBUG) & DEBUG_PVL)
Parameter
	*parameter = value_bits_parameter (value_bits);
clog << *parameter
	 << "<<< JP2_Metadata::value_bits_parameter" << endl;
return parameter;
#else
return value_bits_parameter (value_bits);
#endif
}


Parameter*
JP2_Metadata::value_bits_parameter
	(
	const Array&	value_bits
	)
{
Assignment*
	parameter = new Assignment (VALUE_BITS_PARAMETER);
bool
	has_negative = false,
	has_zero = false;
int
	datum,
	index = value_bits.size ();
while (index--)
	{
	datum = (int)value_bits[index];
	if (datum < 0)
		has_negative = true;
	else
	if (datum == 0)
		has_zero = true;
	}
if (has_negative ||
	has_zero)
	{
	ostringstream
		comment;
	comment << '\n';
	if (has_negative)
		{
		comment << "\nNegative bits indicate signed values of abs (bits)";
		if (has_zero)
			comment << ";\n  ";
		}
	if (has_zero)
		comment << "Zero bits indicate variable number of bits";
	comment << '.';
	parameter->comment (comment.str ());
	}
*parameter = value_bits;
return parameter;
}


int
JP2_Metadata::type_flag_from_code
	(
	Type_Code	type_code
	)
{
int
	flag = 0;
switch (type_code)
	{
	case   SIGNATURE_TYPE:
	flag = SIGNATURE_FLAG; break;
	case   FILE_TYPE_TYPE:
	flag = FILE_TYPE_FLAG; break;
	case   JP2_HEADER_TYPE:
	flag = JP2_HEADER_FLAG; break;
	case   IMAGE_HEADER_TYPE:
	flag = IMAGE_HEADER_FLAG; break;
	case   BITS_PER_COMPONENT_TYPE:
	flag = BITS_PER_COMPONENT_FLAG; break;
	case   COLOUR_SPECIFICATION_TYPE:
	flag = COLOUR_SPECIFICATION_FLAG; break;
	case   PALETTE_TYPE:
	flag = PALETTE_FLAG; break;
	case   COMPONENT_MAPPING_TYPE:
	flag = COMPONENT_MAPPING_FLAG; break;
	case   CHANNEL_DEFINITION_TYPE:
	flag = CHANNEL_DEFINITION_FLAG; break;
	case   RESOLUTION_TYPE:
	flag = RESOLUTION_FLAG; break;
	case   CAPTURE_RESOLUTION_TYPE:
	flag = CAPTURE_RESOLUTION_FLAG; break;
	case   DEFAULT_DISPLAY_RESOLUTION_TYPE:
	flag = DEFAULT_DISPLAY_RESOLUTION_FLAG; break;
	case   CONTIGUOUS_CODESTREAM_TYPE:
	flag = CONTIGUOUS_CODESTREAM_FLAG; break;
	case   INTELLECTUAL_PROPERTY_TYPE:
	flag = INTELLECTUAL_PROPERTY_FLAG; break;
	case   XML_TYPE:
	flag = XML_FLAG; break;
	case   UUID_TYPE:
	flag = UUID_FLAG; break;
	case   UUID_INFO_TYPE:
	flag = UUID_INFO_FLAG; break;
	case   UUID_LIST_TYPE:
	flag = UUID_LIST_FLAG; break;
	case   URL_TYPE:
	flag = URL_FLAG; break;
	case   ASSOCIATION_TYPE:
	flag = ASSOCIATION_FLAG; break;
	case   LABEL_TYPE:
	flag = LABEL_FLAG; break;
	case   PLACEHOLDER_TYPE:
	flag = PLACEHOLDER_FLAG; break;
	}
return flag;
}


JP2_Metadata::Type_Code
JP2_Metadata::type_code_from_flag
	(
	int		type_flag
	)
{
Type_Code
	code = INVALID_CODE;
switch (type_flag)
	{
	case   SIGNATURE_FLAG:
	code = SIGNATURE_TYPE; break;
	case   FILE_TYPE_FLAG:
	code = FILE_TYPE_TYPE; break;
	case   JP2_HEADER_FLAG:
	code = JP2_HEADER_TYPE; break;
	case   IMAGE_HEADER_FLAG:
	code = IMAGE_HEADER_TYPE; break;
	case   BITS_PER_COMPONENT_FLAG:
	code = BITS_PER_COMPONENT_TYPE; break;
	case   COLOUR_SPECIFICATION_FLAG:
	code = COLOUR_SPECIFICATION_TYPE; break;
	case   PALETTE_FLAG:
	code = PALETTE_TYPE; break;
	case   COMPONENT_MAPPING_FLAG:
	code = COMPONENT_MAPPING_TYPE; break;
	case   CHANNEL_DEFINITION_FLAG:
	code = CHANNEL_DEFINITION_TYPE; break;
	case   RESOLUTION_FLAG:
	code = RESOLUTION_TYPE; break;
	case   CAPTURE_RESOLUTION_FLAG:
	code = CAPTURE_RESOLUTION_TYPE; break;
	case   DEFAULT_DISPLAY_RESOLUTION_FLAG:
	code = DEFAULT_DISPLAY_RESOLUTION_TYPE; break;
	case   CONTIGUOUS_CODESTREAM_FLAG:
	code = CONTIGUOUS_CODESTREAM_TYPE; break;
	case   INTELLECTUAL_PROPERTY_FLAG:
	code = INTELLECTUAL_PROPERTY_TYPE; break;
	case   XML_FLAG:
	code = XML_TYPE; break;
	case   UUID_FLAG:
	code = UUID_TYPE; break;
	case   UUID_INFO_FLAG:
	code = UUID_INFO_TYPE; break;
	case   UUID_LIST_FLAG:
	code = UUID_LIST_TYPE; break;
	case   URL_FLAG:
	code = URL_TYPE; break;
	case   ASSOCIATION_FLAG:
	code = ASSOCIATION_TYPE; break;
	case   LABEL_FLAG:
	code = LABEL_TYPE; break;
	case   PLACEHOLDER_FLAG:
	code = PLACEHOLDER_TYPE; break;
	}
return code;
}


std::string
JP2_Metadata::JP2_validity_report
	(
	unsigned int	validity_flags
	)
{
ostringstream
	report;
report
	<< "    Required JP2 boxes:" << endl;
int
	flag;
for (flag  = MIN_REQUIRED_BOX_FLAG;
	 flag <= MAX_REQUIRED_BOX_FLAG;
	 flag <<= 1)
	report
		<< "      " << box_name (type_code_from_flag (flag))
			<< ((validity_flags & flag) ? "" : " not") << " present." << endl;
if (boxes_are_complete (validity_flags))
	report << "        All required JP2 boxes are present." << endl;

if (validity_flags & (OPTIONAL_ONCE_BOXES | OPTIONAL_BOXES))
	{
	report << "    Optional JP2 boxes:" << endl;
	for (;
		 flag <= MAX_BOX_FLAG;
		 flag <<= 1)
		if (validity_flags & flag)
			report << "      " << box_name (type_code_from_flag (flag)) << endl;
	}
return report.str ();
}

/*------------------------------------------------------------------------------
	Codestream Segments
*/
void
JP2_Metadata::contiguous_codestream_parameters
	(
	Aggregate*	box,
	long long	content_size,
	long long	data_position
	)
{
JP2_Validity |= CONTIGUOUS_CODESTREAM_FLAG;
if (content_size == 0)
	content_size = -1;	//	Indefinite length.
data_position_parameters (box, content_size, data_position);

if (! Codestream_Parameters)
	Codestream_Parameters = box;

if (content_size > 0)
	{
	//	Add codestream segment parameters.
	long
		amount_difference = Data_Amount - content_size;
	add_codestream_segments (Data_Buffer, content_size, data_position);
	Data_Amount += amount_difference;
	}
}


void
JP2_Metadata::add_codestream_segments
	(
	const unsigned char*	data,
	long					amount,
	long long				data_position
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::add_codestream_segments:" << endl
	 << "              data @ " << (void*)data << endl
	 << "            amount = " << amount << endl
	 << "     data_position = " << data_position << endl
	 << "    segment_length = "
	 	<< segment_length (data, amount) << endl;
#endif
if (data &&
	amount > 0)
	{
	Data_Buffer = data;
	Data_Amount = amount;
	while (Data_Amount &&
		   Data_Amount >= segment_length (Data_Buffer, Data_Amount))
		{
		Marker_Code
			marker = get_unsigned_short_integer
				(string ("Codestream segment ") + MARKER_PARAMETER);
		int
			length = 2;
		if (! (marker_flag_from_code (get_short (data)) & DELIMITER_SEGMENTS))
			length = get_unsigned_short_integer
				(string ("Codestream segment ") + LENGTH_PARAMETER);

		add_codestream_segment (marker, Data_Buffer, length, data_position);

		if (marker == EOC_MARKER)
			//	End of codestream.
			break;

		if (data_position >= 0)
			{
			data_position += (Data_Buffer - data);
			data = Data_Buffer;
			}
		#if ((DEBUG) & DEBUG_PVL)
		clog << "    next segment:" << endl
			 << "     data_position = " << data_position << endl
			 << "       Data_Buffer @ " << (void*)Data_Buffer << endl
			 << "       Data_Amount = " << Data_Amount << endl
			 << "    segment_length = "
	 			<< segment_length (Data_Buffer, Data_Amount) << endl;
		#endif
		}
	}
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::add_codestream_segments" << endl;
#endif
}


void
JP2_Metadata::add_codestream_segment
	(
	Marker_Code				marker,
	const unsigned char*	content,
	int						segment_length,
	long long				data_position
	)
{
if (content)
	{
	Data_Buffer = content;
	if ((Data_Amount = segment_length - 2) > 0)	//	Exclude marker field.
		 Data_Amount -= 2;						//	Exclude length field.
	if ( Data_Amount < 0)
		 Data_Amount = 0;

	if (! Codestream_Parameters)
		{
		ostringstream
			message;
		message
			<< "Attempt to add a " << segment_name (marker)
				<< " (" << marker_number (marker) << ") segment" << endl
			<< "but no " << box_name (CONTIGUOUS_CODESTREAM_TYPE) << " (\""
				<< type_name (CONTIGUOUS_CODESTREAM_TYPE)
				<< "\") box is present.";
		throw JP2_Logic_Error (message.str (), ID);
		}
	Codestream_Parameters->add (codestream_segment
		(marker, segment_length, data_position));
	}
}


std::string
JP2_Metadata::segment_name
	(
	Marker_Code	marker_code
	)
{
for (const Marker_Name*
		table = MARKER_NAMES;
		table->marker;
	  ++table)
	if (table->marker == marker_code)
		return table->name;
return UNKNOWN_NAME;
}


std::string
JP2_Metadata::marker_number
	(
	Marker_Code	marker_code
	)
{
ostringstream
	number;
number << "0x" << hex << uppercase << setfill ('0') << marker_code;
return number.str ();
}


JP2_Metadata::Marker_Code
JP2_Metadata::segment_marker
	(
	const unsigned char*	data,
	long					amount
	)
{
if (amount >= 2)
	return get_short (data);
return (Marker_Code)INVALID_CODE;
}


int
JP2_Metadata::segment_length
	(
	const unsigned char*	data,
	long					amount
	)
{
int
	length = 0;
if (amount >= 2)
	{
	if (marker_flag_from_code (get_short (data))
			& DELIMITER_SEGMENTS)
		length = 2;
	else
	if (amount >= 4)
		length = get_short (data + 2);
	}
return length;
}


Aggregate*
JP2_Metadata::codestream_segment
	(
	Marker_Code	marker_code,
	int			segment_length,
	long long	data_position
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::add_codestream_segment: "
		<< segment_name (marker_code)
		<< " (" << marker_number (marker_code) << ')' << endl
	 << "    segment_length = " << segment_length << endl
	 << "     data_position = " << data_position << endl
	 << "       Data_Buffer @ " << (void*)Data_Buffer << endl
	 << "       Data_Amount = " << Data_Amount << endl;
#endif
if (segment_length < 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Invalid codestream segment length specified: "
			<< segment_length << endl
		<< "for " << segment_name (marker_code)
			<< " (" << marker_number (marker_code) << ") segment"
		<< data_position_report (data_position, '.');
	throw invalid_argument (message.str ());
	}

//	Segment basic parameters:

Aggregate*
	segment = new Aggregate (segment_name (marker_code));
segment->add (Assignment (MARKER_PARAMETER)
	= Integer (marker_code, Value::UNSIGNED, 16));
data_position_parameters (segment, segment_length, data_position, false);

//	Validity check:

if (Codestream_Validity == 0 &&
	marker_code != SOC_MARKER)
	{
	//	The first marker must be the SOC.
	ostringstream
		message;
	message
		<< "The first codestream segment must have the "
			<< segment_name (SOC_MARKER)
			<< " (" << marker_number (SOC_MARKER) << ") marker" << endl
		<< "The first codestream segment found: "
			<<  segment_name (marker_code) << '"'
			<< " (" << marker_number (marker_code) << ')'
		<< data_position_report (data_position, '.');
	throw JP2_Logic_Error (message.str (), ID);
	}
if (Codestream_Validity == SOC_FLAG &&
	marker_code != SIZ_MARKER)
	{
	//	The second marker must be the SIZ.
	ostringstream
		message;
	message
		<< "The second codestream segment must have the "
			<< segment_name (SIZ_MARKER)
			<< " (" << marker_number (SIZ_MARKER) << ") marker" << endl
		<< "The second codestream segment found: "
			<<  segment_name (marker_code) << '"'
			<< " (" << marker_number (marker_code) << ')'
		<< data_position_report (data_position, '.');
	throw JP2_Logic_Error (message.str (), ID);
	}

//	Segment content parameters:

switch (marker_code)
	{
	//	Delimiting markers.
	case EOC_MARKER:
	case SOC_MARKER:
	case SOD_MARKER:
	case EPH_MARKER:
		break;

	//	Segments.
	case SOT_MARKER:	SOT_parameters (segment);	break;
	case SIZ_MARKER:	SIZ_parameters (segment);	break;
	case COD_MARKER:	COD_parameters (segment);	break;
	case COC_MARKER:	COC_parameters (segment);	break;
	case RGN_MARKER:	RGN_parameters (segment);	break;
	case QCD_MARKER:	QCD_parameters (segment);	break;
	case QCC_MARKER:	QCC_parameters (segment);	break;
	case POC_MARKER:	POC_parameters (segment);	break;
	case TLM_MARKER:	TLM_parameters (segment);	break;
	case PLM_MARKER:	PLM_parameters (segment);	break;
	case PLT_MARKER:	PLT_parameters (segment);	break;
	case PPM_MARKER:	PPM_parameters (segment, data_position);	break;
	case PPT_MARKER:	PPT_parameters (segment, data_position);	break;
	case SOP_MARKER:	SOP_parameters (segment);	break;
	case CRG_MARKER:	CRG_parameters (segment);	break;
	case COM_MARKER:	COM_parameters (segment);	break;
	default:
		data_position_parameters (segment, Data_Amount, data_position);
	}
Codestream_Validity |= marker_flag_from_code (marker_code);
#if ((DEBUG) & DEBUG_PVL)
clog << "    segment parameters -" << endl
	 << *segment
	 << "<<< JP2_Metadata::add_codestream_segment" << endl;
#endif
return segment;
}

/*..............................................................................
	Codestream segment parameters
*/
void
JP2_Metadata::SIZ_parameters
	(
	Aggregate*	segment
	)
{
#if ((DEBUG) & DEBUG_PVL)
clog << ">>> JP2_Metadata::SIZ_parameters" << endl;
int
	value;
#endif
segment->add (Assignment (CAPABILITY_PARAMETER)
	= Integer (
		#if ((DEBUG) & DEBUG_PVL)
		value =
		#endif
		get_unsigned_short_integer
		(segment->name () + ' ' + CAPABILITY_PARAMETER),
		Value::UNSIGNED, 16));
Size_2D
	reference_grid_size,
	tile_size;
Point_2D
	image_offsets,
	tile_offsets;
segment->add (Assignment (REFERENCE_GRID_WIDTH_PARAMETER)
	= Integer (reference_grid_size.Width = get_unsigned_integer
		(segment->name () + ' ' + REFERENCE_GRID_WIDTH_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (REFERENCE_GRID_HEIGHT_PARAMETER)
	= Integer (reference_grid_size.Height = get_unsigned_integer
		(segment->name () + ' ' + REFERENCE_GRID_HEIGHT_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (HORIZONTAL_IMAGE_OFFSET_PARAMETER)
	= Integer (image_offsets.X = get_unsigned_integer
		(segment->name () + ' ' + HORIZONTAL_IMAGE_OFFSET_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (VERTICAL_IMAGE_OFFSET_PARAMETER)
	= Integer (image_offsets.Y = get_unsigned_integer
		(segment->name () + ' ' + VERTICAL_IMAGE_OFFSET_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (TILE_WIDTH_PARAMETER)
	= Integer (tile_size.Width = get_unsigned_integer
		(segment->name () + ' ' + TILE_WIDTH_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (TILE_HEIGHT_PARAMETER)
	= Integer (tile_size.Height = get_unsigned_integer
		(segment->name () + ' ' + TILE_HEIGHT_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (HORIZONTAL_TILE_OFFSET_PARAMETER)
	= Integer (tile_offsets.X = get_unsigned_integer
		(segment->name () + ' ' + HORIZONTAL_TILE_OFFSET_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (VERTICAL_TILE_OFFSET_PARAMETER)
	= Integer (tile_offsets.Y = get_unsigned_integer
		(segment->name () + ' ' + VERTICAL_TILE_OFFSET_PARAMETER),
		Value::UNSIGNED));
unsigned int
	components,
	total_tiles;
segment->add (Assignment (IMAGE_BANDS_PARAMETER)
	= Integer (components = get_unsigned_short_integer
		(segment->name () + ' ' + IMAGE_BANDS_PARAMETER),
		Value::UNSIGNED));
total_tiles = (unsigned int)(
	ceil (((double)reference_grid_size.Width  - tile_offsets.X)
		/ tile_size.Width) *
	ceil (((double)reference_grid_size.Height - tile_offsets.Y)
		/ tile_size.Height));
#if ((DEBUG) & DEBUG_PVL)
clog << "             capability = " << value << endl
	 << "    reference grid size = " << reference_grid_size << endl
	 << "             image size = "
	 	<< (reference_grid_size.Width  - image_offsets.X) << "w, "
		<< (reference_grid_size.Height - image_offsets.Y) << 'h' << endl
	 << "          image offsets = " << image_offsets << endl
	 << "              tile size = " << tile_size << endl
	 << "           tile offsets = " << tile_offsets << endl
	 << "                  tiles = " << total_tiles << endl
	 << "             components = " << components << endl;
#endif

//	If not set from the image header box.
if (Image_Size.is_empty ())
	Image_Size.size (
		reference_grid_size.Width  - image_offsets.X,
		reference_grid_size.Height - image_offsets.Y);
if (Image_Bands == 0)
	Image_Bands = components;

unsigned int
	*pixel_width  = new unsigned int[components],
	*pixel_height = new unsigned int[components];
int
	datum;
Array
	Ssiz  (Value::SEQUENCE),
	XRsiz (Value::SEQUENCE),
	YRsiz (Value::SEQUENCE);
for (unsigned int
		component = 0;
		component < components;
	  ++component)
	{
	if ((datum = get_byte (segment->name () + ' ' + VALUE_BITS_PARAMETER) + 1)
			< 0)
		datum = -(datum & 0x7F);
	Ssiz.add (new Integer (datum));
	XRsiz.add (new Integer (pixel_width[component] = get_unsigned_byte
		(segment->name () + ' ' + HORIZONTAL_SAMPLE_SPACING_PARAMETER),
		Value::UNSIGNED));
	YRsiz.add (new Integer (pixel_height[component] = get_unsigned_byte
		(segment->name () + ' ' + VERTICAL_SAMPLE_SPACING_PARAMETER),
		Value::UNSIGNED));
	}	
segment->add (value_bits_parameter (Ssiz));

Assignment
	*parameter;
parameter = new Assignment (HORIZONTAL_SAMPLE_SPACING_PARAMETER);
*parameter = XRsiz;
segment->add (parameter);

parameter = new Assignment (VERTICAL_SAMPLE_SPACING_PARAMETER);
*parameter = YRsiz;
segment->add (parameter);

if (! (Codestream_Validity & SIZ_FLAG))
	{
	//	Set the cache values.
	pixel_precision (&Ssiz, segment->name () + " codestream segment");

	Pixel_Width  = new unsigned int[Image_Bands];
	memcpy (Pixel_Width,  pixel_width,  Image_Bands * sizeof (unsigned int));
	Pixel_Height = new unsigned int[Image_Bands];
	memcpy (Pixel_Height, pixel_height, Image_Bands * sizeof (unsigned int));

	Reference_Grid_Size = reference_grid_size;
	Tile_Size = tile_size;
	Image_Offsets = image_offsets;
	Tile_Offsets = tile_offsets;
	Total_Tiles = total_tiles;
	}
#if ((DEBUG) & DEBUG_PVL)
clog << "<<< JP2_Metadata::SIZ_parameters" << endl;
#endif
}


void
JP2_Metadata::SOT_parameters
	(
	Aggregate*	segment
	)
{
segment->add (Assignment (TILE_INDEX_PARAMETER)
	= Integer (get_unsigned_short_integer
		(segment->name () + ' ' + TILE_INDEX_PARAMETER)));
segment->add (Assignment (TILE_PART_LENGTH_PARAMETER)
	= Integer (get_unsigned_integer
		(segment->name () + ' ' + TILE_PART_LENGTH_PARAMETER))
		.units ("bytes"));
segment->add (Assignment (TILE_PART_INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + TILE_PART_INDEX_PARAMETER)));
segment->add (Assignment (TOTAL_TILE_PARTS_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + TOTAL_TILE_PARTS_PARAMETER)));;
}


void
JP2_Metadata::COD_parameters
	(
	Aggregate*	segment
	)
{
Assignment
	*parameter;

//	Scod:
int
	Scod = get_unsigned_byte (segment->name () + ' ' + CODING_STYLE_PARAMETER);
parameter = new Assignment (CODING_STYLE_PARAMETER);
parameter->comment (string ("\nEntropy coder precincts:\n") +
	"  Precinct size " +
	((Scod & ENTROPY_CODER_STYLE) ?
		(string ("defined in the ") + PRECINCT_SIZE_PARAMETER + " parameter.") :
		 string ("= 32768 x 32768")) + '\n' +
	((Scod & SOP_FLAG) ? "  " : "  No ") + "SOP marker segments used\n" +
	((Scod & EPH_FLAG) ? "  " : "  No ") + "EPH marker used");
*parameter = Integer (Scod, Value::UNSIGNED, 16);
segment->add (parameter);

//	SGcod:
int
	progression_order = get_unsigned_byte
		(segment->name () + ' ' + PROGRESSION_ORDER_PARAMETER);
if (Progression_Order < 0)
	Progression_Order = progression_order;
parameter = new Assignment (PROGRESSION_ORDER_PARAMETER);
parameter->comment (string ("\nProgression order:\n") +=
	((progression_order < (int)(sizeof (PROGRESSION_ORDERS) / sizeof (char*))) ?
		PROGRESSION_ORDERS[progression_order] : "Unknown"));
*parameter = Integer (progression_order, Value::UNSIGNED);
segment->add (parameter);
int
	quality_layers = get_unsigned_short_integer
		(segment->name () + ' ' + QUALITY_LAYERS_PARAMETER);
if (! Quality_Layers)
	Quality_Layers = quality_layers;
segment->add (Assignment (QUALITY_LAYERS_PARAMETER)
	= Integer (quality_layers,
		Value::UNSIGNED));
segment->add (Assignment (MULTIPLE_COMPONENT_TRANSFORM_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + MULTIPLE_COMPONENT_TRANSFORM_PARAMETER),
		Value::UNSIGNED));

//	SPcod:
coding_style_parameters (segment, Scod & ENTROPY_CODER_STYLE);
}


void
JP2_Metadata::coding_style_parameters
	(
	Aggregate*	segment,
	bool		has_precinct_sizes
	)
{
int
	levels,
	datum;
Assignment
	*parameter;
ostringstream
	comment;
segment->add (Assignment (RESOLUTION_LEVELS_PARAMETER)
	= Integer (levels = get_unsigned_byte
		(segment->name () + ' ' + RESOLUTION_LEVELS_PARAMETER) + 1,
		Value::UNSIGNED));
if (! Resolution_Levels)
	Resolution_Levels = levels + 1;

parameter = new Assignment (CODE_BLOCK_WIDTH_PARAMETER);
datum = get_unsigned_byte
	(segment->name () + ' ' + CODE_BLOCK_WIDTH_PARAMETER);
comment << "\nCode-block width exponent offset " << datum << '.';
parameter->comment (comment.str ());
*parameter= Integer (1 << (datum + 2), Value::UNSIGNED);
segment->add (parameter);

parameter = new Assignment (CODE_BLOCK_HEIGHT_PARAMETER);
datum = get_unsigned_byte
	(segment->name () + ' ' + CODE_BLOCK_HEIGHT_PARAMETER);
comment.str ("");
comment << "\nCode-block height exponent offset " << datum << '.';
parameter->comment (comment.str ());
*parameter= Integer (1 << (datum + 2), Value::UNSIGNED);
segment->add (parameter);

parameter = new Assignment (CODE_BLOCK_STYLE_PARAMETER);
datum = get_unsigned_byte
	(segment->name () + ' ' + CODE_BLOCK_STYLE_PARAMETER);
comment.str ("");
comment
	<< "\nCode-block style:\n"
	<< ((datum & SELECTIVE_ARITHMETIC_BYPASS_FLAG) ? "  S" : "  No s")
		<< "elective arithmetic coding bypass.\n"
	<< ((datum & RESET_CONTEXT_PROBABILITIES) ? "  R" : "  No r")
		<< "eset of context probabilities on coding pass boundaries.\n"
	<< ((datum & TERMINATION_FLAG) ? "  T" : "  No t")
		<< "ermination on each coding pass.\n"
	<< ((datum & VERTICALLY_CAUSAL_CONTEXT_FLAG) ? "  V" : "  No v")
		<< "erticallly causal context.\n"
	<< ((datum & PREDICTABLE_TERMINATION_FLAG) ? "  P" : "  No p")
		<< "redictable termination.\n"
	<< ((datum & SEGMENTATION_SYMBOLS_FLAG) ? "  S" : "  No s")
		<< "egmentation symbols are used.";
parameter->comment (comment.str ());
*parameter = Integer (datum, Value::UNSIGNED, 16);
segment->add (parameter);

parameter = new Assignment (TRANSFORM_PARAMETER);
datum = get_unsigned_byte
	(segment->name () + ' ' + TRANSFORM_PARAMETER);
parameter->comment (string ("\nWavelet transformation used:\n  ") +=
	((datum < (int)(sizeof (TRANSFORMS) / sizeof (char*))) ?
		TRANSFORMS[datum] : "Unknown"));
*parameter = Integer (datum, Value::UNSIGNED);
segment->add (parameter);
if (Transform < 0)
	Transform = datum;

if (has_precinct_sizes)
	{
	Array
		list (Value::SEQUENCE),
		*pair;
	while (levels--)
		{
		pair = new Array (Value::SEQUENCE);
		datum = get_unsigned_byte
			(segment->name () + ' ' + PRECINCT_SIZE_PARAMETER);
		pair->add (new Integer (1 << (datum & 0x0F)));
		pair->add (new Integer (1 << ((datum & 0xF0) >> 4)));
		list.add (pair);
		}
	parameter = new Assignment (PRECINCT_SIZE_PARAMETER);
	parameter->comment ("\nPrecinct (width, height) by resolution level.");
	*parameter = list;
	segment->add (parameter);
	}
}


void
JP2_Metadata::COC_parameters
	(
	Aggregate*	segment
	)
{
if (! Image_Bands)
	no_Image_Bands_exception (segment->name ());

segment->add (Assignment (COMPONENT_INDEX_PARAMETER)
	= Integer (((Image_Bands < 257) ?
		(unsigned int)get_unsigned_byte
			(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER) :
		(unsigned int)get_unsigned_short_integer
			(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER)),
		Value::UNSIGNED));

//	Scoc:
int
	Scoc = get_unsigned_byte (segment->name () + ' ' + CODING_STYLE_PARAMETER);
Assignment
	*parameter = new Assignment (CODING_STYLE_PARAMETER);
parameter->comment (string ("\nEntropy coder precincts:\n") +
	"  Precinct size " +
	((Scoc & ENTROPY_CODER_STYLE) ?
		(string ("defined in the ") + PRECINCT_SIZE_PARAMETER + " parameter.") :
		 string ("= 32768 x 32768")));
*parameter = Integer (Scoc, Value::UNSIGNED, 16);
segment->add (parameter);

//	SPcoc:
coding_style_parameters (segment, Scoc & ENTROPY_CODER_STYLE);
}


void
JP2_Metadata::RGN_parameters
	(
	Aggregate*	segment
	)
{
if (! Image_Bands)
	no_Image_Bands_exception (segment->name ());

segment->add (Assignment (COMPONENT_INDEX_PARAMETER)
	= Integer (((Image_Bands < 257) ?
		(unsigned int)get_unsigned_byte
			(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER) :
		(unsigned int)get_unsigned_short_integer
			(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER)),
		Value::UNSIGNED));
segment->add (Assignment (ROI_STYLE_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + ROI_STYLE_PARAMETER),
		Value::UNSIGNED));
segment->add (Assignment (IMPLICIT_SHIFT_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + IMPLICIT_SHIFT_PARAMETER),
		Value::UNSIGNED));
}


void
JP2_Metadata::QCD_parameters
	(
	Aggregate*	segment
	)
{quantization_parameters (segment);}


void
JP2_Metadata::quantization_parameters
	(
	Aggregate*	segment
	)
{
int
	datum = get_unsigned_byte
		(segment->name () + ' ' + QUANTIZATION_STYLE_PARAMETER),
	style = datum & 3;
Assignment
	*parameter = new Assignment (QUANTIZATION_STYLE_PARAMETER);
parameter->comment (string ("\nQuantization:\n  ") +
	((style == NO_QUANTIZATION) ? "None" :
	((style == QUANTIZATION_SCALAR_DERIVED) ?
		"Scalar derived; values signalled for N-subL LL sub-band only." :
	((style == QUANTIZATION_SCALAR_EXPOUNDED) ?
		"Scalar expounded; values signalled for each sub-band." :
		"Unknown"))));
*parameter = Integer (style, Value::UNSIGNED);
segment->add (parameter);

parameter = new Assignment (TOTAL_GUARD_BITS_PARAMETER);
*parameter = Integer ((datum & 0xE0) >> 5, Value::UNSIGNED);
segment->add (parameter);

Array
	list = Array (Value::SEQUENCE),
	*array;
Value
	*entry;
while (Data_Amount)
	{
	//	Exponent
	datum = get_unsigned_byte (segment->name () + ' ' + STEP_SIZE_PARAMETER);
	if (style == NO_QUANTIZATION)
		entry = new Integer ((datum & 0xF8) >> 3, Value::UNSIGNED);
	else
		{
		//	Mantissa
		array = new Array (Value::SEQUENCE);
		array->add (new Integer ((datum & 0xF8) >> 3, Value::UNSIGNED));
		datum <<= 8;
		datum |= get_unsigned_byte
			(segment->name () + ' ' + STEP_SIZE_PARAMETER);
		array->add (new Integer (datum & 0x07FF, Value::UNSIGNED));
		entry = array;
		}
	list.add (entry);
	}
parameter = new Assignment (STEP_SIZE_PARAMETER);
parameter->comment (string ("\n") +
	((style == NO_QUANTIZATION) ?
		"Reversible transform dynamic range exponent" :
		"Irreversible transform quantization step size (exponent, mantissa)") +
	" by sub-band.");
*parameter = list;
segment->add (parameter);
}


void
JP2_Metadata::QCC_parameters
	(
	Aggregate*	segment
	)
{
if (! Image_Bands)
	no_Image_Bands_exception (segment->name ());

segment->add (Assignment (COMPONENT_INDEX_PARAMETER)
	= Integer (((Image_Bands < 257) ?
		get_unsigned_byte
			(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER) :
		get_unsigned_short_integer
			(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER)),
		Value::UNSIGNED));

quantization_parameters (segment);
}


void
JP2_Metadata::POC_parameters
	(
	Aggregate*	segment
	)
{
if (! Image_Bands)
	no_Image_Bands_exception (segment->name ());

Array
	levels (Value::SEQUENCE),
	components (Value::SEQUENCE),
	layers (Value::SEQUENCE),
	progression_orders (Value::SEQUENCE),
	*levels_range,
	*components_range;
int
	datum;
while (Data_Amount)
	{
	levels_range = new Array (Value::SEQUENCE);
	levels_range->add (new Integer (get_unsigned_byte
		(segment->name () + ' ' + LEVEL_INDEX_PARAMETER),
		Value::UNSIGNED));
	components_range = new Array (Value::SEQUENCE);
	components_range->add (new Integer 
		(((Image_Bands < 257) ?
			get_unsigned_byte
				(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER) :
			get_unsigned_short_integer
				(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER)),
		Value::UNSIGNED));
	layers.add (new Integer (get_unsigned_short_integer
		(segment->name () + ' ' + LAYER_INDEX_PARAMETER),
		Value::UNSIGNED));
	levels_range->add (new Integer (get_unsigned_byte
		(segment->name () + ' ' + LEVEL_INDEX_PARAMETER),
		Value::UNSIGNED));
	levels.add (levels_range);
	datum =
		(Image_Bands < 257) ?
			get_unsigned_byte
				(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER) :
			get_unsigned_short_integer
				(segment->name () + ' ' + COMPONENT_INDEX_PARAMETER);
	if (datum == 0)
		datum = 256;
	components_range->add (new Integer (datum, Value::UNSIGNED));
	components.add (components_range);
	progression_orders.add (new Integer (get_unsigned_byte
		(segment->name () + ' ' + PROGRESSION_ORDER_PARAMETER),
		Value::UNSIGNED));
	}
string
	comment ("\nIndex (start, end] ranges.");
Assignment
	*parameter;
parameter = new Assignment (LEVEL_INDEX_PARAMETER);
parameter->comment (comment);
*parameter = levels;
segment->add (parameter);

parameter = new Assignment (COMPONENT_INDEX_PARAMETER);
parameter->comment (comment);
*parameter = components;
segment->add (parameter);

parameter = new Assignment (LAYER_INDEX_PARAMETER);
*parameter = layers;
segment->add (parameter);

parameter = new Assignment (PROGRESSION_ORDER_PARAMETER);
*parameter = progression_orders;
segment->add (parameter);
}


void
JP2_Metadata::TLM_parameters
	(
	Aggregate*	segment
	)
{
segment->add (Assignment (INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + INDEX_PARAMETER),
		Value::UNSIGNED));

unsigned int
	index_size,
	length_size = get_unsigned_byte
		(segment->name () + ' ' +
		TILE_INDEX_SIZE_PARAMETER + " and " + TILE_PART_LENGTH_SIZE_PARAMETER);
length_size >>= 4;
index_size = length_size & 0x3;
length_size = (((length_size >> 2) & 0x3) + 1) << 1;
segment->add (Assignment (TILE_INDEX_SIZE_PARAMETER)
	= Integer (index_size, Value::UNSIGNED).units ("bytes"));
segment->add (Assignment (TILE_PART_LENGTH_SIZE_PARAMETER)
	= Integer (length_size, Value::UNSIGNED).units ("bytes"));

Array
	indices (Value::SEQUENCE),
	lengths (Value::SEQUENCE);
while (Data_Amount)
	{
	if (index_size)
		indices.add (new Integer (((index_size == 1) ?
			get_unsigned_byte 
				(segment->name () + ' ' + TILE_INDEX_PARAMETER) :
			get_unsigned_short_integer
				(segment->name () + ' ' + TILE_INDEX_PARAMETER)),
			Value::UNSIGNED));
	lengths.add (new Integer (((length_size == 2) ?
			get_unsigned_short_integer
				(segment->name () + ' ' + TILE_PART_LENGTH_PARAMETER) :
			get_unsigned_integer
				(segment->name () + ' ' + TILE_PART_LENGTH_PARAMETER)),
			Value::UNSIGNED));
	}
if (index_size)
	segment->add (Assignment (TILE_INDEX_PARAMETER)
		= indices);
segment->add (Assignment (TILE_PART_LENGTH_PARAMETER)
	= lengths.units ("bytes"));
}


void
JP2_Metadata::PLM_parameters
	(
	Aggregate*	segment
	)
{
segment->add (Assignment (INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + INDEX_PARAMETER),
		Value::UNSIGNED));

if (! PLM_Packet_Length_Array)
	{
	//	First PLM segment; add the new packet lengths parameter and array.
	PLM_Packet_Length_Array = new Array (Value::SEQUENCE);
	PLM_Packet_Length_Array->units ("bytes");
	segment->add (Assignment (PACKET_LENGTH_PARAMETER)
		= *PLM_Packet_Length_Array);
	}
else
	//	Continuation PLM segment; append to the existing lengths array.
	segment->add (Assignment (CONTINUATION_PARAMETER)
		= String ("true"));

long
	datum;
while (Data_Amount)
	{
	if (PLM_Packet_Length_Bytes_Remaining == 0)
		PLM_Packet_Length_Bytes_Remaining = get_unsigned_byte
			(segment->name () + " byte count");
	datum = get_unsigned_byte
		(segment->name () + ' ' + PACKET_LENGTH_PARAMETER);
	PLM_Packet_Length <<= 7;
	PLM_Packet_Length |= datum & 0x7F;
	if (! --PLM_Packet_Length_Bytes_Remaining)
		{
		PLM_Packet_Length_Array->add (new Integer
			(PLM_Packet_Length, Value::UNSIGNED));
		PLM_Packet_Length = 0;
		}
	}
}


void
JP2_Metadata::PLT_parameters
	(
	Aggregate*	segment
	)
{
segment->add (Assignment (INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + INDEX_PARAMETER),
		Value::UNSIGNED));

if (! PLT_Packet_Length_Array)
	{
	//	First PLT segment; add the new packet lengths parameter and array.
	PLT_Packet_Length_Array = new Array (Value::SEQUENCE);
	PLT_Packet_Length_Array->units ("bytes");
	segment->add (Assignment (PACKET_LENGTH_PARAMETER)
		= *PLT_Packet_Length_Array);
	}
else
	//	Continuation PLT segment; append to the existing lengths array.
	segment->add (Assignment (CONTINUATION_PARAMETER)
		= String ("true"));

long
	datum;
while (Data_Amount)
	{
	datum = get_unsigned_byte
		(segment->name () + ' ' + PACKET_LENGTH_PARAMETER);
	PLT_Packet_Length <<= 7;
	PLT_Packet_Length |= datum & 0x7F;
	if (! (PLT_Packet_Length_Bytes_Remaining = (datum & 0x80)))
		{
		PLT_Packet_Length_Array->add (new Integer
			(PLT_Packet_Length, Value::UNSIGNED));
		PLT_Packet_Length = 0;
		}
	}
}


void
JP2_Metadata::PPM_parameters
	(
	Aggregate*	segment,
	long long	data_position
	)
{
segment->add (Assignment (INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + INDEX_PARAMETER),
		Value::UNSIGNED));

if (data_position >= 0)
	data_position++;
data_position_parameters (segment, Data_Amount, data_position);
}


void
JP2_Metadata::PPT_parameters
	(
	Aggregate*	segment,
	long long	data_position
	)
{
segment->add (Assignment (INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + INDEX_PARAMETER),
		Value::UNSIGNED));

if (data_position >= 0)
	data_position++;
data_position_parameters (segment, Data_Amount, data_position);
}


void
JP2_Metadata::SOP_parameters
	(
	Aggregate*	segment
	)
{
segment->add (Assignment (INDEX_PARAMETER)
	= Integer (get_unsigned_byte
		(segment->name () + ' ' + INDEX_PARAMETER),
		Value::UNSIGNED));
}


void
JP2_Metadata::CRG_parameters
	(
	Aggregate*	segment
	)
{
Array
	horizontal_offsets (Value::SEQUENCE),
	vertical_offsets (Value::SEQUENCE);
while (Data_Amount)
	{
	horizontal_offsets.add (new Integer (get_unsigned_short_integer
		(segment->name () + ' ' + HORIZONTAL_COMPONENT_OFFSET_PARAMETER),
		Value::UNSIGNED));
	vertical_offsets.add (new Integer (get_unsigned_short_integer
		(segment->name () + ' ' + VERTICAL_COMPONENT_OFFSET_PARAMETER),
		Value::UNSIGNED));
	}

Assignment
	*parameter;
parameter = new Assignment (HORIZONTAL_COMPONENT_OFFSET_PARAMETER);
parameter->comment (string ("\nOffset in units of 1/65536 of the ")
	+ HORIZONTAL_SAMPLE_SPACING_PARAMETER + " by component.");
*parameter = horizontal_offsets;
segment->add (parameter);

parameter = new Assignment (VERTICAL_COMPONENT_OFFSET_PARAMETER);
parameter->comment (string ("\nOffset in units of 1/65536 of the ")
	+ VERTICAL_SAMPLE_SPACING_PARAMETER + " by component.");
*parameter = vertical_offsets;
segment->add (parameter);
}


void
JP2_Metadata::COM_parameters
	(
	Aggregate*	segment
	)
{
int
	data_type = get_unsigned_short_integer
		(segment->name () + ' ' + DATA_TYPE_PARAMETER);
segment->add (Assignment (DATA_TYPE_PARAMETER)
	= Integer (data_type, Value::UNSIGNED));

if (data_type == DATA_TYPE_TEXT)
	segment->add (Assignment (TEXT_DATA_PARAMETER)
		= String (get_string (Data_Amount, segment->name ()), String::TEXT));
else
	{
	//	DATA_TYPE_BINARY
	Array
		sequence (Value::SEQUENCE);
	while (Data_Amount)
		sequence.add (new Integer (get_unsigned_byte
			(segment->name () + ' ' + BINARY_DATA_PARAMETER),
			Value::UNSIGNED, 16));
	segment->add (Assignment (BINARY_DATA_PARAMETER) = sequence);
	}
}


int
JP2_Metadata::marker_flag_from_code
	(
	Marker_Code	marker_code
	)
{
int
	flag = 0;
switch (marker_code)
	{
	case   SOC_MARKER:
	flag = SOC_FLAG; break;
	case   SOD_MARKER:
	flag = SOD_FLAG; break;
	case   EPH_MARKER:
	flag = EPH_FLAG; break;
	case   EOC_MARKER:
	flag = EOC_FLAG; break;
	case   SIZ_MARKER:
	flag = SIZ_FLAG; break;
	case   COD_MARKER:
	flag = COD_FLAG; break;
	case   COC_MARKER:
	flag = COC_FLAG; break;
	case   RGN_MARKER:
	flag = RGN_FLAG; break;
	case   QCD_MARKER:
	flag = QCD_FLAG; break;
	case   QCC_MARKER:
	flag = QCC_FLAG; break;
	case   POC_MARKER:
	flag = POC_FLAG; break;
	case   SOT_MARKER:
	flag = SOT_FLAG; break;
	case   TLM_MARKER:
	flag = TLM_FLAG; break;
	case   PLM_MARKER:
	flag = PLM_FLAG; break;
	case   PLT_MARKER:
	flag = PLT_FLAG; break;
	case   PPM_MARKER:
	flag = PPM_FLAG; break;
	case   PPT_MARKER:
	flag = PPT_FLAG; break;
	case   SOP_MARKER:
	flag = SOP_FLAG; break;
	case   CRG_MARKER:
	flag = CRG_FLAG; break;
	case   COM_MARKER:
	flag = COM_FLAG; break;
	}
return flag;
}


JP2_Metadata::Marker_Code
JP2_Metadata::marker_code_from_flag
	(
	int		marker_flag
	)
{
Marker_Code
	mark = (Marker_Code)INVALID_CODE;
switch (marker_flag)
	{
	case   SOC_FLAG:
	mark = SOC_MARKER; break;
	case   SOD_FLAG:
	mark = SOD_MARKER; break;
	case   EPH_FLAG:
	mark = EPH_MARKER; break;
	case   EOC_FLAG:
	mark = EOC_MARKER; break;
	case   SIZ_FLAG:
	mark = SIZ_MARKER; break;
	case   COD_FLAG:
	mark = COD_MARKER; break;
	case   COC_FLAG:
	mark = COC_MARKER; break;
	case   RGN_FLAG:
	mark = RGN_MARKER; break;
	case   QCD_FLAG:
	mark = QCD_MARKER; break;
	case   QCC_FLAG:
	mark = QCC_MARKER; break;
	case   POC_FLAG:
	mark = POC_MARKER; break;
	case   SOT_FLAG:
	mark = SOT_MARKER; break;
	case   TLM_FLAG:
	mark = TLM_MARKER; break;
	case   PLM_FLAG:
	mark = PLM_MARKER; break;
	case   PLT_FLAG:
	mark = PLT_MARKER; break;
	case   PPM_FLAG:
	mark = PPM_MARKER; break;
	case   PPT_FLAG:
	mark = PPT_MARKER; break;
	case   SOP_FLAG:
	mark = SOP_MARKER; break;
	case   CRG_FLAG:
	mark = CRG_MARKER; break;
	case   COM_FLAG:
	mark = COM_MARKER; break;
	}
return mark;
}


std::string
JP2_Metadata::codestream_validity_report
	(
	unsigned int	validity_flags
	)
{
ostringstream
	report;
report
	<< "    Required Codestream main header segments:" << endl;
int
	flag;
for (flag  = MIN_MAIN_REQUIRED_SEGMENT_FLAG;
	 flag <= MAX_MAIN_REQUIRED_SEGMENT_FLAG;
	 flag <<= 1)
	report
		<< "      " << segment_name (marker_code_from_flag (flag))
			<< ((validity_flags & flag) ? "" : " not") << " present." << endl;
if (segments_are_complete (validity_flags))
	report << "        All required segments are present." << endl;

if (validity_flags & (MAIN_OPTIONAL_SEGMENTS | OPTIONAL_SEGMENTS))
	{
	report << "    Optional segments:" << endl;
	for (;
		 flag <= MIN_TILE_ONLY_FLAG;
		 flag <<= 1)
		if (validity_flags & flag)
			report << "      "
				<< segment_name (marker_code_from_flag (flag)) << endl;
	}

if (validity_flags & TILE_ONLY_SEGMENTS)
	{
	report << "    Tile-part segments:" << endl;
	for (flag  = MIN_TILE_ONLY_FLAG;
		 flag <= MAX_TILE_ONLY_FLAG;
		 flag <<= 1)
		if (validity_flags & flag)
			report << "      "
				<< segment_name (marker_code_from_flag (flag)) << endl;
	}

if (validity_flags & EOC_FLAG)
	report << "    " << segment_name (EOC_MARKER) << " found." << endl;

return report.str ();
}

/*------------------------------------------------------------------------------
	Data converters
*/
std::string
JP2_Metadata::to_string
	(
	unsigned int	value
	)
{
idaeim::Strings::String
	representation;
int
	shift = 24,
	mask = 0x7F000000;
for (shift = 24;
	 shift >= 0;
	 shift -= 8,
	 	mask >>= 8)
	representation += (char)((value & mask) >> shift);
//	Escape any non-printable characters.
representation.special_to_escape ();
return representation;
}


std::string
JP2_Metadata::get_string
	(
	long			size,
	const string&	description
	)
{
#if ((DEBUG) & DEBUG_GET_VALUE)
clog << ">>> JP2_Metadata::get_string: " << size << ' ' << description << endl;
#endif
idaeim::Strings::String
	string_buffer;
unsigned char
	character;
if (size < 0)
	size = 0;
while (size)
	{
	character = get_unsigned_byte (description);
	--size;
	if (character == 0)
		break;
	string_buffer += character;
	}
//	Escape any non-printable characters.
string_buffer.special_to_escape ();

if (size)
	{
	//	Consume remaining data.
	#if ((DEBUG) & DEBUG_GET_VALUE)
	clog << "    remaining size = " << size << endl;
	#endif
	if (size > Data_Amount)
		size = Data_Amount;
	Data_Amount -= size;
	Data_Buffer += size;
	}

#if ((DEBUG) & DEBUG_GET_VALUE)
clog << "<<< JP2_Metadata::get_string: " << string_buffer << endl;
#endif
return string_buffer;
}


long long
JP2_Metadata::get_long_integer
	(
	const string&	description
	)
{
if (! Data_Buffer ||
	Data_Amount < LONG_INTEGER_FIELD_SIZE)
	throw_data_exception
		(description, "long integer", LONG_INTEGER_FIELD_SIZE);
long long
	value (0);
memcpy (&value, Data_Buffer, LONG_INTEGER_FIELD_SIZE);
MSB_native (value);
Data_Buffer += LONG_INTEGER_FIELD_SIZE;
Data_Amount -= LONG_INTEGER_FIELD_SIZE;
return value;
}


long long
get_long
	(
	const void* data
	)
{
long long
	value (0);
memcpy (&value, data, JP2_Metadata::LONG_INTEGER_FIELD_SIZE);
MSB_native (value);
return value;
}


void
put_long
	(
	const long long&	value,
	void*				data
	)
{
long long
	datum (value);
MSB_native (datum);
memcpy (data, &datum, JP2_Metadata::LONG_INTEGER_FIELD_SIZE);
}


int
JP2_Metadata::get_integer
	(
	const string&	description
	)
{return (int)get_unsigned_integer (description);}


unsigned int
JP2_Metadata::get_unsigned_integer
	(
	const string&	description
	)
{
if (! Data_Buffer ||
	Data_Amount < INTEGER_FIELD_SIZE)
	throw_data_exception (description, "integer", INTEGER_FIELD_SIZE);
unsigned int
	value (0);
memcpy (&value, Data_Buffer, INTEGER_FIELD_SIZE);
MSB_native (value);
Data_Buffer += INTEGER_FIELD_SIZE;
Data_Amount -= INTEGER_FIELD_SIZE;
return value;
}


int
get_int
	(
	const void*	data
	)
{
unsigned int
	value (0);
memcpy (&value, data, JP2_Metadata::INTEGER_FIELD_SIZE);
MSB_native (value);
return value;
}


void
put_int
	(
	const int&	value,
	void*		data
	)
{
int
	datum (value);
MSB_native (datum);
memcpy (data, &datum, JP2_Metadata::INTEGER_FIELD_SIZE);
}


short
JP2_Metadata::get_short_integer
	(
	const string&	description
	)
{return (short)get_unsigned_short_integer (description);}


unsigned short
JP2_Metadata::get_unsigned_short_integer
	(
	const string&	description
	)
{
if (! Data_Buffer ||
	Data_Amount < 2)
	throw_data_exception (description, "short integer", 2);
unsigned short
	value (0);
memcpy (&value, Data_Buffer, 2);
MSB_native (value);
Data_Buffer += 2;
Data_Amount -= 2;
return value;
}


unsigned short
get_short
	(
	const void* data
	)
{
unsigned short
	value (0);
memcpy (&value, data, 2);
MSB_native (value);
return value;
}


void
put_short
	(
	const short&	value,
	void*			data
	)
{
short
	datum (value);
MSB_native (datum);
memcpy (data, &datum, 2);
}


int
JP2_Metadata::get_byte
	(
	const string&	description
	)
{
if (! Data_Buffer ||
	Data_Amount < 1)
	throw_data_exception (description, "byte", 1);
int
	value (*Data_Buffer);
++Data_Buffer;
--Data_Amount;
return value;
}


unsigned int
JP2_Metadata::get_unsigned_byte
	(
	const string&	description
	)
{
if (! Data_Buffer ||
	Data_Amount < 1)
	throw_data_exception (description, "byte", 1);
unsigned int
	value (*Data_Buffer & 0xFF);
++Data_Buffer;
--Data_Amount;
return value;
}


long long
JP2_Metadata::get_value
	(
	int				bits,
	const string&	description
	)
{
#if ((DEBUG) & DEBUG_GET_VALUE)
clog << ">>> JP2_Metadata::get_value: "
		<< bits << "-bit " << description << endl;
#endif
long long
	value = 0;
if (bits)
	{
	bool
		signed_value = false;
	if (bits < 0)
		{
		bits = -bits;
		if (bits < 64 &&	//	64-bit value does not need sign extension.
			bits > 1)		//	1-bit value is always unsigned.
			signed_value = true;
		}
	#if ((DEBUG) & DEBUG_GET_VALUE)
	clog << "    signed value = " << boolalpha << signed_value << endl;
	#endif
	if (bits > (int)(sizeof (long) << 3))
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to get a " << bits << "-bit " << description << " value.";
		throw out_of_range (message.str ());
		}

	//	Number of data bytes holding bits.
	int
		bytes = bytes_of_bits (bits);
	#if ((DEBUG) & DEBUG_GET_VALUE)
	clog << "bytes = " << bytes << endl;
	#endif
	if (! Data_Buffer ||
		Data_Amount < bytes)
		{
		ostringstream
			data_size_description;
		data_size_description << bits << "-bit value";
		throw_data_exception
			(description, data_size_description.str (), bytes);
		}

	//	Move the data bytes into the local variable.
	while (bytes)
		{
		value <<= 8;
		value |= get_unsigned_byte (description);
		--bytes;
		}

	if (signed_value &&
		(value & (1 << (bits - 1))))
		{
		//	Sign extension.
		value |= ~pixel_bits_mask (bits);
		#if ((DEBUG) & DEBUG_GET_VALUE)
		clog << "    sign extended" << endl;
		#endif
		}
	}
#if ((DEBUG) & DEBUG_GET_VALUE)
clog << "<<< JP2_Metadata::get_value: " << value << endl;
#endif
return value;
}


void
JP2_Metadata::throw_data_exception
	(
	const string&	description,
	const string&	data_type,
	int				amount_required
	)
{
ostringstream
	message;
if (! Data_Buffer)
	message << ID << endl;
message << "Invalid";
if (! description.empty ())
	message << ' ' << description;
message << ' ' << data_type << " data; ";
if (Data_Buffer)
	{
	message << Data_Amount << " of "
		<< amount_required << " required bytes.";
	throw JP2_Logic_Error (message.str (), ID);
	}
message << " no data buffer.";
throw invalid_argument (message.str ());
}




/*==============================================================================
	Utility:
*/
unsigned int
bytes_of_bits
	(
	unsigned int	bits
	)
{return (bits >> 3) + ((bits % 8) ? 1 : 0);}


unsigned long long
pixel_bits_mask
	(
	unsigned int	bits
	)
{
unsigned long long
	mask = 0;
while (bits--)
	{
	mask <<= 1;
	mask  |= 1;
	}
return mask;
}


}	//	namespace HiRISE
}	//	namespace UA

