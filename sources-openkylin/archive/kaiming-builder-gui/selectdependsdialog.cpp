#include "selectdependsdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QStringList>
#include <QGroupBox>
#include <QHeaderView>
#include <QSet>
#include <QList>
#include <QTextEdit>
#include <QScrollArea>
#include <QMovie>
#include <QThread>
#include <QSet>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QProcess>

#include "logic/KMBuilderApi.h"
#include "logic/KMRef.h"
#include "globalinfo.h"
#include "common/utils.h"
#include "common/KMInstalledAppQuery.h"

#define INIT_TABLE(tablewidget) \
    tablewidget->setColumnCount(2); \
    tablewidget->setHorizontalHeaderLabels({"id", "version"}); \
    tablewidget->verticalHeader()->setVisible(false); \
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers); \
    tablewidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); \
    tablewidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); \
    tablewidget->horizontalHeader()->setHighlightSections(true); \
    tablewidget->setSelectionMode(QAbstractItemView::ExtendedSelection); \
    tablewidget->setSelectionBehavior(QAbstractItemView::SelectRows); \
    tablewidget->setMinimumHeight(300);

#define INIT_LABEL_TEXTEDIT(edit, placeholder) \
    { \
        edit = new QTextEdit(q_ptr); \
        edit->setFixedHeight(200); \
        edit->setReadOnly(true);\
        edit->setPlaceholderText(placeholder); \
    }

class SelectDependsWorker : public QObject
{
    Q_OBJECT
public:
    explicit SelectDependsWorker(QObject * parent = nullptr);
    virtual ~SelectDependsWorker();

public slots:
    // 任务处理
    void query(const QString &base, const QString &runtime);

    bool isReady() const { return m_ready; }

private:
    bool m_ready = false;
};

SelectDependsWorker::SelectDependsWorker(QObject * parent)
    : QObject(parent)
{}

SelectDependsWorker::~SelectDependsWorker()
{}

void SelectDependsWorker::query(const QString &base, const QString &runtime)
{
    GlobelInfo::instance().setDepends(KMBuilderSearch::search("depend", base, runtime));
    m_ready = true;
}

class QryDependsInBaseAndRuntimeWorker : public QObject
{
    Q_OBJECT
public:
    explicit QryDependsInBaseAndRuntimeWorker(QObject * parent = nullptr);
    virtual ~QryDependsInBaseAndRuntimeWorker();

    bool isReady() const { return m_ready; }
    void setReady(bool ready) { m_ready = ready; }

    bool baseInstalled() const { return m_baseInstalled; }
    bool runtimeInstalled() const { return m_runtimeInstalled; }

    const QVector<InstalledPackage>& result() const { return m_results; }

    bool contains(const QString& key) const { return m_matchs.contains(key); }

public slots:
    // 任务处理
    void query(const QString &base, const QString &runtime, const QString& ids);

private:
    QVector<InstalledPackage> m_results;
    QSet<QString> m_matchs;

    bool m_ready = false;
    bool m_baseInstalled = false;
    bool m_runtimeInstalled = false;
};

QryDependsInBaseAndRuntimeWorker::QryDependsInBaseAndRuntimeWorker(QObject * parent)
    : QObject(parent)
{}

QryDependsInBaseAndRuntimeWorker::~QryDependsInBaseAndRuntimeWorker()
{}

