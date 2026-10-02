#ifndef _UDF_DATA_SET_H_
#define _UDF_DATA_SET_H_

#include <stdint.h>
#include <list>
#include <time.h>
#include <memory>
#include <cstring>

#pragma pack(1)

#define MaxFileSize    1073741823   //1073741823=1G

//descriptor tag (ECMA 167 3/7.2)
//16 Bytes
struct descriptor_tag {
    uint16_t	tagIdent;
    uint16_t	descVersion;
    uint8_t     tagChecksum;     //校验和
    uint8_t     reserved;
    uint16_t	tagSerialNum;
    uint16_t	descCRC;
    uint16_t	descCRCLength;
    uint32_t	tagLocation;
};

//CS0 Ch arspec (ECMA 167 1/7.2.1)
struct tag_char_sepc{
    uint8_t charSetType;
    uint8_t charSetInfo[63];
};

//Extent Descriptor (ECMA 167 3/7.1)
struct tag_extent_ad{
    uint32_t extLength;
    uint32_t extLocation;
};

//Timestamp (ECMA 167 1/7.3)
//12bytes
struct time_stamp{
    uint16_t typeAndTimezone;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t centiseconds;
    uint8_t hundredsOfMicroseconds;
    uint8_t microseconds;
} ;

//Entity Identifier (ECMA 167 1/7.4)
//32Bytes
struct tag_imp_id{
    uint8_t flags;
    uint8_t ident[23];
    uint8_t osClass;
    uint8_t osIdentifier;
    uint8_t identSuffix[6];
};

//Primary Volume Descriptor (ECMA 167 3/10.1)
//提供关于卷的基本信息，如卷序列号、字符集、录制时间等。这个描述符对于识别、管理和操作光盘数据非常重要
struct tag_primary_vol_desc {
    //该数据结构的类型标识符，用于标识该块数据是一个卷描述符
    descriptor_tag descTag;
    //卷描述符序列号，用于标识光盘镜像中的卷描述符位置
    uint32_t        volDescSeqNum;
    //主卷描述符编号，表示这是一个主卷描述符，它指示该描述符属于光盘镜像的主描述符
    uint32_t        primaryVolDescNum;
    //存储光盘的卷标，可以是任何 32 字节的字符串，用于标识该卷
    uint8_t         volIdent[32];
    //卷的序列号，表示当前卷在卷集中的位置。对于多卷集，序列号可以帮助识别当前卷。
    uint16_t        volSeqNum;
    //最大卷序列号，指定该卷集中的最大卷数
    uint16_t        maxVolSeqNum;
    //交换级别，表示该卷支持的交换级别。通常UDF的交换级别用于指定该卷的兼容性与其他系统之间的互操作性。
    uint16_t        interchangeLvl;
    //最大交换级别，指示该卷支持的最大交换级别
    uint16_t        maxInterchangeLvl;
    //该字段指示卷支持的字符集的列表，常用于文件系统中的字符编码（例如 UTF-8）
    uint32_t        charSetList;
    //指示该卷支持的最大字符集数量
    uint32_t        maxCharSetList;
    //存储卷集的标识符，可以是一个最大 128 字节的字符串
    uint8_t         volSetIdent[128];
    //指定用于卷描述符部分的字符集类型（例如 ISO 8859-1，UTF-8 等）。
    tag_char_sepc   descCharSet;
    //用于说明文本部分的字符集类型，与 descCharSet 类似，但用于非描述符文本内容
    tag_char_sepc   explanatoryCharSet;
    //存储卷摘要信息，用于提供有关卷的简要概述。
    tag_extent_ad   volAbstract;
    //存储卷的版权信息，通常指向版权描述区域。
    tag_extent_ad    volCopyright;
    //标识生成该卷的应用程序，提供该卷是由哪个应用程序创建的
    tag_imp_id      appIdent;
    //记录卷创建或修改的日期和时间
    time_stamp      recordingDateAndTime;
    //标识卷的发行商或制造商
    tag_imp_id      impIdent;
    //一个 64 字节的区域，用于发行商的特殊用途
    uint8_t         impUse[64];
    //指示前一个卷描述符的位置，通常用于多卷镜像中，帮助寻找前一个卷的描述符
    uint32_t        predecessorVolDescSeqLocation;
    //用于表示卷的一些特性或状态
    uint16_t        flags;
    //保留字段，预留给未来扩展使用，不做实际用途。
    uint8_t         reserved[1558];
};


