#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include <codecvt>
#include <locale>
#include <iconv.h>
#include <string.h>
#include <vector>
#include <queue>
#include <random>

#include "udfappender.h"
#include "cs0encode.h"



UdfAppender::UdfAppender()
{
    m_pBlankBuffer = new uint8_t[2048];
    memset(m_pBlankBuffer,0,2048);

    m_pCrcBuffer = new uint8_t[2048];
    memset(m_pCrcBuffer,0,2048);

    memset(m_pVolSetIden, 0, 256);
    memset(m_volumeName, 0, 256);
    m_volumeNameLength = -1;

    memset(m_OutPutDirName,0,sizeof(char)*512);

    m_IcbIndex = 17;
    m_FileIcbLocation = 4;
    m_PartitionLength = 4;
    m_FileIcbAllocLocation = 0;

    m_FileDirAttr.m_IsRoot = true;
    m_FileDirAttr.m_IcbSectorLoca = 2;
}

UdfAppender::~UdfAppender()
{
    delete []m_pCrcBuffer;
    m_pCrcBuffer = NULL;

    delete []m_pBlankBuffer;
    m_pBlankBuffer = NULL;

    if(m_prev_file_tree){
        delete m_prev_file_tree;
        m_prev_file_tree = NULL;
    }
}

void UdfAppender::SetOffsetIndex(const int index)
{
    m_offset_block_index = index;
}

void UdfAppender::SetFileTree(FileNode* file_tree)
{
    m_prev_file_tree = file_tree;
}

void UdfAppender::PrintFileTree(FileNode *file_node)
{
    if(file_node == NULL){
        return;
    }
    std::queue<FileNode*> node_queue;
    node_queue.push(file_node);

    while(!node_queue.empty()){
        FileNode* currentNode = node_queue.front();
        node_queue.pop();

        if(currentNode == NULL){
            return;
        }

        printf("entrylocation:%d allocat location:%d\n",currentNode->m_file_entry.descTag.tagLocation,
               currentNode->m_file_entry.Allocationdescriptors[0].extPosition);

        if(file_node->m_fileid_list.size() > 0){

            for(auto index : currentNode->m_fileid_list){
                printf("%s fileid tag:%d allocat icb location:%d\n",index.filename_utf8,index.file_indent_desc.descTag.tagLocation,
                       index.file_indent_desc.icb.extLocation.logicalBlockNum);

            }
        }
        printf("\n\n");
        for(auto sub_node : currentNode->m_sub_node_list){
            node_queue.push(sub_node);
        }
    }
}


void UdfAppender::WriteFileNodeToFile(FileNode *file_node)
{
    if(file_node == NULL){
        return;
    }

    //calc crc
    uint8_t *pBuff=(uint8_t*)(&file_node->m_file_entry);
    DescriptorTagTest(pBuff);
    write(m_outputDirFileId, &file_node->m_file_entry, UDF_BLOCK_SIZE);

    if(file_node->m_fileid_list.size() > 0){
        int already_write_length = 0;
        for(auto index : file_node->m_fileid_list){

            int LengthId= 38 + index.filename_length + index.padd_length;
            uint8_t *pBuff = new uint8_t[LengthId];
            memset(pBuff, 0, LengthId);
            memcpy(pBuff, &index.file_indent_desc, 38);

            memcpy(pBuff + 38, index.filename, index.filename_length);
            DescriptorTagTest(pBuff);
            write(m_outputDirFileId, pBuff, LengthId);
            already_write_length += LengthId;

        }

        uint8_t Temp[2048];
        memset(Temp,0,2048);
        int bye_align = 2048 - (already_write_length % 2048);
        write(m_outputDirFileId, Temp, bye_align);
    }
}



bool UdfAppender::BuildAppendUDF()
{
    clock_gettime(CLOCK_REALTIME, &m_systime);
    FillTime(&m_burnDateAndTime, m_systime);

    //copy old iso_info
    m_outputDirFileId = open(m_OutPutDirName, O_CREAT | O_RDWR, 0777);
    if(m_outputDirFileId == -1){
        return false;
    }

    //write blank spliter
    WriteBlankSector(16);
    WriteProbeVolume();
    WriteBlankSector(13);

    //locate the image header
    int main_vol_location = m_offset_block_index + 32;
    int vol_integrity_location = main_vol_location + 32;
    int avdp_location = main_vol_location + 224;
    int fsd_location = avdp_location + 32;


    //Write vol intergrity and terminate
    int32_t prev_file_count  = m_LogicalVolIntegrityDesc.p_impUse[32];
    int32_t prev_dir_count = m_LogicalVolIntegrityDesc.p_impUse[36];
    int32_t file_entry_offset = CalcFileTreeBlockLength(m_prev_file_tree);

    //追加刻录的数据描述信息没有root
    m_FileDirAttr.m_IcbSectorLoca = fsd_location + 2 + file_entry_offset - m_origin_disc_info.pd.partitionStartingLocation;
    m_FileIcbLocation = fsd_location + 2 + file_entry_offset - m_origin_disc_info.pd.partitionStartingLocation;

    //update pd length
    m_PartitionLength = (fsd_location - m_origin_disc_info.pd.partitionStartingLocation) + file_entry_offset + 2;
    //只追加新增文件列表信息和二进制文件内容信息
    GetFileDirAttr(&m_FileDirAttr);

    const int pd_starting_location =  m_origin_disc_info.pd.partitionStartingLocation;
    const int fsd_pd_offset_location = fsd_location - pd_starting_location;


    //Write main vol desc sequent
    udf_disc disc_info = BuildUdfDiskInfo(main_vol_location, fsd_pd_offset_location, vol_integrity_location,
                                          pd_starting_location, m_PartitionLength);
    WriteMainVolDescriSequ(disc_info);
    WriteBlankSector(vol_integrity_location - disc_info.terdes.descTag.tagLocation - 1);

    tag_logic_vol_integrity_desc lvid = BuildLogVolIntegDesciSequ(vol_integrity_location, prev_file_count + m_FileDirAttr.m_FileCount,
                                                                  prev_dir_count + m_FileDirAttr.m_DirCount, m_PartitionLength, 0x1B);
    write(m_outputDirFileId, &lvid, UDF_BLOCK_SIZE);

    tag_terminating_desc terminate = BuildTerminatingDesc(vol_integrity_location + 1);
    write(m_outputDirFileId, &terminate, UDF_BLOCK_SIZE);

    WriteBlankSector(avdp_location - vol_integrity_location - 2);

    //Write avdp
    int res_vol_location = m_origin_disc_info.pd.partitionStartingLocation + m_PartitionLength;
    m_avdp = BuildAVDP(avdp_location, main_vol_location,res_vol_location);
    write(m_outputDirFileId, &m_avdp, UDF_BLOCK_SIZE);

    WriteBlankSector(fsd_location - avdp_location - 1);

    //Write fsd  and terminated block
    tag_file_set_desc tmp_fsd = BuildFileSetDesc(fsd_location, pd_starting_location);
    write(m_outputDirFileId, &tmp_fsd, UDF_BLOCK_SIZE);
    tag_terminating_desc fsd_terminate = BuildTerminatingDesc(tmp_fsd.descTag.tagLocation + 1);
    write(m_outputDirFileId, &fsd_terminate, UDF_BLOCK_SIZE);

    PrintFileTree(m_prev_file_tree);

    //calculate binary file addr
    m_FileIcbAllocLocation = m_FileIcbLocation;
    BuildDirFileStruct(&m_FileDirAttr, &m_append_file_tree);
    MergeFileTree(m_prev_file_tree, &m_append_file_tree);

    UpdateOriginFileTree(m_prev_file_tree, fsd_location - pd_starting_location);
    WriteFileTreeInfo(m_prev_file_tree);

    FillData(&m_FileDirAttr);

    //write backup disc_info
    udf_disc backup_disc_info = BuildUdfDiskInfo(res_vol_location, fsd_pd_offset_location, vol_integrity_location, pd_starting_location, m_PartitionLength);
    WriteMainVolDescriSequ(backup_disc_info);

    //write end avdp
    WriteEndAvdpInfo(259);
    close(m_outputDirFileId);

    return true;
}

