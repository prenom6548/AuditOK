#include "documentreader.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QTemporaryFile>

DocumentReader::DocumentReader(QObject *parent)
    : QObject{parent}
{
    // PDF 文本提取使用 pdftotext 命令行工具，无需预创建对象
}

DocumentReader::~DocumentReader()
{
    // PDF 文本提取使用 pdftotext 命令行工具，无资源需要释放
}

QString DocumentReader::getAllText(const QString filePath)
{
    KyInfo() << "Start to get all text : " << filePath;

    if (filePath.isEmpty()) {
        KyInfo() << "File path is empty.";
        return QString();
    }

    QFileInfo fileInfo(filePath);
    QMimeDatabase mimeData;
    QString mimeType = mimeData.mimeTypeForFile(fileInfo).name();

    KyInfo() << "Mime type is " << mimeType;
    if (mimeType == "application/pdf") {
        return getAllTextForPdf(filePath);
    }

    if (mimeType == "text/plain") {
        return getAllTextForPlainText(filePath);
    }

    if ((mimeType == "application/wps-office.docx") ||
        (mimeType == "application/wps-office.wps")  ||
        (mimeType == "application/wps-office.doc")  ||
        (mimeType == "application/wps-office.pptx") ||
        (mimeType == "application/wps-office.ppt")  ||
        (mimeType == "application/vnd.openxmlformats-officedocument.presentationml.presentation") ||
        (mimeType == "application/vnd.ms-powerpoint") ||
        (mimeType == "application/vnd.ms-works") ||
        (mimeType == "application/msword") ||
        (mimeType == "application/vnd.openxmlformats-officedocument.wordprocessingml.document") ||
        (mimeType == "application/x-ole-storage") ){
        return getAllTextForOfficeFile(filePath);
    }

    // OFD格式单独处理,格式为zip,后缀为ofd
    if ((mimeType == "application/ofd") || ((mimeType == "application/zip") && QFileInfo(filePath).suffix().toLower() == "ofd")){
        return getAllTextForOfficeFile(filePath);
    }

    return QString();
}

QString DocumentReader::getAllTextForPdf(const QString filePath)
{
    KyInfo() << "Start to get all pdf text : " << filePath;

#ifdef PDF_SUPPORT_ENABLED
    // 使用 pdftotext 命令行工具提取 PDF 文本
    // pdftotext 是 poppler-utils 包的一部分

    // 创建临时文件存储提取的文本
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        KyInfo() << "Failed to create temporary file for PDF text extraction";
        return QString();
    }
    QString tempFileName = tempFile.fileName();
    tempFile.close();

    // 调用 pdftotext 命令
    QProcess process;
    QStringList args;
    args << "-layout" << filePath << tempFileName;

    process.execute("pdftotext", args);

    if (process.exitCode() != 0) {
        KyInfo() << "pdftotext failed with exit code: " << process.exitCode();
        return QString();
    }

    // 读取提取的文本
    QFile resultFile(tempFileName);
    if (!resultFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        KyInfo() << "Failed to open temporary file for reading PDF text";
        return QString();
    }

    QTextStream in(&resultFile);
    QString allText = in.readAll();
    resultFile.close();

    // 删除临时文件
    QFile::remove(tempFileName);

    KyInfo() << "End to get all pdf text : " << filePath;
    return allText;
#else
    // 不支持的架构下不支持PDF文件读取，文档问答功能不支持PDF文件
    KyInfo() << "PDF support is disabled for this architecture. Document Q&A does not support PDF files.";
    return QString();
#endif
}

QString DocumentReader::getAllTextForPlainText(const QString filePath)
{
    KyInfo() << "Start to get all plain text : " << filePath;

    QFile plainFile(filePath);
    bool ret = plainFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!ret) {
        KyInfo() << "Cannot to open plain text : " << filePath;
        return QString();
    }

    QTextStream textSteam(&plainFile);
    QString allText;
    while (!textSteam.atEnd()) {
        QString line = textSteam.readLine();
        allText.append(line);
        if (!textSteam.atEnd()) {
            allText.append("\n");
        }
    }

    KyInfo() << "End to get all plain text : " << filePath;

    return allText;
}

QString DocumentReader::getAllTextForOfficeFile(const QString filePath)
{
    KyInfo() << "Start to get all text for office file : " << filePath;

    QString allText;
    QString mimetype = UkuiFileMetadata::MimeUtils::strictMimeType(filePath, {}).name();

    UkuiFileMetadata::ExtractorManager extractors;
    QList<UkuiFileMetadata::Extractor*> exList = extractors.fetchExtractors(mimetype);

    for (UkuiFileMetadata::Extractor *ex : exList) {
        UkuiFileMetadata::SimpleExtractionResult result(filePath, mimetype, UkuiFileMetadata::ExtractionResult::ExtractPlainText);
        ex->extract(&result);
        allText.append(result.text());
    }

    KyInfo() << "End to get all text for office file : " << filePath;
    return allText;
}