struct tag_udf_entity_Id{
    uint8_t  uFlags;
    uint8_t  aID[23];
    uint16_t uUDFRevision;
    uint8_t  uOSClass;
    uint8_t  uOSIdentifier;
    uint8_t  aReserved[4];
};

//Implementation Use Volume Descriptor (ECMA 167 3/10.4)
//用于存储与卷相关的实现细节，特别是那些由实现者定义的自定义数据，主要用于记录一些应用或系统的特定信息
struct tag_imp_use_vol_desc{
    //该字段标识当前数据块的类型
    descriptor_tag    descTag;
    //卷描述符序列号，用于标识当前卷描述符的位置
    uint32_t          volDescSeqNum;
    //该字段用于标识生成此卷描述符的实现者或系统，即卷的创建者或发行商。
    tag_udf_entity_Id impIdent;
    //该字段指示逻辑卷标识符（logVolIdent）所使用的字符集类型。这告诉系统如何解释逻辑卷标识符的字符编码
    tag_char_sepc     lviCharSet;
    //用于存储卷的逻辑标识符。通常，逻辑卷标识符是一个字符串，用于唯一标识卷
    uint8_t           logVolIdent[128];
    //该字段用于存储与卷相关的附加信息。它通常是一个 108 字节的字符串或数据块，可以包括卷的特性、版本信息、描述或其他非标准信息
    uint8_t           logVolInfo[108];
    //该字段用于标识卷的实现版本或生成该卷描述符的应用程序或系统。实现 ID 有助于标识该卷与特定应用、工具或系统的关联。这对于系统和应用程序来说可能是一个唯一的标识符，用于了解卷的创建环境。
    tag_imp_id        umplementationID;
    //该字段为实现者提供了一个 128 字节的区域，用于存储与卷的实现相关的自定义数据。
    uint8_t           umplementationUse[128];
    //这是一个保留字段，预留给未来的扩展使用
    uint8_t           reserved[1536];
};

struct tag_reg_id {
    uint8_t uFlags;
    uint8_t aID[23];
    uint8_t aReserved[8];
};

//Short Allocation Descriptor (ECMA 167 4/14.14.1)
struct tag_short_ad{
    uint32_t extLength;
    uint32_t extPosition;
};

//Partition Header Descriptor (ECMA 167 4/14.3)
struct tag_partition_header_desc{
    uint8_t         reserve1[8];
    tag_short_ad    unallocatedSpaceBitmap;
    uint8_t         reserve2[16];
    tag_short_ad    freedSpaceBitmap;
    uint8_t         reserved3[88];
};

//Partition Descriptor (ECMA 167 3/10.5)
//用于描述存储设备或虚拟磁盘上的一个分区。它包含了有关分区的各类元数据，确保数据能够正确地存储、访问和管理
struct  tag_partition_desc {
    //用于区分不同类型的描述符
    descriptor_tag              descTag;
    //这是一个32位的无符号整数，表示该卷描述符的序列号。卷描述符序列号用于标识和排序多个卷描述符，以便在文件系统中管理多个卷
    uint32_t                    volDescSeqNum;
    //包含与分区相关的标志位。每个位可以用来指示分区的某些特性或状态（如是否可启动、是否可写等）
    uint16_t                    partitionFlags;
    //表示分区的编号。每个磁盘或虚拟磁盘的分区都有一个唯一的编号，用于区分不同的分区
    uint16_t                    partitionNumber;
    //这是一个用于标识分区内容的注册标识符。它通常用于指定分区内存储的内容类型，如数据、文件系统等
    tag_reg_id                  partitionContents;
    //这是一个描述分区内容使用方式的结构体。它可能包含关于如何使用该分区的附加信息，例如，分区的文件系统类型或存储的特定数据类型。
    tag_partition_header_desc   partitionContentsUse;
    //这是一个32位无符号整数，表示分区的访问类型。可能的访问类型包括只读、读写等，指示该分区允许的访问权限
    uint32_t                    accessType;
    //表示分区的起始位置。该值指定了分区相对于设备起始位置的偏移量，通常用于定位该分区的开始位置
    uint32_t                    partitionStartingLocation;
    //这是一个32位无符号整数，表示分区的长度。它指定了分区的大小（即从起始位置到分区结束的位置的字节数）
    uint32_t                    partitionLength;
    //这是一个实施标识符，用于唯一标识某种特定实施。它可能与分区的特殊用途或特性有关，如某些厂商特定的实现
    tag_imp_id                  impIdent;
    //这是一个128字节的数组，用于实施特定的用途。它可以存储与分区相关的任何额外信息，通常由实施方定义。
    uint8_t                     impUse[128];
    //保留区域
    uint8_t                     reserved[1692];
};

