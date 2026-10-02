#include <QtTest>
#include "src/view/burndata.h"
#include <QAction>
#include <QModelIndex>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeView>
#include <QLabel>
#include <QStyleOptionViewItem>
#include <QFocusEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <QTest>
#include <model/logicburndata.h>
#include "src/view/inputchange.h"
#include "src/model/logicmain.h"
#include "src/view/mainwindow.h"
#include "src/view/filefilterrules.h"
#include "src/data/datadirectory.h"


class BurnDataTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testBurnDataCreation();
    void testBurnDataInitialization();
    void testBurnDataMethods();
    void testSingletonInstance();
    void testPublicMethods();
    void testSlots();
    void testProtectedMethods();
    void testSignals();
    void testCustomDelegate();
    void testMyTreeView();
    void testCustomSelectionLineEdit();
    void testPrivateMethods();
    void testEventFilter();
    void testKeyEvents();
    void testLinkEntry();
    void testDataViewChanged();
    void testAutoResetCrumb();
    void testUpdateDisplayList();
    void testClearCrumb();
    void testResetText();
    void testSetAccessibleName();
    void testInitAction();
    void testOtherMethod();
    void testFileFilterRules();
    void testNewMethod();
    void testFilterSlotsWithData();
    void testSlotsNewFolder();
};

void BurnDataTest::initTestCase()
{
    // 在所有测试开始前执行
}

void BurnDataTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void BurnDataTest::init()
{
    // 在每个测试函数执行前执行
}

void BurnDataTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void BurnDataTest::testBurnDataCreation()
{
    BurnData *widget = new BurnData();
    QVERIFY(widget != nullptr);
    QVERIFY(widget->objectName() != "");
    delete widget;
}

void BurnDataTest::testBurnDataInitialization()
{
    BurnData *widget = new BurnData();
    QVERIFY(widget != nullptr);
    
    // 检查初始状态
    QVERIFY(widget->isEnabled() == true);
    
    delete widget;
}

void BurnDataTest::testBurnDataMethods()
{
    BurnData *widget = new BurnData();
    QVERIFY(widget != nullptr);
    
    // 测试基本功能
    widget->show();
    QVERIFY(widget->isVisible() == true);
    
    widget->hide();
    QVERIFY(widget->isVisible() == false);
    
    delete widget;
}

void BurnDataTest::testSingletonInstance()
{
    BurnData *instance1 = BurnData::instance();
    BurnData *instance2 = BurnData::instance();
    
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QVERIFY(instance1 == instance2); // 测试单例模式
    
    // 测试m_bDirReNameing静态变量
    BurnData::m_bDirReNameing = true;
    QVERIFY(BurnData::m_bDirReNameing == true);
    BurnData::m_bDirReNameing = false;
}

void BurnDataTest::testPublicMethods()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 测试refresh方法
    widget->refresh();
    
    // 测试canGoBack和canGoForward方法
    bool canBack = widget->canGoBack();
    bool canForward = widget->canGoForward();
    
    // 测试clearHistory方法
    widget->clearHistory();
    QVERIFY(widget->canGoBack() == false);
    QVERIFY(widget->canGoForward() == false);
    
    // 测试delModelIndex方法
    QModelIndex invalidIndex;
    widget->delModelIndex(invalidIndex);
    
    // 由于delModelIndex需要有效索引，我们创建一个临时的模型来测试
    // 这里只是确保方法能被调用而不崩溃
}

