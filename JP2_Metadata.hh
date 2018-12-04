/*	JP2_Metadata

HiROC CVS ID: $Id: JP2_Metadata.hh,v 2.5 2012/09/28 14:01:20 castalia Exp $

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

#ifndef _JP2_Metadata_
#define _JP2_Metadata_

#include	<string>

// PIRL++
#include "Dimensions.hh"

//	Forward references.
namespace idaeim::PVL {
class Parameter;
class Aggregate;
class Array;
}

namespace UA::HiRISE
{
//PIRL++ Dimensions classes.
using PIRL::Size_2D;
using PIRL::Rectangle;
using PIRL::Point_2D;


/**	<i>JP2_Metadata</i> has an idaeim::PVL::Aggregate with cached image
	characterization values.

	Commonly used metadata is cached in fast access data members.

	@author		Bradford Castalia; UA/HiROC
	@version	$Revision: 2.5 $
*/
class JP2_Metadata
{
public:
/*==============================================================================
	Types:
*/
//!	JP2 box {@link type_code(const std::string&, bool) type code} value.
typedef	unsigned int			Type_Code;

/**	Codestream {@link segment_marker(const unsigned char*, long) segment
	marker code} value.
*/
typedef unsigned short			Marker_Code;

//!	A constant name string.
typedef const char* const		Name_String;

/*==============================================================================
	Constants:
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*------------------------------------------------------------------------------
	JP2 Boxes
*/
//!	Box Type codes.
static const Type_Code
	SIGNATURE_TYPE,
	FILE_TYPE_TYPE,
	JP2_HEADER_TYPE,
		IMAGE_HEADER_TYPE,
		BITS_PER_COMPONENT_TYPE,
		COLOUR_SPECIFICATION_TYPE,
		PALETTE_TYPE,
		COMPONENT_MAPPING_TYPE,
		CHANNEL_DEFINITION_TYPE,
		RESOLUTION_TYPE,
			CAPTURE_RESOLUTION_TYPE,
			DEFAULT_DISPLAY_RESOLUTION_TYPE,
	CONTIGUOUS_CODESTREAM_TYPE,
	INTELLECTUAL_PROPERTY_TYPE,
	XML_TYPE,
	UUID_TYPE,
	UUID_INFO_TYPE,
		UUID_LIST_TYPE,
		URL_TYPE,
	ASSOCIATION_TYPE,
	LABEL_TYPE,
	PLACEHOLDER_TYPE;

//!	JP2 boxes flagged as added in the JP2_Validity bit field.
enum
	{
	//	Required boxes:
	MIN_REQUIRED_BOX_FLAG					= 1 << 0,
	SIGNATURE_FLAG							= 1 << 0,
	FILE_TYPE_FLAG							= 1 << 1,
	JP2_HEADER_FLAG							= 1 << 2,
	IMAGE_HEADER_FLAG						= 1 << 3,
	COLOUR_SPECIFICATION_FLAG				= 1 << 4,
	CONTIGUOUS_CODESTREAM_FLAG				= 1 << 5,
	MAX_REQUIRED_BOX_FLAG					= 1 << 5,
		REQUIRED_BOXES						= SIGNATURE_FLAG |
											  FILE_TYPE_FLAG |
											  JP2_HEADER_FLAG |
											  IMAGE_HEADER_FLAG |
											  COLOUR_SPECIFICATION_FLAG |
											  CONTIGUOUS_CODESTREAM_FLAG,
	//	Optional, once-only, boxes:
	MIN_OPTIONAL_ONE_BOX_FLAG				= 1 << 6,
	BITS_PER_COMPONENT_FLAG					= 1 << 6,
	PALETTE_FLAG							= 1 << 7,
	COMPONENT_MAPPING_FLAG					= 1 << 8,
	CHANNEL_DEFINITION_FLAG					= 1 << 9,
	RESOLUTION_FLAG							= 1 << 10,
	CAPTURE_RESOLUTION_FLAG					= 1 << 11,
	DEFAULT_DISPLAY_RESOLUTION_FLAG			= 1 << 12,
	MAX_OPTIONAL_ONE_BOX_FLAG				= 1 << 12,
		OPTIONAL_ONCE_BOXES					= BITS_PER_COMPONENT_FLAG |
											  PALETTE_FLAG |
											  COMPONENT_MAPPING_FLAG |
											  CHANNEL_DEFINITION_FLAG |
											  RESOLUTION_FLAG |
											  CAPTURE_RESOLUTION_FLAG |
											  DEFAULT_DISPLAY_RESOLUTION_FLAG,
	//	Optional boxes:
	MIN_OPTIONAL_BOX_FLAG					= 1 << 13,
	INTELLECTUAL_PROPERTY_FLAG				= 1 << 13,
	XML_FLAG								= 1 << 14,
	UUID_FLAG								= 1 << 15,
	UUID_INFO_FLAG							= 1 << 16,
	UUID_LIST_FLAG							= 1 << 17,
	URL_FLAG								= 1 << 18,
	ASSOCIATION_FLAG						= 1 << 19,
	LABEL_FLAG								= 1 << 20,
	PLACEHOLDER_FLAG						= 1 << 21,
	MAX_OPTIONAL_BOX_FLAG					= 1 << 21,
		OPTIONAL_BOXES						= INTELLECTUAL_PROPERTY_FLAG |
											  XML_FLAG |
											  UUID_FLAG |
											  UUID_INFO_FLAG |
											  UUID_LIST_FLAG |
											  URL_FLAG |
											  ASSOCIATION_FLAG |
											  LABEL_FLAG |
											  PLACEHOLDER_FLAG,
	MAX_BOX_FLAG							= 1 << 21
	};

/**	Used to indicate an invalid box type or segment marker code.

	@see	type_code(const std::string&)
*/
static const Type_Code
	INVALID_CODE;

/**	The name of an unknown box type or segment marker.

	@see	box_name(Type_Code)
	@see	segment_name(Marker_Code)
*/
static Name_String
	UNKNOWN_NAME;

//!	Box binary value field sizes in bytes.
static const int
	INTEGER_FIELD_SIZE,
	LONG_INTEGER_FIELD_SIZE;

//!	Box descriptive name parameter.
static Name_String
	NAME_PARAMETER;
//!	Box type code parameter.
static Name_String
	TYPE_PARAMETER;
//!	Box length parameter.
static Name_String
	LENGTH_PARAMETER;
//!	Box file position parameter.
static Name_String
	POSITION_PARAMETER;

//!	Signature box parameter.
static Name_String
	SIGNATURE_PARAMETER;
//!	The JP2 file format signature in the first 12 bytes of a file.
static const unsigned char
	JP2_SIGNATURE[12];
//!	Signature box content.
static const unsigned int
	SIGNATURE_CONTENT;

/**	Parameter name for specifying the position of data in the Source file
	(byte offset from the beginning of the file).
*/
static Name_String
	DATA_POSITION_PARAMETER;

//!	Parameter name for specifying the length (bytes) of a data block.
static Name_String
	DATA_LENGTH_PARAMETER;

//!	Parameter name for specifying the bit precision of a value.
static Name_String
	VALUE_BITS_PARAMETER;

//!	File Type box parameters.
static Name_String
	BRAND_PARAMETER,
	MINOR_VERSION_PARAMETER,
	COMPATIBILITY_LIST_PARAMETER;
//!	JP2 compatibility value.
static Name_String
	JP2_COMPATIBLE;

//!	Image Header box parameters.
static Name_String
	HEIGHT_PARAMETER,
	WIDTH_PARAMETER,
	IMAGE_BANDS_PARAMETER,
	COMPRESSION_TYPE_PARAMETER,
	COLORSPACE_UNKNOWN_PARAMETER,
	INTELLECTUAL_PROPERTY_PARAMETER;

//!	Colour Specification box parameters.
static Name_String
	SPECIFICATION_METHOD_PARAMETER,
	PRECEDENCE_PARAMETER,
	COLOURSPACE_APPROXIMATION_PARAMETER,
	ENUMERATED_COLOURSPACE_PARAMETER;
//!	Specification method values.
static const int
	ENUMERATED_COLOURSPACE,
	RESTRICTED_ICC_PROFILE;

//!	Palette box parameters.
static Name_String
	ENTRIES_PARAMETER,
	COLUMNS_PARAMETER,
	VALUES_PARAMETER;

//!	Component Mapping and Channel Definition parameter.
static Name_String
	MAP_PARAMETER;
//!	Component map indices.
static const int
	COMPONENT_INDEX,
	MAP_TYPE_INDEX,
	PALETTE_INDEX;
//!	Map type values.
static const int
	DIRECT_USE,
	PALETTE_MAPPING;

//!	Channel Definition indices.
static const int
	CHANNEL_INDEX,
	CHANNEL_TYPE_INDEX,
	CHANNEL_ASSOCIATION_INDEX;
//!	Channel type values.
static const int
	COLOUR_IMAGE_DATA,
	OPACITY,
	PREMULTIPLIED_OPACITY;
//!	Channel association values.
static const int
	IMAGE_ASSOCIATION,
	NO_ASSOCIATION;

//!	Capture and Default Resolution parameters.
static Name_String
	VERTICAL_NUMERATOR_PARAMETER,
	VERTICAL_DENOMINATOR_PARAMETER,
	VERTICAL_EXPONENT_PARAMETER,
	HORIZONTAL_NUMERATOR_PARAMETER,
	HORIZONTAL_DENOMINATOR_PARAMETER,
	HORIZONTAL_EXPONENT_PARAMETER;

//!	Codestream parameters.
static Name_String
	CODESTREAM_PARAMETER;

//!	UUID parameter.
static Name_String
	UUID_PARAMETER;
//! UUID value count.
static const int
	UUID_SIZE;

//!	URL parameters.
static Name_String
	VERSION_PARAMETER,
	FLAGS_PARAMETER,
	URL_PARAMETER;

//!	Text parameters.
static Name_String
	TEXT_PARAMETER;

//!	JPIP Placeholder parameters.
static Name_String
	ORIGINAL_BOX_PARAMETER,
	BIN_ID_PARAMETER,
	EQUIVALENT_BOX_PARAMETER,
	CODESTREAM_ID_PARAMETER,
	TOTAL_CODESTREAMS_PARAMETER,
	EXTENDED_BOX_LIST_PARAMETER;
//!	Placeholder box flags value bit masks.
static const int
	PLACEHOLDER_FLAGS_ORIGINAL_MASK,
	PLACEHOLDER_FLAGS_EQUIVALENT_MASK,
	PLACEHOLDER_FLAGS_CODESTREAM_MASK,
	PLACEHOLDER_FLAGS_MULTIPLE_CODESTREAM_MASK;

/*------------------------------------------------------------------------------
	Codestream Segments
*/
//!	Marker codes.
static const Marker_Code
	//	Delimiters:
	SOC_MARKER,
	SOD_MARKER,
	EPH_MARKER,
	EOC_MARKER,

