#ifndef _UDF_APPEND_BUILDER_H_
#define _UDF_APPEND_BUILDER_H_

#include "udfdataset.h"
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include "udfread.h"
#include <vector>


using namespace std;

#ifndef UDF_BLOCK_SIZE
#  define UDF_BLOCK_SIZE  2048
#endif

const uint32_t MAX_SINGLE_PART = 524287 * 2048;

class BaseFile
{
public:
    tag_file_entry m_file_entry;
};

class BaseDir : BaseFile
{
public:
    std::list<BaseFile*> m_subFoldList;
};



//文件信息
class RecordFile
{
public:
    RecordFile(){
        m_FileSize = 0;
        m_FileBlockSize = 0;
        m_IcbSectorLoca = 0;
        m_DataSectorLoca = 0;
        //fileid 文件头 + cs0编码文件名长度 + padding(4字节对齐)
        m_InfoLength=0;
        memset(m_Name,0,256);
        memset(m_Path,0,256);
    }

    ~RecordFile(){}
public:
    //文件目录信息字段的长度(单位是字节)
    int       m_InfoLength;
    //父目录的完整路径(以/结尾)
    char      m_Path[512];
    //目录或文件的名称
    char      m_Name[512];
    //文件的大小
    int64_t   m_FileSize;
    int64_t   m_FileBlockSize;
    int       m_DataSectorLoca;
    //ICB信息的地址(block)
    int       m_IcbSectorLoca;
};


//目录信息
class RecordDir: public RecordFile
{
public:
    RecordDir(){
        m_SumInfoLen = 0;
        m_DirCount = 0;
        m_FileCount = 0;
        m_FileSum = 0;
        m_DirSum = 0;
        m_IsRoot = false;
        m_IsDir = false;
        m_SubFoldList.clear();
    }

    ~RecordDir(){
        while(!m_SubFoldList.empty()){
            RecordDir* pTemp = (RecordDir*)m_SubFoldList.back();
            m_SubFoldList.pop_back();
            delete pTemp;
        }
    }

public:
    //所有子文件和子目录的文件名称的字节长度(字节)
    int       m_SumInfoLen;
    //子目录数量
    int       m_DirCount;
    //所有子目录总数(多层次)
    int       m_DirSum;
    //子文件数量
    int       m_FileCount;
    //所有子文件总数(多层次)
    int       m_FileSum;
    bool      m_IsRoot;
    //判断是否是目录
    bool      m_IsDir;
    std::list<RecordFile*>  m_SubFoldList;

    struct timespec m_last_access_time;        //最后访问时间
    struct timespec m_last_modify_time ;       //最后修改时间
    struct timespec m_last_status_changed_time; //最后状态变更时间
};

struct full_file_identifier {
    uint8_t         filename[256];
    int             filename_length;
    char            filename_utf8[256];
    tag_file_indent_desc file_indent_desc;
    int   padd_length;
};

class FileNode
{
public:
    FileNode() = default;
    ~FileNode(){
        for(auto index : m_sub_node_list){
            delete index;
            index = nullptr;
        }
    }
    void addChild(FileNode* node){
        m_sub_node_list.push_back(node);
    }
public:
    tag_file_entry m_file_entry;
    std::vector<full_file_identifier> m_fileid_list;
    std::vector<FileNode*> m_sub_node_list;
    int m_parent_dir_location;
    int m_parent_unique_id;
    char m_parent_dir_path[512];
    char m_item_name[512];

};


class UdfAppender
{
public:
    UdfAppender();
    ~UdfAppender();

     //设置文件信息
     bool PreAppendBuild(string input_dir_path, string volumeid, string output_path);
     //build udf image
     bool BuildAppendUDF();
     //初始化刻录
     bool InitBurnUDF();
     //设置之前刻录数据的头信息
     void SetUdfImageHeader(udf_disc disInfo);
     //设置的LVID信息
     void SetLVID(tag_logic_vol_integrity_desc lvid);
     //追加刻录的偏移地址信息
     void SetOffsetIndex(const int index);
     //旧的文件树
     void SetFileTree(FileNode* file_tree);

     void PrintFileTree(FileNode* file_node);


private:
    //构造空白扇区
    int WriteBlankSector(int blockCount);

    //构造卷识别序列
    int WriteProbeVolume();