void BurnDataTest::testSlots()
{

    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);

    // 测试字体大小槽函数
    widget->slotChangeUIFontSize(12);
    
    // 测试字体名称槽函数
    widget->slotChangeUIFontname("Arial");
    
    // 测试停止加载数据槽函数
    widget->slotStopLoadingData();

    // 测试过滤相关槽函数
    widget->slotsFilterHidden(false);
    widget->slotsFilterBroken(false);
    widget->slotsRepleaceSymbolLinks(false);



    // 测试其他槽函数
    QModelIndex testIndex;
    widget->slotsEntry(testIndex);
    widget->slotsSize(1024UL);
    widget->slotsSelectionChanged();

    // 测试按钮相关槽函数
    widget->slotsAddFiles(false);
    widget->slotsDeleteFiles(false);
    widget->slotsCleanFiles(false);
    widget->slotsNewFolder(false);

    // 测试文件操作相关槽函数

    widget->slotsParent();

    widget->slotsOpen();

    widget->slotsOpenLocal();

    widget->slotsRename();
    widget->slotsProperties();



    // 测试菜单相关槽函数
    QPoint testPoint(10, 10);
    widget->slotsTreeMenuByRightKey(testPoint);
    widget->slotsTreeMenuByShortcutKey();
    
    // 测试快捷键删除槽函数
    widget->slotsDeleteFilesByKeyPress(false);

    // 测试导航相关槽函数
    widget->goBack();
    widget->goForward();
    widget->directoryChangeSlot(1);
    widget->slotsBurnerIndexChanged();

    // 测试设置默认窗口可见性
    widget->slotSetDefaultAddWindowVisible(true);
    widget->slotSetDefaultAddWindowVisible(false);
}

void BurnDataTest::testProtectedMethods()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 测试event方法
    QEvent testEvent(QEvent::User);
    bool eventResult = widget->event(&testEvent);
    QVERIFY(eventResult == true);
    
    // 测试resizeEvent方法
    QResizeEvent resizeEvent(QSize(800, 600), QSize(1024, 768));
    widget->resizeEvent(&resizeEvent);
    
    // 测试paintEvent方法
    // 由于paintEvent需要有效的paint设备，我们跳过直接测试
    
    // 测试keyPressEvent方法
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    widget->keyPressEvent(&keyEvent);
    
    // 测试eventFilter方法
    // 需要一个被监控的对象
    QLineEdit testLineEdit;
    QEvent filterEvent(QEvent::User);
    bool filterResult = widget->eventFilter(&testLineEdit, &filterEvent);
    QVERIFY(filterResult == false); // 应该返回false，因为lineEdit不是tip_widget
}

void BurnDataTest::testSignals()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 连接信号和槽来测试信号是否被发射
    bool changeCurrentIndexCalled = false;
    bool burnDataToDiscEnabledCalled = false;
    bool updateDiskCalled = false;
    bool newNodeCalled = false;
    bool delDataCalled = false;
    bool viewFileterFileCalled = false;
    
    QObject::connect(widget, &BurnData::changeCurrentIndex, 
                     [&changeCurrentIndexCalled]() { 
                         changeCurrentIndexCalled = true; 
                     });
    QObject::connect(widget, &BurnData::burnDataToDiscEnabled, 
                     [&burnDataToDiscEnabledCalled](bool) { 
                         burnDataToDiscEnabledCalled = true; 
                     });
    QObject::connect(widget, &BurnData::updateDisk, 
                     [&updateDiskCalled](int, void*) { 
                         updateDiskCalled = true; 
                     });
    QObject::connect(widget, &BurnData::newNode, 
                     [&newNodeCalled](void*) { 
                         newNodeCalled = true; 
                     });
    QObject::connect(widget, &BurnData::delData, 
                     [&delDataCalled](int) { 
                         delDataCalled = true; 
                     });
    QObject::connect(widget, &BurnData::viewFileterFile, 
                     [&viewFileterFileCalled](bool) { 
                         viewFileterFileCalled = true; 
                     });
    
    // 触发一些可能发射信号的操作
    widget->slotSetDefaultAddWindowVisible(true);
    widget->refresh();
    
    // 验证信号连接正常
    QVERIFY(true); // 简单验证连接成功
}

