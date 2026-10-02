#include "udfreader.h"
#include <fcntl.h>
#include <pthread.h>
#include "ecma167.h"


 int UdfReader::parse_udf_image(const char* file_path)
 {
    //open iso
    if(!open_iso_file(std::string(file_path))){
        return -1;
    }

    //udf check
    if (read_probe_volume() < 0) {
        return -1;
    }

     /* Find Anchor Volume Descriptor */
    if (read_avdp() < 0) {
        return -1;
    }

    //including pvd lvd pd  unallocatedSpace TerminateDesc
    if (search_vds() < 0) {
        return -1;
    }

    //find fsd
    if(read_fsd() < 0){
        return -1;
    }

    parse_file_tree();
    close_iso_file();

    return 0;
 }


int UdfReader::read_probe_volume(){
    //ECMA 167 Volume, BEA01
    static const char bea[]    = {'\0',  'B',  'E',  'A',  '0',  '1', '\1'};
    //ECMA 167 Volume, NSR02
    static const char nsr_02[] = {'\0',  'N',  'S',  'R',  '0',  '2', '\1'};
    //ECMA 167 Volume, NSR03
    static const char nsr_03[] = {'\0',  'N',  'S',  'R',  '0',  '3', '\1'};
    //ECMA 167 Volume, TEA01
    static const char tea[]    = {'\0',  'T',  'E',  'A',  '0',  '1', '\1'};
    //ECMA 167 Volume, NULL
    static const char nul[]    = {'\0', '\0', '\0', '\0', '\0', '\0', '\0'};

    uint8_t  buf[UDF_BLOCK_SIZE];
    uint32_t lba;
    int      bea_seen = 0;
    //从第16块开始读取
    for (lba = 16; lba < 256; lba++) {  
        //读取1块数据
        if (read_iso_block(lba, buf, 1) == 1) {
            //判断是否为ECMA 167 Volume, TEA01
            if (!memcmp(buf, tea, sizeof(tea))) {
                udf_error("ECMA 167 Volume Recognition failed (no NSR descriptor)\n");
                return -1;
            }
            //判断是否为ECMA 167 Volume, NULL
            if (!memcmp(buf, nul, sizeof(nul))) {
                break;
            }
            //判断是否为ECMA 167 Volume, BEA01
            if (!memcmp(buf, bea, sizeof(bea))) {
                udf_trace("ECMA 167 Volume, BEA01\n");
                bea_seen = 1;
            }
    
            if (bea_seen) {
                if (!memcmp(buf, nsr_02, sizeof(nsr_02))) {
                    udf_trace("ECMA 167 Volume, NSR02\n");
                    return 0;
                }
                if (!memcmp(buf, nsr_03, sizeof(nsr_03))) {
                    udf_trace("ECMA 167 Volume, NSR03\n");
                    return 0;
                }
            }
        }
    }

    udf_error("ECMA 167 Volume Recognition failed\n");
    return -1;
}

bool UdfReader::open_iso_file(const std::string &iso_path)
{
    m_iso_fd = open(iso_path.c_str(), O_RDONLY);
    if (m_iso_fd == -1)
    {
        return false;
    }
    return true;
}

bool UdfReader::close_iso_file()
{
    close(m_iso_fd);
    return true;
}

uint32_t UdfReader::file_block_size(){
    int64_t pos = lseek(m_iso_fd, 0, SEEK_END);
    if (pos < 0) {
        return 0;
    }
    return (uint32_t)(pos / UDF_BLOCK_SIZE);
}

