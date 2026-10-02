#include "udfread.h"

#include "blockinput.h"
#include "ecma167.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "cs0encode.h"
#include "udfappender.h"


static int atomic_pointer_compare_and_exchange(void *atomic, void *oldval, void *newval)
{
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    int result;
    pthread_mutex_lock(&lock);
    result = *(void**)atomic == oldval;
    if (result) {
        *(void**)atomic = newval;
    }
    pthread_mutex_unlock(&lock);
    return result;
}





static char *_str_dup(const char *s)
{
    size_t len = strlen(s);
    char *p = (char *)malloc(len + 1);
    if (p) {
        memcpy(p, s, len + 1);
    } else {
        udf_error("out of memory\n");
    }
    return p;
}


/* Domain Identifiers, UDF 2.1.5.2 */

static const char lvd_domain_id[]  = "*OSTA UDF Compliant";
static const char meta_domain_id[] = "*UDF Metadata Partition";

static int _check_domain_identifier(const struct entity_id *eid, const char *value)
{
    return (!memcmp(value, eid->identifier, strlen(value))) ? 0 : -1;
}



static uint32_t _read_blocks(udfread_block_input *input,
                             uint32_t lba, void *buf, uint32_t nblocks,
                             int flags)
{
    int result;

    if (!input || (int)nblocks < 1) {
        return 0;
    }

    result = input->read(input, lba, buf, nblocks, flags);

    return result < 0 ? 0 : (uint32_t)result;
}

static int _read_descriptor_block(udfread_block_input *input, uint32_t lba, uint8_t *buf)
{
    if (_read_blocks(input, lba, buf, 1, 0) == 1) {
        return decode_descriptor_tag(buf);
    }
    return -1;
}

static int save_iso_block_to_file(udfread_block_input *input, int startIndex, int endIndex, std::string filepath){

    uint8_t buf[UDF_BLOCK_SIZE];
    int file_id = open(filepath.c_str(), O_CREAT | O_RDWR, 0777);
    for(int index=startIndex; index<=endIndex; ++index){
        int tag_id = _read_descriptor_block(input,index, buf);
        write(file_id,buf,UDF_BLOCK_SIZE);
    }
    close(file_id);
    return 0;
}

/*
 * Disc probing
 */