void QryDependsInBaseAndRuntimeWorker::query(const QString &base, const QString &runtime, const QString& keys)
{
    m_results.clear();
    m_matchs.clear();
    m_ready = false;
    m_runtimeInstalled = false;
    m_baseInstalled = false;

    QString dpkgListFile;

    KMInstalledAppQuery query;
    if (!runtime.isEmpty())
    {
        std::optional<Ref> runtimeRef = KMInstalledAppQuery::parsePref(runtime);
        QVector<Ref> runtimes = query.kind("runtime")
                                     .id(runtimeRef->id)
                                     .module("devel")
                                     .version(runtimeRef->version)
                                     .query(KMInstalledAppQuery::User);

        if (!runtimes.empty())
        {
            Ref ref = runtimes.at(0);
            dpkgListFile = KMInstalledAppQuery::getUserInstallBaseDir() + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version + "/files/dpkg.list";
            m_runtimeInstalled = true;
            m_baseInstalled = true;
        }
    }

    if (dpkgListFile.isEmpty())
    {
        std::optional<Ref> baseRef = KMInstalledAppQuery::parsePref(base);
        QVector<Ref> bases = query.kind("base")
                                  .id(baseRef->id)
                                  .module("devel")
                                  .version(baseRef->version)
                                  .query(KMInstalledAppQuery::User);
        if (!bases.empty())
        {
            Ref ref = bases.at(0);
            dpkgListFile = KMInstalledAppQuery::getUserInstallBaseDir() + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version + "/files/dpkg.list";
            m_baseInstalled = true;
        }
    }

    QStringList ids = keys.split(",");
    QFile file(dpkgListFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.startsWith("ii"))
            {
                QStringList sections = line.split(QRegularExpression("\\s+"));
                InstalledPackage dpkg;
                dpkg.isInstalled = true;
                dpkg.p_name = sections.at(1);
                dpkg.p_version = sections.at(2);
                bool match = false;
                QString idTrim;
                for (auto& id : ids)
                {
                    idTrim = id.trimmed();
                    if (dpkg.p_name.contains(idTrim))
                    {
                        m_matchs.insert(idTrim);
                        match = true;
                    }
                }
                if (match)
                {
                    m_results.append(dpkg);
                }
            }
        }

        file.close();
    }

    m_ready = true;
}

class QryDependsInAptWorker : public QObject
{
    Q_OBJECT
public:
    explicit QryDependsInAptWorker(QObject * parent = nullptr);
    virtual ~QryDependsInAptWorker();

    bool isReady() const { return m_ready; }
    void setReady(bool ready) { m_ready = ready; }

    const QStringList& result() const { return m_results; }

    bool contains(const QString& key) const { return m_matchs.contains(key); }

public slots:
    // 任务处理
    void query(const QString &keys);

private:
    bool m_ready = false;

    QStringList m_results;
    QSet<QString> m_matchs;
};

QryDependsInAptWorker::QryDependsInAptWorker(QObject * parent)
    : QObject(parent)
{}

QryDependsInAptWorker::~QryDependsInAptWorker()
{}

void QryDependsInAptWorker::query(const QString &keys)
{
    m_results.clear();
    m_matchs.clear();
    m_ready = false;

    QStringList ids = keys.split(",", Qt::SkipEmptyParts);
    QString id;
    for (const QString &key: ids)
    {
        id = key.trimmed();
        if (id.isEmpty())
        {
            continue;
        }

        QString command = QString("apt search %1 | grep '^%2'").arg(id).arg(id);
        QProcess process;
        process.start("sh", QStringList() << "-c" << command);
        if (!process.waitForFinished(5000))
        { // 超时5秒
            continue;
        }

        if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        {
            continue;
        }

        QString output = process.readAllStandardOutput();
        if (output.isEmpty())
        {
            continue;
        }

        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        bool found = false;
        for (const QString &line : lines)
        {
            // 跳过描述行（以空格开头）
            if (line.startsWith(' '))
            {
                continue;
            }
            m_results << line;
            found = true;
        }

        if (found)
        {
            m_matchs.insert(id);
        }
    }

    m_ready = true;
}

class SelectDependsDialogPrivate : public QObject
{
    Q_OBJECT

public:
    SelectDependsDialogPrivate(SelectDependsDialog * parent, const QString &base, const QString &runtime, const QStringList selectedDepends = QStringList());
    virtual ~SelectDependsDialogPrivate();

    void setupUi();

public slots:
    void onSearchButtonClicked();
    void transferData(QTableWidget * from, QTableWidget * to, bool right = false);

private:
    bool checkAndSearchPackages();
    void showWarning(QString message);

    void searchDepends();
    void setSelectedDepend();

private:
    Q_DECLARE_PUBLIC(SelectDependsDialog)
    SelectDependsDialog * q_ptr;

    QString m_base;
    QString m_runtime;
    QStringList m_selected_depends;
    QVector<KMRef> m_refs;