	//	Fixed information.
	SIZ_MARKER,

	//	Functional.
	COD_MARKER,
	COC_MARKER,
	RGN_MARKER,
	QCD_MARKER,
	QCC_MARKER,
	POC_MARKER,
	SOT_MARKER,

	//	Pointer.
	TLM_MARKER,
	PLM_MARKER,
	PLT_MARKER,
	PPM_MARKER,
	PPT_MARKER,

	//	Bitstream.
	SOP_MARKER,

	//	Informational.
	CRG_MARKER,
	COM_MARKER,

	//	Reserved.
	RESERVED_DELIMITER_MARKER_MIN,
	RESERVED_DELIMITER_MARKER_MAX;

//!	Codestream validity flags.
enum
	{
	//	Main header; required:
	MIN_MAIN_REQUIRED_SEGMENT_FLAG			= 1 << 0,
	SOC_FLAG								= 1 << 0,
	SIZ_FLAG								= 1 << 1,
	//		COD optional in tile-part.
	QCD_FLAG								= 1 << 2,
	COD_FLAG								= 1 << 3,
	MAX_MAIN_REQUIRED_SEGMENT_FLAG			= 1 << 3,
		MAIN_REQUIRED_SEGMENTS				= SOC_FLAG |
											  SIZ_FLAG |
											  QCD_FLAG |
											  COD_FLAG,
	//	Main header only; optional:
	MIN_MAIN_OPTIONAL_SEGMENT_FLAG			= 1 << 4,
	TLM_FLAG								= 1 << 4,
	PLM_FLAG								= 1 << 5,
	PPM_FLAG								= 1 << 6,
	MAX_MAIN_OPTIONAL_SEGMENT_FLAG			= 1 << 6,
		MAIN_OPTIONAL_SEGMENTS				= TLM_FLAG |
											  PLM_FLAG |
											  PPM_FLAG,
	//	Optional main header or tile-part:
	MIN_OPTIONAL_SEGMENT_FLAG				= 1 << 7,
	COC_FLAG								= 1 << 7,
	RGN_FLAG								= 1 << 8,
	QCC_FLAG								= 1 << 9,
	POC_FLAG								= 1 << 10,
	EPH_FLAG								= 1 << 11,
	CRG_FLAG								= 1 << 12,
	COM_FLAG								= 1 << 13,
	MAX_OPTIONAL_SEGMENT_FLAG				= 1 << 13,
		OPTIONAL_SEGMENTS					= COC_FLAG |
											  RGN_FLAG |
											  QCC_FLAG |
											  POC_FLAG |
											  EPH_FLAG |
											  CRG_FLAG |
											  COM_FLAG,
	//	Tile-part and bitstream.
	MIN_TILE_ONLY_FLAG						= 1 << 14,
	SOT_FLAG					 			= 1 << 14,
	PPT_FLAG								= 1 << 15,
	PLT_FLAG								= 1 << 16,
	SOD_FLAG								= 1 << 17,
	MAX_TILE_ONLY_FLAG						= 1 << 17,
		TILE_ONLY_SEGMENTS					= SOT_FLAG |
											  PPT_FLAG |
											  PLT_FLAG |
											  SOD_FLAG,
	//		Bitstream only.
	SOP_FLAG								= 1 << 18,
	//	End of codestream after all tile-parts and bitstream.
	EOC_FLAG								= 1 << 19,

	//	Delimiter segments:
	DELIMITER_SEGMENTS						= SOC_FLAG |
											  SOD_FLAG |
											  EPH_FLAG |
											  EOC_FLAG,

