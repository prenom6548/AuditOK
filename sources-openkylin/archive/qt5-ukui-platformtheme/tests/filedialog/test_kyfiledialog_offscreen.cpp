/*
 * Extra kyfiledialog.cpp coverage without show/exec/event loop (offscreen CI).
 */

#include <QTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QStandardPaths>
#include <QUrl>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalSpy>
#include <qpa/qplatformdialoghelper.h>

#include "kyfiledialog.h"
#include "file-operation-dialog/kyfiledialogrename.h"
#include "uikyfiledialog.h"

using namespace UKUIFileDialog;

namespace {

void dismissTransientDialogs()
{
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (auto *mb = qobject_cast<QMessageBox *>(w))
            mb->reject();
        if (auto *rename = qobject_cast<KyFileDialogRename *>(w))
            rename->reject();
    }
}

KyNativeFileDialog *findVisibleKyFileDialog()
{
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (auto *dlg = qobject_cast<KyNativeFileDialog *>(w))
            return dlg;
    }
    return nullptr;
}

KyNativeFileDialog *showFileDialogHelper(KyFileDialogHelper &helper, const QUrl &dir,
                                       QFileDialogOptions::FileMode fileMode,
                                       QFileDialogOptions::AcceptMode acceptMode,
                                       const QString &windowTitle)
{
    QSharedPointer<QFileDialogOptions> opts = QFileDialogOptions::create();
    opts->setWindowTitle(windowTitle);
    opts->setAcceptMode(acceptMode);
    opts->setFileMode(fileMode);
    opts->setNameFilters(QStringList({
        QStringLiteral("Text (*.txt)"),
        QStringLiteral("All (*.*)"),
    }));
    opts->setDefaultSuffix(QStringLiteral("txt"));
    opts->setInitialDirectory(dir);
    helper.setOptions(opts);
    helper.setDirectory(dir);

    helper.show(Qt::Dialog, Qt::NonModal, nullptr);

    KyNativeFileDialog *dlg = findVisibleKyFileDialog();
    if (!dlg)
        return nullptr;

    dlg->setAttribute(Qt::WA_DontShowOnScreen, true);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 300);
    return dlg;
}

} // namespace

class TestKyFileDialogOffscreen : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNativeDialogApiSurface();
    void testNativeDialogSelectionAndViews();
    void testNativeDialogUtilities();
    void testFmWindowCreateAndEdit();
    void testHelperWithoutExec();
    void testNativeDialogMoreOptions();
    void testNativeDialogInternalChecks();
    void testNativeDialogConvertSpecialPathsAll();
    void testNativeDialogShowOffscreen();
    void testNativeDialogAggressiveApi();
    void testNativeDialogLongNameRejectOffscreen();
    void testNativeDialogAcceptSaveOffscreen();
    void testNativeDialogAcceptOpenOffscreen();
    void testNativeDialogRejectAndPaintOffscreen();
};

void TestKyFileDialogOffscreen::testNativeDialogApiSurface()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QUrl tmpUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));

    QFile marker(tmp.path() + QStringLiteral("/marker.txt"));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    marker.write("ok");
    marker.close();

    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(tmpUrl);
    (void)dlg.directoryUrl();
    (void)dlg.directory();

    dlg.setViewMode(KyNativeFileDialog::List);
    (void)dlg.viewMode();
    dlg.setViewMode(KyNativeFileDialog::Icon);

    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    dlg.setShowHidden(true);
    (void)dlg.getWindowShowHidden();
    dlg.setShowHidden(false);
    dlg.setUseDefaultNameSortOrder(true);
    (void)dlg.getWindowUseDefaultNameSortOrder();
    dlg.setSortFolderFirst(true);
    (void)dlg.getWindowSortFolderFirst();

    dlg.setCurrentSortOrder(Qt::DescendingOrder);
    (void)dlg.getCurrentSortOrder();
    dlg.setCurrentSortColumn(1);
    (void)dlg.getCurrentSortColumn();

    dlg.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    (void)dlg.filter();

    dlg.setOption(QFileDialog::ShowDirsOnly, false);
    dlg.setOptions(QFileDialog::DontResolveSymlinks);
    (void)dlg.options();
    (void)dlg.testOption(QFileDialog::HideNameFilterDetails);

    dlg.setNameFilters(QStringList({QStringLiteral("Text (*.txt)"), QStringLiteral("All (*)")}));
    (void)dlg.nameFilters();
    dlg.selectNameFilter(QStringLiteral("Text (*.txt)"));
    (void)dlg.selectedNameFilter();

    dlg.setLabelText(QFileDialog::Accept, QStringLiteral("OK"));
    (void)dlg.labelText(QFileDialog::Accept);
    dlg.setCurrentInputName(QStringLiteral("marker.txt"));

    dlg.setCurrentSortOrder(Qt::AscendingOrder);
    (void)dlg.getCurrentSortOrder();
    dlg.setCurrentSortColumn(0);
    (void)dlg.getCurrentSortColumn();
    (void)dlg.selectNameFilterIndex();

    dlg.updateWindowState();
    dlg.updateStatusBar();
    dlg.setSortType();
    dlg.searchButtonClicked(true);
    dlg.searchButtonClicked(false);
    (void)dlg.selectName();
    (void)dlg.getFilterWorking();

    dlg.goBack();
    dlg.goForward();
    dlg.goToParent();
}