    QLineEdit * m_searchLineEdit;
    QPushButton * m_searchButton;
    QLabel * m_loadingGif;
    QMovie * m_movie;
    QWidget *m_tableContainer;
    QTableWidget * m_leftTable;
    QTableWidget * m_selectedTable;
    QPushButton * m_btnConfirm;
    QPushButton * m_btnCancel;

    QVector<SelectedDepend> m_depends;
    QLabel *m_warningLabel;
    QTabWidget *m_tabWidget ;
    QWidget * m_runtimeORBaseTab;
    QWidget * m_aptTab;
    QWidget * m_unmatchedTab;
    QTableWidget *m_runtimeORBaseTable;
    QTextEdit *m_runtimeORBaseEdit;
    QTextEdit *m_aptEdit;
    QTextEdit *m_unmatchedEdit;

    SelectDependsWorker m_worker;
    QThread m_threadQryDepends;

    QryDependsInBaseAndRuntimeWorker m_qryDepsInBaseWorker;
    QThread m_threadQryDependsInBase;
    QString m_keys;

    QryDependsInAptWorker m_qryDepsInAptWorker;
    QThread m_threadQryDependsInApt;
    bool m_aptReady = false;
};

SelectDependsDialog::SelectDependsDialog(QWidget *parent, const QString &base, const QString &runtime, const QStringList selectedDepends)
    : QDialog(parent),
      d_ptr(new SelectDependsDialogPrivate(this, base, runtime, selectedDepends))
{
    Q_D(SelectDependsDialog);
    setFixedSize(960, 700);
    m_selected_depends = selectedDepends;
    d->setupUi();

    connect(d->m_btnConfirm, &QPushButton::clicked, this, [=]() {
        d->m_depends.clear();

        int count = d->m_selectedTable->rowCount();
        for (int row = 0; row < count; row++) {
            QTableWidgetItem *idItem = d->m_selectedTable->item(row, 0);
            QTableWidgetItem *versionItem = d->m_selectedTable->item(row, 1);

            SelectedDepend depend;
            depend.id = idItem->text();
            depend.version = versionItem->text();
            d->m_depends << depend;
        }
        accept();
    });

    connect(d->m_btnCancel, &QPushButton::clicked, this, &SelectDependsDialog::reject);
}

const QVector<SelectedDepend>& SelectDependsDialog::getSelected()
{
    Q_D(SelectDependsDialog);

    return d->m_depends;
}

SelectDependsDialog::~SelectDependsDialog()
{}

SelectDependsDialogPrivate::SelectDependsDialogPrivate(SelectDependsDialog * parent, const QString &base, const QString &runtime, const QStringList selectedDepends)
    : QObject(parent),
      q_ptr(parent),
      m_base(base),
      m_runtime(runtime),
      m_selected_depends(selectedDepends)
{}

SelectDependsDialogPrivate::~SelectDependsDialogPrivate()
{
}

void SelectDependsDialogPrivate::setSelectedDepend()
{
    for (auto depend: m_selected_depends)
    {
        QStringList id_version = depend.split("/");
        QString id_text = id_version[0];
        QString version_text = "";
        if (id_version.size() >= 2)
        {
            version_text = id_version[1];
        }
        QTableWidgetItem * id = new QTableWidgetItem;
        id->setText(id_text);
        id->setToolTip(id_text);
        int newRow = m_selectedTable->rowCount();
        m_selectedTable->insertRow(newRow);
        m_selectedTable->setItem(newRow, 0, id);

        QTableWidgetItem * version = new QTableWidgetItem;
        version->setText(version_text);
        version->setToolTip(version_text);
        m_selectedTable->setItem(newRow, 1, version);
    }
}

