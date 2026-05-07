// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
// 
// GenDC.h
// GenDC structures and defines header.
// Version 1.1.1.1
// 2022/1/6
//
//-----------------------------------------------------------------------------
//  Copyright (c) 2018 by Zebra Electronic Systems Ltd.
//  Section: GenDC
//  Project: GenICam
//  Author:  Stephane Maurice
//  $Header$ GenDC.h
// 
// Note: Starting with version 1.1.1.0, the GenDC.H header uses a new "GenDC" namespace.
//       It also uses "enum class" instead of "enum" and const variables insted fo defines 
//       to limit the scope of the GenDC enumerations and predefined value. 
//
//  License: This file is published under the license of the EMVA GenICam  Standard Group.
//  A text file describing the legal terms is included in  your installation as 'GenICam_license.pdf'.
//  If for some reason you are missing  this file please contact the EMVA or visit the website
//  (http://www.genicam.org) for a full copy.
//
//  THIS SOFTWARE IS PROVIDED BY THE EMVA GENICAM STANDARD GROUP "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE EMVA GENICAM STANDARD  GROUP
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT  LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  DATA, OR PROFITS;
//  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  THEORY OF LIABILITY,
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Signal GenDC.H inclusion.
#ifndef _GENDC_H_
#define _GENDC_H_

// Headers:
#ifndef _STDINT
#include <stdint.h>
#endif
#ifndef PFNC_H
#include "PFNC.h"
#endif