//Recorded Address (ECMA 167 4/7.1)
struct tag_lb_addr{
    //逻辑块地址
    uint32_t logicalBlockNum;
    //分区编号
    uint16_t partitionReferenceNum;
};

//Long Allocation Descriptor (ECMA 167 4/14.14.2)
//16bytes
struct  tag_long_ad{
    //数据的长度单位是字节
    uint32_t  extLength;
    tag_lb_addr extLocation;
    //扩展类型信息
    uint8_t   impUse[6];
};

struct tag_domain_id{
    uint8_t   uFlags;
    uint8_t   aID[23];
    uint16_t  uUDFRevision;
    uint8_t   domainFlag;
    uint8_t   aReserved[5];
};

//Generic Partition Map (ECMA 167 3/10.7.1)
struct tag_generic_partition_map{
    uint8_t   partitionMapType;
    uint8_t   partitionMapLength;
    uint16_t  volSequNo;
    uint16_t  partitionNo;
};

//Logical Volume Descriptor (ECMA 167 3/10.6)
//用于描述磁盘的逻辑结构以及与卷相关的元数据。
struct  tag_logical_vol_desc {
    //描述符标记
    descriptor_tag              descTag; //16
    //逻辑卷描述符的序列号，标识该卷描述符在UDF文件系统中的位置
    uint32_t                    volDescSeqNum;
    //字符集描述符，指定了该卷使用的字符集。
    tag_char_sepc               descCharSet;
    //逻辑卷的标识符。这个字段包含了一个128字节的数组，用于存储逻辑卷的标识符
    uint8_t                     logicalVolIdent[128];
    //逻辑块大小。该字段指定了该逻辑卷的每个逻辑块的大小，单位通常是字节
    uint32_t                    logicalBlockSize;
    //域标识符。这个字段标识了该逻辑卷所属的域（domain），即该卷的管理范围或空间
    tag_domain_id               domainIdent;
    //文件集描述符扩展的长地址。这个字段是指向文件集描述符的地址扩展（FSD），该文件集描述符包含有关卷中所有文件的信息
    tag_long_ad                 fileSetDescSequExt;
    //映射表长度。该字段指定了映射表的长度，映射表用于描述文件数据如何在物理磁盘上进行映射。
    uint32_t                    mapTableLength;
    //分区映射数目。该字段指定了卷上分区映射的数量。
    uint32_t                    numPartitionMaps;
    //实现标识符。该字段用于指定卷的实现标识符，通常用于标识该卷由哪个厂商或实现方式所创建
    tag_imp_id                  impIdent;
    //实现使用字段。这个字段用于存储与卷实现相关的128字节的自定义数据。它可能包含实现特定的附加信息
    uint8_t                     impUse[128];
    //完整性序列扩展的地址。该字段是指向完整性数据的扩展地址，完整性数据用于验证卷的完整性和一致性
    tag_extent_ad               integritySeqExt;
    //分区映射。这个字段用于存储有关卷分区的映射信息。它描述了卷中不同逻辑区域的物理存储位置
    tag_generic_partition_map   p_partitionMaps;
    //保留字段。此字段是保留区域
    uint8_t                     p_reserved[2048-446];
};


//Unallocated Space Descriptor (ECMA 167 3/10.8)
//在UDF中，未分配空间描述符 用于标记那些不属于文件系统已分配区域的磁盘空间。这些区域可以用于未来的数据分配或存储用途。
struct tag_unallocated_space_desc{
    //标识符
    descriptor_tag   descTag;
    //表示卷描述符序列号
    uint32_t         volDescSeqNum;
    //表示已分配描述符的数量。已分配描述符是文件系统中的结构，用于标记已被使用的空间或区域
    uint32_t         numAllocDescs;
    //这是一个保留字节数组，用于填充结构体
    uint8_t          p_reserved[2048-24];
};

//Terminating Descriptor (ECMA 3/10.9)
//用于标识描述符链的结束
struct  tag_terminating_desc {
    // 描述符标签
    descriptor_tag   descTag;
    //保留字段
    uint8_t          reserved[2032];
};