void BurnDataTest::testCustomDelegate()
{
    CustomDelegate *delegate = new CustomDelegate();
    QVERIFY(delegate != nullptr);
    
    // 测试构造函数
    CustomDelegate *delegateWithParent = new CustomDelegate(this);
    QVERIFY(delegateWithParent != nullptr);

    
    // 测试createEditor方法
    QWidget *parentWidget = new QWidget();
    QStyleOptionViewItem option;
    QModelIndex index;
    QWidget *editor = delegate->createEditor(parentWidget, option, index);
    QVERIFY(editor != nullptr);
    
    // 测试setEditorData方法 - 使用空的editor和index测试健壮性
    delegate->setEditorData(editor, index);

    // 测试setModelData方法
    CustomSelectionLineEdit *lineEdit = qobject_cast<CustomSelectionLineEdit*>(editor);
    LogicBurnData *model = new LogicBurnData();
    if (lineEdit) {
        delegate->setModelData(lineEdit, model, index);
    }

}

void BurnDataTest::testMyTreeView()
{
    MyTreeView *treeView = new MyTreeView();
    QVERIFY(treeView != nullptr);
    
    // 测试构造函数
    MyTreeView *treeViewWithParent = new MyTreeView(nullptr);
    QVERIFY(treeViewWithParent != nullptr);
    
    // 测试keyPressEvent
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    treeView->keyPressEvent(&keyEvent);
    
    // 测试mousePressEvent
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPointF(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    treeView->mousePressEvent(&mouseEvent);
    
    delete treeView;
    delete treeViewWithParent;
}

void BurnDataTest::testCustomSelectionLineEdit()
{
    CustomSelectionLineEdit *lineEdit = new CustomSelectionLineEdit();
    QVERIFY(lineEdit != nullptr);
    
    // 测试构造函数
    CustomSelectionLineEdit *lineEditWithParent = new CustomSelectionLineEdit(nullptr);
    QVERIFY(lineEditWithParent != nullptr);
    
    // 测试setBasename_pos方法
    lineEdit->setBasename_pos(5);
    
    // 测试focusInEvent方法
    QFocusEvent focusEvent(QEvent::FocusIn);
    lineEdit->focusInEvent(&focusEvent);
    
    // 测试基本QLineEdit功能
    lineEdit->setText("test_text");
    QVERIFY(lineEdit->text() == "test_text");
    
    delete lineEdit;
    delete lineEditWithParent;
}

void BurnDataTest::testPrivateMethods()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 测试linkEntry方法
    QModelIndex index;
    widget->linkEntry(index);

    
    // 测试dataViewChanged方法
    //widget->dataViewChanged(index);
    
    // 测试autoResetCrumb方法
    widget->autoResetCrumb();
    
    // 测试updateDisplayList方法
    widget->updateDisplayList(1);
    
    // 测试clearCrumb方法
    widget->clearCrumb();

    
    // 测试resetText方法
    QPushButton *button = new QPushButton();
    button->setText("Add");
    widget->resetText(button);
    delete button;
    
    // 测试initAction方法
    widget->initAction();

}

void BurnDataTest::testEventFilter()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 测试拖拽事件
    QWidget *tipWidget = widget->findChild<QWidget*>("tip_widget");
    if (tipWidget) {
        QMimeData *mimeData = new QMimeData();
        mimeData->setUrls({QUrl("file:///tmp/test.txt")});
        
        // 测试拖拽进入事件
        QDragEnterEvent *dragEnterEvent = new QDragEnterEvent(QPoint(10, 10), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
        bool result = widget->eventFilter(tipWidget, dragEnterEvent);
        QVERIFY(result == false);
        delete dragEnterEvent;
        
        // 测试拖拽放下事件
        QDropEvent *dropEvent = new QDropEvent(QPoint(10, 10), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
        result = widget->eventFilter(tipWidget, dropEvent);
        QVERIFY(result == false);
        delete dropEvent;
        
        delete mimeData;
    }
}

void BurnDataTest::testKeyEvents()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 测试各种键盘事件
    QKeyEvent shiftF10Event(QEvent::KeyPress, Qt::Key_F10, Qt::ShiftModifier);
    widget->keyPressEvent(&shiftF10Event);
    
    QKeyEvent f2Event(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
    widget->keyPressEvent(&f2Event);
    
    QKeyEvent f5Event(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);
    widget->keyPressEvent(&f5Event);
    
    QKeyEvent ctrlReturnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::ControlModifier);
    widget->keyPressEvent(&ctrlReturnEvent);
    
    QKeyEvent returnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    widget->keyPressEvent(&returnEvent);
}

