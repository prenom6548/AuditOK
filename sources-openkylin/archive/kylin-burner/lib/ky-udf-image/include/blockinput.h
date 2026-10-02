#ifndef UDFREAD_DEFAULT_BLOCKINPUT_H_
#define UDFREAD_DEFAULT_BLOCKINPUT_H_

#include "stdint.h"
#include "udfdataset.h"

#ifndef UDF_BLOCK_SIZE
#  define UDF_BLOCK_SIZE  2048
#endif

typedef struct udfread_block_input udfread_block_input;

struct udfread_block_input {

    int      (*close) (udfread_block_input *);

    int      (*read)  (udfread_block_input *, uint32_t lba, void *buf, uint32_t nblocks, int flags);

    uint32_t (*size)  (udfread_block_input *);
    udf_disc udfdisc;
    udf_disc backup_udf_disc;
    tag_anchor_vol_desc_ptr avdp;
    tag_logic_vol_integrity_desc lvid;
};


udfread_block_input *block_input_new(const char *path);

#endif
