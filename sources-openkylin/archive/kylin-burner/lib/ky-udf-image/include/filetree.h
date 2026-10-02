/******************
根目录
├── 目录1
│   ├── 目录3
│   │   └── 文件4
│   └── 文件2
├── 目录2
│   ├── 目录4
│   └── 文件3
└── 文件1

File Entry(ECMA 167 4/14.17)
在UDF文件系统中用于存储文件或目录的元数据信息

根目录 file_entry
根目录 file_id_info
目录1  file_id_info
目录2 file_id_info
文件1 file_id_info

        目录1 file_entry
        目录1 file_id_info
        目录3 file_id_info
        文件2 file_id_info

            目录3 file_entry
            目录3 file_id_info
            文件4 file_id_info

                文件4 file_entry

            文件2 file_entry

        目录2 file_entry
        目录2 file_id_info
        目录4 file_id_info
        文件3 file_id_info

            目录4 file_entry
            目录4 file_id_info

            文件3 file_entry

        文件1 file_entry

文件4 content
文件2 content
文件3 content
文件1 content
******************/



/****************
file_entry  2048 字节
*****************
parent dir file-id 40 字节 (38 Info + 2 padding)
子目录1  file-id     38Info + cs0编码文件名长度  + 4字节padding
子目录2  file-id     38Info + cs0编码文件名长度  + 4字节padding
子文件1  file-id     38Info + cs0编码文件名长度  + 4字节padding
padding  2048 - 上面的信息和
****************/




/*****************
 *
 * fsd.root_icb  --->  root file_entry   ----> file_list(icb filename characteristic)
 *
 * *************/


/**********************************
    UDF镜像的逻辑结构
    author:yff
    0~15    zero
    16		extend sequence start descriptor tag
    17		NSR descriptor tag
    18		extend sequence end descriptor tag
    19~31	zero
    32		基本卷描述符
    33		实现使用卷描述符
    34		分区描述符
    35		逻辑卷描述符  LVD
    36		未分配空间描述符
    37		终止描述符
    38~47	zero
    48		基本卷描述符
    49		实现使用卷描述符
    50		分区描述符
    51		逻辑卷描述符
    52		未分配空间描述符
    53		终止描述符
    54~63	zero
    64		逻辑倦一致性描述符
    65		终止描述符
    66~255	zero
    256		卷描述符定位指针
    257~261 zero
    262		文件集描述符 fsd       ------>pd start location(262) + root icb offset2
    263     终止描述符
    264     root_dir file_entry
    265     root_sub file_id
    icb     文件信息
    ----
    文件比特流
    最后逻辑扇区	卷描述符定位指针
************************************/


/***************************
 * LVD --> fsd location offset index;
 * pd.startBlock + fsd offset index = fsd location
 * fsd(rooticb) --> root icb block offset --> pd.start + rooticbblockoffset = root file_entry addr
 *file_entry dir --> sud dir/file list icb
 *
 *
 * pd start pos--------
 * offset 0 fsd
 * offset 1 terminated
 * offset 2 root file_entry
 * offset 3 file list info(filename)
 * offset 4 sub1 file_entry
 * offset 5 sub1 list_info(filename)
 * offset 6 sub2 file_entry
 * offset 7 sub1 list_info(filename)
 * offset 8 subfile file_entry
 * offset 9 blank info
 *
 * file binary
 *************************/




