// Include all GenDC related definitions in the GenDC namespace
// -------------------------------------------------------------
namespace GenDC
{
// GenDC standard values:
// -----------------------------------
// -----------------------------------
constexpr  uint32_t GDC_SIGNATURE                = 0x43444E47; // "GNDC" Signature (not null terminated)
constexpr  uint8_t  GDC_VERSION_MAJOR            = 0x01;
constexpr  uint8_t  GDC_VERSION_MINOR            = 0x01;
constexpr  uint8_t  GDC_VERSION_SUBMINOR         = 0x00;
constexpr  uint8_t  GDC_FLOW_TABLE_VERSION_MAJOR = 0x01;
constexpr  uint8_t  GDC_FLOW_TABLE_VERSION_MINOR = 0x00;

// Header Types
constexpr  uint16_t GDC_HEADER_TYPE_MASK         = 0xF000;
constexpr  uint16_t GDC_UNDEFINED_HEADER         = 0x0000;
constexpr  uint16_t GDC_CONTAINER_HEADER         = 0x1000;
constexpr  uint16_t GDC_COMPONENT_HEADER         = 0x2000;
constexpr  uint16_t GDC_PART_HEADER_MIN          = 0x4000; // 0x4000-0x4FFF reserved for sub Part Headers type.
constexpr  uint16_t GDC_PART_HEADER_MAX          = (GDC_PART_HEADER_MIN + 0x0FFF);
constexpr  uint16_t GDC_PART_HEADER              = (GDC_PART_HEADER_MIN + 0x0000);
constexpr  uint16_t GDC_FLOW_TABLE_HEADER        = 0x7000;
                                                
// Component Types (from SFNC)                  
constexpr  uint16_t GDC_UNDEFINED_COMPONENT      = 0x0000;
constexpr  uint16_t GDC_INTENSITY                = 0x0001;
constexpr  uint16_t GDC_INFRARED                 = 0x0002;
constexpr  uint16_t GDC_ULTRAVIOLET              = 0x0003;
constexpr  uint16_t GDC_RANGE                    = 0x0004;
constexpr  uint16_t GDC_REFLECTANCE              = 0x0005;
constexpr  uint16_t GDC_CONFIDENCE               = 0x0006;
constexpr  uint16_t GDC_SCATTER                  = 0x0007;
constexpr  uint16_t GDC_DISPARITY                = 0x0008;
constexpr  uint16_t GDC_MULTISPECTRAL            = 0x0009;
constexpr  uint16_t GDC_EXTENDED_COMPONENT       = 0x8000;
constexpr  uint16_t GDC_METADATA                 = (GDC_EXTENDED_COMPONENT   + 0x0001);
constexpr  uint16_t GDC_CUSTOM_COMPONENT_MIN     = (GDC_EXTENDED_COMPONENT   + 0x7F00); // + 0x0000-0x00FE valid
constexpr  uint16_t GDC_CUSTOM_COMPONENT_MAX     = (GDC_CUSTOM_COMPONENT_MIN + 0x00FE);
constexpr  uint16_t GDC_CUSTOM_COMPONENT         = (GDC_CUSTOM_COMPONENT_MIN + 0x0000);
constexpr  uint16_t GDC_RESERVED_COMPONENT       = (GDC_EXTENDED_COMPONENT   + 0x7FFF);
                                                
// Part types                                   
constexpr  uint16_t GDC_UNDEFINED_PART           = (0x0000);
constexpr  uint16_t GDC_GENERIC_PART_TYPE_MASK   = (GDC_PART_HEADER + 0x0F00);
constexpr  uint16_t GDC_GENERIC_PART_METADATA    = (GDC_PART_HEADER + 0x0000);
constexpr  uint16_t GDC_GENERIC_PART_1D          = (GDC_PART_HEADER + 0x0100);
constexpr  uint16_t GDC_GENERIC_PART_2D          = (GDC_PART_HEADER + 0x0200);
constexpr  uint16_t GDC_GENERIC_PART_CUSTOM      = (GDC_PART_HEADER + 0x0F00);
                                                
constexpr  uint8_t  GDC_PART_SPECIFIC_CUSTOM            = (0xF0);
constexpr  uint16_t GDC_PART_SPECIFIC_CUSTOM_INDEX_MASK = (0x0F);
constexpr  uint16_t GDC_METADATA_GENICAM_CHUNK          = (GDC_GENERIC_PART_METADATA + 0x00);
constexpr  uint16_t GDC_METADATA_GENICAM_XML            = (GDC_GENERIC_PART_METADATA + 0x01);
constexpr  uint16_t GDC_METADATA_CUSTOM                 = (GDC_GENERIC_PART_METADATA + GDC_PART_SPECIFIC_CUSTOM); // + 0x0-0xF valid
constexpr  uint16_t GDC_1D                              = (GDC_GENERIC_PART_1D + 0x00);
constexpr  uint16_t GDC_1D_CUSTOM                       = (GDC_GENERIC_PART_1D + GDC_PART_SPECIFIC_CUSTOM); // + 0x0-0xF valid
constexpr  uint16_t GDC_2D                              = (GDC_GENERIC_PART_2D + 0x00);
constexpr  uint16_t GDC_2D_JPEG                         = (GDC_GENERIC_PART_2D + 0x01);
constexpr  uint16_t GDC_2D_JPEG2000                     = (GDC_GENERIC_PART_2D + 0x02);
constexpr  uint16_t GDC_2D_H264                         = (GDC_GENERIC_PART_2D + 0x03);
constexpr  uint16_t GDC_2D_CUSTOM                       = (GDC_GENERIC_PART_2D + GDC_PART_SPECIFIC_CUSTOM); // + 0x0-0xF valid
constexpr  uint16_t GDC_CUSTOM                          = (GDC_GENERIC_PART_CUSTOM); // + 0x0-0xFE valid


// GenDC objects forward declarations:
// -----------------------------------
// -----------------------------------
struct GenDCContainerHeader;
struct GenDCComponentHeader;
struct GenDCPartHeader;

// GenDC types definitions.
// ------------------------
// ------------------------

// Pack all the structures.
#pragma pack(push, 1)

// GenDC Container typedef and enumerations.
enum class GenDCSignature : uint32_t { GenDC = GDC_SIGNATURE };
typedef    uint32_t         GenDCVersionValue;
typedef    uint8_t          GenDCVersionField;
namespace GenDCVersionCurrent
{
const GenDCVersionField  Major = GDC_VERSION_MAJOR;
const GenDCVersionField  Minor = GDC_VERSION_MINOR;
const GenDCVersionField  SubMinor = GDC_VERSION_SUBMINOR;
}; // Ex: Version 01.01.00

typedef    uint16_t     GenDCHeaderTypeShared;
enum class GenDCHeaderType : GenDCHeaderTypeShared
{ HeaderMask = GDC_HEADER_TYPE_MASK,
  Undefined  = GDC_UNDEFINED_HEADER,
  Container  = GDC_CONTAINER_HEADER,
  Component  = GDC_COMPONENT_HEADER,
  Part       = GDC_PART_HEADER,
  FlowTable  = GDC_FLOW_TABLE_HEADER
};
typedef uint32_t GenDCHeaderSize;
typedef uint16_t GenDCHeaderFlags;
union   GenDCContainerFlagsGroup;
typedef uint64_t GenDCContainerId;
typedef uint64_t GenDCDataSize;
typedef size_t   GenDCAllocationDataSize;
typedef int64_t  GenDCOffset;
typedef uint32_t GenDCDescriptorSize;
typedef GenDCContainerHeader GenDCDescriptor;
typedef uint32_t GenDCComponentCount;
typedef uint8_t  GenDCContainerBuffer;
typedef uint64_t GenDCContainerBufferSize;
typedef uint8_t  GenDCDataU8;
typedef uint16_t GenDCDataU16;
typedef uint32_t GenDCDataU32;
typedef uint64_t GenDCDataU64;
typedef float    GenDCDataFloat;
typedef void     GenDCDataVoid;
typedef struct   GenDCuint24_t /* 3 bytes pixel. Ex: RGB8. */
{
    // Storage for the RGB or BGR 8 bit values
    uint8_t PixelValue[3];

    // Default copy constructor.
    GenDCuint24_t(const  GenDCuint24_t &DataU24Source)
    {
        for (int j = 0; j < 3; j++)
            PixelValue[j] = DataU24Source.PixelValue[j];
    };

    // Default conversion.
    operator uint32_t() const
    {
        uint32_t PixelValue32;
        for (int j = 0; j < 3; j++)
            ((uint8_t*)(&PixelValue32))[j] = PixelValue[j];
        return (PixelValue32);
    };
} GenDCDataU24;

// GenDC Component typedef and enumerations.
typedef uint16_t GenDCGroupId;
typedef uint16_t GenDCSourceId;
typedef uint16_t GenDCRegionId;
typedef uint32_t GenDCRegionOffset;
typedef int64_t  GenDCTimestamp;
typedef uint64_t GenDCComponentTypeId;
enum class       GenDCComponentType : GenDCComponentTypeId
{
    Undefined  = GDC_UNDEFINED_COMPONENT, Intensity     = GDC_INTENSITY,
    Infrared   = GDC_INFRARED,            Ultraviolet   = GDC_ULTRAVIOLET,
    Range      = GDC_RANGE,               Reflectance   = GDC_REFLECTANCE,
    Confidence = GDC_CONFIDENCE,          Disparity     = GDC_DISPARITY,
    Scatter    = GDC_SCATTER,             Multispectral = GDC_MULTISPECTRAL,
    Extended   = GDC_EXTENDED_COMPONENT,  Metadata      = GDC_METADATA,
    Custom     = GDC_CUSTOM_COMPONENT,    Reserved      = GDC_RESERVED_COMPONENT
};
typedef PfncFormat GenDCFormat;
typedef uint16_t   GenDCPartCount;

// GenDC Part typedef and enumerations.
typedef uint64_t GenDCSize;
typedef int64_t  GenDCSizeSigned;
typedef uint64_t GenDCSize1D;
typedef uint32_t GenDCSize2D;
typedef uint16_t GenDCPadding;
typedef uint64_t GenDCDataSize;
typedef uint16_t GenDCFlowId;
typedef uint32_t GenDCFlowCount;
typedef uint64_t GenDCFlowSize;
typedef GenDCOffset GenDCFlowOffset;
typedef uint64_t GenDCInfoTypeSpecific1D;
typedef GenDCInfoTypeSpecific1D GenDCGenICamLayoutId;

typedef    GenDCHeaderTypeShared  GenDCPartHeaderType;
enum class GenDCGenericPartType : GenDCPartHeaderType
{
    Undefined                  = GDC_UNDEFINED_PART,
    GenericMetadata            = GDC_GENERIC_PART_METADATA,
    Generic1D                  = GDC_GENERIC_PART_1D,
    Generic2D                  = GDC_GENERIC_PART_2D,
    GenericSpecificCustom      = GDC_PART_SPECIFIC_CUSTOM,
    GenericCustom              = GDC_GENERIC_PART_CUSTOM, 
    GenericMask                = GDC_GENERIC_PART_TYPE_MASK,
    GenericCustomPartIndexMask = GDC_PART_SPECIFIC_CUSTOM_INDEX_MASK
};
enum class GenDCPartType : GenDCPartHeaderType
{
    GenICamChunk    = GDC_METADATA_GENICAM_CHUNK, GenICamXML = GDC_METADATA_GENICAM_XML,
    CustomMetadata  = GDC_METADATA_CUSTOM,        Array1D    = GDC_1D,
    Custom1D        = GDC_1D_CUSTOM,              Array2D    = GDC_2D,
    JPEG            = GDC_2D_JPEG,                JPEG2000   = GDC_2D_JPEG2000,
    H264            = GDC_2D_H264,                Custom2D   = GDC_2D_CUSTOM,
    Custom          = GDC_CUSTOM
};

// General utility types and enumerations.
typedef uint8_t     GenDCReserved8;
typedef uint16_t    GenDCReserved16;
typedef uint32_t    GenDCReserved32;
typedef uint64_t    GenDCReserved64;
typedef const char* GenDCErrorMessage;
namespace GenDCPredefined // Namespace to avoid global scope but still have automatic type conversion.
{
enum GenDCPredefinedComponent : GenDCComponentCount { AllComponents = 0xFFFFFFFF, AllComponentsNoMetadata = 0xFFFFFFFE, InvalidComponentIndex = 0xFFFFFFFD};
enum GenDCPredefinedFlow      : GenDCFlowCount      { AllFlows = 0xFFFFFFFF };
enum GenDCPredefinedPart      : GenDCPartCount      { AllParts = 0xFFFF, InvalidPartIndex = 0xFFFE};
enum GenDCPredefinedSize      : GenDCSize           { FullSize = 0x8FFFFFFFFFFFFFFF, Center = 0x8FFFFFFFFFFFFFFE };
enum GenDCPredefinedReserved  : GenDCDataU8         { ReservedDefault = 0x00 };
}

// ---------------------------------------------------------------------------------------------
// Utility Functions:

// Returns the Offset in a Container buffer of any Descriptor's field relative to the Container start.
// Note: The 2 arguments must be located in the same Container memory buffer.
 template<class ContainerBufferObject, class ContainerBufferObjectField> 
inline GenDCOffset GenDCDescriptorFieldOffsetCalculate(ContainerBufferObject& OwnerContainer, ContainerBufferObjectField& ContainerField)
{
    return((GenDCOffset)(((GenDCDataU8*)(&ContainerField)) - ((GenDCDataU8*)(&OwnerContainer))));
}

//GenDC headers structures:
//-----------------------------------------
//-----------------------------------------

// Container's Flags bit fields structure.
// ---------------------------------------
union GenDCContainerFlagsGroup    // Container's Flags.
{
    GenDCHeaderFlags Value = 0;
    struct
    {
        GenDCHeaderFlags TimestampPTP     : 1;
        GenDCHeaderFlags ComponentInvalid : 1;
        GenDCHeaderFlags Reserved         : 14;
    } Field;
};

// Container's VariableFields flags bit fields structure.
// ------------------------------------------------------
union GenDCContainerVariableFieldsGroup    // Container's Flags.
{
    GenDCHeaderFlags Value = 0;
    struct
    {
        GenDCHeaderFlags DataSize : 1;
        GenDCHeaderFlags SizeX : 1;
        GenDCHeaderFlags SizeY : 1;
        GenDCHeaderFlags RegionOffset : 1;
        GenDCHeaderFlags Format : 1;
        GenDCHeaderFlags Timestamp : 1;
        GenDCHeaderFlags ComponentCount : 1;
        GenDCHeaderFlags ComponentInvalid : 1;
        GenDCHeaderFlags Reserved : 8;
    } Field;
};

// Container's Version fields structure.
// ------------------------------------------------------
struct GenDCContainerVersion
{
    GenDCVersionField Major    = GDC_VERSION_MAJOR;
    GenDCVersionField Minor    = GDC_VERSION_MINOR;
    GenDCVersionField SubMinor = GDC_VERSION_SUBMINOR;
};

// GenDC Container header base.
//-----------------------------
struct GenDCContainerHeaderBase
{
    GenDCSignature                     Signature = GenDCSignature::GenDC;
    GenDCContainerVersion              Version;
    GenDCReserved8                     Reserved = GenDCPredefined::ReservedDefault;
    GenDCHeaderType                    HeaderType = GenDCHeaderType::Container;
    GenDCContainerFlagsGroup           Flags;
    GenDCHeaderSize                    HeaderSize = sizeof(GenDCContainerHeaderBase);
    GenDCContainerId                   Id = 0;
    GenDCContainerVariableFieldsGroup  VariableFields;
    GenDCReserved16                    Reserved16 = GenDCPredefined::ReservedDefault;
    GenDCReserved32                    Reserved32 = GenDCPredefined::ReservedDefault;
    GenDCDataSize                      DataSize = 0;
    GenDCOffset                        DataOffset = 0; // Must be null by default since DataSize = 0.
    GenDCDescriptorSize                DescriptorSize = sizeof(GenDCContainerHeaderBase);
    GenDCComponentCount                ComponentCount = 0;
    // Offset ComponentOffsets[ComponentCount]; field not included in base class.
};

// GenDC Container header (including a ComponentOffset[] array member).
//---------------------------------------------------------------------
constexpr GenDCComponentCount GDC_CONTAINER_COMPONENT_COUNT_MIN = 1;
struct GenDCContainerHeader : GenDCContainerHeaderBase
{
    // Offset ComponentOffsets[ComponentCount]; minimal Component Offset(s) included.
    // This allow to use Container.ComponentOffsets[ComponentIndex].
    // Note: sizeof(GenDCContainerHeader) must not be used if ComponentCount > 1.
    // Note: Only ContaineHeader reference (GenDCContainerHeader &) can be used as destination to operator =
    //       since the default copy constructor will not copy all the ComponentOffset members.
    //       Use only ContaineHeader &Container = AnotherContaineHeader.
    GenDCOffset ComponentOffsets[GDC_CONTAINER_COMPONENT_COUNT_MIN] = { 0 };