//Logical Volume Integrity Descriptor (ECMA 167 3/10.10)
//保证文件系统中逻辑卷的完整性和一致性。逻辑卷一致性描述符提供了关于逻辑卷如何维护一致性的信息，包括用于检查和恢复数据一致性的信息。
//这个描述符对于保证存储介质的数据安全性、恢复能力以及防止数据丢失至关重要
struct tag_logic_vol_integrity_desc {
    //描述符标签
    descriptor_tag  descTag;
    //是一个表示该逻辑卷一致性描述符创建或更新的时间点的时间
    //这个时间戳提供了关于该逻辑卷一致性信息的创建时间，帮助跟踪文件系统的变更历史，可能用于恢复操作或日志记录。
    time_stamp       recordingDateAndTime;
    //指示当前逻辑卷的完整性类型。例如，它可能指示某种特定的检查点或数据一致性模型，系统可以基于此字段确定如何处理逻辑卷的一致性
    uint32_t         integrityType;
    //这个字段指示下一个一致性区域的起始位置。
    tag_extent_ad    nextIntegrityExt;
    //字段提供了逻辑卷的唯一标识符，确保逻辑卷能够在不同的文件系统和存储设备中被唯一地识别。
    uint64_t         uniqueId;
    //这是一个 24 字节的保留字段，用于对齐结构体并为未来的扩展保留空间
    uint8_t          reserved[24];
    //位无符号整数，表示逻辑卷中分区的数量
    uint32_t         numOfPartitions;
    //用于存储与逻辑卷一致性相关的额外信息
    uint32_t         lengthOfImpUse;
    //该字段指示逻辑卷上空闲空间表的位置。空闲空间表记录了逻辑卷上哪些区域是空闲的，这对于文件系统的空间管理和一致性非常重要。
    uint32_t         freeSpaceTable;
    //该字段指示逻辑卷上大小表的位置。大小表记录了文件系统的空间布局、文件的大小和分布等信息，这有助于维护卷的一致性和完整性。
    uint32_t         sizeTable;
    //该字段包含与逻辑卷一致性相关的额外信息，用于系统在恢复数据时使用。它存储着一些重要的使用数据，帮助文件系统进行一致性校验或其他操作
    uint8_t          p_impUse[46];
    //这是一个保留字段，填充剩余空间，确保该结构体的总大小为2048字节
    uint8_t          p_reserved[2048-88-46];
};

//Volume Descriptor Goto Pointer (ECMA 167 3/10.3)
struct tag_anchor_vol_desc_ptr{
    //字段的标签
    descriptor_tag      descTag;
    //主卷描述信息序列的地址信息
    tag_extent_ad       mainVolDescSequExt;
    //备用描述信息的地址信息
    tag_extent_ad       ResVolDescSequExt;
    //保留字段
    uint8_t             reserved[2016];
};


//File Set Descriptor (ECMA 167 4/14.1)
//FSD用于描述一个文件集（File Set）的元数据。文件集是UDF文件系统中的一个重要组成部分，代表了UDF磁盘上的一个文件存储区域
struct tag_file_set_desc{
    //描述符的标签字段
    descriptor_tag descTag;
    //记录文件集创建或修改的时间戳
    time_stamp      recordingDateAndTime;
    //文件集的交换级别
    uint16_t         interchangeLvl;
    //指示文件集所能支持的最大交换级别
    uint16_t         maxInterchangeLvl;
    //表示文件集支持的字符集的标识符
    uint32_t         charSetList;
    //该字段表明文件集能够支持的最大字符集数目
    uint32_t         maxCharSetList;
    //表示文件集的编号
    //每个文件集都有一个唯一的编号，用于标识该文件集。这个编号通常在多卷的文件系统中用于区分不同的文件集。
    uint32_t         fileSetNum;
    //表示文件集描述符的编号
    uint32_t         fileSetDescNum;
    //逻辑卷标识符使用的字符集 此字段指明用于表示逻辑卷标识符的字符集。它帮助UDF文件系统识别卷标的编码方式
    tag_char_sepc    logicalVolIdentCharSet; 
    //存储逻辑卷标识符。
    uint8_t          logicalVolIdent[128];
    //表示文件集标识符使用的字符集
    tag_char_sepc    fileSetCharSet;
    //该字段包含文件集的标识符，通常是一个32字节的字符串，用于唯一标识该文件集。
    uint8_t          fileSetIdent[32];
    //保留字段，未使用。
    uint8_t          p_reserved1[64];
    //表示文件集的根目录的指针
    tag_long_ad      rootDirectoryICB;
    //域标识符通常用于表示文件集所属的域（例如，文件系统所属的域）
    tag_domain_id    domainIdent;
    //如果文件集描述符有多个扩展，这个字段会指向下一个文件集描述符的控制块，以形成一个链
    tag_long_ad      nextExt; 
    //保留字段
    uint8_t          p_reserved2[1584];
};