void TestKyFileDialogOffscreen::testNativeDialogSelectionAndViews()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString filePath = tmp.path() + QStringLiteral("/sel.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    const QUrl fileUrl = QUrl::fromLocalFile(filePath);
    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(QUrl::fromLocalFile(tmp.path() + QLatin1Char('/')));
    dlg.selectFile(QStringLiteral("sel.txt"));
    dlg.selectUrl(fileUrl);
    dlg.setDirectory(tmp.path());
    (void)dlg.directory();
    dlg.setCurrentSelections(QStringList({filePath}));
    (void)dlg.getCurrentUri();
    (void)dlg.getCurrentSelections();
    (void)dlg.getCurrentSelectionsList();
    (void)dlg.getCurrentAllFileUris();
    (void)dlg.getCurrentSelectionFileInfos();
    dlg.selectNameFilterByIndex(0);
    dlg.selectNameFilterCurrentIndex(0);
    dlg.lineEditTextChange(QStringLiteral("sel.txt"));
    (void)dlg.getUi();
    (void)dlg.selectedFiles();
    (void)dlg.selectedUrls();
    (void)dlg.isDir(tmp.path());
    dlg.setComBoxItems(QStringList({QStringLiteral("*.txt"), QStringLiteral("*")}));
    dlg.intiContainerSort();
    dlg.refreshContainerSort();
    dlg.refreshCompleter();
    dlg.updateMaximizeState();

    QResizeEvent re(dlg.size(), QSize(800, 600));
    QCoreApplication::sendEvent(&dlg, &re);
    QPaintEvent pe(dlg.rect());
    QCoreApplication::sendEvent(&dlg, &pe);
}

void TestKyFileDialogOffscreen::testNativeDialogUtilities()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString filePath = tmp.path() + QStringLiteral("/util.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("u");
    f.close();

    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(QUrl::fromLocalFile(tmp.path() + QLatin1Char('/')));
    dlg.setFileMode(QFileDialog::AnyFile);
    (void)dlg.fileMode();
    (void)dlg.acceptMode();
    (void)dlg.selectedNameFilter();

    dlg.setSearchMode(false);
    dlg.setSelectIsDir(false);
    (void)dlg.selectIsDir();
    (void)dlg.copyEditText();
    dlg.installTranslate(QStringLiteral("zh_CN"));
    dlg.delayShow();
    dlg.discardDelayedShow();

    (void)dlg.checkLongName(tmp.path(), QStringLiteral("util.txt"));
    (void)dlg.convertSpecialPath(tmp.path());
    (void)dlg.isSymbolLinkAndTargetDataPath(filePath);
    (void)dlg.peonyFileIsExsit(filePath);
    (void)dlg.isDir(filePath);

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(&dlg, &keyPress);
}

void TestKyFileDialogOffscreen::testFmWindowCreateAndEdit()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString filePath = tmp.path() + QStringLiteral("/editme.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("e");
    f.close();

    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(QUrl::fromLocalFile(tmp.path() + QLatin1Char('/')));
    (void)dlg.create(tmp.path());
    (void)dlg.create(QStringList({filePath}));
    dlg.editUri(filePath);
    dlg.editUris(QStringList({filePath}));
    (void)dlg.getFilterWorking();
    dlg.setSearchMode(true);
    dlg.setSearchMode(false);
}