bool UdfAppender::InitBurnUDF()
{
    clock_gettime(CLOCK_REALTIME, &m_systime);
    FillTime(&m_burnDateAndTime, m_systime);

    //copy old iso_info
    m_outputDirFileId = open(m_OutPutDirName, O_CREAT | O_RDWR, 0777);
    if(m_outputDirFileId == -1){
        return false;
    }

    //write blank spliter
    WriteBlankSector(16);
    WriteProbeVolume();
    WriteBlankSector(13);

    //locate the image header
    int main_vol_location = 32;
    int vol_integrity_location = main_vol_location + 32;
    int avdp_location = main_vol_location + 224;
    int fsd_location = avdp_location + 32;

    const int partitionStartingLocation = 288;

    //追加刻录的数据描述信息没有root
    m_FileDirAttr.m_IcbSectorLoca = fsd_location + 2  - partitionStartingLocation;
    m_FileIcbLocation = fsd_location + 4 - partitionStartingLocation;

    //初始化长度 = fsd + td + root_entry + list = 4
    m_PartitionLength = 4;
    //只追加新增文件列表信息和二进制文件内容信息
    GetFileDirAttr(&m_FileDirAttr);

    int fsd_pd_offset_location = fsd_location - partitionStartingLocation;
    //Write main vol desc sequent
    udf_disc disc_info = BuildUdfDiskInfo(main_vol_location, fsd_pd_offset_location, vol_integrity_location, partitionStartingLocation, m_PartitionLength);
    WriteMainVolDescriSequ(disc_info);

    WriteBlankSector(vol_integrity_location - disc_info.terdes.descTag.tagLocation - 1);

    tag_logic_vol_integrity_desc lvid = BuildLogVolIntegDesciSequ(vol_integrity_location, m_FileDirAttr.m_FileSum,
                                                                  m_FileDirAttr.m_DirSum + 1, m_PartitionLength, 0x1A);//dir add root
    write(m_outputDirFileId, &lvid, UDF_BLOCK_SIZE);

    tag_terminating_desc terminate = BuildTerminatingDesc(vol_integrity_location + 1);
    write(m_outputDirFileId, &terminate, UDF_BLOCK_SIZE);

    WriteBlankSector(avdp_location - vol_integrity_location - 2);

    //avdp
    int res_vol_location = partitionStartingLocation + m_PartitionLength;
    m_avdp = BuildAVDP(avdp_location, main_vol_location,res_vol_location);
    write(m_outputDirFileId, &m_avdp, UDF_BLOCK_SIZE);

    WriteBlankSector(fsd_location - avdp_location - 1);

    //fsd and terminated
    tag_file_set_desc tmp_fsd = BuildFileSetDesc(fsd_location, partitionStartingLocation);
    write(m_outputDirFileId, &tmp_fsd, UDF_BLOCK_SIZE);
    tag_terminating_desc fsd_terminate = BuildTerminatingDesc(tmp_fsd.descTag.tagLocation + 1);
    write(m_outputDirFileId, &fsd_terminate, UDF_BLOCK_SIZE);

    //指向文件树之后的第一个位置
    m_FileIcbAllocLocation = m_FileIcbLocation;
    BuildDirFileStruct(&m_FileDirAttr, &m_append_file_tree);
    UpdateOriginFileTree(&m_append_file_tree,0);

    PrintFileTree(&m_append_file_tree);

    WriteFileTreeInfo(&m_append_file_tree);
    WriteAppendFileData(&m_append_file_tree, partitionStartingLocation);

    //write backup disc_info
    udf_disc backup_disc_info = BuildUdfDiskInfo(res_vol_location, fsd_pd_offset_location,
                                                 vol_integrity_location,partitionStartingLocation, m_PartitionLength);
    WriteMainVolDescriSequ(backup_disc_info);

    //write end avdp
    WriteEndAvdpInfo(265);
    close(m_outputDirFileId);
    return true;
}

bool UdfAppender::PreAppendBuild(string input_dir_path, string volumeid, string output_path)
{
    //volumeid
    strcpy(m_pVolSetIden, volumeid.c_str());
    int outputLength = encode_utf8(m_volumeName, volumeid.c_str(), 256);
    m_volumeNameLength = outputLength;

    //output file path
    strcpy(m_OutPutDirName, output_path.c_str());
    strcpy(m_FileDirAttr.m_Name, basename((char*)input_dir_path.c_str()));
    int parent_dir_length = input_dir_path.length() - strlen(basename((char*)input_dir_path.c_str()));
    memcpy(m_FileDirAttr.m_Path, input_dir_path.c_str(),parent_dir_length);

    //if exist delete
    remove(m_OutPutDirName);
    return true;
}



void UdfAppender::SetUdfImageHeader(udf_disc disInfo)
{
    m_origin_disc_info = disInfo;
}


void UdfAppender::SetLVID(tag_logic_vol_integrity_desc lvid)
{
    m_LogicalVolIntegrityDesc = lvid;
}

int UdfAppender::WriteBlankSector(int blockCount)
{
    if(blockCount <= 0){
        return 0;
    }

    for(int n=0; n<blockCount; n++){
        write(m_outputDirFileId, m_pBlankBuffer, 2048);
    }

    return blockCount;
}

int UdfAppender::MergeFileTree(FileNode *prev_file_tree, FileNode *next_file_tree)
{
    if((prev_file_tree == NULL) || (next_file_tree == NULL)){
        return -1;
    }
    if(next_file_tree->m_fileid_list.size() <= 1){
        return -2;
    }

    int root_file_id_location = prev_file_tree->m_fileid_list[0].file_indent_desc.descTag.tagLocation;
    for(int index=1; index< next_file_tree->m_fileid_list.size(); ++index){
        next_file_tree->m_fileid_list[index].file_indent_desc.descTag.tagLocation = root_file_id_location;
        prev_file_tree->m_fileid_list.push_back(next_file_tree->m_fileid_list.at(index));
    }

    int file_id_list_length = 0;
    for(int index = 0; index< prev_file_tree->m_fileid_list.size(); ++index){
        int index_file_id_length = 38 + prev_file_tree->m_fileid_list.at(index).filename_length + prev_file_tree->m_fileid_list.at(index).padd_length;
        file_id_list_length += index_file_id_length;
    }


    if(file_id_list_length != prev_file_tree->m_file_entry.Allocationdescriptors[0].extLength){
        prev_file_tree->m_file_entry.Allocationdescriptors[0].extLength = file_id_list_length;
    }
    prev_file_tree->m_file_entry.informationLength = file_id_list_length;

    for(int index=0; index< next_file_tree->m_sub_node_list.size(); ++index){
        prev_file_tree->m_sub_node_list.push_back(next_file_tree->m_sub_node_list.at(index));
    }
    next_file_tree->m_sub_node_list.clear();

    return 0;
}