void SelectDependsDialogPrivate::setupUi()
{
    Q_Q(SelectDependsDialog);

    QVBoxLayout * vLayout = new QVBoxLayout;
    q->setLayout(vLayout);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QWidget *containerWidget = new QWidget;
    QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);


    //搜索框
    QHBoxLayout * searchLayout = new QHBoxLayout;
    QLabel * idPackage = new QLabel("id");
    m_searchLineEdit = new QLineEdit;
    m_searchLineEdit->setFixedWidth(600);
    m_searchLineEdit->setPlaceholderText(tr("Support multiple dependencies separated by commas."));
    m_searchButton = new QPushButton("🔍");
    // 等待图标
    m_loadingGif = new QLabel;
    m_loadingGif->setFixedSize(24,24);
    // 环境检测等待动画
    m_movie = new QMovie(":/images/loading.gif", QByteArray(), m_loadingGif);
    m_loadingGif->setMovie(m_movie);
    m_loadingGif->setVisible(false);
    searchLayout->addStretch();
    searchLayout->addWidget(idPackage);
    searchLayout->addWidget(m_searchLineEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_loadingGif);
    searchLayout->addStretch();
    // 左右表格
    m_tableContainer = new QWidget();
    QHBoxLayout * tableLayout = new QHBoxLayout(m_tableContainer);
    {
        QGroupBox * selectGroup = new QGroupBox(tr("Please Select"));
        QVBoxLayout * selectGroupLayout = new QVBoxLayout;
        selectGroup->setLayout(selectGroupLayout);
        m_leftTable = new QTableWidget;
        selectGroupLayout->addWidget(m_leftTable);
        INIT_TABLE(m_leftTable)

        QVBoxLayout * transLayout = new QVBoxLayout;
        QPushButton * right = new QPushButton(" >> ");
        right->setFlat(true);
        right->setFixedSize(36, 36);
        QPushButton * left = new QPushButton(" << ");
        left->setFlat(true);
        left->setFixedSize(36, 36);
        transLayout->addStretch();
        transLayout->addWidget(right);
        transLayout->addSpacing(15);
        transLayout->addWidget(left);
        transLayout->addStretch();

        QGroupBox * selectedGroup = new QGroupBox(tr("Selected"));
        QVBoxLayout * selectedGroupGroupLayout = new QVBoxLayout;
        selectedGroup->setLayout(selectedGroupGroupLayout);
        m_selectedTable = new QTableWidget;
        selectedGroupGroupLayout->addWidget(m_selectedTable);
        INIT_TABLE(m_selectedTable)

        tableLayout->addWidget(selectGroup);
        tableLayout->addLayout(transLayout);
        tableLayout->addWidget(selectedGroup);

        //右栏设置已选择depend
        setSelectedDepend();

        connect(right, &QPushButton::clicked, this, [=]() {
            this->transferData(m_leftTable, m_selectedTable, true);
        });

        connect(left, &QPushButton::clicked, this, [=]() {
            this->transferData(m_selectedTable, m_leftTable);
        });
    }

    m_warningLabel = new QLabel;
    m_warningLabel->setStyleSheet("color: red; font-weight: bold;");
    m_warningLabel->hide();

    m_tabWidget =new QTabWidget;
    m_tabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_tabWidget->setFixedHeight(260);  // 设置整个控件高度
    {
        m_runtimeORBaseTab = new QWidget;
        QVBoxLayout *runtimeORBaseLayout = new QVBoxLayout(m_runtimeORBaseTab);
        m_runtimeORBaseTable = new QTableWidget;
        m_runtimeORBaseTable->setColumnCount(2);
        m_runtimeORBaseTable->setHorizontalHeaderLabels({"id", "version"});
        m_runtimeORBaseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_runtimeORBaseTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_runtimeORBaseTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        m_runtimeORBaseTable->horizontalHeader()->setHighlightSections(true);
        m_runtimeORBaseTable->setSelectionMode(QAbstractItemView::NoSelection);
        m_runtimeORBaseTable->verticalHeader()->setVisible(false);
        runtimeORBaseLayout->addWidget(m_runtimeORBaseTable);
        m_tabWidget->addTab(m_runtimeORBaseTab, tr("Base or runtime already exists"));
    }
    {
        m_aptTab = new QWidget;
        QVBoxLayout *aptLayout = new QVBoxLayout(m_aptTab);
        QString placeholderText2 = tr("No relevant content was found in the search");
        INIT_LABEL_TEXTEDIT(m_aptEdit, placeholderText2);
        aptLayout->addWidget(m_aptEdit);
        m_tabWidget->addTab(m_aptTab, tr("Can be converted from deb source"));
    }
    {
        m_unmatchedTab = new QWidget;
        QVBoxLayout *unmatchedLayout = new QVBoxLayout(m_unmatchedTab);
        QString placeholderText3 = tr("No relevant content was found in the search");
        INIT_LABEL_TEXTEDIT(m_unmatchedEdit, placeholderText3);
        unmatchedLayout->addWidget(m_unmatchedEdit);
        m_tabWidget->addTab(m_unmatchedTab, tr("Source code construction required"));
    }
    for (int i = 0; i < m_tabWidget->count(); ++i)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        m_tabWidget->setTabVisible(i, false);