static int _probe_volume(udfread_block_input *input)
{
    /* Volume Recognition (ECMA 167 2/8, UDF 2.60 2.1.7) */

    static const char bea[]    = {'\0',  'B',  'E',  'A',  '0',  '1', '\1'};
    static const char nsr_02[] = {'\0',  'N',  'S',  'R',  '0',  '2', '\1'};
    static const char nsr_03[] = {'\0',  'N',  'S',  'R',  '0',  '3', '\1'};
    static const char tea[]    = {'\0',  'T',  'E',  'A',  '0',  '1', '\1'};
    static const char nul[]    = {'\0', '\0', '\0', '\0', '\0', '\0', '\0'};

    uint8_t  buf[UDF_BLOCK_SIZE];
    uint32_t lba;
    int      bea_seen = 0;

    for (lba = 16; lba < 256; lba++) {
        if (_read_blocks(input, lba, buf, 1, 0) == 1) {

            /* Terminating Extended Area Descriptor */
            if (!memcmp(buf, tea, sizeof(tea))) {
                udf_error("ECMA 167 Volume Recognition failed (no NSR descriptor)\n");
                return -1;
            }
            if (!memcmp(buf, nul, sizeof(nul))) {
                break;
            }
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

static int _read_avdp(udfread_block_input *input)
{
    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba = 256;

    /*
     * Find Anchor Volume Descriptor Pointer.
     * It is in block 256, last block or (last block - 256)
     * (UDF 2.60, 2.2.3)
     */

    /* try block 256 */
    tag_id = _read_descriptor_block(input, lba, buf);
    if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
        /* try last block */
        if (!input->size) {
            udf_error("Can't find Anchor Volume Descriptor Pointer\n");
            return -1;
        }

        lba = input->size(input) - 1;
        tag_id = _read_descriptor_block(input, lba, buf);
        if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {

            /* try last block - 256 */
            lba -= 256;
            tag_id = _read_descriptor_block(input, lba, buf);
            if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
                udf_error("Can't find Anchor Volume Descriptor Pointer\n");
                return -1;
            }
        }
    }

    udf_log("Found Anchor Volume Descriptor Pointer from lba %u\n", lba);
    memcpy(&input->avdp, buf, UDF_BLOCK_SIZE);
    return 1;
}

static int _read_last_avdp(udfread_block_input* input)
{
    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba = -1;


    if (!input->size) {
        udf_error("Can't find Anchor Volume Descriptor Pointer\n");
        return -1;
    }
#if 0
    lba = input->size(input) - 1;
    tag_id = _read_descriptor_block(input, lba, buf);
    if (tag_id != ECMA_AnchorVolumeDescriptorPointer)
    {
        return -1;
    }
#endif


    lba = input->size(input) - 1;
    tag_id = _read_descriptor_block(input, lba, buf);
    if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {

        /* try last block - 256 */
        lba -= 256;
        tag_id = _read_descriptor_block(input, lba, buf);
        if (tag_id != ECMA_AnchorVolumeDescriptorPointer) {
            udf_error("Can't find Anchor Volume Descriptor Pointer\n");
            return -1;
        }
    }

    udf_log("Found Anchor Volume Descriptor Pointer from lba %u\n", lba);
    memcpy(&input->avdp, buf, UDF_BLOCK_SIZE);
    return 1;
}



struct volume_descriptor_set {
    struct partition_descriptor      pd;
    struct primary_volume_descriptor pvd;
    struct logical_volume_descriptor lvd;
};

static int _search_vds(udfread_block_input *input, int part_number, tag_extent_ad loc, udf_disc* udfdis)

{
    struct volume_descriptor_pointer vdp;
    uint8_t  buf[UDF_BLOCK_SIZE];
    int      tag_id;
    uint32_t lba;
    uint32_t end_lba;
    int      have_part = 0, have_lvd = 0, have_pvd = 0, have_usd = 0;

next_extent:
    udf_trace("reading Volume Descriptor Sequence at lba %u, len %u bytes\n", loc.extLocation, loc.extLength);

    end_lba = loc.extLocation + loc.extLength / UDF_BLOCK_SIZE;

    /* parse Volume Descriptor Sequence */
    for (lba =  loc.extLocation; lba < end_lba; lba++) {

        tag_id = _read_descriptor_block(input, lba, buf);

        switch (tag_id) {

        case ECMA_VolumeDescriptorPointer:
            decode_vdp(buf, &vdp);
            loc.extLength = vdp.next_extent.length;
            loc.extLocation = vdp.next_extent.lba;
            goto next_extent;

        case ECMA_PrimaryVolumeDescriptor:
            udf_log("Primary Volume Descriptor in lba %u\n", lba);
            memcpy(&udfdis->pvd, buf, UDF_BLOCK_SIZE);
            have_pvd = 1;
            break;

        case ECMA_LogicalVolumeDescriptor:
            udf_log("Logical volume descriptor in lba %u\n", lba);
            memcpy(&udfdis->lvd, buf, UDF_BLOCK_SIZE);
            have_lvd = 1;
            break;

        case ECMA_ImpUseVolumeDescriptor:
            memcpy(&udfdis->imp, buf, UDF_BLOCK_SIZE);
            break;

        case ECMA_UnAllocatedSpaceDesc:
            memcpy(&udfdis->usd, buf, UDF_BLOCK_SIZE);
            have_usd = 1;
            break;

        case ECMA_PartitionDescriptor:
          udf_log("Partition Descriptor in lba %u\n", lba);
          if (!have_part) {
              memcpy(&udfdis->pd, buf, UDF_BLOCK_SIZE);
              have_part = (part_number < 0 || part_number == input->udfdisc.pd.partitionNumber);
          }
          break;

        case ECMA_TerminatingDescriptor:
            udf_trace("Terminating Descriptor in lba %u\n", lba);
            memcpy(&udfdis->terdes, buf, UDF_BLOCK_SIZE);
            return (have_part && have_lvd) ? 0 : -1;
        }
    }
    return (have_part && have_lvd) ? 0 : -1;
}

static int _read_vds(udfread_block_input *input, int part_number)
{
    if (_read_last_avdp(input) < 0) {
        return -1;
    }

    if (_search_vds(input, part_number,input->avdp.mainVolDescSequExt, &input->udfdisc)) {
        return -1;
    }

    if (_search_vds(input, part_number,input->avdp.ResVolDescSequExt, &input->backup_udf_disc)) {
        return -1;
    }
    return 0;
}



static int _map_metadata_partition(udfread_block_input *input,
                                   struct udf_partitions *part,
                                   uint32_t lba, uint32_t mirror_lba,
                                   const struct partition_descriptor *pd)
{
    struct file_entry *fe;
    uint8_t       buf[UDF_BLOCK_SIZE];
    int           tag_id;
    unsigned int  i;

    /* resolve metadata partition location (it is virtual partition inside another partition) */
    udf_trace("Reading metadata file entry: lba %u, mirror lba %u\n", lba, mirror_lba);

    for (i = 0; i < 2; i++) {

        if (i == 0) {
            tag_id = _read_descriptor_block(input, pd->start_block + lba, buf);
        } else {
            tag_id = _read_descriptor_block(input, pd->start_block + mirror_lba, buf);
        }

        if (tag_id != ECMA_ExtendedFileEntry) {
            udf_error("read metadata file %u: unexpected tag %d\n", i, tag_id);
            continue;
        }

        fe = decode_ext_file_entry(buf, UDF_BLOCK_SIZE, pd->number);
        if (!fe) {
            udf_error("parsing metadata file entry %u failed\n", i);
            continue;
        }

        if (fe->content_inline) {
            udf_error("invalid metadata file (content inline)\n");
        } else if (!fe->u.ads.num_ad) {
            udf_error("invalid metadata file (no allocation descriptors)\n");
        } else if (fe->file_type == UDF_FT_METADATA) {
            part->p[1].lba = pd->start_block + fe->u.ads.ad[0].lba;
            udf_log("metadata file at lba %u\n", part->p[1].lba);
        } else if (fe->file_type == UDF_FT_METADATA_MIRROR) {
            part->p[1].mirror_lba = pd->start_block + fe->u.ads.ad[0].lba;
            udf_log("metadata mirror file at lba %u\n", part->p[1].mirror_lba);
        } else {
            udf_error("unknown metadata file type %u\n", fe->file_type);
        }

        free_file_entry(&fe);
    }

    if (!part->p[1].lba && part->p[1].mirror_lba) {
        /* failed reading primary location, must use mirror */
        part->p[1].lba        = part->p[1].mirror_lba;
        part->p[1].mirror_lba = 0;
    }

    return part->p[1].lba ? 0 : -1;
}

static int _parse_udf_partition_maps(udfread_block_input *input,
                                     struct udf_partitions *part,
                                     const struct volume_descriptor_set *vds)
{

    int num_type1_partition = 0;


    for (int i = 0; i < input->udfdisc.lvd.numPartitionMaps; i++) {

        /* Partition map, ECMA 167 3/10.7 */
        uint8_t  type = input->udfdisc.lvd.p_partitionMaps.partitionMapType;
        uint8_t  len  = input->udfdisc.lvd.p_partitionMaps.partitionMapLength;
        uint16_t ref;

        if (len < 2) {
            udf_error("invalid partition map length %d\n", (int)len);
            break;
        }

        udf_trace("map %u: type %u\n", i, type);

        if (type == 1) {

            /* ECMA 167 Type 1 partition map */
            if (len != 6) {
                udf_error("invalid type 1 partition map length %d\n", (int)len);
                break;
            }

            ref = input->udfdisc.lvd.p_partitionMaps.partitionNo;
            udf_log("partition map: %u: type 1 partition, ref %u\n", i, ref);

            if (num_type1_partition) {
                udf_error("more than one type1 partitions not supported\n");
            } else if (ref != vds->pd.number) {
                udf_error("Logical partition %u refers to another physical partition %u (expected %u)\n", i, ref, vds->pd.number);
            } else {
                part->num_partition   = 1;
                part->p[0].number     = i;
                part->p[0].lba        = vds->pd.start_block;
                part->p[0].mirror_lba = 0; /* no mirror for data partition */
                num_type1_partition++;
            }

        }
        //map += len;
    }

    return num_type1_partition ? 0 : -1;
}




static void _free_dir(struct udf_dir **pp)
{
    if (pp && *pp) {
        struct udf_dir *p = *pp;
        uint32_t i;

        if (p->subdirs) {
            for (i = 0; i < p->num_entries; i++) {
                _free_dir(&(p->subdirs[i]));
            }
            free(p->subdirs);
        }

        if (p->files) {
            for (i = 0; i < p->num_entries; i++) {
                if(p->files[i].filename_length > 0){
                    free(p->files[i].filename_utf8);
                }
            }
            free(p->files);
        }

        free(p);

        *pp = NULL;
    }
}


/*
 * Metadata
 */

static int _partition_index(udfread *udf, uint16_t partition_number)
{
    if (partition_number == udf->part.p[0].number) {
        return 0;
    } else if (udf->part.num_partition > 1 && partition_number == udf->part.p[1].number) {
        return 1;
    }

    udf_error("unknown partition %u\n", partition_number);
    return -1;
}

/* read metadata blocks. If read fails, try from mirror (if available). */
static int _read_metadata_blocks(udfread *udf, uint8_t *buf,
                                const struct tag_long_ad loc)
{
    int      tag_id;
    uint32_t lba, i, got;
    int      part_idx;


    part_idx = _partition_index(udf, 0);
    if (part_idx < 0) {
        return -1;
    }

    /* read first block. Parse and check tag. */

    lba  = udf->input->udfdisc.pd.partitionStartingLocation + loc.extLocation.logicalBlockNum;
    tag_id = _read_descriptor_block(udf->input, lba, buf);

    if (tag_id < 0) {
        return -1;
    }

    /* read following blocks without tag parsing and checksum validation */

    for (i = 1; i <= (loc.extLength - 1) / UDF_BLOCK_SIZE; i++) {

        lba  = udf->input->udfdisc.pd.partitionStartingLocation + loc.extLocation.logicalBlockNum + i;
        buf += UDF_BLOCK_SIZE;

        got = _read_blocks(udf->input, lba, buf, 1, 0);
        if (got != 1) {
            return -1;
        }
    }

    return tag_id;
}

static uint8_t *_read_metadata(udfread *udf, struct tag_long_ad icb, int *tag_id)
{
    uint32_t  num_blocks = (icb.extLength + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
    uint8_t  *buf;

    if (num_blocks < 1) {
        return NULL;
    }

    buf = (uint8_t *)malloc(num_blocks * UDF_BLOCK_SIZE);
    if (!buf) {
        udf_error("out of memory\n");
        return NULL;
    }

    *tag_id = _read_metadata_blocks(udf, buf, icb);
    if (*tag_id < 0) {
        udf_log("reading icb blocks failed\n");
        free(buf);
        return NULL;
    }

    return buf;
}

static struct tag_file_entry *_read_file_entry(udfread *udf,
                                           struct tag_long_ad icb)
{
    struct tag_file_entry *fe = new tag_file_entry;

    uint8_t  *buf;
    int       tag_id;


    buf = _read_metadata(udf, icb, &tag_id);

    if (!buf) {
        udf_error("reading file entry failed\n");
        return NULL;
    }

    switch (tag_id) {
        case ECMA_FileEntry:
            memcpy(fe, buf, UDF_BLOCK_SIZE);
            break;
        default:
            udf_error("_read_file_entry: unknown tag %d\n", tag_id);
            break;
    }

    free(buf);
    return fe;
}

static int _parse_dir(udfread *udf, const uint8_t *data, uint32_t length, struct udf_dir *dir)
{
    struct file_identifier fid;
    const uint8_t *p   = data;
    const uint8_t *end = data + length;
    int            tag_id;

    if (length < 16) {
        return 0;
    }

    while (p < end - 16) {
        size_t used;

        if (dir->num_entries == UINT32_MAX) {
            return 0;
        }

        tag_id = decode_descriptor_tag(p);
        if (tag_id != ECMA_FileIdentifierDescriptor) {
            udf_error("unexpected tag %d in directory file\n", tag_id);
            return -1;
        }

        dir->files = (struct udf_file_identifier *)_safe_realloc(dir->files, sizeof(dir->files[0]) * (dir->num_entries + 1));
        if (!dir->files) {
            return -1;
        }

        used = decode_file_identifier(p, (size_t)(end - p), &fid);
        if (used == 0) {
            /* not enough data. keep the entries we already have. */
            break;
        }

        p += used;

        if (fid.characteristic & CHAR_FLAG_PARENT) {
            continue;
        }
        if (fid.filename_len < 1) {
            continue;
        }

        dir->files[dir->num_entries].characteristic = fid.characteristic;
        dir->files[dir->num_entries].icb = fid.icb;
        dir->files[dir->num_entries].filename_utf8 = _cs0_to_mutf8(fid.filename, fid.filename_len);
        dir->files[dir->num_entries].filename_length = fid.filename_len;
        memcpy(&dir->files[dir->num_entries].file_indent_desc, (uint8_t*)(p - used), 38);
        dir->files[dir->num_entries].used_length = used;
        memcpy(dir->files[dir->num_entries].filename, fid.filename, fid.filename_len);

        int real_length = fid.filename_len + 38;
        dir->files[dir->num_entries].padLength = (4 * ((real_length + 3) / 4)) - real_length;


        if (!dir->files[dir->num_entries].filename) {
            continue;
        }

        /* Skip empty file identifiers.
         * Not strictly compilant (?), \0 is allowed in
         * ECMA167 file identifier.
         */
#if 0
        if (!dir->files[dir->num_entries].filename[0]) {
            udf_error("skipping empty file identifier\n");
            free(dir->files[dir->num_entries].filename);
            continue;
        }
#endif

        dir->num_entries++;
    }

    return 0;
}


std::vector<full_file_identifier> _parse_dir(udfread *udf, const uint8_t *data, uint32_t length)
{
    struct file_identifier fid;
    const uint8_t *p   = data;
    const uint8_t *end = data + length;
    int            tag_id;
    std::vector<full_file_identifier> result;


    if (length < 16) {
        return result;
    }

    while (p < end - 16) {
        size_t used;

        tag_id = decode_descriptor_tag(p);
        if (tag_id != ECMA_FileIdentifierDescriptor) {
            udf_error("unexpected tag %d in directory file\n", tag_id);
            return result;
        }

        used = decode_file_identifier(p, (size_t)(end - p), &fid);
        if (used == 0) {
            /* not enough data. keep the entries we already have. */
            break;
        }

        p += used;

        full_file_identifier index_file_id;
        strcpy(index_file_id.filename_utf8, _cs0_to_mutf8(fid.filename, fid.filename_len));
        index_file_id.filename_length = fid.filename_len;
        memcpy(index_file_id.filename, fid.filename, fid.filename_len);
        memcpy(&index_file_id.file_indent_desc, (uint8_t*)(p - used), 38);

        int real_length = fid.filename_len + 38;
        index_file_id.padd_length = (4 * ((real_length + 3) / 4)) - real_length;
        result.push_back(index_file_id);
    }

    return result;
}

static struct udf_dir *_read_dir_file(udfread *udf, const struct tag_long_ad loc)
{
    struct udf_dir *dir = NULL;
    uint8_t        *data;
    int             tag_id;

    udf_trace("directory size %u bytes\n", loc.extLength);

    data = _read_metadata(udf, loc, &tag_id);
    if (!data) {
        udf_error("reading directory file failed\n");
        return NULL;
    }

    dir = (struct udf_dir *)calloc(1, sizeof(struct udf_dir));
    if (dir) {
        if (_parse_dir(udf,data, loc.extLength, dir) < 0) {
            _free_dir(&dir);
        }
    }

    free(data);
    return dir;
}

static struct udf_dir *_read_dir(udfread *udf, const struct tag_long_ad icb)
{
    struct tag_file_entry *fe;
    struct udf_dir    *dir = NULL;

    fe = _read_file_entry(udf, icb);

    if (!fe) {
        udf_error("error reading directory file entry\n");
        return NULL;
    }

    if (fe->icbTag.fileType != ECMA_FT_DIR) {
        udf_error("directory file type is not directory\n");
        delete fe;
        return NULL;
    }

    if (fe->Allocationdescriptors[0].extLength == 0) {
        udf_error("empty directory file");
    } else {
        tag_long_ad alloc_long_ad;
        alloc_long_ad.extLength = fe->Allocationdescriptors[0].extLength;
        alloc_long_ad.extLocation.logicalBlockNum = fe->Allocationdescriptors[0].extPosition;
        dir = _read_dir_file(udf, alloc_long_ad);
        dir->file_entry = *fe;
    }

    delete fe;
    return dir;
}


static struct udf_dir *_read_subdir(udfread *udf, struct udf_dir *dir, uint32_t index)
{

    if (!(dir->files[index].characteristic & CHAR_FLAG_DIR)) {
        return NULL;
    }

    if (!dir->subdirs) {
        struct udf_dir **subdirs = (struct udf_dir **)calloc(sizeof(struct udf_dir *), dir->num_entries);
        if (!subdirs) {
            udf_error("out of memory\n");
            return NULL;
        }
        if (!atomic_pointer_compare_and_exchange(&dir->subdirs, NULL, subdirs)) {
            free(subdirs);
        }
    }

    if (!dir->subdirs[index]) {
        tag_long_ad icb_ad;
        icb_ad.extLocation.logicalBlockNum = dir->files[index].icb.lba;
        icb_ad.extLength = dir->files[index].icb.length;

        struct udf_dir *subdir = _read_dir(udf, icb_ad);
        if (!subdir) {
            return NULL;
        }
        if (!atomic_pointer_compare_and_exchange(&dir->subdirs[index], NULL, subdir)) {
            _free_dir(&subdir);
        }
    }

    return dir->subdirs[index];
}

static int _scan_dir(const struct udf_dir *dir, const char *filename, uint32_t *index)
{
    uint32_t i;

    for (i = 0; i < dir->num_entries; i++) {
        if (!strcmp(filename, dir->files[i].filename_utf8)) {
            *index = i;
            return 0;
        }
    }

    udf_log("file %s not found\n", filename);
    return -1;
}



static int _find_file(udfread *udf, const char *path,
                      struct udf_dir **p_dir,
                      const struct udf_file_identifier **p_fid)
{
    const struct udf_file_identifier *fid = NULL;
    struct udf_dir *current_dir;
    char *tmp_path, *save_ptr, *token;

    current_dir = udf->root_dir;
    if (!current_dir) {
        return -1;
    }

    tmp_path = _str_dup(path);
    if (!tmp_path) {
        return -1;
    }

    token = strtok_r(tmp_path, "/\\", &save_ptr);
    if (token == NULL) {
        udf_trace("_find_file: requested root dir\n");
    }

    while (token) {
        uint32_t index;
        if (_scan_dir(current_dir, token, &index) < 0) {
            udf_log("_find_file: entry %s not found\n", token);
            goto error;
        }
        fid = &current_dir->files[index];

        token = strtok_r(NULL, "/\\", &save_ptr);

        if (fid->characteristic & CHAR_FLAG_DIR) {
            current_dir = _read_subdir(udf, current_dir, index);
            if (!current_dir) {
                goto error;
            }
        } else if (token) {
            udf_log("_find_file: entry %s not found (parent is file, not directory)\n", token);
            goto error;
        } else {
            // found a file, make sure we won't return directory data
            current_dir = NULL;
        }
    }

    if (p_fid) {
        if (!fid) {
            udf_log("no file identifier found for %s\n", path);
            goto error;
        }
        *p_fid = fid;
    }
    if (p_dir) {
        *p_dir = current_dir;
    }

    free(tmp_path);
    return 0;

error:
    free(tmp_path);
    return -1;
}


/*
 * Volume access API
 */

int udfread_open_input(udfread *udf, udfread_block_input *input/*, int partition*/)
{
    struct volume_descriptor_set vds;

    if (!udf || !input || !input->read) {
        return -1;
    }

    if (_probe_volume(input) < 0) {
        return -1;
    }

    /* read Volume Descriptor Sequence */
    if (_read_vds(input, 0) < 0) {
        return -1;
    }

    udf->input = input;
    int lvid_location = input->udfdisc.lvd.integritySeqExt.extLocation;

    uint8_t  buf[UDF_BLOCK_SIZE];
    int tag_id = _read_descriptor_block(input, lvid_location, buf);
    if(tag_id == ECMA_LogicVolIntegrityDesc){
        memcpy(&input->lvid, buf, UDF_BLOCK_SIZE);
    }

    /* read fsd */
    tag_long_ad fsd_loc = udf->input->udfdisc.lvd.fileSetDescSequExt;
    int lba  = udf->input->udfdisc.pd.partitionStartingLocation + fsd_loc.extLocation.logicalBlockNum;
    tag_id = _read_descriptor_block(udf->input, lba, buf);
    if (tag_id == ECMA_FileSetDescriptor) {
        memcpy(&udf->input->udfdisc.fsd, buf, UDF_BLOCK_SIZE);
    }

    udf->root_dir = _read_dir(udf, udf->input->udfdisc.fsd.rootDirectoryICB);
    udf->volume_identifier = _cs0_to_mutf8(input->udfdisc.pvd.volIdent, input->udfdisc.pvd.volIdent[31]);
    if (udf->volume_identifier) {
        udf_log("Volume Identifier: %s\n", udf->volume_identifier);
    }
    memcpy(udf->volume_set_identifier, vds.pvd.volume_set_identifier, 128);

    return 0;


#if 0
    tag_partition_desc prev_pd, new_pd;
    tag_logical_vol_desc new_lvd;
    tag_id = _read_descriptor_block(input,35,buf);
    memcpy(&prev_pd, buf, UDF_BLOCK_SIZE);

    tag_id = _read_descriptor_block(input,19587, buf);
    memcpy(&new_pd, buf, UDF_BLOCK_SIZE);

    tag_id = _read_descriptor_block(input,19586, buf);
    memcpy(&new_lvd, buf, UDF_BLOCK_SIZE);


    for(int index=28311; index<=28576; ++index){
        tag_id = _read_descriptor_block(input,index, buf);

        if(tag_id != 0){
            printf("blockIndex:%d    tag_id:%d\n",index,tag_id);
        }

    }

    //std::string part1_iso = "/home/yff/kylin-project/udf-tools/output/part1.iso";
    std::string part2_iso = "/home/yff/kylin-project/udf-tools/output/win_part2.iso";
   // save_iso_block_to_file(input,0,17503, part1_iso);
    save_iso_block_to_file(input,93952,102975, part2_iso);
#endif


#if 0
    if (_parse_udf_partition_maps(input, &udf->part, &vds) < 0) {
        return -1;
    }
#endif

}

int udfread_open(udfread *udf, const char *path)
{
    udfread_block_input *input;
    int result;

    if (!path) {
        return -1;
    }

    input = block_input_new(path);
    if (!input) {
        return -1;
    }

    result = udfread_open_input(udf, input);

    if (result < 0) {
        if (input->close) {
            input->close(input);
        }
    }

    return result;
}

void udfread_close(udfread *udf)
{
    if (udf) {
        if (udf->input) {
            if (udf->input->close) {
                udf->input->close(udf->input);
            }
            udf->input = NULL;
        }

        _free_dir(&udf->root_dir);
        free(udf->volume_identifier);
        free(udf);
    }
}

const char *udfread_get_volume_id(udfread *udf)
{
    if (udf) {
        return udf->volume_identifier;
    }
    return NULL;
}

size_t udfread_get_volume_set_id (udfread *udf, void *buffer, size_t size)
{
    if (udf) {
        if (size > sizeof(udf->volume_set_identifier)) {
            size = sizeof(udf->volume_set_identifier);
        }
        memcpy(buffer, udf->volume_set_identifier, size);
        return sizeof(udf->volume_set_identifier);
    }
    return 0;
}



static UDFDIR *_new_udfdir(udfread *udf, struct udf_dir *dir)
{
    UDFDIR *result;

    if (!dir) {
        return NULL;
    }

    result = (UDFDIR *)calloc(1, sizeof(UDFDIR));
    if (result) {
        result->dir = dir;
        result->udf = udf;
    }

    return result;
}

UDFDIR *udfread_opendir(udfread *udf, const char *path)
{
    struct udf_dir *dir = NULL;

    if (!udf || !udf->input || !path) {
        return NULL;
    }

    if (_find_file(udf, path, &dir, NULL) < 0) {
        return NULL;
    }

    return _new_udfdir(udf, dir);
}

UDFDIR *udfread_opendir_at(UDFDIR *p, const char *name)
{
    struct udf_dir *dir = NULL;
    uint32_t index;

    if (!p || !name) {
        return NULL;
    }

    if (_scan_dir(p->dir, name, &index) < 0) {
        udf_log("udfread_opendir_at: entry %s not found\n", name);
        return NULL;
    }

    dir = _read_subdir(p->udf, p->dir, index);

    return _new_udfdir(p->udf, dir);
}

struct udfread_dirent *udfread_readdir(UDFDIR *p, struct udfread_dirent *entry)
{
    const struct udf_file_identifier *fi;

    if (!p || !entry || !p->dir) {
        return NULL;
    }

    if (p->current_file >= p->dir->num_entries) {
        return NULL;
    }

    fi = &p->dir->files[p->current_file];

    memset(entry->d_name, 0x00, 512);
    strcpy(entry->d_name, fi->filename_utf8);

    if (fi->characteristic & CHAR_FLAG_PARENT) {
        entry->d_type = UDF_DT_DIR;
        strcpy(entry->d_name, "..");
    } else if (fi->characteristic & CHAR_FLAG_DIR) {
        entry->d_type = UDF_DT_DIR;
    } else {
        entry->d_type = UDF_DT_REG;
    }

    p->current_file++;

    return entry;
}

void udfread_rewinddir(UDFDIR *p)
{
    if (p) {
        p->current_file = 0;
    }
}

void udfread_closedir(UDFDIR *p)
{
    free(p);
}

static UDFFILE *_file_open(udfread *udf, const char *path, const struct udf_file_identifier *fi)
{
    struct tag_file_entry *fe;
    UDFFILE *result;

    if (fi->characteristic & CHAR_FLAG_DIR) {
        udf_log("error opening file %s (is directory)\n", path);
        return NULL;
    }

    tag_long_ad long_ad;
    long_ad.extLength = fi->icb.length;
    long_ad.extLocation.logicalBlockNum = fi->icb.lba;

    fe = _read_file_entry(udf, long_ad);
    if (!fe) {
        udf_error("error reading file entry for %s\n", path);
        return NULL;
    }

    result = (UDFFILE *)calloc(1, sizeof(UDFFILE));
    if (!result) {
        delete fe;
        fe = NULL;
        return NULL;
    }

    result->udf = udf;
    result->fe  = fe;

    return result;
}

UDFFILE *udfread_file_open(udfread *udf, const char *path)
{
    const struct udf_file_identifier *fi = NULL;

    if (!udf || !udf->input || !path) {
        return NULL;
    }

    if (_find_file(udf, path, NULL, &fi) < 0) {
        return NULL;
    }

    return _file_open(udf, path, fi);
}

UDFFILE *udfread_file_openat(UDFDIR *dir, const char *name)
{
    uint32_t index;

    if (!dir || !name) {
        return NULL;
    }

    if (_scan_dir(dir->dir, name, &index) < 0) {
        udf_log("udfread_file_openat: entry %s not found\n", name);
        return NULL;
    }

    return _file_open(dir->udf, name, &dir->dir->files[index]);
}

int64_t udfread_file_size(UDFFILE *p)
{
    if (p) {
        return (int64_t)p->fe->informationLength;
    }
    return -1;
}

void udfread_file_close(UDFFILE *p)
{
    if (p) {
        delete p->fe;
        p->fe = NULL;
        free(p->block_mem);
        free(p);
    }
}


static uint32_t _file_lba(UDFFILE *p, uint32_t file_block, uint32_t *extent_length)
{
    const struct tag_file_entry *fe;
    unsigned int i;
    uint32_t     ad_size;

    fe = p->fe;

    for (i = 0; i < 1; i++) {
        const struct tag_short_ad ad = fe->Allocationdescriptors[0];
        ad_size = (ad.extLength + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
        if (file_block < ad_size) {
            if (!ad.extPosition) {
                /* empty file / no allocated space */
                return 0;
            }

            if (extent_length) {
                *extent_length = ad_size - file_block;
            }
            return p->udf->input->udfdisc.pd.partitionStartingLocation + ad.extPosition + file_block;
        }

        file_block -= ad_size;
    }

    return 0;
}

static int _file_lba_exists(UDFFILE *p)
{
    if (!p) {
        return 0;
    }

    return 1;
}



uint32_t udfread_read_blocks(UDFFILE *p, void *buf, uint32_t file_block, uint32_t num_blocks, int flags)
{
    uint32_t i;

    if (!num_blocks || !buf) {
        return 0;
    }

    if (!_file_lba_exists(p)) {
        return 0;
    }

    for (i = 0; i < num_blocks; ) {
        uint32_t extent_length = 0;
        uint32_t lba;
        uint8_t *block = (uint8_t *)buf + UDF_BLOCK_SIZE * i;

        lba = _file_lba(p, file_block + i, &extent_length);
        udf_trace("map block %u to lba %u\n", file_block + i, lba);

        if (!lba) {
            /* unallocated/unwritten block or EOF */
            uint32_t file_blocks = (udfread_file_size(p) + UDF_BLOCK_SIZE - 1) / UDF_BLOCK_SIZE;
            if (file_block + i < file_blocks) {
                udf_trace("zero-fill unallocated / unwritten block %u\n", file_block + i);
                memset(block, 0, UDF_BLOCK_SIZE);
                i++;
                continue;
            }
            udf_error("block %u outside of file (size %u blocks)\n", file_block + i, file_blocks);
            break;
        }

        if (extent_length > num_blocks - i) {
            extent_length = num_blocks - i;
        }

        extent_length = _read_blocks(p->udf->input, lba, block, extent_length, flags);
        if (extent_length < 1) {
            break;
        }
        i += extent_length;
    }
    return i;
}

/*
 * byte stream
 */

static ssize_t _read(UDFFILE *p, void *buf, size_t bytes)
{
    /* start from middle of block ?
     * maximal file size, i.e. position, is 2^32 * block size
     */

    size_t pos_off = p->pos % UDF_BLOCK_SIZE;
    uint32_t file_block = (uint32_t)(p->pos / UDF_BLOCK_SIZE);
    if (pos_off) {
        size_t chunk_size = UDF_BLOCK_SIZE - pos_off;
        if (!p->block_valid) {
            if (udfread_read_blocks(p, p->block, file_block, 1, 0) != 1) {
                return -1;
            }
            p->block_valid = 1;
        }
        if (chunk_size > bytes) {
            chunk_size = bytes;
        }
        memcpy(buf, p->block + pos_off, chunk_size);
        p->pos += (uint64_t)chunk_size;
        return (ssize_t)chunk_size;
    }

    /* read full block(s) ? */
    if (bytes >= UDF_BLOCK_SIZE) {
        uint32_t num_blocks = bytes / UDF_BLOCK_SIZE;
        num_blocks = udfread_read_blocks(p, buf, file_block, num_blocks, 0);
        if (num_blocks < 1) {
            return -1;
        }
        p->pos += num_blocks * UDF_BLOCK_SIZE;
        return num_blocks * UDF_BLOCK_SIZE;
    }

    /* read beginning of a block */
    if (udfread_read_blocks(p, p->block, file_block, 1, 0) != 1) {
        return -1;
    }
    p->block_valid = 1;
    memcpy(buf, p->block, bytes);
    p->pos += bytes;
    return (ssize_t)bytes;
}



#define ALIGN(p, align) \
  (uint8_t *)( ((uintptr_t)(p) + ((align)-1)) & ~((uintptr_t)((align)-1)))

ssize_t udfread_file_read(UDFFILE *p, void *buf, size_t bytes)
{
    uint8_t *bufpt = (uint8_t *)buf;

    /* sanity checks */
    if (!p || !buf) {
        return -1;
    }
    if ((ssize_t)bytes < 0 || (int64_t)bytes < 0) {
        return -1;
    }

    if (p->pos >= p->fe->informationLength) {
        return 0;
    }

    /* limit range to file size */
    if (p->pos + bytes > p->fe->informationLength) {
        bytes = (size_t)(p->fe->informationLength - p->pos);
    }

    /* allocate temp storage for input block */
    if (!p->block) {
        p->block_mem = malloc(2 * UDF_BLOCK_SIZE);
        if (!p->block_mem) {
            return -1;
        }
        p->block = ALIGN(p->block_mem, UDF_BLOCK_SIZE);
    }

    /* read chunks */
    while (bytes > 0) {
        ssize_t r = _read(p, bufpt, bytes);
        if (r < 0) {
            if (bufpt != buf) {
                /* got some bytes */
                break;
            }
            /* got nothing */
            return -1;
        }
        bufpt += r;
        bytes -= (size_t)r;
    }

    return (intptr_t)bufpt - (intptr_t)buf;
}

int64_t udfread_file_tell(UDFFILE *p)
{
    if (p) {
        return (int64_t)p->pos;
    }
    return -1;
}

int64_t udfread_file_seek(UDFFILE *p, int64_t pos, int whence)
{
    if (!p) {
        return -1;
    }

    switch (whence) {
        case UDF_SEEK_CUR:
            pos = udfread_file_tell(p) + pos;
            break;
        case UDF_SEEK_END:
            pos = udfread_file_size(p) + pos;
            break;
        case UDF_SEEK_SET:
            break;
        default:
            return -1;
    }

    if (pos >= 0 && pos <= udfread_file_size(p)) {
        p->pos = (uint64_t)pos;
        p->block_valid = 0;
        return udfread_file_tell(p);
    }

    return -1;
}

int read_file_tree(udfread* udf, FileNode* file_node, struct tag_long_ad icb)
{
    //find root file_entry
    tag_file_entry* fe = _read_file_entry(udf, icb);
    file_node->m_file_entry = *fe;


    if(file_node->m_file_entry.icbTag.fileType == 4){
        tag_long_ad alloc_long_ad;
        alloc_long_ad.extLength = fe->Allocationdescriptors[0].extLength;
        alloc_long_ad.extLocation.logicalBlockNum = fe->Allocationdescriptors[0].extPosition;

        uint8_t        *data;
        int             tag_id;
        data = _read_metadata(udf, alloc_long_ad, &tag_id);
        if (!data) {
            udf_error("reading directory file failed\n");
            return -1;
        }

        std::vector<full_file_identifier>  file_id_vector = _parse_dir(udf, data, alloc_long_ad.extLength) ;
        file_node->m_fileid_list = file_id_vector;
        free(data);

        for(full_file_identifier index : file_id_vector){
            if(index.file_indent_desc.fileCharacteristics & CHAR_FLAG_PARENT){
                continue;
            }
            FileNode* sub_node = new FileNode();
            file_node->m_sub_node_list.push_back(sub_node);
            read_file_tree(udf,sub_node,index.file_indent_desc.icb);
        }

    }
    delete fe;
    return 0;
}