    // Default constructor for "GenDCContainerHeader &".
    GenDCContainerHeader() {;};

    // Deleted default copy constructor since it will not copy all the ComponentOffsets[] members.
    // It would also not point to the real Container header memory Offset.
    // Use a reference instead (Ex: GenDCContainerHeader& MyContainer = ...;) or a pointer.
    GenDCContainerHeader(const GenDCContainerHeader &) = delete;
};

// Component's Flags bit fields structure.
// ---------------------------------------
union GenDCComponentFlagsGroup // Component's Flags.
{
    GenDCHeaderFlags Value = 0;
    struct
    {
        GenDCHeaderFlags Invalid : 1;
        GenDCHeaderFlags Unallocated : 15;
    } Field;
};

// GenDC Component header base.
// ----------------------------
struct GenDCComponentHeaderBase
{
    GenDCHeaderType     HeaderType = GenDCHeaderType::Component;
    GenDCComponentFlagsGroup Flags;
    GenDCHeaderSize     HeaderSize = 0;
    GenDCReserved16     Reserved = 0;
    GenDCGroupId        GroupId = 0;
    GenDCSourceId       SourceId = 0;
    GenDCRegionId       RegionId = 0;
    GenDCRegionOffset   RegionOffsetX = 0;
    GenDCRegionOffset   RegionOffsetY = 0;
    GenDCTimestamp      Timestamp = 0;
    GenDCComponentType  TypeId = GenDCComponentType::Undefined;
    GenDCFormat         Format = (GenDCFormat)0;
    GenDCReserved16     Reserved2 = GenDCPredefined::ReservedDefault;
    GenDCPartCount      PartCount = 0;
    // Offset           PartOffsets[PartCount]; // This field is not included in base class.
};

// GenDC Component  header (including a PartOffset[] array member).
//-----------------------------------------------------------------
constexpr GenDCPartCount GDC_COMPONENT_PART_COUNT_MIN = 1;
struct GenDCComponentHeader : GenDCComponentHeaderBase
{
    // Offset PartOffsets[PartCount]; minimal Component Offset(s) included.
    // This allow to use Component.PartOffsets[ComponentIndex].
    // Note: sizeof(Component Header) must not be used if ComponentCount > 1.
    //       Only Component header reference (Part Header &) can be used as destination to operator = 
    //       since the default copy constructor will not copy all the PartOffsets members.
    //       Use only GenDCComponentHeader &Component = AnotherGenDCComponentHeader.
    GenDCOffset PartOffsets[GDC_COMPONENT_PART_COUNT_MIN] = { 0 };

