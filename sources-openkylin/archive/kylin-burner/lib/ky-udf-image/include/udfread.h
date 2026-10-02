#ifndef UDFREAD_H_
#define UDFREAD_H_


#include <stdint.h>    /* *int_t */
#include <sys/types.h> /* *size_t */
#include "cs0encode.h"
#include "ecma167.h"
#include "udfdataset.h"




struct udfread;
struct udf_dir;
class FileNode;

struct udf_file_identifier {
    char           *filename_utf8;       /* MUTF-8 */
    uint8_t        filename[256];
    int padLength;
    int            filename_length;
    long_ad  icb;            /* location of file entry */
    uint8_t         characteristic; /* CHAR_FLAG_* */
    tag_file_indent_desc file_indent_desc;
    tag_file_entry file_entry;
    int  used_length;
};

struct udf_dir {
    tag_file_entry               file_entry;
    uint32_t                     num_entries;
    struct udf_file_identifier  *files;
    struct udf_dir             **subdirs;
};

struct udfread_dir {
    udfread              *udf;
    struct udf_dir       *dir;
    uint32_t              current_file;
};

/* opaque handle for UDF volume */
typedef struct udfread udfread;

/* Logical Partitions from Logical Volume Descriptor */
struct udf_partitions {
    uint32_t num_partition;
    struct {
        uint16_t number;
        uint32_t lba;
        uint32_t mirror_lba;
    } p[2];
};

struct udfread_block_input;

struct udfread {

    udfread_block_input *input;

    /* Volume partitions */
    struct udf_partitions part;

    /* cached directory tree */
    struct udf_dir *root_dir;

    char *volume_identifier;
    char volume_set_identifier[128];

};


/* Additional File Types (UDF 2.60, 2.3.5.2) */

enum udf_file_type {
    UDF_FT_METADATA        = 250,
    UDF_FT_METADATA_MIRROR = 251,
};

struct udfread_block_input;


int  read_file_tree(udfread* udf, FileNode* file_node, struct tag_long_ad icb);


/**
 *  Open UDF image
 *
 * @param p  udfread object
 * @param input  UDF image access functions
 * @return 0 on success, < 0 on error
 */
int udfread_open_input (udfread *, struct udfread_block_input *input);

/**
 *  Open UDF image
 *
 * @param p  udfread object
 * @param path  path to device or image file
 * @return 0 on success, < 0 on error
 */
int udfread_open (udfread *, const char *path);

/**
 *  Close UDF image
 *
 * @param p  udfread object
 */
void udfread_close (udfread *);

/**
 *  Get UDF Volume Identifier
 *
 * @param p  udfread object
 * @return Volume ID as null-terminated MUTF-8 string, NULL if error. Returned pointer is valid until udfread_close().
 */
const char *udfread_get_volume_id (udfread *);

/**
 *  Get UDF Volume Set Identifier
 *
 * @param p  udfread object
 * @param buffer buffer to receive volume set id
 * @param size buffer size
 * @return Volume set id size, 0 if error
 */
size_t udfread_get_volume_set_id (udfread *, void *buffer, size_t size);


/*
 * Directory access
 */

/* File types for d_type */
enum {
    UDF_DT_UNKNOWN = 0,
    UDF_DT_DIR,
    UDF_DT_REG,
};

/* Directory stream entry */
struct udfread_dirent {
    unsigned int  d_type;    /* UDF_DT_* */
    char   d_name[512];    /* MUTF-8 */
};

/* opaque handle for directory stream */
typedef struct udfread_dir UDFDIR;

/**
 *  Open directory stream
 *
 * @param p  udfread object
 * @param path  path to the directory (MUTF-8)
 * @return directory stream handle on the directory, or NULL if it could not be opened.
 */
UDFDIR *udfread_opendir (udfread *, const char *path);

/**
 *  Open directory stream
 *
 *  Directory name may contain special chars (/, \, ...).
 *
 * @param dir  parent directory stream handle
 * @param name  name of the directory to open from dir (MUTF-8)
 * @return directory stream handle on the directory, or NULL if it could not be opened.
 */