udf_disc UdfAppender::BuildUdfDiskInfo(int startIndex, int fsd_index, int intergrity_index, int pd_start_location, int partition_length)
{
    udf_disc disc_info;
    disc_info.pvd = BuildPrimaryVolumeDesc(startIndex);
    disc_info.imp = BuildImplementationDesc(startIndex + 1);
    disc_info.lvd = BuildLogicalVolumeDesc(startIndex + 2, fsd_index, intergrity_index);
    disc_info.pd = BuildPartitionDesc(startIndex + 3, pd_start_location, partition_length);
    disc_info.usd = BuildUnAllocatedSpaceDesc(startIndex + 4);
    disc_info.terdes = BuildTerminatingDesc(startIndex + 5);
    disc_info.pvd.recordingDateAndTime = m_burnDateAndTime;

    return disc_info;
}

int UdfAppender::UpdateOriginFileTree(FileNode* file_node, int start_blockIndex)
{
    if(file_node == NULL){
        return -1;
    }

    std::queue<FileNode*> node_queue;
    node_queue.push(file_node);

    int start_file_entry_location = start_blockIndex + 2; //offset = fsd + terminated

    file_node->m_file_entry.accessTime = m_burnDateAndTime;
    file_node->m_file_entry.modificationTime = m_burnDateAndTime;
    file_node->m_file_entry.attrmodifyTime = m_burnDateAndTime;


    //udate tag location
    while(!node_queue.empty()){
        FileNode* currentNode = node_queue.front();
        node_queue.pop();

        if(currentNode != file_node){
            currentNode->m_file_entry.attrmodifyTime = m_burnDateAndTime;
        }

        currentNode->m_file_entry.descTag.tagLocation = start_file_entry_location++;

        if(currentNode->m_fileid_list.size() > 0){
            currentNode->m_file_entry.Allocationdescriptors[0].extPosition = start_file_entry_location;
            for(int index=0; index<currentNode->m_fileid_list.size(); ++index){
               currentNode->m_fileid_list[index].file_indent_desc.descTag.tagLocation = start_file_entry_location;
            }
            start_file_entry_location += currentNode->m_file_entry.logicalBlocksRecorded;
        }

        for(auto sub_node : currentNode->m_sub_node_list){
            node_queue.push(sub_node);
        }
    }

    int unique_id = file_node->m_fileid_list[1].file_indent_desc.icb.impUse[2];
    //update fileId location
    file_node->m_parent_dir_location = file_node->m_file_entry.descTag.tagLocation;
    file_node->m_fileid_list[0].file_indent_desc.icb.impUse[2] = 0;

    node_queue.push(file_node);
    while(!node_queue.empty()){
        FileNode* currentNode = node_queue.front();
        node_queue.pop();

        if(currentNode->m_fileid_list.size() > 0){

            for(int index=0; index<currentNode->m_fileid_list.size(); ++index){
               //root
               if(currentNode->m_fileid_list[index].file_indent_desc.fileCharacteristics == 0xa){
                   currentNode->m_fileid_list[index].file_indent_desc.icb.extLocation.logicalBlockNum =
                          currentNode->m_parent_dir_location;
                   if(currentNode != file_node){
                       currentNode->m_fileid_list[index].file_indent_desc.icb.impUse[2] =
                               currentNode->m_parent_unique_id;
                   }
               }else{
                   currentNode->m_fileid_list[index].file_indent_desc.icb.extLocation.logicalBlockNum =
                           currentNode->m_sub_node_list[index - 1]->m_file_entry.descTag.tagLocation;

                    currentNode->m_fileid_list[index].file_indent_desc.icb.impUse[2] = unique_id;
                    currentNode->m_sub_node_list[index - 1]->m_file_entry.uniqueID = unique_id;
                    unique_id++;
               }
            }
            start_file_entry_location++;
        }

        for(auto sub_node : currentNode->m_sub_node_list){
            sub_node->m_parent_dir_location = currentNode->m_file_entry.descTag.tagLocation;
            sub_node->m_parent_unique_id = currentNode->m_file_entry.uniqueID;
            node_queue.push(sub_node);
        }

    }
    return 0;
}

int UdfAppender::WriteProbeVolume()
{
    if(!m_outputDirFileId){
        return -1;
    }

    memset(m_pBlankBuffer, 0, 2048);
    m_pBlankBuffer[1]='B';	m_pBlankBuffer[2]='E';
    m_pBlankBuffer[3]='A';	m_pBlankBuffer[4]='0';
    m_pBlankBuffer[5]='1';	m_pBlankBuffer[6]=1;
    write(m_outputDirFileId, m_pBlankBuffer, 2048);

    m_pBlankBuffer[1]='N';	m_pBlankBuffer[2]='S';
    m_pBlankBuffer[3]='R';	m_pBlankBuffer[4]='0';
    m_pBlankBuffer[5]='2';
    write(m_outputDirFileId, m_pBlankBuffer, 2048);

    m_pBlankBuffer[1]='T';	m_pBlankBuffer[2]='E';
    m_pBlankBuffer[3]='A';	m_pBlankBuffer[4]='0';
    m_pBlankBuffer[5]='1';
    write(m_outputDirFileId, m_pBlankBuffer, 2048);

    m_pBlankBuffer[1]=0;	m_pBlankBuffer[2]=0;
    m_pBlankBuffer[3]=0;	m_pBlankBuffer[4]=0;
    m_pBlankBuffer[5]=0;	m_pBlankBuffer[6]=0;

    return 0;
}

int UdfAppender::WriteMainVolDescriSequ(udf_disc discInfo)
{
    if(m_outputDirFileId < 0){
        return -1;
    }

    write(m_outputDirFileId, &discInfo.pvd, UDF_BLOCK_SIZE);
    write(m_outputDirFileId, &discInfo.imp, UDF_BLOCK_SIZE);
    write(m_outputDirFileId, &discInfo.lvd, UDF_BLOCK_SIZE);
    write(m_outputDirFileId, &discInfo.pd, UDF_BLOCK_SIZE);
    write(m_outputDirFileId, &discInfo.usd, UDF_BLOCK_SIZE);
    write(m_outputDirFileId, &discInfo.terdes, UDF_BLOCK_SIZE);

    return 0;
}

tag_primary_vol_desc UdfAppender::BuildPrimaryVolumeDesc(int tagLocation)
{
    tag_primary_vol_desc pvd;

    memset(&pvd, 0, sizeof(tag_primary_vol_desc));
    pvd.descTag.tagIdent = 1;
    pvd.descTag.descVersion = 2;
    pvd.descTag.tagLocation = tagLocation;
    pvd.descTag.tagSerialNum = 0;
    pvd.descTag.reserved = 0;
    pvd.descTag.descCRCLength = 496;

    pvd.volDescSeqNum = 0;
    pvd.primaryVolDescNum = 0;
    memcpy(pvd.volIdent, m_volumeName, m_volumeNameLength);
    pvd.volIdent[31] = m_volumeNameLength;

    pvd.volSeqNum = 1;
    pvd.maxVolSeqNum = 1;
    pvd.interchangeLvl = 2;
    pvd.maxInterchangeLvl = 3;
    pvd.charSetList = 1;
    pvd.maxCharSetList = 1;

    std::string uuid = GenerateShortUUID();
    string total_set_volume_id = uuid + string(m_pVolSetIden);


    uint8_t encode_set_volume_id[256];
    int output_length = encode_utf8(encode_set_volume_id,total_set_volume_id.c_str(),256);
    memcpy(pvd.volSetIdent, encode_set_volume_id, output_length);
    pvd.volSetIdent[127] = output_length;


    pvd.descCharSet.charSetType = 0;
    memcpy(pvd.descCharSet.charSetInfo, p_desc_char_set_info, 63);

    pvd.explanatoryCharSet.charSetType = 0;
    memcpy(pvd.explanatoryCharSet.charSetInfo, p_desc_char_set_info, 63);

    pvd.volAbstract.extLength=0;
    pvd.volAbstract.extLocation=0;
    pvd.volCopyright.extLength=0;
    pvd.volCopyright.extLocation=0;

    pvd.appIdent.flags=0;

    pvd.recordingDateAndTime = m_burnDateAndTime;

    pvd.impIdent.flags = 0;
    memcpy(pvd.impIdent.ident, p_win_impIdent_ident, 23);
    pvd.impIdent.osClass = 6;
    pvd.impIdent.osIdentifier = 0;
    pvd.predecessorVolDescSeqLocation = 0;
    pvd.flags = 0;

    uint8_t *pBuff=(uint8_t *)(&pvd);
    DescriptorTagTest(pBuff);

    return pvd;
}