int32_t UdfReader::read_iso_block(uint32_t block_index, uint8_t *buffer, uint32_t block_length){

    uint64_t bytes, got;
    uint64_t  pos;
    //计算需要读取的字节数
    bytes = (uint64_t)block_length * UDF_BLOCK_SIZE;
    got   = 0;
    //计算需要读取的块的偏移量
    pos   = (uint64_t)block_index * UDF_BLOCK_SIZE;
    //读取数据
    while (got < bytes) {
        ssize_t ret = pread(m_iso_fd, ((char*)buffer) + got, bytes - got, pos + (off_t)got);

        if (ret <= 0) {
            if (ret < 0 && errno == EINTR) {
                continue;
            }
            if (got < UDF_BLOCK_SIZE) {
                return ret;
            }
            break;
        }
        got += (size_t)ret;
    }

    return got / UDF_BLOCK_SIZE;
}

int UdfReader::read_avdp(){

    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba = 256;

    /*
     * Find Anchor Volume Descriptor Pointer.
     * It is in block 256, last block or (last block - 256)
     * (UDF 2.60, 2.2.3)
     */
    /* try block 256 */
    tag_id = read_descriptor_block(lba, buf);
    if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
        /* try last block */
        if (!file_block_size()) {
            udf_error("Can't find Anchor Volume Descriptor Pointer\n");
            return -1;
        }

        lba = file_block_size() - 1;
        tag_id = read_descriptor_block(lba, buf);
        if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
            /* try last block - 256 */
            lba -= 256;
            tag_id = read_descriptor_block(lba, buf);
            if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
                udf_error("Can't find Anchor Volume Descriptor Pointer\n");
                return -1;
            }
        }
    }
    udf_log("Found Anchor Volume Descriptor Pointer from lba %u\n", lba);

    memcpy(&m_avdp, buf, UDF_BLOCK_SIZE);
    return 1;
}

int UdfReader::search_vds()
{
    struct volume_descriptor_pointer vdp;
    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba;
    uint32_t end_lba;
    int      have_part = 0, have_lvd = 0, have_pvd = 0;

next_extent:
    uint32_t start_lba = m_avdp.mainVolDescSequExt.extLocation;
    end_lba =  m_avdp.mainVolDescSequExt.extLocation + m_avdp.mainVolDescSequExt.extLength / UDF_BLOCK_SIZE;

    /* parse Volume Descriptor Sequence */
    for (lba = start_lba; lba < end_lba; lba++) {
        tag_id = read_descriptor_block(lba, buf);
        switch (tag_id) {

        case ECMA_VolumeDescriptorPointer:
            decode_vdp(buf, &vdp);
            //loc = &vdp.next_extent;
            goto next_extent;

        case ECMA_PrimaryVolumeDescriptor:
            udf_log("Primary Volume Descriptor in lba %u\n", lba);
            memcpy(&m_primary_vol_desc, buf, UDF_BLOCK_SIZE);
            have_pvd = 1;
            break;

        case ECMA_LogicalVolumeDescriptor:
            udf_log("Logical volume descriptor in lba %u\n", lba);
            memcpy(&m_logic_vol_desc, buf, UDF_BLOCK_SIZE);
            have_lvd = 1;
            break;

        case ECMA_PartitionDescriptor:
          udf_log("Partition Descriptor in lba %u\n", lba);
          if (!have_part) {
              memcpy(&m_partition_desc, buf, UDF_BLOCK_SIZE);
              have_part = (0 == m_partition_desc.partitionNumber);
              udf_log("  partition %u at lba %u, %u blocks\n", m_partition_desc.partitionNumber, m_partition_desc.partitionStartingLocation,
                      m_partition_desc.partitionLength);
          }
          break;
        case ECMA_UnAllocatedSpaceDesc:
            memcpy(&m_unallocated_space_desc, buf, UDF_BLOCK_SIZE);
            break;
        case ECMA_TerminatingDescriptor:
            udf_trace("Terminating Descriptor in lba %u\n", lba);
            memcpy(&m_terminating_desc, buf, UDF_BLOCK_SIZE);
            return (have_part && have_lvd) ? 0 : -1;

        case ECMA_ImpUseVolumeDescriptor:
            memcpy(&m_impl_use_vol_desc, buf, UDF_BLOCK_SIZE);
            break;
        }
        if (have_part && have_lvd && have_pvd) {
            /* got everything interesting, skip rest blocks */
            return 0;
        }
    }
    return (have_part && have_lvd) ? 0 : -1;
}