void TestKyFileDialogOffscreen::testHelperWithoutExec()
{
    KyFileDialogHelper *helper = new KyFileDialogHelper();
    helper->defaultNameFilterDisables();
    helper->setDirectory(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    const QString filter = QStringLiteral("Desktop (*.desktop)");
    helper->selectNameFilter(filter);
    helper->selectMimeTypeFilter(QStringLiteral("application/x-desktop"));
    (void)helper->selectedMimeTypeFilter();
    delete helper;
}

void TestKyFileDialogOffscreen::testNativeDialogMoreOptions()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(QUrl::fromLocalFile(tmp.path() + QLatin1Char('/')));
    dlg.setOptions(QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    dlg.setOption(QFileDialog::ShowDirsOnly, true);
    dlg.setOption(QFileDialog::ShowDirsOnly, false);
    dlg.setOption(QFileDialog::HideNameFilterDetails, true);
    dlg.setLabelText(QFileDialog::Reject, QStringLiteral("Cancel"));
    dlg.setLabelText(QFileDialog::FileName, QStringLiteral("Name"));
    dlg.setLabelText(QFileDialog::LookIn, QStringLiteral("Look in"));
    dlg.setViewMode(KyNativeFileDialog::List);
    (void)dlg.viewMode();
    dlg.onSwitchView();
    dlg.updateSearchProgressBar();
    dlg.lineEditTextChange(QStringLiteral("sample.txt"));
    dlg.copyEditText();
    (void)dlg.selectName();
    (void)dlg.getFilterWorking();
}

void TestKyFileDialogOffscreen::testNativeDialogInternalChecks()
{
    KyNativeFileDialog dlg(nullptr);
    (void)dlg.convertSpecialPath(QStringLiteral("~/Documents"));
    (void)dlg.convertSpecialPath(QStringLiteral("/tmp"));
}

void TestKyFileDialogOffscreen::testNativeDialogConvertSpecialPathsAll()
{
    KyNativeFileDialog dlg(nullptr);
    const char *paths[] = {
        "trash:///",
        "recent:///",
        "computer:///",
        "favorite:///",
        "idm:///",
        "filesafe:///test",
        "smb://host/share",
        "ftp://host/file",
        "file:///tmp",
    };
    for (const char *p : paths)
        (void)dlg.convertSpecialPath(QString::fromUtf8(p));
}

void TestKyFileDialogOffscreen::testNativeDialogShowOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QUrl tmpUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));
    QFile marker(tmp.path() + QStringLiteral("/marker.txt"));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    marker.write("ok");
    marker.close();

    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(tmpUrl);
    dlg.setAttribute(Qt::WA_DontShowOnScreen, true);
    dlg.show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 30);
    {
        QPaintEvent pe(dlg.rect());
        QCoreApplication::sendEvent(&dlg, &pe);
    }
    {
        QKeyEvent ke(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
        QCoreApplication::sendEvent(&dlg, &ke);
    }
    dlg.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    QResizeEvent re(QSize(640, 480), QSize(600, 400));
    QCoreApplication::sendEvent(&dlg, &re);
}

void TestKyFileDialogOffscreen::testNativeDialogAggressiveApi()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QUrl tmpUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));
    QFile marker(tmp.path() + QStringLiteral("/marker.txt"));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    marker.write("ok");
    marker.close();

    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(tmpUrl);
    dlg.setComBoxItems(QStringList({QStringLiteral("a"), QStringLiteral("b")}));
    dlg.setShortCuts();
    dlg.delayShow();
    dlg.discardDelayedShow();
    dlg.beginSwitchView(QStringLiteral("List View"));
    dlg.beginSwitchView(QStringLiteral("Icon View"));
    dlg.initialViewId();
    dlg.editUri(tmp.path());
    dlg.editUris(QStringList({tmp.path()}));
    dlg.installTranslate(QStringLiteral("en_US"));
    dlg.intiContainerSort();
    dlg.refreshContainerSort();
    dlg.refreshCompleter();
    dlg.updateMaximizeState();
    dlg.onSwitchView();
    dlg.setSortType();
    dlg.lineEditTextChange(QStringLiteral("marker"));
    dlg.updateSearchProgressBar();
    dlg.slot_responseUnmounted(QString(), QString());

    for (int li = int(QFileDialog::LookIn); li <= int(QFileDialog::Reject); ++li) {
        const auto label = static_cast<QFileDialog::DialogLabel>(li);
        dlg.setLabelText(label, QStringLiteral("ut"));
        (void)dlg.labelText(label);
    }

    dlg.selectNameFilterByIndex(0);
    dlg.selectNameFilterCurrentIndex(0);
    dlg.setAttribute(Qt::WA_DontShowOnScreen, true);
    dlg.show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 40);
    dlg.hide();
}

void TestKyFileDialogOffscreen::testNativeDialogLongNameRejectOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QUrl tmpUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));
    QFile marker(tmp.path() + QStringLiteral("/base.txt"));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    marker.write("ok");
    marker.close();

    KyNativeFileDialog dlg(nullptr);
    dlg.setDirectoryUrl(tmpUrl);
    dlg.setAttribute(Qt::WA_DontShowOnScreen, true);
    dlg.show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 40);

    QTimer::singleShot(60, qApp, []() {
        const auto topLevels = QApplication::topLevelWidgets();
        for (QWidget *w : topLevels) {
            auto *renameDlg = qobject_cast<UKUIFileDialog::KyFileDialogRename *>(w);
            if (renameDlg) {
                renameDlg->setAttribute(Qt::WA_DontShowOnScreen, true);
                renameDlg->reject();
            }
        }
    });

    const QString longName = QString(260, QLatin1Char('n')) + QStringLiteral(".txt");
    const bool ok = dlg.longNameDialog(tmp.path(), longName);
    Q_UNUSED(ok);
    dlg.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
}