	MAX_SEGMENT_FLAG						= 1 << 19
	};

//!	Marker code parameter.
static Name_String
	MARKER_PARAMETER;

//!	SOT parameters.
static Name_String
	TILE_INDEX_PARAMETER,
	TILE_PART_LENGTH_PARAMETER,
	TILE_PART_INDEX_PARAMETER,
	TOTAL_TILE_PARTS_PARAMETER;

//!	SIZ parameters.
static Name_String
	CAPABILITY_PARAMETER,
	REFERENCE_GRID_WIDTH_PARAMETER,
	REFERENCE_GRID_HEIGHT_PARAMETER,
	HORIZONTAL_IMAGE_OFFSET_PARAMETER,
	VERTICAL_IMAGE_OFFSET_PARAMETER,
	TILE_WIDTH_PARAMETER,
	TILE_HEIGHT_PARAMETER,
	HORIZONTAL_TILE_OFFSET_PARAMETER,
	VERTICAL_TILE_OFFSET_PARAMETER,
	HORIZONTAL_SAMPLE_SPACING_PARAMETER,
	VERTICAL_SAMPLE_SPACING_PARAMETER;

//!	COD parameters.
static Name_String
	CODING_STYLE_PARAMETER,
	PROGRESSION_ORDER_PARAMETER,
	QUALITY_LAYERS_PARAMETER,
	MULTIPLE_COMPONENT_TRANSFORM_PARAMETER,
	RESOLUTION_LEVELS_PARAMETER,
	CODE_BLOCK_WIDTH_PARAMETER,
	CODE_BLOCK_HEIGHT_PARAMETER,
	CODE_BLOCK_STYLE_PARAMETER,
	TRANSFORM_PARAMETER,
	PRECINCT_SIZE_PARAMETER;
//!	Coding style bit flag masks.
static const int
	ENTROPY_CODER_STYLE,
	SOP_STYLE,
	EPH_STYLE;
//!	Progression order values.
static const int
	LRCP_PROGRESSION_ORDER,
	RLCP_PROGRESSION_ORDER,
	RPCL_PROGRESSION_ORDER,
	PCRL_PROGRESSION_ORDER,
	CPRL_PROGRESSION_ORDER;
static Name_String
	PROGRESSION_ORDERS[];
//!	Code block style bit flag masks.
static const int
	SELECTIVE_ARITHMETIC_BYPASS_FLAG,
	RESET_CONTEXT_PROBABILITIES,
	TERMINATION_FLAG,
	VERTICALLY_CAUSAL_CONTEXT_FLAG,
	PREDICTABLE_TERMINATION_FLAG,
	SEGMENTATION_SYMBOLS_FLAG;
//!	Transform values.
static const int
	TRANSFORM_IRREVERSIBLE,
	TRANSFORM_REVERSIBLE;
static Name_String
	TRANSFORMS[];

//!	COC parameters.
static Name_String
	COMPONENT_INDEX_PARAMETER;

//!	RGN parameters.
static Name_String
	ROI_STYLE_PARAMETER,
	IMPLICIT_SHIFT_PARAMETER;

//!	QCD parameters.
static Name_String
	QUANTIZATION_STYLE_PARAMETER,
	TOTAL_GUARD_BITS_PARAMETER,
	STEP_SIZE_PARAMETER;
//!	Quantization style bit field masks.
static const int
	NO_QUANTIZATION,
	QUANTIZATION_SCALAR_DERIVED,
	QUANTIZATION_SCALAR_EXPOUNDED;

//!	POC parameters.
static Name_String
	LEVEL_INDEX_PARAMETER,
	LAYER_INDEX_PARAMETER;

//!	TLM parameters.
static Name_String
	INDEX_PARAMETER,
	TILE_INDEX_SIZE_PARAMETER,
	TILE_PART_LENGTH_SIZE_PARAMETER;

//!	PLM and PLT parameters.
static Name_String
	PACKET_LENGTH_PARAMETER,
	CONTINUATION_PARAMETER;

//!	CRG parameters.
static Name_String
	HORIZONTAL_COMPONENT_OFFSET_PARAMETER,
	VERTICAL_COMPONENT_OFFSET_PARAMETER;

//!	COM parameters.
static Name_String
	DATA_TYPE_PARAMETER,
	BINARY_DATA_PARAMETER,
	TEXT_DATA_PARAMETER;
//!	Data type values.
static const int
	DATA_TYPE_BINARY,
	DATA_TYPE_TEXT;

/*==============================================================================
	Constructors:
*/
/**	Construct a JP2_Metadata with no data content.
*/
JP2_Metadata ();

/*	Construct a copy of JP2_Metadata.

	@param	JP2_metadata	The JP2_Metadata to be copied.
*/
JP2_Metadata (const JP2_Metadata& JP2_metadata);

/**	Destroy this JP2_Metadata.

	Any {@link locally_owned_content() locally owned content} data
	is freed.
*/
virtual ~JP2_Metadata ();

/*==============================================================================
	Accessors:
*/
/**	Set the name of the metadata source.

	@param	name	The name to be used as the source of metadata.
	@return	This JP2_Metadata.
	@see	source_name()
*/
inline JP2_Metadata& source_name (const std::string& name)
	{Source_Name = name; return *this;}

/**	Get the name of the metadata source.

	The type name is the string representation of the {@link type_code()
	type code} bytes that are stored as the second value of each JP2 box.

	@return	The name of the source that provided the metadata. This will be
		the empty string if the source is unknown.
*/
inline std::string source_name () const
	{return Source_Name;}

/**	Get the size of the JP2 source image.

	<b>N.B.</b>: Use the image_width() and image_height() methods for
	fast access to the image size values.

	@return	A Size_2D object containing the image width and height.
		These values will be zero until the JP2 source has been {@link
		open() opened}.
*/
inline Size_2D image_size () const
	{return Image_Size;}

/**	Get the width of the JP2 source image.

	@return	The width of the image in pixels. This will be zero until
		the JP2 source has been {@link open() opened}.
*/
inline unsigned int image_width () const
	{return Image_Size.Width;}

/**	Get the height of the JP2 source image.

	@return	The height of the image in lines. This will be zero until
		the JP2 source has been {@link open() opened}.
*/
inline unsigned int image_height () const
  {return Image_Size.Height;}

/**	Get the number of bands (components) in the JP2 image.

	@return	The number of bands in the JP2 image. This will be zero
		until the JP2 source has been {@link open() opened}.
*/
inline unsigned int image_bands () const
	{return Image_Bands;}

/**	Get the number of bytes per pixel.

	@param	band	The image band for which to get the number of pixel bytes.
		Images with the same {@link pixel_precision(int) pixel precision}
		for each band may omit the argument. The value should be in the
		range 0 - {@link image_bands() bands}.
	@return	The number of storage bytes used by each pixel. This will be
		-1 until the JP2 source has been {@link open() opened}, or if
		the specified band is outside the range of available bands.
*/
int pixel_bytes (int band = 0) const;

/**	Get the pixel precision.

	@param	band	The image band for which to get the number of pixel bytes.
		Images with the same {@link pixel_precision(int) pixel precision}
		for each band may omit the argument. The value should be in the
		range 0 - {@link image_bands() bands}.
	@return	The number of valid bits used by each pixel. This will be
		-1 until the JP2 source has been {@link open() opened}.
*/
int pixel_precision (int band = 0) const;

/**	Get the signedness of the pixel data.

	@param	band	The image band for which to get the number of pixel bytes.
		Images with the same {@link pixel_precision(int) pixel precision}
		for each band may omit the argument. The value should be in the
		range 0 - {@link image_bands() bands}.
	@return	true if the pixel data is treated as signed; false
		otherwise. This information will be meaningless until the JP2
		source has been {@link open() opened}.
*/
bool pixel_signed (int band = 0) const;

/**	Get the size of a pixel in reference grid units.

	The size of a pixel is determined from the codestream SIZ segment.
	Pixel size is measured in codestream reference grid units. The pixel
	size may different in in the horizontal and vertical dimensions and
	for each {@link image_bands() image band}. The {@link
	#HORIZONTAL_SAMPLE_SPACING_PARAMETER} and {@link
	#VERTICAL_SAMPLE_SPACING_PARAMETER} parameters have the pixel size
	values.

	@param	band	The image band for which to get the number of pixel
		bytes. Images with the same {@link pixel_precision(int) pixel
		precision} for each band may omit the argument. The value should
		be in the range 0 - {@link image_bands() bands}.
	@return	A Size_2D containing the size of the pixel in reference grid
		units for the specified image band.
*/
Size_2D pixel_size (int band = 0) const;

/**	Get the producer UUID found in the JP2 source.

	The producer UUID will be from the first occurance of a UUID List box
	with a single entry paired with a single URL box in a UUID Info super
	box. Any other UUID values will be ignored. This rule conforms to the
	PDS/JP2 specification for providing the data producer UUID and the
	URL for the associated PDS label file.

	<b>N.B.</b>: There may be additional UUID values is the JP2 source
	that do not qualify as a producer UUID, or occur after the first
	qualifying UUID is found.

	@return	A 16 byte (UUID_SIZE) array. <b>N.B.</b>: This is not a
		null-terminated C-string character array. This will be NULL if a
		qualifying UUID has not been added.
*/
inline unsigned char* producer_UUID () const
	{return Producer_UUID;}

/**	Get the PDS label file URL found in the JP2 source.

	The label URL will be from the first occurance of a UUID Info super
	box with a single URL box paired with a UUID List box with a single
	UUID entry. Any other URL values will be ignored. This rule conforms
	to the PDS/JP2 specification for providing the data producer UUID and
	the URL for the associated PDS label file.

	<b>N.B.</b>: There may be additional URL values in the JP2 source
	that do not qualify as described here, or occur after the first
	qualifying URL is found.

	@return	A URL string. This will be empty if a qualifying URL has not
		been added. The URL is likely to be a simple filename for a
		sibling PDS label file to the JP2 source file.
	@see	producer_UUID()
*/
inline std::string label_URL () const
	{return Label_URL;}

/**	Get the image reference grid size.

	The values are obtained from the first SIZ segment in the codestream
	main header.

	@return	A Size_2D containing the image reference grid size.
*/
inline Size_2D reference_grid_size () const
	{return Reference_Grid_Size;}

/**	Get the codestream tile size.

	The values are obtained from the first SIZ segment in the codestream
	main header.

	@return	A Size_2D containing the codestream tile size.
*/
inline Size_2D tile_size () const
	{return Tile_Size;}

/**	Get the image position offsets on the {@link reference_grid_size()
	reference grid}.

	The values are obtained from the first SIZ segment in the codestream
	main header.

	@return	A Point_2D containing the image position offsets.
*/
inline Point_2D image_offsets () const
	{return Image_Offsets;}

/**	Get the tile grid position offsets on the {@link reference_grid_size()
	reference grid}.

	The values are obtained from the first SIZ segment in the codestream
	main header.

	@return	A Point_2D containing the tile grid position offsets.
*/
inline Point_2D tile_offsets () const
	{return Tile_Offsets;}

/**	Get the total number of tiles in the codestream.

	The value is determined by the first SIZ segment in the codestream
	main header.

	@return	The total number of tiles in the codestream. Typically an
		image codestream is composed of a single tile.
*/
inline unsigned int total_tiles () const
	{return Total_Tiles;}

/**	Get the number of codestream resolution levels.


	The value is obtained from the first set of coding style parameters,
	typically from the COD (coding style default) segment, in the
	codestream main header.

	The number of available resolution levels is determined from the
	first set of coding style parameters,
	typically from the COD (coding style default) segment, in the
	codestream main header. The resolution level
	available in specific tile segments may be lower still, but that
	won't be known until the tiles have been read.

	<b>N.B.</b>: The number of resolution levels is one more than the
	number of JPEG2000 decomposition levels.

	@return	The number of codestream resolution levels. This will be zero
		until the number of levels has been determined by {@link
		add_codestream_segment(Marker_Code, const unsigned char*, int,
		long long) adding a codestream COD segment}.
*/
inline unsigned int resolution_levels () const
	{return Resolution_Levels;}

/**	Get the number of image quality layers.

	The value is obtained from the first COD (coding style default)
	segment in the codestream main header.

	@return	The default number of image quality layers. The number of
		quality levels available in specific tile segments may be
		different, but that won't be known until the tiles have been
		read.
*/
inline unsigned int quality_layers () const
	{return Quality_Layers;}

/**	Get the codestream encoding progression order.

	The value is obtained from the first COD (coding style default)
	segment in the codestream main header.

	The descripton for the known progression order codes is provided by
	the PROGRESSION_ORDERS string array.

	@return	A codestream encoding progression order code:

		- LRCP_PROGRESSION_ORDER: Layer-Resolution-Component-Position
		- RLCP_PROGRESSION_ORDER: Resolution-Layer-Component-Position
		- RPCL_PROGRESSION_ORDER: Resolution-Position-Component-Layer
		- PCRL_PROGRESSION_ORDER: Position-Component-Resolution-Layer
		- CPRL_PROGRESSION_ORDER: Component-Position-Resolution-Layer
*/
inline int progression_order () const
	{return Progression_Order;}

/**	Get the codestream transform.

	The value is obtained from the first set of coding style parameters,
	typically from the COD (coding style default) segment, in the
	codestream main header.

	The descripton for the known transform codes is provided by
	the TRANSFORMS string array.

	@return	A codestream transform code:

		- TRANSFORM_IRREVERSIBLE: 9-7 irreversible filter
		- TRANSFORM_REVERSIBLE:   5-3 reversible filter
*/
inline int transform () const
	{return Transform;}

/**	Reset all metadata to their initial values.

	All {@link parameters() parameters} are cleared and cached values
	are reset to their unset values.

	<b>N.B.</b>: The {@link source_name() source name} is not cleared.

	@return	This JP2_Metadata.
*/
JP2_Metadata& reset ();

/*==============================================================================
	Parameters
*/
/**	Get the Aggregate of all metadata parameters.

	@return	A pointer to the Aggregate of all current metadata parameters.
*/
idaeim::PVL::Aggregate* metadata_parameters () const
	{return Parameters;}


/**	Test for complete metadata.

	Complete metadata has all {@link boxes_are_complete() required JP2
	boxes} and {@link segments_are_complete() required codestream
	segments}.

	@return	true if the metadata is complete; false otherwise.
*/
inline bool is_complete ()
	{return boxes_are_complete () && segments_are_complete ();}

/**	Get a validity report for all metadata.

	A combined {@link JP2_validity_report(int) JP2 boxes validity report}
	and {@link codestream_validity_report(int) codestream segments validity
	report} is produced using the current metadata validity state.

	@return	A validity report string.
*/
inline std::string validity_report ()
	{return
		JP2_validity_report (JP2_Validity) +
		(Codestream_Validity ?
			codestream_validity_report (Codestream_Validity) : "");}

/**	Get the the current data content address.

	Whenever {@link add_JP2_box(Type_Code, int, const unsigned char*,
	long long, long long) JP2 box data is added} or {@link
	add_codestream_segment(Marker_Code, const unsigned char*, int,
	long long) codestream segment data is added} the data content address
	is re-initialized and then the address is advanced as parameter
	values are obtained from the data.

	@return The current data content address. This will be NULL if
		no data has been added.
*/
inline const unsigned char* box_data () const
	{return Data_Buffer;}

/**	Get the the current {@link add_JP2_box(Type_Code, int,
	const unsigned char*, long long, long long) JP2 box} data content amount.

	Whenever {@link add_JP2_box(Type_Code, int, const unsigned char*,
	long long, long long) JP2 box data is added} or {@link
	add_codestream_segment(Marker_Code, const unsigned char*, int,
	long long) codestream segment data is added} the data content amount
	is re-initialized and then the amount is decremented as parameter
	values are obtained from the data.

	@return The current box data content amount. This will be -1 if
		no data has been added.
*/
inline long data_amount () const
	{return Data_Amount;}

/*------------------------------------------------------------------------------
	JP2 boxes
*/
/**	Add a JP2 data box to the metadata parameters.

	A JP2 data box is composed of a header sequence followed by data
	content. The box is represented in the JP2_Metadata as an Aggregate
	of Parameters:
<dl>
<dt>{@link NAME_PARAMETER, Name}
<dd>A {@link box_name(Type_Code) descriptive name for the box} based on
	the box type code.

<dt>{@link TYPE_PARAMETER, Type}
<dd>The box type code in hexadecimal value representation.

<dt>{@link POSITION_PARAMETER, ^Position}
<dd>The position of the box in its source file as a byte offset. This
	parameter will not be present if the specified box position is negative.

<dt>{@link LENGTH_PARAMETER, Length}
<dd>The total length of the box including the header sequence and the
	data content (which may include sub-boxes). This will be -1 for an
	indefinite box length that extends to the end of the source file and
	the size of the source file is not known.
</dl>
	Additional, box-specific parameters will follow. This may include
	sub-boxes that are added as Aggregate parameters to the current
	box.

	Additional parameter values are obtained from the box data content.
	As parameter values are obtained the data content is consumed by
	advancing the current address of the {@link box_data() box data}
	content and decrementing the current {@link data_amount() amount}
	of available box data. A JP2_Logic_Error will be thrown if required
	parameter values for a JP2 box can not be obtained because the
	amount of available data has been exhausted.

	</b>N.B.</b>: If the box type is known to contain sub-boxes the data
	content will be used to {@link add_JP2_boxes(const unsigned char* data,
	long long, long long) add the sub-boxes} to the current parent box.

	@param	type_code	A Type_Code value specifying the type of JP2 box.
		The box type bytes are expected to be the ASCII values of a four
		character string, which is used as the {@link
		type_name(Type_Code, bool) name} of the resulting box Aggregate.
	@param	header_length	The length, in bytes, of the box header
		section. The box header must have at least 8 bytes: the 32-bit
		box length value followed by a 32-bit Type_Code value. However,
		if the initial box length value is 1 this indicates that an
		additional 64-bit extended box length value was provided that is
		the actual box length.
	@param	content	The address of the data content buffer. This will
		initialize the {@link box_data() current data address}.
		<b>N.B.</b>: The first byte of the content data immediately
		follows the box header sequence.
	@param	amount	The amount, in bytes, of box data content.
		<b>N.B.</b>: The content amount does not include the header
		length. This will initialize the {@link data_amount() current
		data amount}. However, if the amount is negative the amount of
		data content is unknown (indefinite length extending to the end
		of the data source), in which case the current data amount is
		initialized to zero.
	@param	data_position	The position of the box in it's source data
		stream as a byte offset from the beginning of the stream to the
		first byte of the box header sequence. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
	@throws	invalid_argument	If the header length is below the 8 byte
		minimum.
	@throws	 JP2_Logic_Error If the content amount is insufficient for the
		specific box type being added.
*/
void add_JP2_box (Type_Code type_code, int header_length,
	const unsigned char* content,
	long long amount, long long data_position = -1);

/**	Add a JP2 data box to the metadata parameters.

	The box header values are obtained from the box data:
<dl>
<dt>Length
<dd>The first four bytes of the box must contain an integer value in MSB
	data order. This value is the length of the box, in bytes, including
	the header sequence and the entire box content (which many include
	sub-boxes). <b.N.B.</b>: If this value is 1 then an additional
	extended box length value must follow the box type code value.
<dt>Type
<dd>The next four bytes of the box must contain a box type code integer
	value in MSB data order. This value uniquely identifies the type of
	JP2 box.
<dt>Extended Length
<dd>If, and only if, the initial length value is 1 an eight byte, long
	integer value in MSB data order must follow the type code value. This
	value is the length of the box, in bytes, including the header
	sequence and the entire box content (which many include sub-boxes).
</dl>
	As values are obtained the data content is consumed by advancing the
	current address of the {@link box_data() box data} content and
	decrementing the current {@link data_amount() amount} of available
	box data.

	These values are then used as if they had been provided to the {@link
	add_JP2_box(Type_Code, int, const unsigned char*, long long, long long)
	alternative add_JP2_box method}.

	@param	data	The address of the box data. The first byte of the
		data must be the first byte of the box header sequence. This will
		initialize the {@link box_data() current data address}. However,
		if NULL nothing is done.
	@param	amount	The amount of data, in bytes in the box including the
		header sequence and the content amount. This will initialize the
		{@link data_amount() current data amount}. However, if not
		positive nothing is done.
	@param	data_position	The position of the box in it's source data
		stream as a byte offset from the beginning of the stream to the
		first byte of the box header sequence. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
	@return	true	If the box was added; false if the data amount is
		less than the amount determined to be required for the type of
		box identified in the data. <b>N.B.</b>: A box may be found to
		have an indefinite length in which case the comparison with the
		data amount is not done.
	@throws	 JP2_Logic_Error If the content amount is insufficient for the
		specific box type being added.
	@see	add_JP2_box(Type_Code, int, const unsigned char*, long long,
				long long)
*/
void add_JP2_box (const unsigned char* data, long long amount,
	long long data_position = -1);

/**	Add JP2 data boxes to the metadata parameters.

	The data buffer is used to add a {@link add_JP2_box(const unsigned
	char*, long long, long long) JP2 data boxes} to the metadata. This is
	repeated until the data buffer content has been exhausted.

	</b>N.B.</b>: The data buffer content is consumed adding boxes until
	insufficient data remains for another box. The sufficiency of
	remaining data is determined by comparing the amount of remaining
	data with the {@link box_length(const unsigned char*, long) box
	length} value obtained from the leading portion of remaining data.
	While unused data is allowed at the end of the data buffer, care must
	be taken to prevent invalid box data from being present lest a bogus
	box length value be found. Thus if the data buffer amount does not
	end at the end of a box any excess data should be cleared with zero
	values.

	@param	data	The address of the box data. The first byte of the
		data must be the first byte of the box header sequence. This will
		initialize the {@link box_data() current data address}. However,
		if NULL nothing is done.
	@param	amount	The amount of data content, in bytes, in the data
		buffer. This will initialize the {@link data_amount() current
		data amount}. However, if not positive, or less than the
		box length determined from the data content, nothing is done.
	@param	data_position	The position of the box in it's source data
		stream as a byte offset from the beginning of the stream to the
		first byte of the box header sequence. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
	@throws	 JP2_Logic_Error If the content amount is insufficient for a
		specific box parameter value being added.
*/
void add_JP2_boxes (const unsigned char* data, long long amount,
	long long data_position = -1);

/**	Get the box type from a data buffer.

	The four bytes starting at offset four of the data buffer contain the
	box type code value. <b>N.B.</b>: The data buffer address must be
	positioned at the beginning of the box data.

	@param	data	The box data buffer address.
	@param	amount	The amount of data, in bytes, in the buffer.
	@return	The box Type_Code obtained from the data. This will be the
		{@link #INVALID_CODE} if the data amount is less than 8.
		<b>N.B.</b>: It is, of course, possible that the data contains
		the INVALID_CODE value; but this would never occur for valid box
		data.
*/
static Type_Code box_type (const unsigned char* data, long amount);

/**	Get the box length from a data buffer.

	The first four bytes of the data buffer contain the box length value.
	However, if the value of this field is 1, then the eight bytes
	starting at offset eight of the data buffer contain the extended box
	length value. <b>N.B.</b>: The data buffer address must be positioned
	at the beginning of the box data.

	@param	data	The box data buffer address.
	@param	amount	The amount of data, in bytes, in the buffer.
	@return	The box length as a long long integer value obtained from the
		data. This will zero if the data amount is less than 4, or less
		than 16 if the first 4 bytes of the data contain the value 1.
		<b>N.B.</b>: It is, of course, possible that the data contains a
		zero length value; but this would never occur for valid box data.
*/
static long long box_length (const unsigned char* data, long amount);

/**	Convert a JP2 box type code to a string.

	Each byte, starting with the most significant byte, of the type code
	value is appended as a character to a string.

	@param	type_code	The box type code value to be converted.
	@param	throw_exception	If true a JP2_Invalid_Argument exception will
		be thrown if the box type code is invalid. If false an {@link
		#UNKNOWN_NAME} will be returned.
	@return	The string repesentation of the type code.
	@throws	JP2_Invalid_Argument	If a type code byte value does not
		correspond to a printable character (<32 or >126), and
		throw_exception is true.
	@see	type_code(const std::string, bool)
*/
static std::string type_name (Type_Code type_code,
	bool throw_exception = false);

/**	Convert a JP2 box type string to an integer code.

	Each character of the string is placed as a byte value in the
	corresponding location of the type code, starting with the most
	significant byte.

	@param	name	The type name string to be converted.
	@param	throw_exception	If true a JP2_Invalid_Argument exception will
		be thrown if the box type code is invalid. If false an {@link
		#INVALID_CODE} will be returned.
	@return	The integer type code value.
	@throws	invalid_argument	If the string does not contain exactly
		four characters or any character is not printable (<32 or >126),
		and throw_exception is true.
	@see	type_name(Type_Code, bool)
*/
static Type_Code type_code (const std::string& name,
	bool throw_exception = false);

/**	Get the descriptive box name for a box type.

	@param	type_code	A JP2 box type code value.
	@return	The box name as used in the {@link #NAME_PARAMETER}
		of each box Parameter Group. This will be {@link #UNKNOWN_NAME}
		if the type code is not known.
*/
static std::string box_name (Type_Code type_code);

/**	Get a JP2 source {@link validity() validity} flag corresponding
	to a JP2 box type code.

	@param	type_code	A JP2 box Type_Code.
	@return	A bit mask with the bit set corresponding to the validity
		value flag for the type code.
*/
static int type_flag_from_code (Type_Code type_code);

/**	Get the JP2 box type code corresponding to a {@link JP2_validity() JP2
	boxes validity} flag.

	@param	type_flag	A JP2 box validity flag.
	@return	The Type_Code corresponding to the validity value flag.
*/
static Type_Code type_code_from_flag (int type_flag);

/**	Get the current JP2 boxes validity flags.

	The bit flags in this value are updated as {@link
	add_JP2_box(Type_Code, int, const void*, long long, long long) boxes
	are added}. There are three groups of validity flags.

	- Required boxes, without which the JP2 source is not valid, that are
	expected to only occur once (exceptions noted) in a source:

<dl>
<dt>{@link #SIGNATURE_FLAG}
<dd>Set if the beginning of the file contains a Signature box. A
	JP2_Logic_Error will be thrown by the add box method if this is not
	the first box added.

<dt>{@link #FILE_TYPE_FLAG}
<dd>Set if the second box of the file is a File Type box. A
	JP2_Logic_Error will be thrown by the add box method if this is not
	the second box added.

<dt>{@link #JP2_HEADER_FLAG}
<dd>Set if the JP2 Header box has been added.

<dt>{@link #IMAGE_HEADER_FLAG}
<dd>Set if the Image Header box has been added. If more than one Image Header
	box is added only the first is used to set image characterization values.

<dt>{@link #COLOUR_SPECIFICATION_FLAG}
<dd>Set if a Colour Specification box has been added. More than one
	Colour Specification box is allowed.

<dt>{@link #CONTIGUOUS_CODESTREAM_FLAG}
<dd>Set if a JPEG2000 Contiguous Codestream box has been added.
</dl>

	- Optional boxes that are expected to only occur once (exceptions
	noted) in a source:

<dl>
<dt>{@link #BITS_PER_COMPONENT_FLAG}
<dd>Set if the Bits Per Component box has been added. This box should only
	be present if the {@link pixel_precision(int) pixel precision} Value_Bits
	from the Image Header box is 255, which indicates that pixel precision
	and/or {@link pixel_signed(int) sign} is not the same for all image
	bands. This box must occur after the the Image Header box.

<dt>{@COMPONENT_MAPPING_FLAG}
	CHANNEL_DEFINITION_FLAG				= 1 << 8,
	RESOLUTION_FLAG						= 1 << 9

	@return	The JP2 validity flags value.
*/
inline unsigned int JP2_validity () const
	{return JP2_Validity;}

inline bool boxes_are_complete () const
	{return boxes_are_complete (JP2_Validity);}

inline static bool boxes_are_complete (unsigned int validity_flags)
	{return (validity_flags & REQUIRED_BOXES) == REQUIRED_BOXES;}

/**	Generate a JP2 boxes validity flags report.

	A report of the {@link JP2_validity() JP2 validity} flags is produced
	as a multi-line string. The presence or absence of each required box
	is reported; if they are all present this is noted. Optional boxes
	are reported if they are present.

	@param	validity_flags	A JP2 validity flags bit map.
	@return	A report string.
*/
static std::string JP2_validity_report (unsigned int validity_flags);

/*..............................................................................
	Data consumers:
*/
private:

//	Obtain one or more boxes:

/**	Get the parameters for a single JP2 box.

	@param	type_code	The Type_Code for the box.
	@param	header_length	The length of the box header sequence.
	@param	box_length	The length, in bytes, of the entire box content
		including the header sequence. <b>N.B.</b>: A super-box includes
		all of its sub-boxes in its content.
	@param	data_position	The position of the box in it's source data
		stream as a byte offset from the beginning of the stream to the
		first byte of the box header sequence. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
	@return	A pointer to a Parameter Aggregate named with the box {@link
		type_name(Type_Code) type name} and containing parameters for the
		box header values followed by content-specific parameters including
		any sub-box parameter groups.
	@throws	JP2_Logic_Error	If the box is not {@link JP2_validity()
		valid} to add because it is the first box being added but is not
		a SIGNATURE_TYPE box, is a SIGNATURE_TYPE box of the wrong
		length, is the second box to be added but is not a
		FILE_TYPE_TYPE box, or is the second addition of a box that can
		only be added once.
	@throws invalid_argument	If the header_size is less than the 8
		byte minimum, greater than a non-zero box length, or there is
		insufficient {@link data_amount() data amount} available for the
		specified box length.
*/
idaeim::PVL::Aggregate* JP2_box (Type_Code type_code,
	int header_length, long long box_length, long long data_position = -1);

idaeim::PVL::Aggregate* JP2_box (long long data_position = -1);

void add_JP2_boxes (idaeim::PVL::Aggregate* container,
	long long data_position = -1);

//	Add the box header parameters to a box.
void box_header_parameters (idaeim::PVL::Aggregate* box,
	Type_Code type_code = 0,
	long long box_length = 0, long long data_position = -1);

/*	Add box-specific parameters to a box:

	Boxes that are a variable length and do not contain information
	determining their content size require that the size be provided.

	Boxes that include, or may include, data position parameters require
	the box position to be passed through.
*/

void signature_parameters (idaeim::PVL::Aggregate* box);

void file_type_parameters (idaeim::PVL::Aggregate* box,
	long long content_size);

void image_header_parameters (idaeim::PVL::Aggregate* box);

void bits_per_component_parameters (idaeim::PVL::Aggregate* box,
	long long content_size);

/**	Set the Pixel_Precision array.

	If there is no Pixel_Precision array it is allocated based on the
	Image_Bands size and filled from the specified Array values. An
	existing array containing 255 values is also reset from the Array.
	Otherwise the existing array is checked for consistency
	with the specified Array values: An existing value of zero is
	reset; any other value must be the same as the corresponding
	Array value.

	@param	values	A pointer to an Array of pixel precision values.
		<b>N.B.</b>: The values may be negative to indicated signed
		pixel values with the effective pixel precision being the
		absolute value. If NULL nothing is done.
	@param	description	A very brief description of the values source
		(a JP2 box or codesegment name).
	@throws	JP2_Logic_Error	If the Image_Bands value is not set or
		does not match the number ov Array values, or an existing
		Pixel_Precision array contains a value that conflicts with
		the corresponding Array value.
*/
void pixel_precision (idaeim::PVL::Array* values,
	const std::string& description);

void colour_specification_parameters (idaeim::PVL::Aggregate* box,
	long long content_size, long long data_position);

void palette_parameters (idaeim::PVL::Aggregate* box);

void component_mapping_parameters (idaeim::PVL::Aggregate* box,
	long long content_size);

void channel_definition_parameters (idaeim::PVL::Aggregate* box);

void resolution_parameters (idaeim::PVL::Aggregate* box);

void UUID_parameters (idaeim::PVL::Aggregate* box,
	long long content_size, long long data_position);

void check_UUID_info_parameters (idaeim::PVL::Aggregate* box);

void UUID_list_parameters (idaeim::PVL::Aggregate* box);

void URL_parameters (idaeim::PVL::Aggregate* box,
	long long content_size);

void text_parameter (idaeim::PVL::Aggregate* box,
	long long content_size);

void placeholder_parameters (idaeim::PVL::Aggregate* box,
	long long content_size, long long data_position);

void contiguous_codestream_parameters (idaeim::PVL::Aggregate* box,
	long long content_size, long long data_position);

//	Parameter definitions that occur in more than one type of box:

void data_position_parameters (idaeim::PVL::Aggregate* group,
	long long content_size, long long data_position,
	bool consume_data = true);

idaeim::PVL::Parameter* value_bits_parameter
	(int count, const std::string& description);
idaeim::PVL::Parameter* value_bits_parameter
	(const idaeim::PVL::Array& value_bits);

idaeim::PVL::Parameter* UUID_parameter
	(const std::string& description);

/*------------------------------------------------------------------------------
	Codestream segments
*/
public:

/**	Add a codestream segment to the metadata parameters.

	@param	marker	A Marker_Code identifying the segment.
	@param	content	The address of the data content buffer. This will
		initialize the {@link box_data() current data address}. <b>N.B.</b>:
		the content does not include the segment marker or length fields.
	@param	segment_length	The total length, in bytes, of the segment.
		The length includes the marker field and, if present, the segment
		length field along with the remaining data content. The {@link
		data_amount() current data amount} is initialized to the segment
		length minus 2 (the length of the marker field). If this is
		positive the amount is reduced by 2 more (the length of the
		length field).  However, if the amount is negative the amount of
		data content is unknown, in which case the current data amount is
		set to zero.
	@param	data_position	The position of the segment in it's source
		data stream as a byte offset from the beginning of the stream to
		the first byte of the segment. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
*/
void add_codestream_segment (Marker_Code marker,
	const unsigned char* content, int segment_length,
	long long data_position = -1);

/**	Add codestream data segments to the metadata parameters.

	The data buffer is used to add a {@link
	add_codestream_segment(Marker_Code, const unsigned char*, int,
	long long) codestream data segment} to the metadata. This is repeated
	until the data buffer content has been exhausted, or an EOC_MARKER
	(end-of-codestream delimiter) has found.

	</b>N.B.</b>: The data buffer content is consumed by adding segments
	until insufficient data remains for another segment, or the EOC
	marker is found. The sufficiency of remaining data is determined by
	comparing the amount of remaining data with the {@link
	segment_length(const unsigned char*, long) box length} value obtained
	from the leading portion of remaining data. While unused data is
	allowed at the end of the data buffer, care must be taken to prevent
	invalid segment data from being present lest a bogus segment length
	value be found. Thus if the data buffer amount does not end at the
	end of a segment any excess data should be cleared with zero values.

	@param	data	The address of the segment data. The first byte of the
		data must be the first byte of the first segment. This will
		initialize the {@link box_data() current data address}. However,
		if NULL nothing is done.
	@param	amount	The amount of data content, in bytes, in the data
		buffer. This will initialize the {@link data_amount() current
		data amount}. However, if not positive, or less than the segment
		length determined from the data content, nothing is done.
	@param	data_position	The position of the box in it's source data
		stream as a byte offset from the beginning of the stream to the
		first byte of the first segment. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
	@throws	 JP2_Logic_Error If the content amount is insufficient for the
		specific segment parameter value being added.
*/
void add_codestream_segments (const unsigned char* data, long amount,
	long long data_position = -1);

/**	Get the segment marker from a data buffer.

	The first two bytes of the data buffer contain the segment marker
	code value.

	@param	data	The box data buffer address.
	@param	amount	The amount of data, in bytes, in the buffer.
	@return	The segment Marker_Code obtained from the data. This will be
		the {@link #INVALID_CODE} if the data amount is less than 2.
		<b>N.B.</b>: It is, of course, possible that the data contains
		the INVALID_CODE value; but this would never occur for valid
		segment data.
*/
static Marker_Code segment_marker (const unsigned char* data, long amount);

/**	Get the segment length from a data buffer.

	The two bytes starting at offset two of the data buffer contain the
	box length value. This value includes the two bytes of the length
	value, but not the two bytes of the preceeding {@link
	segment_marker{const unsigned char*, long) segment marker} value.
	<b>N.B.</b> the returned value will be the total length of the
	segment including the two byte marker.

	Delimiter segments only have a marker with no content. These segments
	do not have a length field. For these segments, and only these
	segments, the returned segment length will be 2.

	@param	data	The box data buffer address.
	@param	amount	The amount of data, in bytes, in the buffer.
	@return	The total segment length as an integer value, including the
		leading segment marker and the length field (if present). This
		will zero if the data amount is less than 4, or less than 2 for a
		delimiter segment with no length field. <b>N.B.</b>: It is, of
		course, possible that the data contains a zero length value; but
		this would never occur for valid segment data.
*/
static int segment_length (const unsigned char* data, long amount);

/**	Get the segment name for a codestream marker.

	@param	marker_code	A codestream segment marker code value.
	@return	The codestream segment name as used in the {@link
		#MARKER_PARAMETER} of each codestream segment Parameter Group.
		This will be {@link #UNKNOWN_NAME} if the marker code is not
		known.
*/
static std::string segment_name (Marker_Code marker_code);

/*	Convert a segment marker code to a string representation.

	@param	marker_code	A Marker_Code value.
	@return	A string representation of the marker code as a hex encoding
		of the value.
*/
static std::string marker_number (Marker_Code marker_code);

/**	Get a codestream segment marker {@link validity() validity} flag
	corresponding to a segment marker code.

	@param	marker_code	A codestream segment Marker_Code.
	@return	A bit mask with the bit set corresponding to the validity
		value flag for the marker code.
*/
static int marker_flag_from_code (Marker_Code markder_code);

/**	Get the codestream segment marker code corresponding to a
	{@link codestream_validity() codestream segments validity} flag .

	@param	marker_flag	A Codestream segment marker validity flag.
	@return	The Marker_Code corresponding to the validity value flag.
*/
static Marker_Code marker_code_from_flag (int marker_flag);

inline unsigned int codestream_validity () const
	{return Codestream_Validity;}

inline bool segments_are_complete () const
	{return segments_are_complete (Codestream_Validity);}

/**	Test for a complete codestream main header segments.

	The {@link codestream_validity() codestream segments validity flags}
	are tested that the MAIN_REQUIRED_SEGMENTS flags are set, which
	indicates that all of the codestream segments required for a complete
	main header have been (@link add_codestream_segment(Marker_Code,
	const unsigned char*, int, long long) added}.

	<b>N.B.</b>: The presence of {@link #TILE_ONLY_SEGMENTS tile-part
	segments} required for a complete codestream are not tested. Reading
	these segments is usually only done as needed, and they are not
	required to be added to the metadata parameters.

	@param	validity_flags	A codestream segments bit flags value.
	@return	true if all codestream main header segements have been
		added to the metadata parameters; false otherwise.
*/
inline static bool segments_are_complete (unsigned int validity_flags)
	{return (validity_flags & MAIN_REQUIRED_SEGMENTS) == MAIN_REQUIRED_SEGMENTS;}

/**	Generate a codestream segments validity flags report.

	A report of the {@link codestream_validity() codestream validity}
	flags is produced as a multi-line string. The presence or absence of
	each required segment is reported; if they are all present this is
	noted. Optional segments are reported if they are present. The
	presence of tile-part segments is reported separately. If the EOC
	(end-of-codestream) delimiter is found it is reported (it is not
	reported if not found).

	@param	validity_flags	A JP2 validity flags bit map.
	@return	A report string.
*/
static std::string codestream_validity_report (unsigned int validity_flags);

/*..............................................................................
	Data consumers:
*/
private:

/**	Get a the parameters for a single codestream segment.

	@param	marker_code	The Marker_Code for the segment.
	@param	segment_length	The total length of the segment, in bytes,
		including the marker field, length field (if present), and the
		entire segment content.
	@param	data_position	The position of the segment in it's source
		data stream as a byte offset from the beginning of the stream to
		the first byte of the segment marker. If negative the {@link
		#POSITION_PARAMETER} will not be included in the box parameters.
	@return	A pointer to a Parameter Aggregate named with the box {@link
		segment_name(Marker_Code) segment name} and containing
		parameters for the segment marker and length followed by
		content-specific parameters.
	@throws	invalid_argument	If the segment length is less than the
		2 byte minimum.
	@throws	JP2_Logic_Error	If the segment is not {@link
		codestream_validity() valid} to add because it is the first
		segment being added but is not an SOC delimiter marker, or is
		the second segment to be added but is not an SIZ segment.
*/
idaeim::PVL::Aggregate* codestream_segment (Marker_Code marker_code,
	int segment_length, long long data_position);

void SOT_parameters (idaeim::PVL::Aggregate* segment);

void SIZ_parameters (idaeim::PVL::Aggregate* segment);

void COD_parameters (idaeim::PVL::Aggregate* segment);

void COC_parameters (idaeim::PVL::Aggregate* segment);

void RGN_parameters (idaeim::PVL::Aggregate* segment);

void QCD_parameters (idaeim::PVL::Aggregate* segment);

void QCC_parameters (idaeim::PVL::Aggregate* segment);

void POC_parameters (idaeim::PVL::Aggregate* segment);

void TLM_parameters (idaeim::PVL::Aggregate* segment);

void PLM_parameters (idaeim::PVL::Aggregate* segment);

void PLT_parameters (idaeim::PVL::Aggregate* segment);

void PPM_parameters (idaeim::PVL::Aggregate* segment,
	long long data_position);

void PPT_parameters (idaeim::PVL::Aggregate* segment,
	long long data_position);

void SOP_parameters (idaeim::PVL::Aggregate* segment);

void CRG_parameters (idaeim::PVL::Aggregate* segment);

void COM_parameters (idaeim::PVL::Aggregate* segment);

void coding_style_parameters (idaeim::PVL::Aggregate* segment,
	bool has_precinct_sizes);

void quantization_parameters (idaeim::PVL::Aggregate* segment);

/*==============================================================================
	Data converters
*/
private:

/**	Convert an integer value to a String.

	Each byte, starting with the most significant byte, of the value
	is appended as a character to a string. Non-printable characters
	are expanded to escape sequences.

	@param	value	The integer value to be converted.
	@return	The string repesentation of the value.
*/
static std::string to_string (unsigned int value);

/**	Get a string from the data buffer.

	The next size bytes, up to but not including the first zero valued
	byte, are copied from the box data into a character string. Any
	unprintable characters in the string are expanded into printable
	escape sequences.

	The {@link box_data() data buffer pointer} is then incremented past
	the size (regardless of how many data bytes were used to form the
	string) - or the currently available data amount - and the data
	buffer {@link data_amount() amount} is decremented accordingly.
	<b>N.B.</b>: If an exception is thrown the data buffer pointer and
	amount values remain unchanged.

	@param	size	The maximum size of the string. This is also the
		amount of data to consume from the buffer regardless of the
		resulting string size. If negative or zero an empty string will
		be returned.
	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	A string representing the bytes obtained from the data.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
*/
std::string get_string (long size,
	const std::string& description = std::string ());

/**	Get a long long integer value from the data buffer.

	The next eight bytes of the box data are assumed to contain the bytes
	of a long long integer value in MSB order. These bytes are copied
	into a long long integer variable, If the host system uses LSB
	ordered values the data bytes are reordered in the variable to the
	native data order.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	A long long integer value obtained from the data buffer content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_integer(const std::string&)
*/
long long get_long_integer (const std::string& description = std::string ());

/**	Get an integer value from the data buffer.

	The next four bytes of the box data are assumed to contain the bytes
	of an integer value in MSB order. These bytes are copied into an
	integer variable, If the host system uses LSB ordered values the data
	bytes are reordered in the variable to the native data order.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	An integer value obtained from the data buffer content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_unsigned_integer(const std::string&)
*/
int get_integer (const std::string& description = std::string ());

/**	Get an unsigned integer value from the data buffer.

	The next four bytes of the box data are assumed to contain the bytes
	of an unsigned integer value in MSB order. These bytes are copied
	into an unsigned integer variable, If the host system uses LSB
	ordered values the data bytes are reordered in the variable to the
	native data order.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	An unsigned integer value obtained from the data buffer content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_long_integer(const std::string&)
*/
unsigned int get_unsigned_integer
	(const std::string& description = std::string ());

/**	Get a short integer value from the data buffer.

	The next two bytes of the box data are assumed to contain the bytes
	of a short integer value in MSB order. These bytes are copied into a
	short integer variable, If the host system uses LSB ordered values
	the data bytes are reordered in the variable to the native data
	order.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	A short integer value obtained from the data buffer content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_unsigned_integer(const std::string&)
*/
short get_short_integer (const std::string& description = std::string ());

/**	Get an unisgned short integer value from the data buffer.

	The next two bytes of the box data are assumed to contain the bytes
	of an unsigned short integer value in MSB order. These bytes are
	copied into an unsigned integer variable, If the host system uses LSB
	ordered values the data bytes are reordered in the variable to the
	native data order.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	An unsigned short integer value obtained from the data buffer
		content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_long_integer(const std::string&)
*/
unsigned short get_unsigned_short_integer
	(const std::string& description = std::string ());

/**	Get a byte value from the data buffer.

	The next byte of the box data is obtained as a signed integer value.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	An integer value obtained from the data buffer content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_unsigned_integer(const std::string&)
*/
int get_byte (const std::string& description = std::string ());

/**	Get byte value from the data buffer.

	The next byte of the box data is obtained as an unsigned signed
	integer value.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	An unsigned integer value obtained from the data buffer content.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
	@see get_long_integer(const std::string&)
*/
unsigned int get_unsigned_byte (const std::string& description = std::string ());

/**	Get a value of some bits precision from the data buffer.

	If the number of bits is negative the most significant bit of the
	value is treated as a sign bit which, if set, is extended in the
	returned value. The actual precision of the data is the absolute
	value of bits.

	The data bits are contained in a whole number of data bytes; e.g. a
	10-bit value will be contained in two data bytes. Any additional bits
	must be padded with zeros in the most significant bits of the bytes.
	The data bytes of the value are expected to occur in MSB order.

	The {@link box_data() data buffer pointer} is then incremented past
	the data content used to make the value and the data buffer {@link
	data_amount() amount} is decremented accordingly. <b>N.B.</b>: If
	an exception is thrown the data buffer pointer and amount values
	remain unchanged.

	<b>N.B.</b>: This method "consumes" the box data content by advancing
	the data buffer pointer and decrementing the data amount.

	@param	bits	The bit precision of the value. Negative bits means
		the value is signed; i.e. the most significant bit of the data value
		is the sign bit.
	@param	description	If a description is provided, and an exception is
		thrown, the description will be included in the exception message.
	@return	The value obtained as a long long. Thus a maximum of 64 bits
		may be obtained. As a special case, if bits is 0, 0 is returned.
	@throws	out_of_range	If the absolute value of bits > 64.
	@throws invalid_argument	If no data buffer is provided.
	@throws	JP2_Logic_Error	If the data buffer amount is less than the
		required number of data bytes.
*/
long long get_value (int bits, const std::string& description = std::string ());


void throw_data_exception (const std::string& description,
	const std::string& data_type, int amount_required);

/*==============================================================================
	Data members:
*/
private:

//!	The source of the metdata.
std::string
	Source_Name;

//	From JP2 boxes:

//!	Dimensions of the source image.
Size_2D
	Image_Size;

//!	Total image bands (components).
unsigned int
	Image_Bands;

//!	Pixel precision bits and signedness.
int*
	Pixel_Precision;

/**	The UUID value found in a UUID List box of a UUID Info super box.

	<b>N.B.</b>: The UUID will be from a UUID List box with a single
	entry paired with a single URL box in the UUID Info super box. Any
	other UUID values will be ignored. This rule conforms to the PDS/JP2
	specification for providing the data producer UUID and the URL for
	the associated PDS label file.

	This will be NULL if no UUID has been found. It will be a 16 byte
	(UUID_SIZE) array otherwise.
*/
unsigned char
	*Producer_UUID;

/**	The URL value found in a URL box of a UUID Info super box.

	<b>N.B.</b>: The URL will be from a UUID Info super box with a single
	URL box pair with a  UUID List box with a single UUID entry. Any
	other URL values will be ignored. This rule conforms to the PDS/JP2
	specification for providing the data producer UUID and the URL for
	the associated PDS label file.

	This will be empty if no URL has been found.
*/
std::string
	Label_URL;

//	From Codestream segments:

//	Image geometry information.
Size_2D
	Reference_Grid_Size,
	Tile_Size;
Point_2D
	Image_Offsets,
	Tile_Offsets;
unsigned int
	*Pixel_Width,
	*Pixel_Height,
	Total_Tiles,
	Quality_Layers,
	Resolution_Levels;
int
	Progression_Order,
	Transform;


//!	Metadata PVL Parameters.
idaeim::PVL::Aggregate
	*Parameters,
	*Codestream_Parameters;

//	PLM/PLT segments continuation accumulator values.
idaeim::PVL::Array
	*PLM_Packet_Length_Array,
	*PLT_Packet_Length_Array;
long long
	PLM_Packet_Length,
	PLT_Packet_Length,
	PLM_Packet_Length_Bytes_Remaining,
	PLT_Packet_Length_Bytes_Remaining;


//!	Current data buffer address, maintained by data converter methods.
const unsigned char*
	Data_Buffer;
//!	Current data buffer content amount, maintained by data converter methods.
long
	Data_Amount;

//!	Bit field of boxes and codestream segments that have been added.
unsigned int
	JP2_Validity,
	Codestream_Validity;

};	//	class JP2_Metadata

