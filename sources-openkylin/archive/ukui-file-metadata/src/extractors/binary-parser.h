/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H
#include <QtCore>
#include <QtConcurrent/QtConcurrent>

#define ULONG unsigned long
#define UCHAR unsigned char
#define USHORT unsigned short

typedef enum {
    Word = 0,
    Excel,
    Ppt
} TYPE;

/* Property Set Storage */
typedef struct pps_tag {
    ULONG	ulSB;
    ULONG	ulSize;
} ppsTag;

typedef struct pps_info_tag {
    ppsTag	tWordDocument;	/* Text stream */
    ppsTag	tWorkBook;
    ppsTag	tPPTDocument;
    ppsTag	tData;		/* Data stream */
    ppsTag	tTable;		/* Table stream */
    ppsTag	tSummaryInfo;	/* Summary Information */
    ppsTag	tDocSummaryInfo;/* Document Summary Information */
    ppsTag	t0Table;	/* Table 0 stream */
    ppsTag	t1Table;	/* Table 1 stream */
    ppsTag	tCurrentUser;
    TYPE		type;
} ppsInfoType;

/* Private type for Property Set Storage entries */
typedef struct pps_entry_tag {
    ULONG	ulNext;
    ULONG	ulPrevious;
    ULONG	ulDir;
    ULONG	ulSB;
    ULONG	ulSize;
    int	iLevel;
    char	szName[32];
    UCHAR	ucType;
} ppsEntryType;

/* SST 字符串的可选头字段（BIFF8 rgb 之后的 cRun / cbExtRst）。
 * fHighByte 只表示字符数组的存储宽度，读取时已归一化为 UTF-16。 */
struct SstStringMetadata {
    ushort richRunCount = 0;             // cRun：每项占 4 字节
    ulong extendedDataByteCount = 0;     // cbExtRst
};

typedef struct readDataParam {
    readDataParam() {
        ulStBlk = 0;
        pFile = NULL;
        ulBBd = NULL;
        tBBdLen = 0;
        usBlkSize = 0;
    }
    ulong ulStBlk;
    FILE *pFile;
    ulong *ulBBd;
    size_t tBBdLen;
    ushort usBlkSize;
} rdPara;

class KBinaryParser : public QObject {
    Q_OBJECT
public:
    KBinaryParser(QObject *parent = 0);
    ~KBinaryParser();

public:
    bool RunParser(QString strFile, QString &content);

private:
    // Word 文本流中的 legacy 控制字符。部分 fSpec 特殊字符需要配合字符属性判断；
    // 当前解析器只按字符值做保守归一化。
    static constexpr ushort kWordCharInlinePicture  = 0x0001; // 内联图片 / 嵌入对象
    static constexpr ushort kWordCharTableCellMark  = 0x0007; // 表格单元格 / 行结束标记
    static constexpr ushort kWordCharFloatingObject  = 0x0008; // 浮动对象锚点
    static constexpr ushort kWordCharColumnBreak     = 0x000E; // 分栏符
    static constexpr ushort kWordCharFieldBegin      = 0x0013; // 域开始
    static constexpr ushort kWordCharFieldSeparator  = 0x0014; // 域分隔（代码 | 结果）
    static constexpr ushort kWordCharFieldEnd        = 0x0015; // 域结束
    static constexpr ushort kWordCharNonBreakingHyphen = 0x001E; // 不换行连字符
    static constexpr ushort kWordCharOptionalHyphen    = 0x001F; // 可选连字符

    // 1 = 该层 field 仍在代码段，0 = 已进入结果段
    using WordFieldStateStack = QVarLengthArray<uchar, 8>;

    // 归一化 Word 文本（删除结构字符 + 处理域），状态需跨 piece 传递
    static void normalizeWordText(const QString &text,
                                  WordFieldStateStack &fieldStack,
                                  int &codeDepth,
                                  QString &out);

    bool bGetPPS(FILE *pFile,
                 const ULONG *aulRootList, size_t tRootListLen, ppsInfoType *pPPS);

    int readData(rdPara &readParam, uchar *aucBuffer, ulong ulOffset, size_t tToRead);

    int InitDocOle(FILE *pFile, long lFilesize, QString &content);
    bool read8DocText(FILE *pFile, const ppsInfoType *pPPS,
                      const ULONG *aulBBD, size_t tBBDLen,
                      const ULONG *aulSBD, size_t tSBDLen,
                      const UCHAR *aucHeader, QString &content);

    // 解析 BIFF8 Workbook 流中的 SST 及其尾随 CONTINUE，并将字符串追加
    // 到 content。true 表示未遇到结构错误；达到内容提取上限而有意截断也
    // 返回 true。false 表示 SST/CONTINUE 无效。调用方在两种结果下都会
    // 停止 Workbook 扫描，返回值只用于区分是否需要报告解析警告。
    bool readSSTRecord(readDataParam &rdParam, ppsInfoType, ulong &ulOff, ushort usPartLen, QString &content);

    // tContainerEnd 是当前容器在 PPT 流中的结束偏移；
    // recursionDepth 由内部递归调用累加，外层入口使用默认值 0。
    ULONG readPPtRecord(FILE* pFile, ppsInfoType* PPS_info, ULONG* aulBBD,
                        size_t tBBDLen, ULONG ulPos, QString &content,
                        size_t tContainerEnd, size_t recursionDepth = 0);

    QString m_strFileName;
    bool m_pptEnterSlideMaster = false;
};

#endif // SEARCHHELPER_H
