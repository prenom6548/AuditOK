#include "tst_listviewmodedelegate.h"
#include <QColor>
#include <QDateTime>
#include <QPainter>
#include <QStyleOptionViewItem>

void TestListViewModeDelegate::init() {
    // 初始化测试环境
    m_delegate = new listViewModeDelegate();
    m_model = new QStandardItemModel();

    // 创建测试数据项
    QStandardItem *item = new QStandardItem("Test Note");
    item->setData(QColor(255, 0, 0).name(), NoteModel::NoteColor);
    item->setData("Test Title", NoteModel::NoteFullTitle);
    item->setData(QDateTime::currentDateTime(), NoteModel::NoteLastModificationDateTime);
    m_model->appendRow(item);

    m_testIndex = m_model->index(0, 0);
}

void TestListViewModeDelegate::cleanup() {
    // 清理测试环境
    delete m_delegate;
    delete m_model;
}

void TestListViewModeDelegate::testConstructor() {
    QVERIFY(m_delegate != nullptr);

    // 检查默认状态
    QCOMPARE(m_delegate->animationState(), QTimeLine::NotRunning);
}

void TestListViewModeDelegate::testColorConversion() {
    // 测试 QColor 到 int 的转换
    QColor testColor(123, 45, 67);
    int colorInt = m_delegate->qcolorToInt(testColor);

    // 转换回 QColor
    QColor convertedColor = m_delegate->intToQcolor(colorInt);

    QCOMPARE(convertedColor.red(), testColor.red());
    QCOMPARE(convertedColor.green(), testColor.green());
    QCOMPARE(convertedColor.blue(), testColor.blue());
}

void TestListViewModeDelegate::testDateTimeParsing() {
    QDateTime testDateTime = QDateTime::currentDateTime();

    // 模拟12小时和24小时制
    m_delegate->m_timeZone = "12";
    QString formattedTime12 = m_delegate->parseDateTime(testDateTime);

    m_delegate->m_timeZone = "24";
    QString formattedTime24 = m_delegate->parseDateTime(testDateTime);

    // 检查格式是否正确
    QVERIFY(!formattedTime12.isEmpty());
    QVERIFY(!formattedTime24.isEmpty());
    QVERIFY(formattedTime12 != formattedTime24);
}

void TestListViewModeDelegate::testStateAndAnimation() {
    // 测试状态变更
    m_delegate->setState(listViewModeDelegate::Insert, m_testIndex);
    QCOMPARE(m_delegate->m_state, listViewModeDelegate::Insert);

    m_delegate->setState(listViewModeDelegate::Remove, m_testIndex);
    QCOMPARE(m_delegate->m_state, listViewModeDelegate::Remove);

    m_delegate->setState(listViewModeDelegate::Normal, m_testIndex);
    QCOMPARE(m_delegate->m_state, listViewModeDelegate::Normal);
}

void TestListViewModeDelegate::testPaintMethods() {
    // 创建模拟 QPainter 和 QStyleOptionViewItem
    QPixmap pixmap(200, 200);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 82);
    option.state = QStyle::State_Enabled;

    // 尝试调用 paint 方法（注意这只是检查是否会崩溃）
    try {
        m_delegate->paint(&painter, option, m_testIndex);
        QVERIFY(true);  // 如果没有抛出异常，则通过
    } catch (...) {
        QFAIL("Paint method threw an unexpected exception");
    }
}

void TestListViewModeDelegate::testSizeHint() {
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 82);

    QSize sizeHint = m_delegate->sizeHint(option, m_testIndex);

    // 检查大小提示
    QCOMPARE(sizeHint.height(), 82);  // 默认行高
    QVERIFY(sizeHint.width() > 0);
}

void TestListViewModeDelegate::testSetActive() {
    // 测试设置活跃状态
    m_delegate->setActive(true);
    QVERIFY(m_delegate->m_isActive);

    m_delegate->setActive(false);
    QVERIFY(!m_delegate->m_isActive);
}

void TestListViewModeDelegate::testSetCurrentSelectedIndex() {
    // 测试设置当前选中索引
    m_delegate->setCurrentSelectedIndex(m_testIndex);

    QCOMPARE(m_delegate->m_currentSelectedIndex, m_testIndex);
}

// 用于生成测试主函数的宏
//QTEST_MAIN(TestListViewModeDelegate)

// 包含自动生成的 moc 文件
#include "tst_listviewmodedelegate.moc"