    // Default constructor for "GenDCComponentHeader &".
    GenDCComponentHeader() { ; };

    // Deleted default copy constructor since it will not copy all the Parts header members.
    // It would also not point to the real Component header memory Offset.
    // Use a reference instead (Ex: CoponenHeader& MyComponent = ...;) or a pointer.
    GenDCComponentHeader(const GenDCComponentHeader &) = delete;
};

// Part's Flags bit fields structure.
// ---------------------------------------
union GenDCPartFlagsGroup    // Part's Flags.
{
    GenDCHeaderFlags Value = 0;
    struct
    {
        GenDCHeaderFlags Unallocated : 16;
    } Field;
};

// GenDC Part header common fields.
// --------------------------------
struct GenDCPartHeaderBase
{
   GenDCPartType   HeaderType = (GenDCPartType)GenDCGenericPartType::Undefined;
   GenDCPartFlagsGroup  Flags;
   GenDCHeaderSize HeaderSize = 0;
   GenDCFormat     Format = (GenDCFormat)0;
   GenDCReserved16 Reserved = GenDCPredefined::ReservedDefault;
   GenDCFlowId     FlowId = 0;
   GenDCFlowOffset FlowOffset = 0;
   GenDCDataSize   DataSize = 0;
   GenDCOffset     DataOffset = 0;
};

// GenDC generic standard Part header.
//------------------------------------
struct GenDCPartHeader : public GenDCPartHeaderBase // Add no field compared to base class.
{
   // Default constructor for "GenDCPartHeader &".
   GenDCPartHeader() { };