void UdfAppender::FillTime(time_stamp *pTime, timespec time_spec)
{
    if(pTime == NULL){
        return;
    }

    struct tm* time_info = localtime(&time_spec.tv_sec);
    pTime->typeAndTimezone = 0x1000;		// 12
    pTime->year = time_info->tm_year + 1900;
    pTime->month = (uint8_t)time_info->tm_mon + 1;
    pTime->day = (uint8_t)time_info->tm_mday;
    pTime->hour = (uint8_t)time_info->tm_hour;
    pTime->minute = (uint8_t)time_info->tm_min;
    pTime->second = (uint8_t)time_info->tm_sec;
    pTime->centiseconds=0;
    pTime->hundredsOfMicroseconds=0;
    pTime->microseconds=0;
}

uint16_t UdfAppender::udf_crc16(uint8_t *data, uint32_t size, uint16_t crc)
{
    while (size--){
        crc = crc_table[(crc >> 8 ^ *(data++)) & 0xffU] ^ (crc << 8);
    }
    return crc;
}

tag_imp_use_vol_desc UdfAppender::BuildImplementationDesc(int tagLocation)
{
    tag_imp_use_vol_desc impvd;
    //实现使用卷描述符
    memset(&impvd, 0, sizeof(tag_imp_use_vol_desc));

    impvd.descTag.tagIdent=4;
    impvd.descTag.descVersion=2;
    impvd.descTag.reserved=0;
    impvd.descTag.descCRCLength=496;
    impvd.descTag.tagLocation = tagLocation;
    impvd.descTag.tagSerialNum = 0;
    // 在卷描述符中为第1个
    impvd.volDescSeqNum=1;

    // 32 udfid
    impvd.impIdent.uFlags=0;
    memcpy(impvd.impIdent.aID,p_udf_impident,23);
    impvd.impIdent.uUDFRevision = 0x0102;
    impvd.impIdent.uOSClass = 6;
    impvd.impIdent.uOSIdentifier=0;

    // 64 charspec
    impvd.lviCharSet.charSetType = 0 ;
    memcpy(impvd.lviCharSet.charSetInfo,p_desc_char_set_info,63);

    // 128 dstring
    memcpy(impvd.logVolIdent, m_volumeName, m_volumeNameLength);
    impvd.logVolIdent[127] = m_volumeNameLength;

    // impvd.logVolInfo全为零
    // 32 impid
    impvd.umplementationID.flags=0;
    impvd.umplementationID.osClass=6;
    impvd.umplementationID.osIdentifier=0;
    memcpy(impvd.umplementationID.ident,p_win_impIdent_ident,23);

    //m_ImpUseVolDesc.umplementationUse全为零
    uint8_t *pBuff=(uint8_t *)(&impvd);
    DescriptorTagTest(pBuff);

    return impvd;
}

tag_partition_desc UdfAppender::BuildPartitionDesc(int tagLocation, int pd_start_loacation, int pd_length)
{
    //  分区描述符
    tag_partition_desc pd;
    memset(&pd,0,sizeof(tag_partition_desc));
    // 16 tag
    pd.descTag.tagIdent=5;
    pd.descTag.descVersion=2;
    pd.descTag.reserved=0;
    pd.descTag.tagSerialNum=0;
    pd.descTag.tagLocation= tagLocation;
    pd.descTag.descCRCLength=496;

    pd.volDescSeqNum = 3;
    pd.partitionFlags = 1;
    pd.partitionNumber=0;//
    //32 RegID
    pd.partitionContents.uFlags= 0;
    memcpy(pd.partitionContents.aID, p_partition_ident, 23);

    //pd.partitionContentsUse暂时全是0// 128 PARTITIONHEADERDESC
    pd.accessType=1; //  read only and  read umlimited
    pd.partitionStartingLocation = pd_start_loacation;

    pd.partitionLength = pd_length;
    pd.impIdent.flags=0;
    memcpy(pd.impIdent.ident, p_win_impIdent_ident, 23);
    pd.impIdent.osClass = 6;
    pd.impIdent.osIdentifier=0;

    uint8_t *pBuff=(uint8_t *)(&pd);
    DescriptorTagTest(pBuff);

    return pd;
}

tag_logical_vol_desc UdfAppender::BuildLogicalVolumeDesc(int tagLocation, int fsd_location, int intergrity_location)
{
    tag_logical_vol_desc lvd;
    // 逻辑卷描述符
    memset(&lvd,0,sizeof(tag_logical_vol_desc));

    // 16 tag
    lvd.descTag.tagIdent=6;
    lvd.descTag.descVersion=2;
    lvd.descTag.reserved=0;
    lvd.descTag.tagSerialNum=0;
    lvd.descTag.tagLocation=tagLocation;
    lvd.descTag.descCRCLength=430;
    uint8_t *pBuff=(uint8_t *)(&lvd);
    lvd.volDescSeqNum = 2;

    //64 charspec
    lvd.descCharSet.charSetType = 0;
    memcpy(lvd.descCharSet.charSetInfo, p_desc_char_set_info, 63);

    //128
    memcpy(lvd.logicalVolIdent, m_volumeName, m_volumeNameLength);
    lvd.logicalVolIdent[127] = m_volumeNameLength;

    //4 uint32
    lvd.logicalBlockSize=2048;

    //32 domainID
    lvd.domainIdent.uFlags=0;
    memcpy(lvd.domainIdent.aID,p_domain_ident,23);
    lvd.domainIdent.uUDFRevision=0x0102;
    lvd.domainIdent.domainFlag = 2;

    //16 long_ad
    lvd.fileSetDescSequExt.extLength=4096;  //文件集描述符占两个逻辑扇区
    lvd.fileSetDescSequExt.extLocation.logicalBlockNum = fsd_location;

    lvd.fileSetDescSequExt.extLocation.partitionReferenceNum=0;

    lvd.mapTableLength=6;  //仅一个分区
    lvd.numPartitionMaps=1;//仅一个分区

    //32 ImpID
    lvd.impIdent.flags=0;
    memcpy(lvd.impIdent.ident, p_win_impIdent_ident, 23);
    lvd.impIdent.osClass=6;
    lvd.impIdent.osIdentifier=0;

    lvd.integritySeqExt.extLength = 32768;
    lvd.integritySeqExt.extLocation= intergrity_location;

    lvd.p_partitionMaps.partitionMapType=1;
    lvd.p_partitionMaps.partitionMapLength=6;
    lvd.p_partitionMaps.volSequNo=1;
    lvd.p_partitionMaps.partitionNo=0;


    DescriptorTagTest(pBuff);

    return lvd;
}