UDFDIR *udfread_opendir_at(UDFDIR *dir, const char *name);

/**
 *  Read directory stream
 *
 *  Read a directory entry from directory stream. Return a pointer to
 *  udfread_dirent struct describing the entry, or NULL for EOF or error.
 *
 * @param p  directory stream
 * @param entry  storege space for directory entry
 * @return next directory stream entry, or NULL if EOF or error.
 */
struct udfread_dirent *udfread_readdir (UDFDIR *, struct udfread_dirent *entry);

/**
 *  Rewind directory stream
 *
 *  Rewind directory stream to the beginning of the directory.
 *
 * @param p  directory stream
 */
void udfread_rewinddir (UDFDIR *);

/**
 *  Close directory stream
 *
 * @param p  directory stream
 */
void udfread_closedir (UDFDIR *);


/*
 * File access
 */

/**
 * The length of one Logical Block
 */

#ifndef UDF_BLOCK_SIZE
#  define UDF_BLOCK_SIZE  2048
#endif



struct udfread_file {
    udfread           *udf;
    struct tag_file_entry *fe;

    /* byte stream access */
    uint64_t    pos;
    uint8_t    *block;
    int         block_valid;

    void       *block_mem;
};
typedef struct udfread_file UDFFILE;

/**
 *  Open a file
 *
 *  Allowed separator chars are \ and /.
 *  Path to the file is always absolute (relative to disc image root).
 *  Path may begin with single separator char.
 *  Path may not contain "." or ".." directory components.
 *
 * @param p  udfread object
 * @param path  path to the file (MUTF-8)
 * @return file object, or NULL if it could not be opened.
 */
UDFFILE *udfread_file_open (udfread *, const char *path);

/**
 *  Open a file from directory
 *
 *  File name may contain special chars (/, \, ...).
 *
 * @param dir  parent directory stream handle
 * @param name  name of the file (MUTF-8)
 * @return file object, or NULL if it could not be opened.
 */
UDFFILE *udfread_file_openat (UDFDIR *dir, const char *name);

/**
 *  Close file object
 *
 * @param p  file object
 */
void udfread_file_close (UDFFILE *);

/**
 *  Get file size
 *
 * @param p  file object
 * @return file size, -1 on error
 */
int64_t udfread_file_size (UDFFILE *);



/**
 *  Read blocks from a file
 *
 * @param p  file object
 * @param buf  buffer for data
 * @param file_block  file block number
 * @param num_blocks  number of blocks to read
 * @return number of blocks read, 0 on error
 */
uint32_t udfread_read_blocks (UDFFILE *, void *buf, uint32_t file_block, uint32_t num_blocks, int flags);


/*
 * Byte streams
 */

enum {
    UDF_SEEK_SET = 0,
    UDF_SEEK_CUR = 1,
    UDF_SEEK_END = 2,
};

/**
 *  Read bytes from a file
 *
 *  Reads the given number of bytes from the file and increment the
 *  current read position by number of bytes read.
 *
 * @param p  file object
 * @param buf  buffer for data
 * @param bytes  number of bytes to read
 * @return number of bytes read, 0 on EOF, -1 on error
 */
ssize_t udfread_file_read (UDFFILE *, void *buf, size_t bytes);

/**
 *  Get current read position of a file
 *
 * @param p  file object
 * @return current read position of the file, -1 on error
 */
int64_t udfread_file_tell (UDFFILE *);

/**
 *  Set read position of a file
 *
 *  New read position is calculated from offset according to the directive whence as follows:
 *    UDF_SEEK_SET  The offset is set to offset bytes.
 *    UDF_SEEK_CUR  The offset is set to its current location plus offset bytes.
 *    UDF_SEEK_END  The offset is set to the size of the file plus offset bytes.
 *
 * @param p  file object
 * @param pos  byte offset
 * @param whence  directive
 * @return current read position of the file, -1 on error
 */
int64_t udfread_file_seek (UDFFILE *, int64_t pos, int whence);



void test_encode_decode();


#endif /* UDFREAD_H_ */