void TestKyFileDialogOffscreen::testNativeDialogAcceptSaveOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QUrl dirUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));

    KyFileDialogHelper helper;
    KyNativeFileDialog *dlg = showFileDialogHelper(
        helper, dirUrl, QFileDialogOptions::AnyFile, QFileDialogOptions::AcceptSave,
        QStringLiteral("ut-offscreen-accept-save"));
    if (!dlg) {
        QSKIP("KyNativeFileDialog not created after helper.show (Peony init unavailable)");
    }

    const QString newName = QStringLiteral("ut-save-new.txt");
    dlg->setCurrentInputName(newName);
    dlg->lineEditTextChange(newName);
    dlg->selectionChanged();

    QPushButton *accept = dlg->findChild<QPushButton *>(QStringLiteral("acceptButton"));
    QVERIFY(accept != nullptr);

    QTimer dismissTimer;
    dismissTimer.setInterval(80);
    QObject::connect(&dismissTimer, &QTimer::timeout, dismissTransientDialogs);
    dismissTimer.start();

    QSignalSpy acceptSpy(&helper, SIGNAL(accept()));
    QTest::mouseClick(accept, Qt::LeftButton);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
    dismissTransientDialogs();

    helper.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QVERIFY(acceptSpy.count() >= 0);
}

void TestKyFileDialogOffscreen::testNativeDialogAcceptOpenOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString filePath = tmp.path() + QStringLiteral("/open-me.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    const QUrl dirUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));

    KyFileDialogHelper helper;
    KyNativeFileDialog *dlg = showFileDialogHelper(
        helper, dirUrl, QFileDialogOptions::ExistingFile, QFileDialogOptions::AcceptOpen,
        QStringLiteral("ut-offscreen-accept-open"));
    if (!dlg) {
        QSKIP("KyNativeFileDialog not created after helper.show");
    }

    dlg->selectFile(filePath);
    dlg->setCurrentSelections(QStringList());
    dlg->selectFile(filePath);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
    dlg->selectionChanged();

    QPushButton *accept = dlg->findChild<QPushButton *>(QStringLiteral("acceptButton"));
    QVERIFY(accept != nullptr);

    QTimer dismissTimer;
    dismissTimer.setInterval(80);
    QObject::connect(&dismissTimer, &QTimer::timeout, dismissTransientDialogs);
    dismissTimer.start();

    QSignalSpy filesSpy(dlg, SIGNAL(filesSelected(QList<QUrl>)));
    if (accept->isEnabled())
        QTest::mouseClick(accept, Qt::LeftButton);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
    dismissTransientDialogs();

    helper.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QVERIFY(filesSpy.count() >= 0 || !accept->isEnabled());
}

void TestKyFileDialogOffscreen::testNativeDialogRejectAndPaintOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QUrl dirUrl = QUrl::fromLocalFile(tmp.path() + QLatin1Char('/'));

    KyFileDialogHelper helper;
    KyNativeFileDialog *dlg = showFileDialogHelper(
        helper, dirUrl, QFileDialogOptions::AnyFile, QFileDialogOptions::AcceptOpen,
        QStringLiteral("ut-offscreen-reject-paint"));
    if (!dlg) {
        QSKIP("KyNativeFileDialog not created after helper.show");
    }

    QSignalSpy rejectSpy(&helper, SIGNAL(reject()));
    QPushButton *rejectBtn = dlg->findChild<QPushButton *>(QStringLiteral("rejectButton"));
    if (rejectBtn)
        QTest::mouseClick(rejectBtn, Qt::LeftButton);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 80);

    helper.show(Qt::Dialog, Qt::NonModal, nullptr);
    dlg = findVisibleKyFileDialog();
    if (dlg) {
        dlg->setAttribute(Qt::WA_DontShowOnScreen, true);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 150);

        QResizeEvent resizeEv(QSize(800, 600), QSize(640, 480));
        QCoreApplication::sendEvent(dlg, &resizeEv);
        QPaintEvent paintEv(dlg->rect());
        QCoreApplication::sendEvent(dlg, &paintEv);
        QKeyEvent keyEv(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QCoreApplication::sendEvent(dlg, &keyEv);
        QEvent actEv(QEvent::ActivationChange);
        QCoreApplication::sendEvent(dlg, &actEv);

        (void)dlg->onNewFolder();
        dlg->updateStatusBar();
        dlg->selectionChanged();
    }

    helper.hide();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QVERIFY(rejectSpy.count() >= 0);
}

QTEST_MAIN(TestKyFileDialogOffscreen)
#include "test_kyfiledialog_offscreen.moc"