tag_unallocated_space_desc UdfAppender::BuildUnAllocatedSpaceDesc(int tagLocation)
{
    // 未分配空间描述符
    tag_unallocated_space_desc usd;
    memset(&usd, 0, sizeof(tag_unallocated_space_desc));
    // 16 tag
    usd.descTag.tagIdent=7;
    usd.descTag.descVersion=2;
    usd.descTag.reserved=0;
    usd.descTag.tagSerialNum=0;
    usd.descTag.tagLocation=tagLocation;

    usd.volDescSeqNum=4;
    usd.numAllocDescs=0;

    //8* x extent ad
    usd.descTag.descCRCLength=8;

    uint8_t *pBuff=(uint8_t *)(&usd);
    DescriptorTagTest(pBuff);
    return usd;
}

tag_terminating_desc UdfAppender::BuildTerminatingDesc(int tagLocation)
{
    tag_terminating_desc terminateDesc;

    memset(&terminateDesc,0,2048);
    terminateDesc.descTag.tagIdent=8;
    terminateDesc.descTag.descVersion=2;
    terminateDesc.descTag.reserved=0;
    terminateDesc.descTag.tagSerialNum=0;
    terminateDesc.descTag.tagLocation= tagLocation;
    terminateDesc.descTag.descCRCLength=496;
    uint8_t *pBuff = (uint8_t *)(&terminateDesc);
    DescriptorTagTest(pBuff);

    return terminateDesc;
}

int UdfAppender::WriteReserveVolDescriSequ()
{
   return 0;
}

tag_logic_vol_integrity_desc UdfAppender::BuildLogVolIntegDesciSequ(int tagLocation, int file_count, int dir_count, int pd_length, int64_t unique_id)
{
    // 逻辑倦一致性描述符
    tag_logic_vol_integrity_desc lvid;
    memset(&lvid,0,sizeof(tag_logic_vol_integrity_desc));
    // 16 tag
    lvid.descTag.tagIdent=9;
    lvid.descTag.descVersion=2;
    lvid.descTag.reserved=0;
    lvid.descTag.tagSerialNum = 0;
    lvid.descTag.tagLocation=tagLocation;

    //12 timestamp
    //FillTime(&lvid.recordingDateAndTime);
    lvid.recordingDateAndTime = m_burnDateAndTime;

    //4 integrity type
    lvid.integrityType=1;

    //8 next integrity extent
    lvid.nextIntegrityExt.extLength=0;
    lvid.nextIntegrityExt.extLocation=0;

    //8 unique id
    lvid.uniqueId=unique_id;

    //24 uint8
    //lvid.reserved[24];
    //4 uint32
    lvid.numOfPartitions=1;
    //4 uint32
    lvid.lengthOfImpUse = 46; //如果不是46 下面也需要修改
    //4 * numOfPartitions
    lvid.freeSpaceTable=0;

    lvid.sizeTable = pd_length;

    lvid.p_impUse[0]=0;
    memcpy(lvid.p_impUse+1, p_win_impIdent_ident, 23);
    lvid.p_impUse[24]=6;
    lvid.p_impUse[25]=0;

    int FileCount = file_count;
    lvid.p_impUse[32] = (FileCount&0x000000ff);
    lvid.p_impUse[33] =(FileCount&0x0000ff00)>>8;
    lvid.p_impUse[34] =(FileCount&0x00ff0000)>>16;
    lvid.p_impUse[35] =(FileCount&0xff000000)>>24;

    int DirectorCount = dir_count;
    lvid.p_impUse[36] = ((DirectorCount)&0x000000ff);
    lvid.p_impUse[37] = ((DirectorCount)&0x0000ff00)>>8;
    lvid.p_impUse[38] = ((DirectorCount)&0x00ff0000)>>16;
    lvid.p_impUse[39] = ((DirectorCount)&0xff000000)>>24;

    lvid.p_impUse[40]=0x02;
    lvid.p_impUse[41]=0x01;

    lvid.p_impUse[42]=0x02;
    lvid.p_impUse[43]=0x01;

    lvid.p_impUse[44]=0x02;
    lvid.p_impUse[45]=0x01;

    lvid.descTag.descCRCLength=118; // 2

    uint8_t *pBuff=(uint8_t *)(&lvid);
    DescriptorTagTest(pBuff);

    return lvid;
}

tag_anchor_vol_desc_ptr UdfAppender::BuildAVDP(int tag_location,int main_vol_location, int res_vol_location)
{
    tag_anchor_vol_desc_ptr avdp;
    memset(&avdp, 0, sizeof(tag_anchor_vol_desc_ptr));

    avdp.descTag.tagIdent = 2;
    avdp.descTag.descVersion = 2;
    avdp.descTag.reserved = 0;
    avdp.descTag.tagSerialNum = 0;
    avdp.descTag.descCRCLength = 496;
    avdp.descTag.tagLocation = tag_location; //256 blockIndex

    avdp.mainVolDescSequExt.extLength = 16*2048;
    avdp.mainVolDescSequExt.extLocation = main_vol_location;

    avdp.ResVolDescSequExt.extLength = 16*2048;
    avdp.ResVolDescSequExt.extLocation = res_vol_location;

    uint8_t *pBuff=(uint8_t *)(&avdp);
    DescriptorTagTest(pBuff);

    return avdp;
}

tag_file_set_desc UdfAppender::BuildFileSetDesc(int tagLocation, int pd_starting_location)
{
    tag_file_set_desc fsd;
    // 文件集描述符
    memset(&fsd,0,sizeof(tag_file_set_desc));

    //16 tag
    fsd.descTag.tagIdent = 256;
    fsd.descTag.descVersion = 2;
    fsd.descTag.reserved = 0;
    fsd.descTag.tagSerialNum = 0;
    fsd.descTag.tagLocation= tagLocation - pd_starting_location;

    fsd.descTag.descCRCLength=496;

    //12 timestamp
    fsd.recordingDateAndTime = m_burnDateAndTime;

    fsd.interchangeLvl = 3;
    fsd.maxInterchangeLvl = 3;
    fsd.charSetList = 1;
    fsd.maxCharSetList = 1;
    fsd.fileSetNum = 0;
    fsd.fileSetDescNum = 0;

    //64 charspec
    fsd.logicalVolIdentCharSet.charSetType = 0;
    memcpy(fsd.logicalVolIdentCharSet.charSetInfo,p_desc_char_set_info,63);

    //128 dstring
    memcpy(fsd.logicalVolIdent, m_volumeName, m_volumeNameLength);
    fsd.logicalVolIdent[127] = m_volumeNameLength;

    //64 charspec
    fsd.fileSetCharSet.charSetType= 0;
    memcpy(fsd.fileSetCharSet.charSetInfo,p_desc_char_set_info,63);

    //32 dstring
    memcpy(fsd.fileSetIdent, m_volumeName, m_volumeNameLength);
    fsd.fileSetIdent[31] = m_volumeNameLength;

    fsd.p_reserved1[0]=0;
    fsd.p_reserved1[31]=0;
    fsd.p_reserved1[32]=0;
    fsd.p_reserved1[63]=0;

    // 16 long_ad
    fsd.rootDirectoryICB.extLength = 2048;
    fsd.rootDirectoryICB.extLocation.logicalBlockNum = tagLocation - pd_starting_location + 2;
    fsd.rootDirectoryICB.extLocation.partitionReferenceNum = 0;

    // 32 domainid
    fsd.domainIdent.uFlags=0;
    memcpy(fsd.domainIdent.aID, p_domain_ident,23);
    //fsd.domainIdent.aID[23] = 0x00;
    fsd.domainIdent.uUDFRevision= 0x0102;
    fsd.domainIdent.domainFlag = 2;

    uint8_t *pBuff = (uint8_t *)(&fsd);
    DescriptorTagTest(pBuff);

    return fsd;
}