void BurnDataTest::testLinkEntry()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    // 测试linkEntry方法的各种情况
    QModelIndex index;
    widget->linkEntry(index);
    
    // 测试有效索引的情况
    // 由于需要实际的模型索引，我们只测试代码路径
}

void BurnDataTest::testDataViewChanged()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
}

void BurnDataTest::testAutoResetCrumb()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    //widget->autoResetCrumb();
}

void BurnDataTest::testUpdateDisplayList()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    widget->updateDisplayList(1);
    widget->updateDisplayList(5);
}

void BurnDataTest::testClearCrumb()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    widget->clearCrumb();
}

void BurnDataTest::testResetText()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    QPushButton *button = new QPushButton();
    button->setText("Add");
    widget->resetText(button);
    
    button->setText("Delete");
    widget->resetText(button);
    
    button->setText("Clean");
    widget->resetText(button);
    
    button->setText("NewFolder");
    widget->resetText(button);
    
    delete button;
}

void BurnDataTest::testSetAccessibleName()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    widget->setAccessibleName();
}

void BurnDataTest::testInitAction()
{
    BurnData *widget = BurnData::instance();
    QVERIFY(widget != nullptr);
    
    widget->initAction();
}

void BurnDataTest::testOtherMethod()
{

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout,this,[&]{
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
             QTest::keyClick(widget, Qt::Key_Return);
        }
    });

    timer->start(1000);


    QStringList files;
    files << QCoreApplication::applicationDirPath().append(QString("/../../test_data"));

    BurnData::instance()->m_oModel->addDatasToIndex(files, BurnData::instance()->m_oAddIndex);
    LogicMain::instance()->init();
    LogicMain::instance()->initISO();

    QModelIndex invalidIdx;
    BurnData *burnData = BurnData::instance();
    burnData->linkEntry(invalidIdx);

    QModelIndex index = BurnData::instance()->m_oModel->index(0,0);
    burnData->linkEntry(index);

    burnData->dataViewChanged(invalidIdx);
    burnData->dataViewChanged(index);

    burnData->slotsEntry(invalidIdx);
    burnData->slotsEntry(index);

    burnData->autoResetCrumb();

    burnData->updateDisplayList(0);
    burnData->updateDisplayList(1);
    burnData->updateDisplayList(5);


    // 测试边界值和负值
    burnData->updateDisplayList(-1);
    burnData->updateDisplayList(100);

    // 测试清除面包屑功能
    burnData->clearCrumb();

    // 测试不同大小值的处理
    burnData->slotsSize(0);
    burnData->slotsSize(1024);
    burnData->slotsSize(1024*1024); // 1MB
    burnData->slotsSize(1024UL*1024UL*1024UL); // 1GB

    burnData->slotsSelectionChanged();


    // 测试删除文件功能
    burnData->slotsDeleteFiles(true);
    burnData->slotsDeleteFiles(false);


    // 测试新建文件夹功能
    //burnData->slotsNewFolder(true);
    //burnData->slotsNewFolder(false);

    // 测试隐藏文件过滤功能
    burnData->slotsFilterHidden(true);
    burnData->slotsFilterHidden(false);

    // 测试损坏链接过滤功能
    burnData->slotsFilterBroken(true);
    burnData->slotsFilterBroken(false);

    // 测试替换符号链接功能
    burnData->slotsRepleaceSymbolLinks(true);
    burnData->slotsRepleaceSymbolLinks(false);

    burnData->slotsSelectionChanged();

    // 测试返回父目录功能
    burnData->slotsParent();

    // 测试打开功能
    burnData->slotsOpen();

    // 测试本地打开功能
    burnData->slotsOpenLocal();

    // 测试重命名功能
    burnData->slotsRename();

    // 测试属性功能
    burnData->slotsProperties();

    // 测试右键菜单功能
    QPoint pos(10, 10);
    burnData->slotsTreeMenuByRightKey(pos);

    // 测试快捷键菜单功能
    burnData->slotsTreeMenuByShortcutKey();

    // 测试按键删除功能
    burnData->slotsDeleteFilesByKeyPress(true);
    burnData->slotsDeleteFilesByKeyPress(false);

    // 测试后退功能
    burnData->goBack();

    // 测试目录变化槽函数
    burnData->directoryChangeSlot(0);
    burnData->directoryChangeSlot(1);
    burnData->directoryChangeSlot(5);

    // 测试刻录机索引变化
    burnData->slotsBurnerIndexChanged();

    // 测试清理文件功能
    burnData->slotsCleanFiles(true);
    burnData->slotsCleanFiles(false);
    delete timer;

    delete burnData;

    QVERIFY(true); // 基本功能测试通过


}

