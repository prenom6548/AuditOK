#ifndef TEST_PAGE1_H
#define TEST_PAGE1_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QApplication>
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include "../../../../UIControl/src/page1.h"

// 测试辅助类，可以访问Page1的受保护成员
class TestablePage1 : public Page1
{
    Q_OBJECT
public:
    TestablePage1() : Page1() {
        // 重新调用测试环境友好的initControlQss方法
        initControlQss();
    }

    // 重写getUdiskPathAndCap方法，用于测试环境
    void getUdiskPathAndCap() {
        // 在测试环境中，我们不执行实际的lsblk命令
        // 而是根据标志决定是否清空diskInfos列表以模拟没有U盘的情况
        qDebug() << "TestablePage1::getUdiskPathAndCap() called, m_simulateNoUdisk =" << m_simulateNoUdisk << ", m_forceNoUdisk =" << m_forceNoUdisk;
        qDebug() << "TestablePage1::getUdiskPathAndCap() called, diskInfos size before =" << diskInfos.size();
        if (m_simulateNoUdisk) {
            diskInfos.clear();
            m_simulateNoUdisk = false;  // 重置标志
            qDebug() << "TestablePage1::getUdiskPathAndCap() cleared diskInfos due to m_simulateNoUdisk";
        } else if (m_forceNoUdisk) {
            // 完全模拟没有U盘的情况，不执行lsblk命令
            diskInfos.clear();
            qDebug() << "TestablePage1::getUdiskPathAndCap() cleared diskInfos due to m_forceNoUdisk";
        } else {
            // 在测试环境中，我们不执行实际的lsblk命令，而是保留现有的diskInfos列表
            // 这样可以确保我们的测试数据不会被清空
            qDebug() << "TestablePage1::getUdiskPathAndCap() - keeping existing diskInfos";
        }
        qDebug() << "TestablePage1::getUdiskPathAndCap() called, diskInfos size after =" << diskInfos.size();
    }
    
    
// 重写initControlQss方法，避免连接会导致显示模态对话框的事件处理器
    void initControlQss() {
        // 调用原始实现，但不连接会导致显示模态对话框的事件处理器
        edit = new line;
        edit->installEventFilter(this);
        topwidget=new QWidget(this);
        fram=new QFrame(this);
        fram->setFrameShape(QFrame::NoFrame);
        fram->setGeometry(40,95,450,320);
        fram->hide();
        topwidget->setFixedSize(450,320);
        tabUdisk2 =new QPushButton(fram);
        tabUdisk2->setFixedSize(128, 128);
        tabUdisk2->setProperty("isRoundButton", true);
        tabUdisk2->setIcon(QIcon::fromTheme("list-add-symbolic"));
        tabUdisk2->setIconSize(QSize(50,50));
        urlIso=new QLabel(topwidget);
        urlIso1=new QLabel(fram);
        QPixmap pix(":/data/button lightmode inactive.png");
        urlIso1->setText(tr("Drag and drop the image file here"));
        urlIso->setWordWrap(true);
        urlIso->setEnabled(false);
        urlIso->setText(tr("Click or drag to add a mirror file"));
        urlIso->setAlignment(Qt::AlignHCenter);
        connect(this, &Page1::setIsoPath, this, &Page1::dealIsoPath);
        urlIso1->setAlignment(Qt::AlignHCenter);
        urlIso1->setWordWrap(true);
        comboUdisk = new QComboBox(this);
        comboUdisk->setFixedSize(400,40);
        comboUdisk->setEnabled(false);
        connect(comboUdisk, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            if (this->m_canChange) {
                if (index >= 0 && m_udiskList.count() > index)
                    this->m_curUdiskName = this->m_udiskList.at(index);
                else
                    this->m_curUdiskName = "";
            }
        });
        QWidget *btnwidget = new QWidget(this);
        findIso=new QPushButton(btnwidget);
        findIso->setFixedSize(128,128);
        findIso->setProperty("isRoundButton", true);
        findIso->setIcon(QIcon::fromTheme("list-add-symbolic"));
        findIso->setIconSize(QSize(50,50));
        findIso->setAccessibleName("kylin-usb-creator_QPushButton_findIso");