void UdfAppender::BuildDirFileStruct(RecordDir *pFileDirAttr, FileNode* file_node)
{

    if((pFileDirAttr == NULL) || (file_node == NULL)){
        return;
    }

    tag_icb_param* pICB = new tag_icb_param;
    memset(pICB, 0, sizeof(tag_icb_param));

    pICB->tagLocation = pFileDirAttr->m_IcbSectorLoca;
    pICB->fileType = 4;
    pICB->informationLength = pFileDirAttr->m_SumInfoLen;

    int dir_logic_block_count = (pFileDirAttr->m_SumInfoLen + 2047) / 2048;
    pICB->logicalBlocksRecorded = dir_logic_block_count;
    pICB->fileLinkCount = pFileDirAttr->m_DirCount + 1;

    if(pFileDirAttr->m_IsRoot == true){
       pICB->uniqueID = 0;
       pFileDirAttr->m_last_access_time = m_systime;
       pFileDirAttr->m_last_modify_time = m_systime;
       pFileDirAttr->m_last_status_changed_time = m_systime;

    }else{
       pICB->uniqueID = m_IcbIndex++;
    }

    pICB->Allocationdescriptors.extLength = (int)pICB->informationLength;
    pICB->Allocationdescriptors.extPosition = pICB->tagLocation + 1;
    strcpy(file_node->m_parent_dir_path, pFileDirAttr->m_Path);
    strcpy(file_node->m_item_name, pFileDirAttr->m_Name);

    WriteFileIcb(pICB, file_node,pFileDirAttr);


    // FILE ID
    tag_file_id_info* pFileId = new tag_file_id_info;
    memset(pFileId,0,sizeof(tag_file_id_info));
    pFileId->tagLocation = pICB->tagLocation + 1;
    pFileId->lengthFileIdent = 0;
    pFileId->icb.extLength = 2048;
    pFileId->icb.extLocation.logicalBlockNum = 2;
    pFileId->descCRCLength = 24;
    pFileId->PadLength = 2;
    //parentdir + dir
    pFileId->fileCharacteristics = 0x0a;

    WriteFileId(pFileId,file_node);


    for(RecordFile* index : pFileDirAttr->m_SubFoldList){
        RecordDir* pTempRecord = (RecordDir*)index;
        pFileId->tagLocation = pICB->tagLocation + 1;

        uint8_t outputCS0_str[512];
        memset(outputCS0_str, 0x00, 512);
        int output_length = encode_utf8(outputCS0_str,pTempRecord->m_Name, 512);

        memcpy(pFileId->fileIdent, outputCS0_str, output_length);

        pFileId->icb.extLength = 2048;
        pFileId->icb.extLocation.logicalBlockNum = pTempRecord->m_IcbSectorLoca;
        pFileId->descCRCLength = pTempRecord->m_InfoLength - 16;// unicode


        if(pTempRecord->m_IsDir){
            //dir
            pFileId->fileCharacteristics = 0x02;
        } else {
            //file
            pFileId->fileCharacteristics = 0x00;
        }

        pFileId->lengthFileIdent = output_length;
        pFileId->PadLength = pTempRecord->m_InfoLength - 38 - pFileId->lengthFileIdent;
        WriteFileId(pFileId,file_node);
    }

    for(RecordFile* index : pFileDirAttr->m_SubFoldList){

        FileNode* next_file_node = new FileNode();
        file_node->m_sub_node_list.push_back(next_file_node);

        RecordDir* pTempRecord = (RecordDir*)index;
        if(pTempRecord->m_IsDir){
             BuildDirFileStruct(pTempRecord, next_file_node) ;
        }
        else{
             BuildFileStruct(pTempRecord, next_file_node) ;
        }
    }

    //release memory
    if(pICB){
        delete pICB;
        pICB = nullptr;
    }
    if(pFileId){
        delete pFileId;
        pFileId = nullptr;
    }
}

void UdfAppender::WriteFileIcb(tag_icb_param *pICB, FileNode* file_node, RecordDir* file_item)
{
    if((pICB == NULL) || (file_node == NULL)){
        return;
    }

    tag_file_entry file_entry;
    memset(&file_entry,0,sizeof(tag_file_entry));

    // 16 tag
    file_entry.descTag.tagIdent= 0x0105;
    file_entry.descTag.descVersion=2;
    file_entry.descTag.tagSerialNum= 0;
    file_entry.descTag.tagLocation = pICB->tagLocation;

    //  20 icb
    file_entry.icbTag.priorRecordedNumDirectEntries = 0;
    file_entry.icbTag.strategyType = 4;
    file_entry.icbTag.strategyParameter=0;
    file_entry.icbTag.numEntries= 1;
    file_entry.icbTag.reserved= 0;
    file_entry.icbTag.fileType= pICB->fileType;
    file_entry.icbTag.flags= 0;//0x0230;

    // 4 uint32
    file_entry.uid = 0xffffffff;

    // 4 uint32
    file_entry.gid = 0xffffffff;

    // 4 uint32
    file_entry.permissions= 0x35ad;
    // 2 uint16
    file_entry.fileLinkCount = pICB->fileLinkCount;
    //  8 uint64
    file_entry.informationLength = pICB->informationLength;
    file_entry.logicalBlocksRecorded = pICB->logicalBlocksRecorded;

    //  12 timestamp
    FillTime(&file_entry.accessTime, file_item->m_last_access_time);
    FillTime(&file_entry.modificationTime, file_item->m_last_modify_time);
    FillTime(&file_entry.attrmodifyTime, file_item->m_last_status_changed_time);

    //  4 uint32
    file_entry.checkpoint=1;

    //  32 IMPID
    file_entry.impIdent.flags=0;
    memcpy(file_entry.impIdent.ident,p_win_impIdent_ident,23);
    file_entry.impIdent.osClass= 0x06;
    file_entry.impIdent.osIdentifier=0;
    file_entry.uniqueID = pICB->uniqueID;


    uint32_t max_part_count = pICB->Allocationdescriptors.extLength / MAX_SINGLE_PART;
    uint32_t left_part_value = pICB->Allocationdescriptors.extLength % MAX_SINGLE_PART;
    if(left_part_value > 0){
        max_part_count += 1;
    }
    file_entry.LengthofAllocationDescriptors= 8*max_part_count;

    uint32_t allocat_start_location = pICB->Allocationdescriptors.extPosition;
    uint32_t total_file_size = pICB->Allocationdescriptors.extLength;
    int part_index = 0;

    while((total_file_size > 0) && (part_index < 10)){

        file_entry.Allocationdescriptors[part_index].extPosition = allocat_start_location;
        if(total_file_size > MAX_SINGLE_PART){
             file_entry.Allocationdescriptors[part_index].extLength = MAX_SINGLE_PART;
             total_file_size -= MAX_SINGLE_PART;
             allocat_start_location += (MAX_SINGLE_PART / UDF_BLOCK_SIZE);
             ++part_index;

        }else{
            file_entry.Allocationdescriptors[part_index].extLength = total_file_size;
            total_file_size = 0;
        }

    }

    file_entry.descTag.descCRCLength = 160 + 8*max_part_count;
    uint8_t *pBuff=(uint8_t*)(&file_entry);
    DescriptorTagTest(pBuff);
    file_node->m_file_entry = file_entry;
}

