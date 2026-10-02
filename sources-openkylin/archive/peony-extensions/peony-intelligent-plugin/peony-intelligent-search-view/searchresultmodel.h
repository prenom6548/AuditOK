/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */
#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H
#include <QStandardItemModel>
#include "file-info.h"
#include "clipboard-utils.h"
#include "PeonyFileInfoJob"
#include "thumbnail-manager.h"
#include "PeonyFileWatcher"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QAbstractItemModel>
#include <QVector>
#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QPainter>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMimeData>
#include <QAbstractTextDocumentLayout>
#include <global-settings.h>
#include <QTextEdit>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <malloc.h>
#include "file-infos-job.h"
class SearchFileItem;
class FileManagerThread;
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)|| defined(_M_ARM64)
    #define ARCH_AVAILABLE_AI
#endif
bool isFileContentOn();
enum AIConditionType{
    allFit=0,
    aiIndexNotFit,
    aiSubsystemNotFit
};

class SearchFileItemModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class SearchFileItem;
public:
    enum CustomRoles {
        IsNoResultsItemRole = Qt::UserRole + 100,  // 标识是否为"暂无结果"项
        IsSpanRowRole = Qt::UserRole + 101          // 标识是否需要跨列显示
    };
    enum ItemIds {
        ROOT_ID = 0,
        FILE_CATEGORY_ID = 1,
        CONTENT_CATEGORY_ID = 2,
        TEXT_CONTENT_CATEGORY_ID = 3,
        FILE_ITEM_BASE_ID = 0x100000,
        CONTENT_ITEM_BASE_ID = 0x200000,
        TEXT_CONTENT_ITEM_BASE_ID = 0x300000,
    };
    explicit SearchFileItemModel(QObject *parent = nullptr);
    ~SearchFileItemModel();
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QMimeData *mimeData(const QModelIndexList& indexes) const override;

    // 数据操作接口
    //void clearAll();
    void keywordchange(QString keyword);
    bool isFileCategory(const QModelIndex &index) const ;
    bool isContentCategory(const QModelIndex &index) const ;
    bool isFileItem(const QModelIndex &index) const;
    bool isContentItem(const QModelIndex &index) const;
    bool isDefaultContentItem(const QModelIndex &index) const;
    bool isTextContentItem(quintptr id) const { return id >= TEXT_CONTENT_ITEM_BASE_ID; }
    void updateSearchstatus(bool isFinish);
    void setClientId(QString clientid);
    void addDefaultContent();
    void setAiCondition(bool isAiavailable);
    void childRemove(const QString uri);
    bool childRename(const QString& oldUri, const QString& newUri);
    bool hasDefaultContent();
    QModelIndex indexFromItem(SearchFileItem *item) const;
    const QModelIndex indexFromUri(const QString &uri, SearchFileItem *item);
    SearchFileItem *itemFromIndex(const QModelIndex &index) const;
    const QModelIndex indexFromItemAndUri(SearchFileItem *item, const QString &uri);
    SearchFileItem *getChildFromUri(QString uri,SearchFileItem *item);
    SearchFileItem *getFileNameItem();
    SearchFileItem *getFileContentItem();
    void finishSearch();
    void setRootUri(const QString &uri);
    void updateNoResultsDisplay();
    void clearNoResultsDisplay(SearchFileItem* parentitem);
    void updateNoContentResultsDisplay();
    void setSelecting(bool isSelecting);
    QString getDefaultString();
public Q_SLOTS:
    void addFileNameResultsByBatch(QStringList fileResults,QString clientid,bool finish);
    void addContentResults(QStringList contentResults,QString clientid);
    void clearResult();
    void cancelFindChildren();
    //void addTextContentResults( QList<QStringList> contentResults,QString clientid);
    void onChanged(const QString &uri);
Q_SIGNALS:
    void setUrisForBatchQueryInfos(const QStringList& uris,SearchFileItem *parentItem,bool isfinish);
    void setUrisForBatchQueryInfosUpdate(const QStringList& uri, SearchFileItem *parentItem);
    void cancelBatchQuery();
    void searchFinished();
    void resizeColFitContent();
    void expand(int index=0);
    void updated();
private:
    //QList<QStringList> m_textContentResults;
    QString m_keyword;
    bool searchFilefinish = false;
    bool searchContentfinish = false;
    //bool searchTextContentFinish = false;
    QString m_clientId;
    bool m_isAiavailable;
    bool m_showFileExtension;
    bool m_hasDefaultContent = false;
    QMutex m_dataMutex;

    QString m_DefaultContent;
    FileManagerThread *m_fileManagerThread = nullptr;
    Peony::FileInfosJob *m_infosJob = nullptr;
    SearchFileItem* m_rootItem;
    SearchFileItem* m_fileCategory = nullptr;
    SearchFileItem* m_contentCategory = nullptr;
    SearchFileItem* m_searchRootItem = nullptr;
    SearchFileItem *m_noResultsItem = nullptr;
    SearchFileItem *m_noContentResultsItem = nullptr;
    SearchFileItem *m_DefaultContentItem = nullptr;
    QString m_root_uri;
    bool m_isSelecting = false;
    QStringList m_waiting_update_queue;
    QTimer *m_changeChildTimer = nullptr;
};