        creatStart=new QPushButton(btnwidget);
        creatStart->setText(tr("Start"));
        creatStart->setEnabled(false);
        // 在测试环境中，我们不连接会导致显示模态对话框的事件处理器
        // connect(findIso,&QPushButton::clicked,this,[=]{
        //     isoPath = QFileDialog::getOpenFileName(this,tr("choose iso file"),QDir::homePath(),"ISO(*.iso)");
        //     dealSelectedFile(isoPath);
        // });
        connect(edit,&QLineEdit::textChanged,this,&Page1::ifStartBtnChange);
        // 在测试环境中，我们保留creatStart按钮的连接，以便测试相关代码
        connect(creatStart,&QPushButton::clicked,[=]{
            QString ss=diskInfos[comboUdisk->currentIndex()]->diskCapicity;
            QFileInfo info(isoPath);
            isoSize = info.size()/1024/1024;
            QString is=QString::number(isoSize,'f',1);
            isoSize=is.toFloat();
            if(ss.right(1)=="M"){
                ss=ss.left(ss.size()-1);
                USize=ss.toFloat();
                qDebug()<<__LINE__<<"TestablePage1::handleCreatStartClick"<<USize;
                if(isoSize>USize){
                    // 在测试环境中，我们不显示模态对话框，而是直接处理用户点击"Yes"的情况
                    // 直接返回，不发射starmake信号
                    return;
                }else{
                    emit starmake(isoPath,diskInfos[comboUdisk->currentIndex()]->devicePath);
                    // 在测试环境中，我们不调用DBus，而是直接返回
                    return;
                }
            }
            if(ss.right(1)=="G"){
                qDebug()<<__LINE__<<"TestablePage1::handleCreatStartClick"<<"ss==="<<ss;
                ss=ss.left(ss.size()-1);
                qDebug()<<__LINE__<<"TestablePage1::handleCreatStartClick"<<"ss-1==="<<ss;
                if(ss.contains(",")){
                    int commaPos=ss.indexOf(",");
                    ss[commaPos]='.';
                }
                USize=ss.toFloat()*1000;
                qDebug()<<__LINE__<<"TestablePage1::handleCreatStartClick"<<USize;
                if(isoSize>USize){
                    // 在测试环境中，我们不显示模态对话框，而是直接处理用户点击"Yes"的情况
                    // 直接返回，不发射starmake信号
                    return;
                }else{
                    emit starmake(isoPath,diskInfos[comboUdisk->currentIndex()]->devicePath);
                    // 在测试环境中，我们不调用DBus，而是直接返回
                    return;
                }
            }
        });
        
        QHBoxLayout *hlay=new QHBoxLayout(this);
        btnwidget->setLayout(hlay);
        hlay->addStretch(100);
        hlay->addWidget(creatStart);
        hlay->addStretch(100);
        QVBoxLayout *vlay1 = new QVBoxLayout(this);
        QVBoxLayout *vlay2 = new QVBoxLayout(this);
        topwidget->setLayout(vlay1);
        fram->setLayout(vlay2);
        vlay1->addWidget(findIso,0,Qt::AlignCenter);
        vlay1->addWidget(urlIso,0,Qt::AlignCenter);
        vlay1->addSpacing(70);
        vlay2->addWidget(tabUdisk2,0,Qt::AlignCenter);
        vlay2->addSpacing(70);
        vlay2->addWidget(urlIso1,0,Qt::AlignCenter);
        vlay2->addSpacing(70);
        QVBoxLayout *vlay =new QVBoxLayout(this);
        vlay->addSpacing(100);
        vlay->addWidget(fram,0,Qt::AlignCenter);
        vlay->addWidget(topwidget,0,Qt::AlignCenter);
        vlay->addSpacing(50);
        vlay->addWidget(comboUdisk,0,Qt::AlignCenter);
        vlay->addSpacing(15);
        QLabel *label = new QLabel;
        label->setText(tr("USB drive will be formatted,please backup your files!"));
        label->setEnabled(false);
        vlay->addWidget(label,0,Qt::AlignCenter);
        vlay->addSpacing(15);
        vlay->addWidget(btnwidget);
        vlay->addSpacing(30);
        // 在测试环境中，我们不初始化U盘插拔监控器
        // udiskPlugWatcherInit();
    }