void UdfAppender::WriteFileId(tag_file_id_info *pFileId, FileNode* file_node)
{
    if((pFileId == NULL) || (file_node == NULL)){
        return;
    }

    tag_file_indent_desc* p_file_ident_desc = new tag_file_indent_desc;
    memset(p_file_ident_desc, 0, sizeof(tag_file_indent_desc));

    p_file_ident_desc->descTag.tagIdent = 0x0101;
    p_file_ident_desc->descTag.descVersion = 2;
    p_file_ident_desc->descTag.reserved = 0;
    p_file_ident_desc->descTag.tagSerialNum = 0;
    p_file_ident_desc->descTag.tagLocation = pFileId->tagLocation;

    // 2 uint16
    p_file_ident_desc->fileVersionNum = 1;

    // 1 uint8
    p_file_ident_desc->fileCharacteristics = pFileId->fileCharacteristics;

    // uint 8
    p_file_ident_desc->lengthFileIdent = pFileId->lengthFileIdent;

    memcpy(&p_file_ident_desc->icb, &pFileId->icb, sizeof(tag_long_ad));

    p_file_ident_desc->p_padding = new uint8_t[pFileId->PadLength];
    memset(p_file_ident_desc->p_padding, 0, pFileId->PadLength);
    p_file_ident_desc->descTag.descCRCLength = pFileId->descCRCLength;

    p_file_ident_desc->icb.impUse[2] = 17;


    full_file_identifier full_file_id;
    full_file_id.file_indent_desc = *p_file_ident_desc;
    memcpy(full_file_id.filename, pFileId->fileIdent, pFileId->lengthFileIdent);
    char* result = _cs0_to_mutf8(pFileId->fileIdent,pFileId->lengthFileIdent);
    strcpy(full_file_id.filename_utf8, result);
    free(result);

    full_file_id.filename_length = pFileId->lengthFileIdent;
    full_file_id.padd_length = pFileId->PadLength;

    file_node->m_fileid_list.push_back(full_file_id);

    if(p_file_ident_desc){
        delete p_file_ident_desc;
        p_file_ident_desc = nullptr;
    }
}

void UdfAppender::BuildFileStruct(RecordDir *pFileDirAttr, FileNode* file_node)
{
    if((pFileDirAttr == NULL) || (file_node == NULL)){
        return;
    }

    tag_icb_param* pICB = new tag_icb_param;
    memset(pICB,0,sizeof(tag_icb_param));

    pICB->tagLocation = pFileDirAttr->m_IcbSectorLoca;

    pICB->fileType=5;
    pICB->informationLength = pFileDirAttr->m_FileSize;
    pICB->logicalBlocksRecorded = pFileDirAttr->m_FileBlockSize/2048;
    pICB->fileLinkCount = 1;
    pICB->uniqueID = m_IcbIndex++;
    pICB->Allocationdescriptors.extLength = (uint32_t)pICB->informationLength;
    pICB->Allocationdescriptors.extPosition = m_FileIcbAllocLocation;

    m_FileIcbAllocLocation += pFileDirAttr->m_FileBlockSize/ 2048;

    strcpy(file_node->m_parent_dir_path, pFileDirAttr->m_Path);
    strcpy(file_node->m_item_name, pFileDirAttr->m_Name);

    WriteFileIcb(pICB,file_node, pFileDirAttr);
}

void UdfAppender::FillData(RecordDir *pFileDirAttr)
{
    uint8_t TempBuffer[2048];
    char ReadedFileName[1024];

    memset(ReadedFileName, 0, 1024);
    memset(TempBuffer, 0, 2048);

    int  inputFileid = -1;
    for(RecordFile* index : pFileDirAttr->m_SubFoldList){

        RecordDir* pTempRecord = (RecordDir*)index;

        if(pTempRecord->m_IsDir){
             FillData(pTempRecord);
         }else{
             //open file
             strcpy(ReadedFileName, pTempRecord->m_Path);
             char* FileName = strcat(ReadedFileName, pTempRecord->m_Name);
             inputFileid = open(FileName, O_RDONLY);
             if(inputFileid < 0){
                 break;
             }
             for(uint32_t Index=0; Index < pTempRecord->m_FileBlockSize / 2048; Index++){
                   //readfile
                   memset(TempBuffer, 0x00, 2048);
                   read(inputFileid, TempBuffer, 2048);
                   write(m_outputDirFileId, TempBuffer, 2048);
             }
             close(inputFileid);
         }
    }
}

int UdfAppender::WriteAppendFileData(FileNode *file_node, int pd_offset)
{
    if(file_node == NULL){
        return -1;
    }
    std::queue<FileNode*> node_queue;
    node_queue.push(file_node);

    while(!node_queue.empty()){
        FileNode* currentNode = node_queue.front();
        node_queue.pop();

        if(currentNode->m_file_entry.icbTag.fileType == 5){

            uint8_t data_buffer[2048];
            char full_read_filename[1024];

            memset(data_buffer, 0, 2048);
            memset(full_read_filename, 0, 1024);


            //open file
            strcpy(full_read_filename, currentNode->m_parent_dir_path);
            char* FileName = strcat(full_read_filename, currentNode->m_item_name);
            int input_file_id = open(FileName, O_RDONLY);
            if(input_file_id < 0){
                return -2;
            }

            int pd_position = currentNode->m_file_entry.Allocationdescriptors[0].extPosition;
            printf("%s pd_location:%d real_location:%d file_size:%ld block_count:%ld \n",full_read_filename,
                   pd_position, pd_offset + pd_position, currentNode->m_file_entry.informationLength, currentNode->m_file_entry.logicalBlocksRecorded);

            lseek(m_outputDirFileId, (pd_offset + pd_position) *UDF_BLOCK_SIZE, SEEK_SET);

            for(uint32_t Index=0; Index < currentNode->m_file_entry.logicalBlocksRecorded; Index++){
                //readfile
                memset(data_buffer, 0x00, 2048);
                int read_size = read(input_file_id, data_buffer, 2048);
                if(read_size <= 0){
                    printf("read file %s error blockIndex: %d",full_read_filename,Index);
                    break;
                }
                write(m_outputDirFileId, data_buffer, 2048);
            }
            close(input_file_id);
        }

        for(auto sub_node : currentNode->m_sub_node_list){
            node_queue.push(sub_node);
        }
    }

    lseek(m_outputDirFileId,0x00, SEEK_END);
    return 0;
}