   // Deleted default copy constructor since it will not copy all Part type specific members.
   // It would also not point to the real Part header memory Offset.
   // Use a reference instead (Ex: GenDCPartHeader& MyPart = ...;) or a pointer.
   GenDCPartHeader(const GenDCPartHeader &) = delete;
};

// GenDC 1D uncompressed Part header.
// ----------------------------------
struct GenDCPartHeader1DBase : GenDCPartHeader
{
    GenDCSize1D     SizeX = 0;
    GenDCPadding    PaddingX = 0;
    GenDCReserved16 PaddingReserved = GenDCPredefined::ReservedDefault;
    GenDCReserved32 InfoReserved    = GenDCPredefined::ReservedDefault;
    GenDCInfoTypeSpecific1D InfoTypeSpecific = 0;
};

// GenDC 1D uncompressed Part header.
// ----------------------------------
struct GenDCPartHeader1D : GenDCPartHeader1DBase
{
};

// GenDC GenICam Chunk Metadata Part header.
// -----------------------------------------
struct GenDCPartHeaderGenICamChunk : GenDCPartHeader1DBase
{
    // For GenICam Chunk metadata Part, InfoTypeSpecific[] should contain the GenICam chunk_layout_id.
};

// GenDC GenICam XML Metadata Part header.
// ---------------------------------------
struct GenDCPartHeaderGenICamXML : GenDCPartHeader1DBase
{
    // For GenICam XML metadata Part, InfoTypeSpecific[] should contain the GenICam XML_layout_id.
};

// GenDC 2D Part header base 
//---------------------------
struct GenDCPartHeader2DBase : public GenDCPartHeader
{
    GenDCSize2D     SizeX = 0;
    GenDCSize2D     SizeY = 0;
    GenDCPadding    PaddingX = 0;
    GenDCPadding    PaddingY = 0;
    GenDCReserved32 InfoReserved = GenDCPredefined::ReservedDefault;
};

// GenDC 2D uncompressed Part header
//-----------------------------------
struct GenDCPartHeader2D : public GenDCPartHeader2DBase
{
};

// GenDC 2D JPEG and JPEG200 standard Type specific info.
//-------------------------------------------------------
struct GenDCInfoTypeSpecificJPEG
{
};

// GenDC 2D JPEG standard Part header.
//------------------------------------
struct GenDCPartHeaderJPEG : public GenDCPartHeader2DBase
{
};

// GenDC 2D JPEG2000 standard Part header.
//----------------------------------------
struct GenDCPartHeaderJPEG2000 : public GenDCPartHeader2DBase
{
};

// Part Info field interpretation structure for H.264.
struct GenDCInfoTypeSpecificH264
{
    uint8_t Reserved = GenDCPredefined::ReservedDefault;
    uint8_t	ProfileIDC = 0;
    struct H264Flags
    {
        uint8_t CS_set0_flag : 1;
        uint8_t CS_set1_flag : 1;
        uint8_t CS_set2_flag : 1;
        uint8_t CS_set3_flag : 1;
        uint8_t	PM : 2;
        uint8_t RF : 2;
    } Flags;
    uint8_t	LevelIDC = 0;
    uint16_t SpropInterleavingDepth = 0;
    uint16_t SpropMaxDonDiff = 0;
    uint32_t SpropDeintBufReq = 0;
    uint32_t SpropInitBufTime = 0;

