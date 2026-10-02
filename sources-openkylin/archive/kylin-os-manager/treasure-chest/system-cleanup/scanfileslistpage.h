#ifndef SCANFILESLISTPAGE_H
#define SCANFILESLISTPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QCheckBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QMenu>
#include "component/text_label.h"
#include "tableheaderview.h"

namespace systemcleanup
{
class CustomTreeWidgetItem : public QTreeWidgetItem {
public:
explicit CustomTreeWidgetItem(QTreeWidget *parent = nullptr) : QTreeWidgetItem(parent) {}
explicit CustomTreeWidgetItem(const QStringList &strings, int type = Type) : QTreeWidgetItem(strings,type) {}
bool operator<(const QTreeWidgetItem &other) const override {
    int column = treeWidget()->sortColumn();
    QString selfText = text(column);
    QString otherText = other.text(column);
    if (selfText.contains(QRegExp("MB|KB|GB|B$", Qt::CaseInsensitive)) && otherText.contains(QRegExp("MB|KB|GB|B$", Qt::CaseInsensitive))) {
        return compareFileSizes(selfText, otherText) < 0;
    }
    return selfText < otherText;
}
private:
int compareFileSizes(const QString& size1, const QString& size2) const {
    qulonglong bytes1 = parseFileSize(size1);
    qulonglong bytes2 = parseFileSize(size2);
    return (bytes1 < bytes2) ? -1 : (bytes1 > bytes2) ? 1 : 0;
}
qulonglong parseFileSize(const QString& sizeStr) const {
    QLocale locale;
    bool ok;
    double value;
    if (sizeStr.endsWith("GB", Qt::CaseInsensitive)) {
        value = locale.toDouble(sizeStr.left(sizeStr.length() - 2), &ok);
        return ok ? value * 1024 * 1024 * 1024 : 0;
    } else if (sizeStr.endsWith("MB", Qt::CaseInsensitive)) {
        value = locale.toDouble(sizeStr.left(sizeStr.length() - 2), &ok);
        return ok ? value * 1024 * 1024 : 0;
    } else if (sizeStr.endsWith("KB", Qt::CaseInsensitive)){
        value = locale.toDouble(sizeStr.left(sizeStr.length() - 2), &ok);
        return ok ? value * 1024 : 0;
    } else if (sizeStr.endsWith("B", Qt::CaseInsensitive)){
        value = locale.toDouble(sizeStr.left(sizeStr.length() - 1), &ok);
        return ok ? value : 0;
    } else {
        value = locale.toDouble(sizeStr, &ok);
        return ok ? value : 0;
    }
}
};

class ScanFilesListPage : public QWidget
{
    Q_OBJECT

public:
    ScanFilesListPage(QWidget *parent = nullptr, bool showMd5 = false);
    ~ScanFilesListPage();
    void addTableWidgetItem(QString path, QByteArray md5 = "");
    void clearTableWidget();
    void sortTableWidget(int col, bool sortUpToDown = true);

private:
    K::TextLabel *m_labelSelect = nullptr; // 已选
    K::TextLabel *m_labelSelectSize = nullptr; // 1.88G(#F68C27)
    K::TextLabel *m_labelTotalSize = nullptr; // / 3.95G
    K::TextLabel *m_labelInfo = nullptr; // 可能包含重要文件，请谨慎删除 （#8C8C8C）

    QTreeWidget *m_tableWidget = nullptr;
    QHeaderView *m_headerView = nullptr;
    TableHeaderView *m_tableHeaderView = nullptr;

    QPushButton *m_btnBack = nullptr;
    QPushButton *m_btnCleanUp = nullptr;

    QMenu *m_menuTimeInterval = nullptr;

    // 翻译内容
    QString m_strBtnBack;
    QString m_strBtnClearUp;
    QString m_strLabelChoosed;
    QString m_strLabelInfo;
    QString m_strTableFileName;
    QString m_strTableFileType;
    QString m_strTableFileTime;
    QString m_strTableFileSize;
    QString m_strTableFileMd5;
    QString m_strMenuTimeIntervalAll;
    QString m_strMenuTimeIntervalLastOneWeek;
    QString m_strMenuTimeIntervalOneWeekAgo;
    QString m_strMenuTimeIntervalOneMonthAgo;
    QString m_strMenuTimeIntervalThreeMonthAgo;
    QString m_strMenuTreeWidgetRightActionOpenPath;

    void initTableWidgetUi();
    void testTableWidgetItem();
    void setTextToolTips(QTreeWidgetItem *item, int indexCol);
    void recursiveFilter(QTreeWidgetItem *item, const QString &filterText);
    QString calculatedSize(_Float64 size);
    QString getTimeDiffDescription(QString pastTime);
    QString getTimeDiffDescription(qint64 pastTime);
    QString getFileType(const QString &filePath);
    QString getFileIconName(const QString &filePath);

    bool sortUpDown = false; 
    bool m_showMd5 = false;

    QHash <QTreeWidgetItem *, QString> m_hashItemPath;
    QHash <QTreeWidgetItem *, QString> m_hashCleanFile;
    QHash <QTreeWidgetItem *, _Float64> m_hashItemSize;
    QHash <QTreeWidgetItem *, _Float64> m_hashChooseFileSize;
    QHash <QTreeWidgetItem *, _Float64> m_hashSearchChooseFileSize;

    _Float64 m_totalSize;
    _Float64 m_chooseSize;
    _Float64 m_searchSize;

    qint64 m_menuTimeIntervalOneWeek = 0;
    qint64 m_menuTimeIntervalOneMonth = 0;
    qint64 m_menuTimeIntervalThreeMonth = 0;

    int testNum = 0;

public Q_SLOTS:
    void slotBackup();
    void slotCleanUp();
    void slotTableSort(int col);
    void slotCleanFileSuccess(QStringList list);
    void slotCheckStateChange(int iState);
    void slotMenuTimeIntervalChange(QAction *action);
    void slotFilterTreeWidget(QString str);
    void slotItemRightClicked(QTreeWidgetItem *item, int column);

Q_SIGNALS:
    void signalBtnBack();
    void signalClearUp(QStringList list);
    void signalChangeHeaderCheckState(int state);
};

}

#endif 