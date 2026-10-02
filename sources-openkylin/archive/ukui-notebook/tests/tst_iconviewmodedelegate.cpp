// tst_iconviewmodedelegate.cpp
#include "tst_iconviewmodedelegate.h"
#include <QPainter>
#include <QStandardItemModel>

void TestIconViewModeDelegate::init()
{
    if (!qApp) {
        QSKIP("This test requires a GUI environment");
    }
    delegate = new iconViewModeDelegate(this);
    model = new QStandardItemModel(this);
}

void TestIconViewModeDelegate::cleanup()
{
    delete delegate;
    delete model;
}

void TestIconViewModeDelegate::testColorConversion()
{
    // 测试颜色转换
    QColor testColor(255, 128, 64);
    int colorInt = delegate->qcolorToInt(testColor);
    QColor convertedColor = delegate->intToQcolor(colorInt);
    
    QCOMPARE(convertedColor.red(), testColor.red());
    QCOMPARE(convertedColor.green(), testColor.green());
    QCOMPARE(convertedColor.blue(), testColor.blue());
}

void TestIconViewModeDelegate::testParseDateTime()
{
    QDateTime testDateTime = QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd hh:mm:ss");
    QString parsedDate = delegate->parseDateTime(testDateTime);
    
    // 验证解析后的日期时间格式
    QVERIFY(!parsedDate.isEmpty());
    QVERIFY(parsedDate.contains(":"));  // 应该包含时间分隔符
}

void TestIconViewModeDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QModelIndex index;
    
    QSize size = delegate->sizeHint(option, index);
    QCOMPARE(size, QSize(220, 246));  // 验证默认尺寸
}

void TestIconViewModeDelegate::testSetState()
{
    QModelIndex index = model->index(0, 0);
    
    // 测试不同状态设置
    delegate->setState(iconViewModeDelegate::Insert, index);
    QCOMPARE(delegate->animationState(), QTimeLine::Running);
    
    delegate->setState(iconViewModeDelegate::Normal, index);
    QTest::qWait(350);  // 等待动画完成
    QCOMPARE(delegate->animationState(), QTimeLine::NotRunning);
}

void TestIconViewModeDelegate::testSetAnimationDuration()
{
    int testDuration = 500;
    delegate->setAnimationDuration(testDuration);
    
    // 触发动画
    QModelIndex index = model->index(0, 0);
    delegate->setState(iconViewModeDelegate::Insert, index);
    
    // 验证动画持续时间
    QTest::qWait(testDuration + 50);
    QCOMPARE(delegate->animationState(), QTimeLine::NotRunning);
}

void TestIconViewModeDelegate::testSetActive()
{
    delegate->setActive(true);
    
    // 创建测试数据
    QStandardItem* item = new QStandardItem;
    item->setData(QString("Test Note"), NoteModel::NoteMdContent);
    item->setData(QDateTime::currentDateTime(), NoteModel::NoteLastModificationDateTime);
    model->appendRow(item);
    
    // 测试绘制
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 220, 246);
    QImage image(option.rect.size(), QImage::Format_ARGB32);
    QPainter painter(&image);
    
    delegate->paint(&painter, option, model->index(0, 0));
}

#include "moc_tst_iconviewmodedelegate.cpp"