    GenDCInfoTypeSpecificH264() : Flags { 0 } {}
} ;

// GenDC 2D H.264 compressed Part header.
// --------------------------------------
struct GenDCPartHeaderH264 : public GenDCPartHeader2DBase
{
   GenDCInfoTypeSpecificH264 InfoTypeSpecificH264;
};

// GenDC Container predefined Descriptor for a single 2D Component declaration.
// Assuming 1 Components with 1 2D Part.
struct GenDCContainerDescriptorComponent2D : GenDCContainerHeaderBase
{
    // Container header base fields followed by a single ComponentOffset.
    GenDCOffset ComponentOffsets[1] = { 0 };

    // Including a Component header base followed by a single PartOffset.
    struct : GenDCComponentHeaderBase
    {
        GenDCOffset PartOffsets[1] = { 0 };

        // Including a Part header.
        struct : GenDCPartHeader2D
        {
        } Part2D;
    } Component;
};

//  GenDC Container predefined Descriptor for a 2D Component with GenICam Chunk data declaration.
//  Assuming 1 Components with 1 2D Part and 1 Metadata Component with GenICam chunk data.
struct GenDCContainerDescriptorComponent2DWithChunkData : GenDCContainerHeaderBase
{
    // Container header base fields followed by the ComponentOffset.
    GenDCOffset ComponentOffsets[2] = { 0, 0 };