int UdfReader::read_fsd()
{
    uint8_t             buf[UDF_BLOCK_SIZE];
    int                 tag_id = -1;
    
    int32_t start_blk = m_logic_vol_desc.fileSetDescSequExt.extLocation.logicalBlockNum;
    int32_t fsd_length = m_logic_vol_desc.fileSetDescSequExt.extLength;

    /* search for File Set Descriptor from the area described by fsd_loc */
    for (start_blk; start_blk <= start_blk + (fsd_length - 1) / UDF_BLOCK_SIZE; start_blk++) {
        
        //tag_id = read_metadata_blocks(buf);
        if (tag_id == ECMA_FileSetDescriptor) {
            break;
        }
        if (tag_id == ECMA_TerminatingDescriptor) {
            break;
        }
        udf_error("unhandled tag %d in File Set Descriptor area\n", tag_id);
    }

    if (tag_id != ECMA_FileSetDescriptor) {
        udf_error("didn't find File Set Descriptor\n");
        return -1;
    }

    memcpy(&m_file_set_desc, buf, UDF_BLOCK_SIZE);
  
    udf_log("root directory in part %u lba %u\n", m_file_set_desc.rootDirectoryICB.extLocation.partitionReferenceNum, 
    m_file_set_desc.rootDirectoryICB.extLocation.logicalBlockNum);

    return 0;
}

int UdfReader::parse_file_tree()
{
   return -1;
}


