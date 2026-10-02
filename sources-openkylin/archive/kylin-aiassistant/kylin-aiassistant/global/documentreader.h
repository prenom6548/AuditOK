#ifndef DOCUMENTREADER_H
#define DOCUMENTREADER_H

#include <QObject>
#include <ukui-log4qt.h>
#include <QMimeDatabase>
#include <QFileInfo>
#include <ukui-file-metadata/extractor.h>
#include <ukui-file-metadata/extractor-manager.h>
#include <ukui-file-metadata/property-info.h>
#include <ukui-file-metadata/simple-extraction-result.h>
#include <ukui-file-metadata/mime-utils.h>

// 使用 pdftotext 命令行工具提取 PDF 文本
// 通过 QProcess 调用系统命令，无需包含任何 PDF 库头文件

class DocumentReader : public QObject
{
    Q_OBJECT
public:
    explicit DocumentReader(QObject *parent = nullptr);
    ~DocumentReader();
    // 获取文件的全部文本内容，异常情况返回空的QString对象
    QString getAllText(const QString filePath);

signals:
private:
    QString getAllTextForPdf(const QString filePath);
    QString getAllTextForPlainText(const QString filePath);
    QString getAllTextForOfficeFile(const QString filePath);
};

#endif // DOCUMENTREADER_H