/*=*****************************************************************************
	Utility:
*/
/**	Get the number of bytes required to hold a specified number of bits.

	@param	bits	Number of bits.
	@return	The number of bytes required to hold the bits.
*/
unsigned int bytes_of_bits (unsigned int bits);

/**	Get a pixel value bit mask for the specified number of bits.

	@param	bits	The number of bits per pixel.
	@return	A bit mask in which a bit is set only where a valid pixel bit
		may occur.
*/
unsigned long long pixel_bits_mask (unsigned int bits);

/**	Get a long long integer value from a data buffer.

	The first eight bytes of the data buffer are assumed to contain the
	bytes of a long long integer value in MSB order. These bytes are
	copied into a long long integer variable, If the host system uses LSB
	ordered values the data bytes are reordered in the variable to the
	native data order.

	@param	data	A pointer to a data buffer that must contain at least
		eight bytes.
	@return	A long long integer value.
*/
long long get_long (const void* data);
void put_long (const long long& value, void* data);

/**	Get an unsigned integer value from a data buffer.

	The first four bytes of the data buffer are assumed to contain the
	bytes of an unsigned integer value in MSB order. These bytes are
	copied into an unsigned integer variable, If the host system uses LSB
	ordered values the data bytes are reordered in the variable to the
	native data order.

	@param	data	A pointer to a data buffer that must contain at least
		four bytes.
	@return	An unsigned integer value.
*/
int get_int (const void* data);
void put_int (const int& value, void* data);

/**	Get a short integer value from a data buffer.

	The first two bytes of the data buffer are assumed to contain the bytes
	of an unsigned short integer value in MSB order. These bytes are copied
	into an unsigned integer variable, If the host system uses LSB
	ordered values the data bytes are reordered in the variable to the
	native data order.

	@param	data	A pointer to a data buffer that must contain at least
		two bytes.
	@return	An unsigned short integer value.
*/
unsigned short get_short (const void* data);
void put_short (const short& value, void* data);


}	//	namespace UA::HiRISE
#endif