class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);
    void adjustText();
    void setMaxLengthLimit(int length);
    void setLimitBytes(bool limitBytes);

    QTextEdit *m_backgroundEdit = nullptr;
    void setMargins(int left, int top, int right, int bottom);
Q_SIGNALS:
    void finishEditRequest();
    void focusLost(); // 自定义信号，用于通知焦点丢失
protected:
    void keyPressEvent(QKeyEvent *e);
    void focusOutEvent(QFocusEvent *e) override;

private:
    int m_max_length_limit = 0;
    bool m_limit_bytes = true;
};

class TreeviewDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TreeviewDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void initIndexOption(QStyleOptionViewItem *option, const QModelIndex &index) const
    {
        return initStyleOption(option, index);
    }

    void setHighlightKeyword(const QString& keyword) {
        m_keyword = keyword;
    }
    void paintSpanRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paintLabel(QStyleOptionViewItem &opt, int aalignment, QList<QColor> colors, QPainter *painter) const;
Q_SIGNALS:
    void isEditing(bool editing) const;
    void reNameFinish(const QString &olduri,const QString &newuri,const QString &fileName) const;
private Q_SLOTS:
    void slot_finishEdit();/* 编辑完成 */
private:
    QString m_keyword;
};


class SearchFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    enum Type {
        FileType=1,
        FileSize,
        DateRange,
        FileName,
        FileLabel,
        ConditionMatchRule,
        FileContent,
        SearchLocation,
    };
    enum FilterFileType {
        ALL_TYPE,
        FILE_FOLDER,
        PICTURE,
        VIDEO,
        TXT_FILE,
        AUDIO,
        WPS_FILE,
        OTHERS
    };
    Q_ENUM(FilterFileType)
    enum FilterFileModifyTime {
        ALL_TIME,
        TODAY,
        YESTERDAY,
        THIS_WEEK,
        LAST_WEEK,
        THIS_MONTH,
        LAST_MONTH,
        THIS_YEAR,
        LAST_YEAR,
        BEFORE_LAST_YEAR
    };
 Q_ENUM(FilterFileModifyTime)
    enum FilterFileSize {
        ALL_SIZE,
        EMPTY,
        TINY,
        SMALL,
        MEDIUM,
        BIG,
        LARGE,
        GREAT
    };
    Q_ENUM(FilterFileSize)

    const QString Folder_Type = "inode/directory";
    const QString Image_Type = "image/";
    const QString Video_Type = "video/";
    const QString Text_Type = "text/";
    const QString Wps_Type = "application/wps-office";
    const QString Audio_Type = "audio/";

    const int ALL_FILE = 0;
    const quint64 K_BASE = 1024;
    const quint64 TINY_BASE = 16 * K_BASE;
    const quint64 SMALL_BASE = K_BASE * K_BASE;
    const quint64 MEDIUM_BASE = 128 * K_BASE * K_BASE;
    const quint64 BIG_BASE = K_BASE * K_BASE * K_BASE;
    const quint64 LARGE_BASE = 4 * K_BASE * K_BASE * K_BASE;
    explicit SearchFilterProxyModel(QObject* parent = nullptr);
    QStringList getFiltereUrilist();
    void update();
    void checkSortSettings();
    void setCurrenturi(QString uri);
    bool startWithChinese(const QString &displayName) const;
Q_SIGNALS:
    void conditionsChange();
public Q_SLOTS:
    void updateconditions(QString conditionsobj);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool hasChildren(const QModelIndex& parent) const override;
    bool checkFileModifyTimeFilter(quint64 modifiedTime) const;
    bool checkFileSizeFilter(quint64 size) const;
    bool checkFileSizeOrTypeFilter(quint64 sizem, bool isDir) const;
    bool checkFileNameFilter(const QString &displayName) const;
    bool checkFileContentFilter(const QString &displayName) const;
    bool checkFileTypeFilter(QString type) const;
    static bool isIdmSpaceUri(const QString &uri);
    static int effectiveFileTypeForIdmSpace(int storedType);
    static int effectiveFileType(int storedType, const QString &uri);
private:
    QStringList urilist;
    QMap<int,int> m_conditions;
    QMap<int,bool>m_contains;
    QStringList m_keywords;
    bool m_keywordsContain = true;
    bool needFileContent = true;
    bool m_use_default_name_sort_order;
    bool m_use_global_sort;
    bool m_folder_first;
    QString m_uri;
    Peony::GlobalSettings *m_settings;
};

using namespace Peony;
class FileManagerThread : public QThread{
Q_OBJECT
public:
    explicit FileManagerThread();
    ~FileManagerThread();

    void batchQueryFileInfos(FileInfosJob *infosJob, SearchFileItem *parentItem,bool isfinish,bool isupdate = false);

protected:
    void run() override;


Q_SIGNALS:
    void finishQueryFileInfos(std::vector<std::shared_ptr<FileInfo> >& fileInfos, SearchFileItem *parentItem,bool isfinish,bool isupdate = false);
    void setParamForBatchQueryInfos(/*std::shared_ptr<FileInfoJob>*/ FileInfosJob *infosJob, SearchFileItem *parentItem,bool isfinish,bool isupdate = false);
};

#endif // SEARCHRESULTMODEL_H