#else
        m_tabWidget->clear();
#endif
    }
    m_tabWidget->setVisible(false);

    // 确定按钮
    QHBoxLayout * confirmLayout = new QHBoxLayout;
    m_btnConfirm = new QPushButton(tr("Confirm"));
    m_btnCancel = new QPushButton(tr("Cancel"));
    confirmLayout->addStretch();
    confirmLayout->addWidget(m_btnConfirm);
    confirmLayout->addWidget(m_btnCancel);
    confirmLayout->addStretch();

    vLayout->addLayout(searchLayout);

    containerLayout->addWidget(m_tableContainer);
    containerLayout->addSpacing(15);
    containerLayout->addWidget(m_warningLabel);
    containerLayout->addWidget(m_tabWidget);
    containerLayout->addSpacing(15);
    scrollArea->setWidget(containerWidget);
    vLayout->addWidget(scrollArea);

    vLayout->addLayout(confirmLayout);
    vLayout->addSpacing(15);

    m_worker.moveToThread(&m_threadQryDepends);
    connect(&m_threadQryDepends, &QThread::started, this, [=]() {
        this->m_worker.query(this->m_base, this->m_runtime);
        this->m_threadQryDepends.quit();
    });
//    m_threadQryDepends.start();

    m_qryDepsInBaseWorker.moveToThread(&m_threadQryDependsInBase);
    connect(&m_threadQryDependsInBase, &QThread::started, this, [=]() {
        this->m_qryDepsInBaseWorker.query(this->m_base, this->m_runtime, this->m_keys);
        this->m_threadQryDependsInBase.quit();
    });

    m_qryDepsInAptWorker.moveToThread(&m_threadQryDependsInApt);
    connect(&m_threadQryDependsInApt, &QThread::started, this, [=]() {
        this->m_qryDepsInAptWorker.query(this->m_keys);
        this->m_threadQryDependsInApt.quit();
    });

    connect(m_searchButton, &QPushButton::clicked, this, &SelectDependsDialogPrivate::onSearchButtonClicked);
//    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &SelectDependsDialogPrivate::onSearchButtonClicked);
}

bool SelectDependsDialogPrivate::checkAndSearchPackages()
{
    const bool hasRuntime = !m_runtime.isEmpty();
    const bool hasBase = !m_base.isEmpty();

    if(!hasRuntime && !hasBase)
    {
        showWarning(tr("Please fill in the base or runtime first"));
        return false;
    }
    if (hasRuntime && hasBase)
    {
        if (m_qryDepsInBaseWorker.runtimeInstalled()) {
            return true;
        }
        if (m_qryDepsInBaseWorker.baseInstalled()) {
            return true;
        }
        showWarning(tr("runtime and base are not installed, please install first"));
        return false;
    }
    if (hasRuntime)
    {
        if (!m_qryDepsInBaseWorker.runtimeInstalled()) {
            showWarning(tr("runtime not installed, please install first"));
            return false;
        }
        return true;
    }
    if (hasBase)
    {
        if (!m_qryDepsInBaseWorker.baseInstalled()) {
            showWarning(tr("base not installed, please install first"));
            return false;
        }
        return true;
    }
    Q_UNREACHABLE();
    return false;
}

void SelectDependsDialogPrivate::showWarning(QString message)
{
    m_warningLabel->setText("⚠ " + message);
    m_warningLabel->show();
}