bool UdfAppender::GetFileDirAttr(RecordDir *pFileDirAttr)
{
    int TempInfoLength = 0;
    RecordDir* indexRecord;

    //parentdir
    char  parentDirPath[512];
    memset(parentDirPath, 0, 512);
    strcpy(parentDirPath, pFileDirAttr->m_Path);

    //fullpath
    char fileDirPath[512];
    memset(fileDirPath, 0x00, 512);
    strcpy(fileDirPath, parentDirPath);
    strcat(fileDirPath, pFileDirAttr->m_Name);

    //open path
    DIR* dir = opendir(fileDirPath);
    if(dir == NULL){
        return false;
    }

    struct dirent* entry;
    struct stat statbuf;

    pFileDirAttr->m_SumInfoLen = 40;  //p_file_info(38) + pad_length(2)

   //read child item
   while((entry = readdir(dir)) != NULL){
        //filter . and ..
        if(strcmp(entry->d_name, ".") == 0  ||  strcmp(entry->d_name, "..") == 0){
            continue;
        }

        indexRecord = new RecordDir;
        strcpy(indexRecord->m_Name, entry->d_name);

        //parent dir end with /
        strcat(fileDirPath, "/");
        strcpy(indexRecord->m_Path, fileDirPath);

        uint8_t outputCS0_str[512];
        memset(outputCS0_str, 0x00, 512);
        int output_length = encode_utf8(outputCS0_str, entry->d_name, 512);

        //文件目录信息的长度
        TempInfoLength = output_length + 38;
        TempInfoLength = 4 * ((TempInfoLength + 3) / 4);

        indexRecord->m_InfoLength = TempInfoLength;
        pFileDirAttr->m_SumInfoLen += TempInfoLength;
        indexRecord->m_IcbSectorLoca = m_FileIcbLocation;


        char file_full_path[1024];
        memset(file_full_path, 0x00, 1024);
        strcpy(file_full_path, fileDirPath);
        strcat(file_full_path, entry->d_name);


        if(entry->d_type & DT_DIR){
            indexRecord->m_IsDir = true;
            pFileDirAttr->m_DirCount++;
            pFileDirAttr->m_DirSum++;

            int sub_list_block_count = GetCurrentDirLength(indexRecord);
            m_FileIcbLocation += (1 + sub_list_block_count);
            m_PartitionLength += (1 + sub_list_block_count);

            if(stat(file_full_path, &statbuf) == 0){
                indexRecord->m_last_access_time = statbuf.st_atim;          //最后访问时间
                indexRecord->m_last_modify_time = statbuf.st_mtim;          //最后修改时间
                indexRecord->m_last_status_changed_time = statbuf.st_ctim;  //最后状态变更时间
            }

            GetFileDirAttr(indexRecord);

        } else{
            m_FileIcbLocation += 1;
            m_PartitionLength += 1;

            if(stat(file_full_path, &statbuf) == 0){
                indexRecord->m_FileSize = statbuf.st_size;
                indexRecord->m_FileBlockSize = statbuf.st_size;

                indexRecord->m_last_access_time = statbuf.st_atim;          //最后访问时间
                indexRecord->m_last_modify_time = statbuf.st_mtim;          //最后修改时间
                indexRecord->m_last_status_changed_time = statbuf.st_ctim;  //最后状态变更时间
            }

            pFileDirAttr->m_FileCount++;
            pFileDirAttr->m_FileSum++;

            //对齐文件块
            if(indexRecord->m_FileSize % 2048 != 0){
                indexRecord->m_FileBlockSize = 2048 * ((indexRecord->m_FileSize + 2047) / 2048);
            }

            m_PartitionLength += indexRecord->m_FileBlockSize / 2048;
        }

        //所有文件和目录的总数
        pFileDirAttr->m_DirSum += indexRecord->m_DirSum;
        pFileDirAttr->m_FileSum += indexRecord->m_FileSum;


        //追加到总的文件大小中
        pFileDirAttr->m_FileSize += indexRecord->m_FileSize;
        //添加到子文件中
        pFileDirAttr->m_SubFoldList.push_back(indexRecord);
      }
      closedir(dir);
      return true;
}

int UdfAppender::GetCurrentDirLength(RecordDir *pFileDirAttr)
{
    int TempInfoLength = 0;

    //parentdir
    char  parentDirPath[512];
    memset(parentDirPath, 0, 512);
    strcpy(parentDirPath, pFileDirAttr->m_Path);

    //fullpath
    char fileDirPath[512];
    memset(fileDirPath, 0x00, 512);
    strcpy(fileDirPath, parentDirPath);
    strcat(fileDirPath, pFileDirAttr->m_Name);

    //open path
    DIR* dir = opendir(fileDirPath);
    if(dir == NULL){
        return -1;
    }

    struct dirent* entry;

    //root length
    int root_sum_length = 40;

   //read child item
   while((entry = readdir(dir)) != NULL){
        //filter . and ..
        if(strcmp(entry->d_name, ".") == 0  ||  strcmp(entry->d_name, "..") == 0){
            continue;
        }

        uint8_t outputCS0_str[512];
        memset(outputCS0_str, 0x00, 512);
        int output_length = encode_utf8(outputCS0_str, entry->d_name, 512);

        //文件目录信息的长度
        TempInfoLength = output_length + 38;
        TempInfoLength = 4 * ((TempInfoLength + 3) / 4);
        root_sum_length += TempInfoLength;
    }
    closedir(dir);
    int blockCount = (root_sum_length + 2047) / 2048;
    return blockCount;

}

int UdfAppender::WriteFileTreeInfo(FileNode *file_node)
{
    if(file_node == NULL){
        return -1;
    }
    std::queue<FileNode*> node_queue;
    node_queue.push(file_node);

    while(!node_queue.empty()){
        FileNode* currentNode = node_queue.front();
        node_queue.pop();

        WriteFileNodeToFile(currentNode);

        for(auto sub_node : currentNode->m_sub_node_list){
            node_queue.push(sub_node);
        }
    }
    return 0;
}

int UdfAppender::CalcFileTreeBlockLength(FileNode *file_node)
{
    int sum_block_length = 0;

    if(file_node->m_fileid_list.size()== 0){
        sum_block_length += 1;
    }else if(file_node->m_fileid_list.size() > 0){
        sum_block_length += 1;
        sum_block_length += file_node->m_file_entry.logicalBlocksRecorded;
    }

    int sub_dir_count = file_node->m_sub_node_list.size();
    for(int index=0; index<sub_dir_count; ++index){
        sum_block_length += CalcFileTreeBlockLength(file_node->m_sub_node_list[index]);
    }

    return sum_block_length;
}

void UdfAppender::DescriptorTagTest(uint8_t *pBuff)
{
    memset(m_pCrcBuffer, 0x00, 2048);
    descriptor_tag*pTag = (descriptor_tag*)pBuff;
    memcpy(m_pCrcBuffer, pBuff+sizeof(descriptor_tag), pTag->descCRCLength);
    pTag->descCRC = udf_crc16( m_pCrcBuffer,pTag->descCRCLength,0);

    uint8_t  CheckSum;
    CheckSum = pBuff[0] + pBuff[1]+ pBuff[2]+
                 pBuff[3] + pBuff[5]+ pBuff[6]+
                 pBuff[7] + pBuff[8]+ pBuff[9]+
                 pBuff[10]+pBuff[11]+pBuff[12]+
                 pBuff[13]+pBuff[14]+pBuff[15];

    pTag->tagChecksum = CheckSum % 256;/**/
}

void UdfAppender::WriteEndAvdpInfo(int avdp_count)
{
    tag_anchor_vol_desc_ptr temp_avdp = m_avdp;
    int start_block_index = m_avdp.ResVolDescSequExt.extLocation +  6;
    for(int index = 0; index<avdp_count; ++index){
        temp_avdp.descTag.tagLocation = index + start_block_index;
        uint8_t *pBuff=(uint8_t *)(&temp_avdp);
        DescriptorTagTest(pBuff);
        write(m_outputDirFileId, &temp_avdp, UDF_BLOCK_SIZE);
    }

}

std::string UdfAppender::GenerateShortUUID() {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static const int num_alphanum = sizeof(alphanum) - 1;
    std::string uuid;
    std::random_device rd;  // 获取随机数生成器
    std::mt19937 rng(rd());  // 以随机数种子初始化Mersenne Twister生成器
    std::uniform_int_distribution<int> dist(0, num_alphanum - 1); // 分布范围
    for (int i = 0; i < 8; ++i) { // 生成8个字符的UUID
        uuid += alphanum[dist(rng)];
    }
    return uuid;
}