    // Including a Component header base followed by the 2D PartOffset.
    struct : GenDCComponentHeaderBase
    {
        GenDCOffset PartOffsets[1] = { 0 };

        // Including Part header.
        struct : GenDCPartHeader2D
        {
        } Part2D;
    } Component;

    // Including a Metadata Component header followed by the Chunk Data PartOffset.
    struct : GenDCComponentHeaderBase
    {
        GenDCOffset PartOffsets[1] = { 0 };

        // Including Part header.
        struct : GenDCPartHeaderGenICamChunk
        {
        } PartChunk;
    } ComponentMetadata;
};

// GenDC Container predefined Descriptor Template declaration for 2D Components.
// Assuming that all the Components and all their 2D Parts are symmetrical.
template<size_t _ComponentCount, size_t _PartCount>
struct GenDCContainerDescriptorTemplate2D : GenDCContainerHeaderBase
{
    // Container header base fields followed by the ComponentOffset.
    GenDCOffset ComponentOffsets[_ComponentCount] = { 0 };

    // Including a Component header base followed by the PartOffset.
    struct : GenDCComponentHeaderBase
    {
        GenDCOffset PartOffsets[_PartCount] = { 0 };

        // Including Part headers.
        struct : GenDCPartHeader2D
        {
        } Parts2D[_PartCount];
    } Components[_ComponentCount];
};

namespace GenDCFlowTableVersionCurrent
   {
   const GenDCVersionField Major = GDC_FLOW_TABLE_VERSION_MAJOR;
   const GenDCVersionField Minor = GDC_FLOW_TABLE_VERSION_MINOR;
   } // Ex: Version 01.00
   
// Flow table Version fields structure.
// ------------------------------------
struct GenDCFlowTableVersion
{
    GenDCVersionField Major = GDC_FLOW_TABLE_VERSION_MAJOR;
    GenDCVersionField Minor = GDC_FLOW_TABLE_VERSION_MINOR;
};

// Flow table base header.
// -------------------------
struct GenDCFlowTableHeader
{
    GenDCHeaderType       HeaderType = GenDCHeaderType::FlowTable;
    GenDCHeaderFlags      Flags = 0;
    GenDCHeaderSize       HeaderSize = sizeof(GenDCFlowTableHeader);
    GenDCFlowTableVersion Version;
    GenDCReserved16       Reserved = GenDCPredefined::ReservedDefault;
    GenDCFlowCount        FlowCount = 0;
};

// Flow table with one entry (Note: sizeof(FlowTable) invalid).
// -------------------------
struct GenDCFlowTable : GenDCFlowTableHeader
{
    GenDCFlowSize FlowSize[1] = { 0 }; // Stub array field for easy access using FlowTable.FlowSize[n].
};

// Flow table template.
// -------------------------
template<GenDCPartCount _FlowCount>
struct GenDCFlowTableTemplate : GenDCFlowTableHeader
{
    GenDCFlowSize FlowSize[_FlowCount] = { 0 };
};

// Restore packing.
#pragma pack(pop)

} // namespace GenDC

#endif // #ifndef _GENDC_H_