void SelectDependsDialogPrivate::onSearchButtonClicked()
{
    m_searchButton->setEnabled(false);
    m_loadingGif->setVisible(true);
    m_movie->start();
    Utils::wait(100);

    m_keys = m_searchLineEdit->text();

    if (!m_worker.isReady())
    {
        m_threadQryDepends.start();
    }

    if (!m_keys.isEmpty())
    {
        m_qryDepsInBaseWorker.setReady(false);
        m_threadQryDependsInBase.start();

        m_qryDepsInAptWorker.setReady(false);
        m_threadQryDependsInApt.start();
    }

    while (!m_worker.isReady())
    {
        Utils::wait(100);
    }

    searchDepends();

    m_movie->stop();
    m_loadingGif->setVisible(false);
    m_searchButton->setEnabled(true);
}

void SelectDependsDialogPrivate::searchDepends()
{
    //获取过滤条件
    QString id = m_searchLineEdit->text();

    for (int i = 0; i < m_tabWidget->count(); ++i)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        m_tabWidget->setTabVisible(i, false);
#else
        m_tabWidget->clear();
#endif
    }
    m_tabWidget->setVisible(false);
    m_leftTable->clearContents();
    m_leftTable->setRowCount(0);
    m_warningLabel->hide();

    if (id.isEmpty())
    {
        int indexOfRow = 0;
        for (auto& depend : GlobelInfo::instance().getDepends())
        {
            //筛选已选择的depend
            if (m_selected_depends.contains(depend.m_id) || m_selected_depends.contains(depend.m_id + "/" + depend.m_version))
            {
                continue;
            }
            m_leftTable->insertRow(indexOfRow);

            QTableWidgetItem * id = new QTableWidgetItem;
            id->setText(depend.m_id);
            id->setToolTip(depend.m_id);
            m_leftTable->setItem(indexOfRow, 0, id);

            QTableWidgetItem * version = new QTableWidgetItem;
            version->setText(depend.m_version);
            version->setToolTip(depend.m_version);
            m_leftTable->setItem(indexOfRow, 1, version);

            ++indexOfRow;
        }

        return ;
    }

    // 1. depend内查找
    auto results = KMBuilderApi::filterDepends(GlobelInfo::instance().getDepends(), id);
    QVector<KMRef> refs = results.first;
    QStringList matchIds = results.second;
    if (!matchIds.isEmpty())
    {
        int indexOfRow = 0;
        for (auto& depend : refs)
        {
            //筛选已选择的depend
            if (m_selected_depends.contains(depend.m_id) || m_selected_depends.contains(depend.m_id + "/" + depend.m_version))
            {
                continue;
            }
            m_leftTable->insertRow(indexOfRow);

            QTableWidgetItem * id = new QTableWidgetItem;
            id->setText(depend.m_id);
            id->setToolTip(depend.m_id);
            m_leftTable->setItem(indexOfRow, 0, id);

            QTableWidgetItem * version = new QTableWidgetItem;
            version->setText(depend.m_version);
            version->setToolTip(depend.m_version);
            m_leftTable->setItem(indexOfRow, 1, version);

            ++indexOfRow;
        }
    }

    m_tabWidget->setVisible(true);

    while (!m_qryDepsInBaseWorker.isReady())
    {
        Utils::wait(100);
    }

    // 2. runtime或者base中查找
    bool hasRuntimeOrBaseResults = false;
    if (checkAndSearchPackages())
    {
        m_runtimeORBaseTable->clearContents();
        m_runtimeORBaseTable->setRowCount(0);
        int indexOfRow = 0;
        for (auto& package : m_qryDepsInBaseWorker.result())
        {
            hasRuntimeOrBaseResults = true;
            m_runtimeORBaseTable->insertRow(indexOfRow);

            QTableWidgetItem * id = new QTableWidgetItem;
            id->setText(package.p_name);
            id->setToolTip(package.p_name);
            m_runtimeORBaseTable->setItem(indexOfRow, 0, id);

            QTableWidgetItem * version = new QTableWidgetItem;
            version->setText(package.p_version);
            version->setToolTip(package.p_version);
            m_runtimeORBaseTable->setItem(indexOfRow, 1, version);

            ++indexOfRow;
        }

        if (hasRuntimeOrBaseResults)
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_tabWidget->setTabVisible(0, true);
#else
            m_tabWidget->addTab(m_runtimeORBaseTab, tr("Base or runtime includes"))
#endif
        }
    }

    // apt源中查找
    while (!m_qryDepsInAptWorker.isReady())
    {
        Utils::wait(100);
    }

    // 3. apt源中查找
    {
        // aptList展示在页面，颜色标注“/”前方字段
        QString displayText;
        for (const QString& item : m_qryDepsInAptWorker.result())
        {
            if (item.contains("/"))
            {
                int splitPos = item.indexOf("/");
                QString prefix = item.left(splitPos);
                QString suffix = item.mid(splitPos);
                displayText += QString("<span style='color:green;'>%1</span>%2").arg(prefix.toHtmlEscaped()).arg(suffix.toHtmlEscaped());
            } else {
                displayText += item.toHtmlEscaped();
            }
            displayText += "<br>";
        }
        m_aptEdit->setHtml(displayText);

        if (!m_qryDepsInAptWorker.result().empty())
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_tabWidget->setTabVisible(1, true);
#else
            m_tabWidget->addTab(m_aptTab, tr("There exists in the deb source"));
#endif
        }
    }

    // 4.未查询到的id(depend/base/runtime都没有)
    {
        QStringList keysNotFound;
        QStringList keys = id.split(",", Qt::SkipEmptyParts);
        for (auto& item : keys)
        {
            QString key = item.trimmed();
            if (matchIds.contains(key) || m_qryDepsInBaseWorker.contains(key) || m_qryDepsInAptWorker.contains(key))
            {
                continue;
            }

            keysNotFound << key;
        }

        if (!keysNotFound.empty())
        {
            m_unmatchedEdit->setPlainText(keysNotFound.join("\n"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_tabWidget->setTabVisible(2, true);
#else
            m_tabWidget->addTab(m_unmatchedTab, tr("Source code construction required"));
#endif
        }
    }
}

void SelectDependsDialogPrivate::transferData(QTableWidget * from, QTableWidget * to, bool right)
{
    // 获取所有选中的行索引（去重处理）
    QSet<int> selectedRows;
    for (QTableWidgetItem *item : from->selectedItems()) {
        selectedRows.insert(item->row());
    }

    // 倒序处理防止删除行时索引变化
    QList<int> rows = selectedRows.values();
    std::sort(rows.rbegin(), rows.rend());
    QStringList toValues = KMBuilderStringUtils::getColumnData(to,0);

    foreach (int row, rows) {
        // 获取数据
        QTableWidgetItem *idItem = from->item(row, 0);
        QTableWidgetItem *versionItem = from->item(row, 1);
        if (right)
        {
            if (toValues.contains(idItem->text()))
            {
                int to_row = toValues.indexOf(idItem->text());
                QString includedVersion = to->item(to_row, 1)->text();
                QString message = tr("The depend %1 of version %2 has already been added. "
                                     "Do you wish to continue adding version %3."
                                     ).arg(idItem->text(), includedVersion, versionItem->text());
                if (!MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(m_tableContainer,QObject::tr("Warning"),
                                                                       message,
                                                                       QObject::tr("Confirm"),
                                                                       QObject::tr("Cancel")))
                {
                    continue;
                }

            }
            toValues.append(idItem->text());
        }

        // 添加到右侧表格
        int newRow = to->rowCount();
        to->insertRow(newRow);

        QTableWidgetItem * id = new QTableWidgetItem;
        id->setText(idItem->text());
        id->setToolTip(idItem->text());
        to->setItem(newRow, 0, id);

        QTableWidgetItem * version = new QTableWidgetItem;
        version->setText(versionItem->text());
        version->setToolTip(versionItem->text());
        to->setItem(newRow, 1, version);

        // 从左侧表格删除
        from->removeRow(row);
    }
}

#include "selectdependsdialog.moc"
#include "moc_selectdependsdialog.cpp"