    //构造主卷描述符序列
    udf_disc BuildUdfDiskInfo(int startIndex, int fsd_index, int intergrity_index, int pd_start_location, int partition_length);
    int WriteMainVolDescriSequ(udf_disc discInfo);
    //pvd
    tag_primary_vol_desc  BuildPrimaryVolumeDesc(int tagLocation);
    //写用户实现描述符imp
    tag_imp_use_vol_desc  BuildImplementationDesc(int tagLocation);
    //写分区描述符 PD
    tag_partition_desc BuildPartitionDesc(int tagLocation, int pd_start_loacation, int pd_length);
    //写逻辑卷描述符 LVD
    tag_logical_vol_desc BuildLogicalVolumeDesc(int tagLocation, int fsd_location, int intergrity_location);
    //写未分配空间描述符 USD
    tag_unallocated_space_desc BuildUnAllocatedSpaceDesc(int tagLocation);
    //写终止描述符 TD
    tag_terminating_desc BuildTerminatingDesc(int tagLocation);

    //写逻辑卷一致性序列
    tag_logic_vol_integrity_desc BuildLogVolIntegDesciSequ(int tagLocation, int file_count, int dir_count, int pd_length, int64_t unique_id);

    //合并刷新及写入文件树
    int MergeFileTree(FileNode* prev_file_tree, FileNode* next_file_tree);
    int UpdateOriginFileTree(FileNode* file_node, int start_blockIndex);
    int WriteFileTreeInfo(FileNode* file_node);

    //写文件集描述符序列
    tag_file_set_desc BuildFileSetDesc(int tagLocation, int pd_starting_location);
    //构造文件目录结构
    void BuildDirFileStruct(RecordDir *pFileDirAttr, FileNode* file_node);
    //写文件条目描述符
    void WriteFileIcb(tag_icb_param *pICB, FileNode* file_node, RecordDir* file_item);
    //写文件标识符描述符
    void WriteFileId(tag_file_id_info* pFileId, FileNode* file_node);
    //构造文件结构
    void BuildFileStruct(RecordDir *pFileDirAttr, FileNode* file_node);
    //添充用户刻录的数据文件
    void FillData(RecordDir *pFileDirAttr);
    int WriteAppendFileData(FileNode *file_node, int pd_offset);
    //得到用户指定的文件目录属性
    bool GetFileDirAttr(RecordDir *pFileDirAttr);
    //写入文件信息
    void WriteFileNodeToFile(FileNode* file_node);

    //构造保留卷描述符序列
    int WriteReserveVolDescriSequ();

    //写定位卷描述符指针
    tag_anchor_vol_desc_ptr BuildAVDP(int tag_location,int main_vol_location, int res_vol_location);
    //主卷标定位指针 avdp
    void WriteEndAvdpInfo(int avdp_count);

    //写时间戳
    void FillTime(time_stamp *pTime, timespec time_spec);

    //check DesTag
    void DescriptorTagTest(uint8_t *pBuff);
    //crc16 校验
    uint16_t udf_crc16(uint8_t *data, uint32_t size, uint16_t crc);
    //生成8位uuid
    std::string GenerateShortUUID();


    //计算目录的深度，暂时未调用
    int CalcFileTreeBlockLength(FileNode* file_node);
    int GetCurrentDirLength(RecordDir *pFileDirAttr);


private:
    udf_disc                        m_origin_disc_info;  //原始卷描述信息
    tag_anchor_vol_desc_ptr         m_avdp;              //卷定位指针信息
    int                             m_offset_block_index;//偏移地址信息
    FileNode*                       m_prev_file_tree = NULL;    //之前的文件树
    tag_logic_vol_integrity_desc    m_LogicalVolIntegrityDesc; //卷一致性描述信息
    string                          m_output_iso_path;  //输出镜像的地址信息

    FileNode m_append_file_tree;

    timespec                    m_systime;
    uint8_t *                   m_pBlankBuffer;
    uint8_t *                   m_pCrcBuffer;
    int                         m_IcbIndex;
    //二进制文件对应的区块地址
    int                         m_FileIcbAllocLocation;
    int                         m_FileIcbLocation;

    //分区块的长度
    int64_t                     m_PartitionLength;

    char                        m_pVolSetIden[256];
    uint8_t                     m_volumeName[256];
    int                         m_volumeNameLength;
    RecordDir                   m_FileDirAttr;
    char                        m_OutPutDirName[512];
    std::list<int>              m_MapFileList;
    //打开文件的fileid
    int                         m_outputDirFileId;
    int                         m_inputFileId;
    time_stamp                  m_burnDateAndTime;
};

#endif