uint8_t * UdfReader::read_metadata(const struct long_ad *icb, int *tag_id)
{
    uint32_t  num_blocks = (icb->length + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
    
    uint8_t  *buf;

    if (num_blocks < 1) {
        return NULL;
    }

    buf = (uint8_t *)malloc(num_blocks * UDF_BLOCK_SIZE);
    if (!buf) {
        udf_error("out of memory\n");
        return NULL;
    }

    *tag_id = read_metadata_blocks(icb, buf);
    if (*tag_id < 0) {
        udf_log("reading icb blocks failed\n");
        free(buf);
        return NULL;
    }

    return buf;
}



int UdfReader::map_metadata_partition(uint32_t lba, uint32_t mirror_lba)
{
    struct file_entry *fe;
    uint8_t       buf[UDF_BLOCK_SIZE];
    int           tag_id;
    unsigned int  i;

    /* resolve metadata partition location (it is virtual partition inside another partition) */
    udf_trace("Reading metadata file entry: lba %u, mirror lba %u\n", lba, mirror_lba);

    for (i = 0; i < 2; i++) {

        if (i == 0) {
            tag_id = read_descriptor_block(m_partition_desc.partitionStartingLocation + lba, buf);
        } else {
            tag_id = read_descriptor_block(m_partition_desc.partitionStartingLocation + mirror_lba, buf);
        }

        if (tag_id != ECMA_ExtendedFileEntry) {
            udf_error("read metadata file %u: unexpected tag %d\n", i, tag_id);
            continue;
        }

        fe = decode_ext_file_entry(buf, UDF_BLOCK_SIZE,m_partition_desc.partitionNumber);
        
        if (!fe) {
            udf_error("parsing metadata file entry %u failed\n", i);
            continue;
        }

        if (fe->content_inline) {
            udf_error("invalid metadata file (content inline)\n");
        } else if (!fe->u.ads.num_ad) {
            udf_error("invalid metadata file (no allocation descriptors)\n");
        } else if (fe->file_type == UDF_FT_METADATA) {
            m_udf_part.p[1].lba = m_partition_desc.partitionStartingLocation + fe->u.ads.ad[0].lba;
            
            udf_log("metadata file at lba %u\n", m_udf_part.p[1].lba);
        } else if (fe->file_type == UDF_FT_METADATA_MIRROR) {
            m_udf_part.p[1].mirror_lba = m_partition_desc.partitionStartingLocation + fe->u.ads.ad[0].lba;
            udf_log("metadata mirror file at lba %u\n", m_udf_part.p[1].mirror_lba);
        } else {
            udf_error("unknown metadata file type %u\n", fe->file_type);
        }
        free_file_entry(&fe);
    }

    if (!m_udf_part.p[1].lba && m_udf_part.p[1].mirror_lba) {
        /* failed reading primary location, must use mirror */
        m_udf_part.p[1].lba        = m_udf_part.p[1].mirror_lba;
        m_udf_part.p[1].mirror_lba = 0;
    }

    return m_udf_part.p[1].lba ? 0 : -1;
}

int UdfReader::read_metadata_blocks(const struct long_ad* loc, uint8_t *buf)
{
    int      tag_id;
    uint32_t lba, i, got;
    int      part_idx;

    int partition_idx = loc->partition;
    int logic_block_index = loc->lba;
    int block_length = loc->length;

    udf_trace("reading metadata from part %u lba %u\n", partition_idx, logic_block_index);

    part_idx = partition_index(m_logic_vol_desc.p_partitionMaps.partitionNo);
    
    if (part_idx < 0) {
        return -1;
    }

    /* read first block. Parse and check tag. */
    lba    = m_logic_vol_desc.p_partitionMaps.volSequNo + logic_block_index;
    tag_id = read_descriptor_block(lba, buf);

    if (tag_id < 0) {
        /* try mirror */
        if (m_udf_part.p[part_idx].mirror_lba) {
            udf_log("read metadata from lba %u failed, trying mirror\n", lba);
            lba    = m_udf_part.p[part_idx].mirror_lba + loc->lba;
            tag_id = read_descriptor_block(lba, buf);
        }

        if (tag_id < 0) {
            udf_error("read metadata from lba %u failed\n", lba);
            return -1;
        }
    }

    /* read following blocks without tag parsing and checksum validation */
    for (i = 1; i <= (block_length - 1) / UDF_BLOCK_SIZE; i++) {

        lba  = m_udf_part.p[part_idx].lba + logic_block_index + i;
        buf += UDF_BLOCK_SIZE;

        got = read_iso_block(lba, buf, 1);
        
        if (got != 1) {
            if (m_udf_part.p[part_idx].mirror_lba) {
                udf_log("read metadata from lba %u failed, trying mirror\n", lba);
                lba = m_udf_part.p[part_idx].mirror_lba + loc->lba + i;
                got = read_iso_block(lba, buf, 1);
            }
            if (got != 1) {
                udf_error("read metadata from lba %u failed\n", lba);
                return -1;
            }
        }
    }
    return tag_id;
}

int UdfReader::partition_index(uint16_t partition_number)
{
    return 1;
}

/* Descriptor Tag (ECMA 167, 3/7.2) */
enum tag_identifier UdfReader::decode_descriptor_tag(const uint8_t *buf)
{
  uint16_t id;
  uint8_t  checksum = 0;
  int      i;

  id = _get_u16(buf + 0);

  /* descriptor tag is 16 bytes */

  /* calculate tag checksum */
  for (i = 0; i < 4; i++) {
      checksum = (uint8_t)(checksum + buf[i]);
  }
  for (i = 5; i < 16; i++) {
      checksum = (uint8_t)(checksum + buf[i]);
  }

  if (checksum != buf[4]) {
      return ECMA_TAG_INVALID;
  }

  return (enum tag_identifier)id;
}

int UdfReader::read_descriptor_block(uint32_t lba, uint8_t *buf){
    if (read_iso_block(lba, buf, 1) == 1) {
        return decode_descriptor_tag(buf);
    }
    return -1;
}