// ICB Tag (ECMA 167 4/14.6)
//tag_icb_tag 结构体用于描述UDF 1.02文件系统中文件或目录条目的信息
//20Byes
struct tag_icb_tag{
    //表示之前记录的直接条目数。此字段用于跟踪在UDF文件系统中当前文件条目之前的直接条目的数量。
    //通常用来优化访问或查找文件时，指示存储介质中的直接条目数量，尤其是在涉及到多层目录或多个文件条目的情况下
    uint32_t  priorRecordedNumDirectEntries;
    //表示ICB条目的策略类型。它用于标识该条目所使用的具体策略。
    uint16_t  strategyType;
    //表示策略参数，通常与strategyType字段一起使用
    uint16_t  strategyParameter;
    //表示条目中包含的实际条目数
    //指出该文件或目录的ICB中实际包含的条目数。这是文件系统中用来确定目录或文件信息的字段之一
    uint16_t  numEntries;
    //保留字段，未使用
    uint8_t   reserved;
    //表示文件类型
    uint8_t   fileType;
    //保留字段，用于未来扩展
    uint8_t   p_reserved[6];
    //表示ICB条目的标志位
    //用于表示当前条目的特性或属性，例如文件的读写权限、是否是隐藏文件等
    uint16_t  flags;
};

//File Entry (ECMA 167 4/14.17)
//在UDF文件系统中用于存储文件或目录的元数据信息
struct tag_file_entry {
    //是一个用于标识描述符的通用结构，用于标识和区分不同类型的描述符
    descriptor_tag	    descTag;
    //ICB(Indirect Control Block)标签，包含指向文件数据或目录的指针。
    tag_icb_tag         icbTag;
    //用户标识符，表示该文件或目录的拥有者的UID,关联到操作系统的用户权限模型，用于控制文件的访问权限
    uint32_t			uid;
    //组标识符，表示该文件或目录所属的组的GID, 用于标识该文件或目录所属的用户组，从而用于访问控制和权限设置
    uint32_t			gid;
    //文件或目录的权限位
    uint32_t			permissions;
    //文件链接计数,表示该文件或目录有多少个硬链接指向它
    uint16_t			fileLinkCount;
    //保留以备未来扩展，当前版本中未使用
    uint8_t             p_reserved_1[6];
    //文件内容的大小(字节数),表示文件的数据长度
    uint64_t			informationLength;
    //表示文件或目录实际占用的磁盘空间(以逻辑块为单位)
    uint64_t			logicalBlocksRecorded;
    //文件的访问时间戳,表示文件最后一次访问的时间，用于文件系统的审计和日志
    time_stamp           accessTime;
    //表示文件内容最后一次修改的时间
    time_stamp           modificationTime;
    //文件属性修改时间戳,记录文件的元数据（如权限、所有者等）最后一次修改的时间
    time_stamp           attrmodifyTime;
    //检查点 用于同步或恢复目的，标记文件系统状态或数据一致性的检查点
    uint32_t			checkpoint;
    //预留字段
    uint8_t             p_reserved_2[16];
    //实现标识符,用于标识文件系统的实现或版本
    tag_imp_id			impIdent;
    //用于唯一标识文件或目录，通常为文件系统中的每个文件或目录分配一个全局唯一的ID
    uint64_t			uniqueID;
    //扩展属性的长度,表示与该文件相关的扩展属性的字节数
    uint32_t	        LengthofExtendedAttributes;
    //分配描述符的长度, 表示文件或目录分配描述符的字节数，描述符用于管理磁盘空间的分配
    uint32_t	        LengthofAllocationDescriptors;
    //分配描述符,用于描述文件数据如何在磁盘上分配存储块。它是一个指向实际存储位置的结构，帮助文件系统管理磁盘上的数据分配
    tag_short_ad        Allocationdescriptors[10];
    //加密区域, 用于存储加密相关的信息，可能用于存储文件的加密密钥或加密属性
    uint8_t             EncryptArea[128];
    //预留字段，长度为2048字节，减去上面各字段的大小, 保留空间，以备未来扩展
    uint8_t             p_reserved[2048-176-8-128 - 72];
};