// 重写与creatStart按钮点击相关的方法，避免在测试环境中显示模态对话框
    void handleCreatStartClick() {
        // 在测试环境中，我们不显示模态对话框，而是直接处理逻辑
        QString ss = diskInfos[comboUdisk->currentIndex()]->diskCapicity;
        QFileInfo info(isoPath);
        isoSize = info.size()/1024/1024;
        QString is = QString::number(isoSize,'f',1);
        isoSize = is.toFloat();
        
        if(ss.right(1)=="M"){
            ss = ss.left(ss.size()-1);
            USize = ss.toFloat();
            qDebug() << __LINE__ << __func__ << USize;
            if(isoSize > USize){
                // 在测试环境中，我们不显示模态对话框，而是直接处理用户点击"Yes"的情况
                // 直接返回，不发射starmake信号
                return;
            } else {
                emit starmake(isoPath, diskInfos[comboUdisk->currentIndex()]->devicePath);
                // 在测试环境中，我们不调用DBus，而是直接返回
                return;
            }
        }
        
        if(ss.right(1)=="G"){
            qDebug() << __LINE__ << __func__ << "ss===" << ss;
            ss = ss.left(ss.size()-1);
            qDebug() << __LINE__ << __func__ << "ss-1===" << ss;
            if(ss.contains(",")){
                int commaPos = ss.indexOf(",");
                ss[commaPos] = '.';
            }
            USize = ss.toFloat()*1000;
            qDebug() << __LINE__ << __func__ << USize;
            if(isoSize > USize){
                // 在测试环境中，我们不显示模态对话框，而是直接处理用户点击"Yes"的情况
                // 直接返回，不发射starmake信号
                return;
            } else {
                emit starmake(isoPath, diskInfos[comboUdisk->currentIndex()]->devicePath);
                // 在测试环境中，我们不调用DBus，而是直接返回
                return;
            }
        }
    }
    
    // 重写getStorageInfo方法，用于测试环境
    void getStorageInfo() {
        if (m_forceNoUdisk) {
            // 强制模拟没有U盘的情况
            m_udiskList.clear();
            comboUdisk->clear();
            comboUdisk->addItem(tr("No USB drive available"));
            comboUdisk->setEnabled(false);
            m_curUdiskName = "";
            creatStart->setEnabled(false);
        } else {
            // 调用原始实现
            Page1::getStorageInfo();
        }
    }
    
    // 提供公共接口来调用protected方法
    void callDragEnterEvent(QDragEnterEvent *event) {
        dragEnterEvent(event);
    }
    
    void callGetStorageInfo() {
        getStorageInfo();
    }
    
    void callDragLeaveEvent(QDragLeaveEvent *event) {
        dragLeaveEvent(event);
    }
    
    void callDropEvent(QDropEvent *event) {
        dropEvent(event);
    }
    
    bool callEventFilter(QObject *watched, QEvent *event) {
        return eventFilter(watched, event);
    }
    
    void callWait(uint sec) {
        wait(sec);
    }
    
    void callGetUdiskPathAndCap() {
        getUdiskPathAndCap();
    }
    
    void callGetUdiskName() {
        getUdiskName();
    }
    
    void callUdiskPlugWatcherInit() {
        udiskPlugWatcherInit();
    }
    
    void callInitControlQss() {
        initControlQss();
    }
    
    void callCreatStartSlots() {
        creatStartSlots();
    }
    
    void callDealDialogCancel() {
        dealDialogCancel();
    }
    
    void callDealSelectedFile(const QString &isoPath) {
        dealSelectedFile(isoPath);
    }
    
    void callDealIsoPath(const QString &isoPath) {
        dealIsoPath(isoPath);
    }
    
    QJsonArray callQStringToJsonArray(const QString &jsonString) {
        return QStringToJsonArray(jsonString);
    }
    
    // 添加公共方法来触发creatStart按钮点击事件
    void triggerCreatStartClick() {
        emit creatStart->clicked();
    }
    
    // 添加公共方法来获取creatStart按钮的状态
    bool isCreatStartEnabled() {
        return creatStart->isEnabled();
    }
    
    // 添加公共方法来检查creatStart按钮的属性
    QVariant getCreatStartProperty(const char* name) {
        return creatStart->property(name);
    }
    
    // 添加公共方法来获取themeStatus的值
    QString getThemeStatus() {
        return themeStatus;
    }
    
    // 辅助方法用于测试
    void simulateUdiskAdded() {
        // 模拟U盘添加
        AvailableDiskInfo *disk = new AvailableDiskInfo("/dev/sdb", "Test USB", "16G");
        diskInfos.append(disk);
        m_udiskList << "Test USB (/dev/sdb) 16G";
        comboUdisk->addItem("Test USB (/dev/sdb) 16G");
        comboUdisk->setEnabled(true);
    }
    
    void simulateNoUdisk() {
        // 模拟没有U盘
        diskInfos.clear();
        m_udiskList.clear();
        comboUdisk->clear();
        comboUdisk->addItem(tr("No USB drive available"));
        comboUdisk->setEnabled(false);
    }
    
    // 添加一个标志来控制是否在测试环境中模拟没有U盘的情况
    void simulateNoUdiskInNextCall() {
        m_simulateNoUdisk = true;
    }
    
    // 添加一个标志来强制模拟没有U盘的情况（不执行lsblk命令）
    void forceNoUdisk() {
        m_forceNoUdisk = true;
    }
    
    // 添加模拟QStorageInfo对象的方法
    void addMockStorageInfo(const QString &devicePath, const QString &displayName, quint64 bytesTotal) {
        MockStorageInfo mockInfo;
        mockInfo.devicePath = devicePath;
        mockInfo.displayName = displayName;
        mockInfo.bytesTotal = bytesTotal;
        m_mockStorageInfos.append(mockInfo);
    }
    
    // 重写getUdiskName方法，使用模拟的QStorageInfo对象
    void getUdiskName() {
        qDebug() << "TestablePage1::getUdiskName() called with mock storage info";
        
        foreach(AvailableDiskInfo* tmp, diskInfos) {
            foreach(const MockStorageInfo &mockDisk, m_mockStorageInfos) {
                if((mockDisk.devicePath.length() == 8 && tmp->devicePath == mockDisk.devicePath)) {
                    tmp->displayName = mockDisk.displayName;
                    float cap = mockDisk.bytesTotal;
                    tmp->diskCapicity = QString::number(cap/1000000000,'f',1) + 'G';
                    continue;
                }
                if(tmp->devicePath + '1' == mockDisk.devicePath) {
                    tmp->displayName = mockDisk.displayName;
                    float cap = mockDisk.bytesTotal;
                    tmp->diskCapicity = QString::number(cap / 1000000000,'f',1) + 'G';
                    continue;
                }
            }
        }
    }
    
