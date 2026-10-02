#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "interface/ukcccommon.h"
using namespace ukcc;

#include <QWidget>
#include <QList>
#include <QFile>
#include <QEvent>
#include <QFocusEvent>
#include <QLocale>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QXmlStreamReader>
#include <QtConcurrent>
#include <QStyledItemDelegate>
#include <QGSettings>
#include <QPainterPath>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include "ksearchlineedit.h"
using namespace kdk;

const QString XML_Source       = "source";
const QString XML_Title        = "translation";
const QString XML_Numerusform  = "numerusform";
const QString XML_Explain_Path = "extra-contents_path";

class SearchWidget : public KSearchLineEdit
{
    Q_OBJECT
public:
    struct SearchBoxStruct {
        QString translateContent;
        QString actualModuleName;
        QString childPageName;
        QString fullPagePath;
    };

    struct SearchDataStruct {
        QString chiese;
        QString pinyin;
    };

public:
    SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget() override;

    bool jumpContentPathWidget(QString path);
    void setLanguage(QString type);
    void addModulesName(QString moduleName, QString searchName, QString translation = "");
    void addTranslationPath(const QString &translation);
    void hiddenSearchItem(QString name , bool show);
    QVariantMap getSearchItems();
    // KSearchLineEdit 内部清空按钮；供标题栏 Tab 序衔接（搜索 → 清空 → 选项）
    QWidget *clearButton() const;

protected:
    void focusInEvent(QFocusEvent *event) override;

Q_SIGNALS:
    void notifyModuleSearch(QString, QString, QString);

private:
    QString translationFileForLocale(const QString &pathTemplate) const;
    void loadxml();
    SearchBoxStruct getModuleBtnString(QString value);
    QString getModulesName(QString name, bool state = true);
    QString removeDigital(QString input);
    QString transPinyinToChinese(QString pinyin);
    QString containTxtData(QString txt);
    void appendChineseData(SearchBoxStruct data);
    void clearSearchData();
    void initExcludeSearch();
    void updateSearchItem(QString plugin_name, bool show,
                          SearchBoxStruct *data = nullptr);
    void appendShortcutSearch();
    void addShortcutSearch();
    // 补全列表出现后选中第一项，便于键盘确认；朗读由读屏跟 selected 事件
    void selectFirstCompletionResult();
    // 覆盖 SDK 默认 NoFocus，使清空按钮可被 Tab 到达（与文件管理器一致）
    void setupClearButtonKeyboardAccess();

    bool m_selectingFirstCompletion = false;
    QWidget *m_clearButton = nullptr;

    QStandardItemModel *m_model;
    QCompleter *m_completer;
    QList<SearchBoxStruct> m_EnterNewPagelist;
    QList<SearchBoxStruct> searchItemList;
    SearchBoxStruct m_searchBoxStruct;
    QString m_xmlExplain;
    QSet<QString> m_xmlFilePath;
    QString m_lang;
    QList<QPair<QString, QString>> m_moduleNameList;//用于存储如 "update"和"Update"
    QList<SearchDataStruct> m_inputList;
    bool m_bIsChinese;
    QString m_searchValue;
    bool m_bIstextEdited;
    QStringList m_defaultRemoveableList;//存储已知全部模块是否存在
    QList<QString> m_TxtList;

    QStringList mExcludeList;
    int count;
    QVariantMap searchMap;
    bool m_shortcutInterfaceValid = true;
    QDBusInterface* m_shortcutInterface = nullptr;
    bool m_searchLoadXml = false;
};
#endif // SEARCHWIDGET_H