//用于描述文件或目录在文件系统中的控制信息
//帮助文件系统管理和定位文件内容，并提供完整性、元数据以及文件的访问控制
struct tag_icb_param{
  //储了该文件或目录的ICB(Integrity Control Block)位置
  uint32_t          tagLocation;
  uint8_t           fileType;
  //该字段表示文件的 链接计数，即有多少个目录项指向该文件
  //这对于处理文件删除（如减少引用计数）非常重要
  uint16_t          fileLinkCount;
  //文件或目录的数据长度，单位通常是字节
  uint64_t          informationLength;
  //文件所占用的 逻辑块数量，即文件内容跨越的逻辑块数
  uint32_t          logicalBlocksRecorded;
  //该字段表示文件或目录的 唯一标识符，通常是一个全局唯一的ID
  uint64_t          uniqueID;
  //分配描述符帮助文件系统决定文件数据如何被存储在物理磁盘上
  tag_short_ad      Allocationdescriptors;
};

// File Identifier Descriptor (ECMA 167 4/14.4)
struct tag_file_indent_desc{
    //标签
    descriptor_tag   descTag;
    //UDF中文件的版本号
    uint16_t         fileVersionNum;
    //文件的属性
    uint8_t          fileCharacteristics;
    //文件标识符的长度 文件夹没有
    uint8_t          lengthFileIdent;
    //信息控制块的地址
    //offset 20 impuse 30~35
    tag_long_ad      icb;
    //offset 36
    uint16_t         lengthOfImpUse;
    //文件标识符字符序列
    uint8_t *        p_fileIdent;
    //填充字段
    uint8_t *        p_padding;
};

struct tag_file_id_info{
    //标签的位置 索引
    uint32_t        tagLocation;
    uint16_t	    descCRCLength;
    uint8_t         lengthFileIdent;
    tag_long_ad     icb;
    uint8_t         fileIdent[256];
    uint8_t         PadLength;
    //CHAR_FLAG_HIDDEN  = 0x01,
    //CHAR_FLAG_DIR     = 0x02,
    //CHAR_FLAG_DELETED = 0x04,
    //CHAR_FLAG_PARENT  = 0x08,
    //文件的属性和分类
    uint8_t         fileCharacteristics;
};



static uint16_t crc_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

// p_a_vol_set_iden
static char p_vol_set_iden[70]={
    'E','-','w','o','r','l','d',0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0
};

static char p_desc_char_set_info[63]={
    'O','S','T','A',' ','C','o','m','p','r',
    'e','s','s','e','d',' ','U','n','i','c',
    'o','d','e',0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,
};

//*Daikin U.S. Comtec Lab
static char p_impIdent_ident[23]={
   '*' ,'D' ,'a' ,'i' ,'k' ,'i' ,'n' , ' ' ,'U' , '.'  ,'S' ,'.',
   ' ' ,'C' ,'o' ,'m' ,'t' ,'e' ,'c' , ' ' ,'L' , 'a'  ,'b'

};

static char p_win_impIdent_ident[23]={
   'M' ,'i' ,'c' ,'r' ,'o' ,'s' ,'o' , 'f' ,'t' , ' '  ,'I' ,'M',
   'A' ,'P' ,'I' ,'2' ,' ' ,'1' ,'.' , '0' ,0x00 , 0x00  ,0x00

};

static char p_udf_impident[23]={
    '*','U','D','F',' ','L','V',' ','I','n',
    'f','o',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00
};

static char p_partition_ident[23]={
    0x2b,0x4e,0x53,0x52,0x30,0x32,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00
};

//*OSTA UDF Compliant
static char p_domain_ident[23]={
    '*', 'O' , 'S' , 'T' , 'A' , ' ' , 'U' , 'D' , 'F', ' ' ,
    'C', 'o' , 'm' , 'p' , 'l' , 'i' , 'a' , 'n' , 't' ,0x00,
    0x00,0x00,0x00
};

static char p_file_set_ident[24]={
    0,'C',0,'H',0,'I',0,'N',0,'A',
    0,'E',0,'-',0,'W',0,'O',0,'R',
    0,'L',0,'D'
};

static char UserFileName[10][256]={
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};

struct udf_disc
{
    tag_primary_vol_desc  pvd;
    tag_logical_vol_desc  lvd;
    tag_partition_desc   pd;
    tag_imp_use_vol_desc imp;
    tag_unallocated_space_desc usd;
    tag_file_set_desc fsd;
    tag_terminating_desc terdes;
};

#endif