private:
    bool m_simulateNoUdisk = false;
    bool m_forceNoUdisk = false;
    
    // 添加模拟QStorageInfo的结构体
    struct MockStorageInfo {
        QString devicePath;
        QString displayName;
        quint64 bytesTotal;
    };
    QList<MockStorageInfo> m_mockStorageInfos;
};

class TestPage1 : public QObject
{
    Q_OBJECT

private slots:
    // 测试套件管理
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 构造函数和初始化测试
    void testConstructor();
    void testInitControlQss();
    
    // 功能方法测试
    void testIfStartBtnChange();
    void testSetThemeStyleLight();
    void testSetThemeStyleDark();
    void testIsCapicityAvailable();
    void testCheckISO();
    void testWait();
    void testQStringToJsonArray();
    
    // 文件处理测试
    void testDealSelectedFile();
    void testDealSelectedFileWithValidPath();
    void testDealIsoPath();
    void testDealIsoPathWithValidPath();
    
    // U盘管理测试
    void testGetStorageInfo();
    void testGetStorageInfoImplementation();
    void testGetUdiskPathAndCap();
    void testGetUdiskName();
    void testUdiskPlugWatcherInit();
    
    // 事件处理测试
    void testEventFilter();
    void testEventFilterWithEditObjectAndMousePressEvent();
    void testEventFilterWithEditObjectAndNonMousePressEvent();
    void testEventFilterWithNonEditObject();
    void testEventFilterWithNoUdiskAvailable();
    void testDragEnterEvent();
    void testDragEnterEventWithValidIso();
    void testDragEnterEventWithInvalidFile();
    void testDragEnterEventWithMultipleFiles();
    void testDragLeaveEvent();
    //void testDropEvent();
    void testDropEventWithValidIso();
    void testDropEventWithEmptyUrls();
    