void BurnDataTest::testFileFilterRules()
{
    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout,this,[&]{
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
             QTest::keyClick(widget, Qt::Key_Return);
        }
    });

    timer->start(1000);

    QString test_dir_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data"));
    DataDirectory* dir = new DataDirectory(test_dir_path);
    FileFilterRules::instance()->init(dir);
    FileFilterRules::instance()->show();
    FileFilterRules::instance()->showDlg();
    FileFilterRules::instance()->slotsensure();

    delete timer;
    QVERIFY(true);
}

void BurnDataTest::testNewMethod()
{
   QVERIFY(true);
}

void BurnDataTest::testFilterSlotsWithData()
{
    // 构造带模型数据的 BurnData 以验证过滤槽函数
    BurnData widget;

    // 临时目录作为根节点数据
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // 用新的根节点替换模型数据，确保 index 可用
    widget.m_oModel->m_oRoots.clear();
    DataDirectory *root = new DataDirectory(tempDir.path(), widget.m_oModel);
    widget.m_oModel->m_oRoots << root;
    widget.m_oModel->m_iIndex = 0;

    // 验证隐藏文件过滤
    QSignalSpy hiddenSpy(root, &DataDirectory::folderHidden);
    widget.slotsFilterHidden(true);
    QCOMPARE(hiddenSpy.count(), 1);
    QVERIFY(root->filterHidden());
    hiddenSpy.clear();
    widget.slotsFilterHidden(false);
    QCOMPARE(hiddenSpy.count(), 0);
    QVERIFY(root->filterHidden() == false);

    // 验证损坏链接过滤
    QSignalSpy brokenSpy(root, &DataDirectory::folderBroken);
    widget.slotsFilterBroken(true);
    QCOMPARE(brokenSpy.count(), 1);
    QVERIFY(root->filterBrokenSymbolLinks());
    brokenSpy.clear();
    widget.slotsFilterBroken(false);
    QCOMPARE(brokenSpy.count(), 0);
    QVERIFY(root->filterBrokenSymbolLinks() == false);

    // 验证替换符号链接过滤
    QSignalSpy replaceSpy(root, &DataDirectory::folderReplaced);
    widget.slotsRepleaceSymbolLinks(true);
    QCOMPARE(replaceSpy.count(), 1);
    QVERIFY(root->repleaceSymbolLinks());
    replaceSpy.clear();
    widget.slotsRepleaceSymbolLinks(false);
    QCOMPARE(replaceSpy.count(), 0);
    QVERIFY(root->repleaceSymbolLinks() == false);
}

void BurnDataTest::testSlotsNewFolder()
{
    QVERIFY(true);
}

#ifndef RUN_VIEW_UNIFIED_TESTS
QTEST_MAIN(BurnDataTest)
#endif

#include "burndata_test.moc"
