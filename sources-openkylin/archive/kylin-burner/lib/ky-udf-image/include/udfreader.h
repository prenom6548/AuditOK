#ifndef _UDF_READER_H_
#define _UDF_READER_H_

#include <string>
#include "cs0encode.h"
#include "ecma167.h"
#include "udfdataset.h"
#include "udfreader.h"
#include "blockinput.h"
#include "udfread.h"


/* Logical Partitions from Logical Volume Descriptor */
struct tag_udf_partitions {
    uint32_t num_partition;
    struct {
        uint16_t number;
        uint32_t lba;
        uint32_t mirror_lba;
    } p[2];
};

class UdfReader
{
public:
    UdfReader() = default;
    ~UdfReader() = default;

public:
    int parse_udf_image(const char* file_path);

private:
    //读取卷识别序列
    int read_probe_volume();
    //读取卷描述符定位指针
    int read_avdp();
    int search_vds();
    int read_fsd();
    int parse_file_tree();
    
private: 
    //iso file
    bool open_iso_file(const std::string &iso_path);
    bool close_iso_file();

    //parse block data   
    uint32_t  file_block_size();
    int32_t read_iso_block(uint32_t block_index, uint8_t *buffer, uint32_t block_length);
    int read_descriptor_block(uint32_t lba, uint8_t *buf);

    //parse info
    int read_metadata_blocks(const struct long_ad* loc, uint8_t *buf);
    int partition_index(uint16_t partition_number);
    enum tag_identifier decode_descriptor_tag(const uint8_t *buf);

    inline int check_domain_identifier(const struct entity_id *eid, const char *value){
        return (!memcmp(value, eid->identifier, strlen(value))) ? 0 : -1;
    }

    //parse meta data  
    int map_metadata_partition(uint32_t lba, uint32_t mirror_lba);
    uint8_t * read_metadata(const struct long_ad *icb, int *tag_id);

    struct file_entry *read_file_entry(const struct long_ad *icb);

private:
    tag_anchor_vol_desc_ptr          m_avdp;
    tag_primary_vol_desc             m_primary_vol_desc;
    tag_partition_desc	             m_partition_desc;
    tag_logical_vol_desc             m_logic_vol_desc;
    tag_imp_use_vol_desc             m_impl_use_vol_desc;
    tag_unallocated_space_desc       m_unallocated_space_desc;
    tag_terminating_desc		     m_terminating_desc;
    tag_file_set_desc		 	     m_file_set_desc;
    tag_udf_partitions               m_udf_part;


private:
    std::string m_iso_path; //iso文件路径
    std::string m_volume_id;//卷ID
    int m_iso_fd;           //iso文件描述符

};

#endif