    // 信号槽测试
    void testDealAuthDialogClose();
    void testDealComboBoxChangeButton();
    void testDealDialogCancel();
    void testCreatStartSlots();
    
    // 集成测试
    void testCompleteWorkflow();
    void testUdiskPlugUnplugScenario();
    
    // 错误处理测试
    //void testErrorConditions();
    void testCapacityComparison();
    
    // 新增测试用例以提高覆盖率
    void testGetUdiskPathAndCapWithMockData();
    void testGetUdiskNameWithMatchingDevices();
    void testGetStorageInfoWithMultipleDevices();
    void testDealSelectedFileWithInvalidISOAndOk();
    void testDealSelectedFileWithInvalidISOAndCancel();
    void testDealIsoPathWithInvalidISOAndOk();
    void testDealIsoPathWithInvalidISOAndCancel();
    
    // 新增测试用例以进一步提高覆盖率
    void testGetStorageInfoEdgeCases();
    void testCreatStartButtonClickWithDifferentConditions();
    void testCreatStartButtonClickWithISOLargerThanUdisk();
    void testCreatStartButtonClickWithCommaInCapacity();
    void testCreatStartButtonClickWithISOLargerThanUdiskInGB();
    void testCreatStartButtonClickWithISOSmallerThanUdiskInGB();
    void testDealIsoPathWithInvalidISO();
    void testDealSelectedFileWithInvalidISO();
    void testGetStorageInfoWithNoDevices();
    void testCreatStartButtonClickWithMessageBox();
    void testDropEvent();
    
    // 新增测试用例以进一步提高覆盖率
    void testCreatStartButtonClickWithCommaInCapacityGB();
    void testStarmakeSignalEmission();
// 新增测试用例以提高对450-483行代码的覆盖率
void testDealSelectedFileWithInvalidISOAndOkResponse();
void testDealSelectedFileWithInvalidISOAndCancelResponse();
void testDealIsoPathWithInvalidISOAndOkResponse();
void testDealIsoPathWithInvalidISOAndCancelResponse();
// 新增测试用例以进一步提高对450-483行代码的覆盖率
void testDealSelectedFileWithInvalidISOAndOkBranch();
void testDealSelectedFileWithInvalidISONoUdisk();
void testDealIsoPathWithInvalidISOAndOkBranch();
void testDealIsoPathWithInvalidISONoUdisk();
// 新增测试用例以提高对292-310行代码的覆盖率
void testGetUdiskNameWithDeviceMatching();
// 新增测试用例以提高对327-344行代码的覆盖率
void testGetStorageInfoLines327To344();
void testGetStorageInfoLines327To344WithIsoPath();
void testGetStorageInfoLines327To344MultipleDisks();
// 新增测试用例以提高对87-128行代码的覆盖率
void testCreatStartButtonClickWithISOLargerThanUdiskInMB();
// 新增测试用例以提高对450-476行代码的覆盖率（模拟非测试模式）
void testDealSelectedFileWithInvalidISONonTestMode();
void testCreatStartButtonClickWithISOSmallerThanUdiskInMB();
void testCreatStartButtonClickWithCommaInCapacityG();
void testCreatStartButtonClickWithCommaInCapacityM();
void testCreatStartButtonClickWithISOLargerThanUdiskInG();
void testCreatStartButtonClickWithISOSmallerThanUdiskInG();
// 新增测试用例以提高对284-313行代码的覆盖率
void testGetUdiskNameWithDevicePathMatching();
void testGetUdiskNameWithPartitionMatching();
void testGetUdiskNameWithMultipleDevices();
// 新增测试用例以提高对515-533行代码的覆盖率
void testDealIsoPathWithInvalidISOAndOkBranchAdditional();
void testDealIsoPathWithInvalidISOAndCancelBranchAdditional();

// 新增测试用例以提高对398-420行代码的覆盖率
void testDealAuthDialogCloseDetailed();
void testSetThemeStyleLightDetailed();
void testSetThemeStyleDarkDetailed();
void testThemeStyleTransitions();

// 新增测试用例以提高对82-144行代码的覆盖率
    void testDealSelectedFileImproved();
    void testCheckISOImproved();
    // 新增测试用例以提高对82-144行代码的覆盖率
    void testCreatStartButtonClickWithDBusCall();
    void testCreatStartButtonClickWithCommaReplacement();
};

#endif // TEST_PAGE1_H