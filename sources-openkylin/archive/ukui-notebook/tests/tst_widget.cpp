#include "tst_widget.h"
#include <QSignalSpy>
#include <QMenu>
#include <QAction>

void TestWidget::init()
{
    // 创建测试环境
    m_widget = new Widget();
    m_noteModel = m_widget->m_noteModel;
    m_proxyModel = m_widget->m_proxyModel;
}

void TestWidget::cleanup()
{
    // 清理测试环境
    delete m_widget;
}

void TestWidget::testConstructor()
{
    // 测试构造函数基本功能
    QVERIFY(m_widget != nullptr);
    QVERIFY(m_noteModel != nullptr);
    QVERIFY(m_proxyModel != nullptr);
}

void TestWidget::testInitialization()
{
    // 测试初始化相关属性
    QVERIFY(m_widget->m_noteView != nullptr);
    QVERIFY(m_widget->m_newKynote != nullptr);
    QVERIFY(m_widget->m_viewChangeButton != nullptr);
}

void TestWidget::testCreateNewNote()
{
    // 记录初始笔记数量
    int initialNoteCount = m_proxyModel->rowCount();

    // 创建新笔记
    m_widget->createNewNote();

    // 验证笔记数量增加
    QCOMPARE(m_proxyModel->rowCount(), initialNoteCount + 1);

    // 验证最新笔记被选中
    QModelIndex currentIndex = m_widget->m_currentSelectedNoteProxy;
    QVERIFY(currentIndex.isValid());
}

void TestWidget::testGenerateNote()
{
    // 生成笔记
    int testNoteId = 100;
    NoteData *generatedNote = m_widget->generateNote(testNoteId);

    // 验证生成的笔记属性
    QCOMPARE(generatedNote->id(), testNoteId);
    QVERIFY(generatedNote->creationDateTime().isValid());
    QVERIFY(generatedNote->lastModificationdateTime().isValid());

    delete generatedNote;
}

void TestWidget::testCreateNewNoteIfEmpty()
{
    // 清空现有笔记
    m_widget->clearNoteSlot();

    // 触发创建新笔记
    m_widget->createNewNoteIfEmpty();

    // 验证已创建笔记
    QCOMPARE(m_proxyModel->rowCount(), 1);
}

void TestWidget::testDeleteSelectedNote()
{
    // 确保至少有一个笔记
    m_widget->createNewNote();
    int initialNoteCount = m_proxyModel->rowCount();

    // 删除选中的笔记
    m_widget->deleteSelectedNote();

    // 验证笔记数量减少
    QCOMPARE(m_proxyModel->rowCount(), initialNoteCount - 1);
}

void TestWidget::testClearNoteSlot()
{
    // 创建多个笔记
    m_widget->createNewNote();
    m_widget->createNewNote();
    m_widget->createNewNote();

    // 清空笔记
    m_widget->clearNoteSlot();

    // 验证笔记数量为0
    QCOMPARE(m_proxyModel->rowCount(), 0);
}

void TestWidget::testSearchFunctionality()
{
    // 创建多个笔记
    m_widget->createNewNote();
    m_widget->createNewNote();

    // 设置测试文本
    m_widget->m_editors[0]->ui->textEdit->setText("Test Search Note 1");
    m_widget->m_editors[1]->ui->textEdit->setText("Test Search Note 2");

    // 模拟搜索
    //    m_widget->onSearchEditTextChanged("Test Search");

    //    // 验证搜索结果
    //    QCOMPARE(m_proxyModel->rowCount(), 2);

    //    // 清空搜索
    //    m_widget->onSearchEditTextChanged("");
    //    QCOMPARE(m_proxyModel->rowCount(), 2);
}

void TestWidget::testOnSearchEditTextChanged()
{
    // 确保测试开始时有干净的环境
    //    m_widget->clearNoteSlot();

    //    // 创建多个笔记
    //    m_widget->createNewNote();
    //    m_widget->createNewNote();

    //    // 确保编辑器已经正确创建
    //    QVERIFY(m_widget->m_editors.size() >= 2);

    //    // 设置测试文本
    //    m_widget->m_editors[0]->ui->textEdit->setText("Unique Test 1");
    //    m_widget->m_editors[1]->ui->textEdit->setText("Another Test 2");

    //    // 触发文本改变
    //    m_widget->onTextEditTextChanged(m_widget->m_editors[0]->m_noteId, 0);
    //    m_widget->onTextEditTextChanged(m_widget->m_editors[1]->m_noteId, 1);

    //    // 搜索唯一内容
    //    m_widget->m_searchLine->setText("Unique");
    //    m_widget->onSearchEditTextChanged("Unique");

    //    // 使用处理事件以确保异步操作完成
    //    QTest::qWait(500);

    //    // 验证只有一个笔记匹配
    //    QCOMPARE(m_proxyModel->rowCount(), 1);

    //    // 清理搜索
    //    m_widget->m_searchLine->clear();
    //    m_widget->onSearchEditTextChanged("");

    //    // 再次等待处理
    //    QTest::qWait(500);

    //    // 验证所有笔记都显示
    //    QCOMPARE(m_proxyModel->rowCount(), 2);
}

void TestWidget::testChangePageSlot()
{
    // 初始为列表模式
    QCOMPARE(m_widget->getListFlag(), 1);

    // 切换到图标模式
    m_widget->changePageSlot();
    QCOMPARE(m_widget->getListFlag(), 0);

    // 切换回列表模式
    m_widget->changePageSlot();
    QCOMPARE(m_widget->getListFlag(), 1);
}

void TestWidget::testSortSlot()
{
    // 创建多个笔记以便排序
    m_widget->createNewNote();
    m_widget->createNewNote();
    m_widget->createNewNote();

    // 触发排序
    m_widget->sortSlot(1);  // 按时间排序

    // 可能需要更复杂的验证逻辑来检查排序
    QVERIFY(m_proxyModel->rowCount() > 0);
}

void TestWidget::testListDoubleClickSlot()
{
    // 创建笔记
    m_widget->createNewNote();

    // 获取第一个笔记的索引
    QModelIndex firstNoteIndex = m_proxyModel->index(0, 0);

    // 模拟双击
    m_widget->listDoubleClickSlot(firstNoteIndex);

    // 验证是否打开了编辑器
    QVERIFY(!m_widget->m_editors.empty());
}

void TestWidget::testOpenMemoWithId()
{
    // 创建笔记
    m_widget->createNewNote();

    // 获取第一个笔记的ID
    int noteId = m_proxyModel->index(0, 0).data(NoteModel::NoteID).toInt();

    // 打开指定ID的备忘录
    m_widget->openMemoWithId(noteId);

    // 验证是否打开了编辑器
    QVERIFY(!m_widget->m_editors.empty());
}

void TestWidget::testExitSlot()
{
    // 记录初始窗口可见性
    bool wasVisible = m_widget->isVisible();

    // 模拟退出
    m_widget->exitSlot();

    // 验证窗口隐藏
    QVERIFY(m_widget->isHidden());

    // 可选：恢复窗口可见性（如果测试需要）
    if (wasVisible)
    {
        m_widget->show();
    }
}

void TestWidget::testKeyPressEvents()
{
    // 创建测试事件
    QKeyEvent ctrlEEvent(QEvent::KeyPress, Qt::Key_E, Qt::ControlModifier);
    m_widget->keyPressEvent(&ctrlEEvent);

    // 验证搜索框获得焦点（这可能需要根据实际实现调整）
    QVERIFY(m_widget->m_searchLine->hasFocus());
}

void TestWidget::testContextMenu()
{
    // 创建笔记
    m_widget->createNewNote();

    // 创建测试菜单动作
    QMenu testMenu;
    QAction deleteAction(tr("delete this note"), &testMenu);
    QAction openAction(tr("open this note"), &testMenu);
    QAction clearAction(tr("clear note list"), &testMenu);

    // 测试删除动作
    m_widget->triggerMenu(&deleteAction);
    QCOMPARE(m_proxyModel->rowCount(), 0);
}

// 用于生成测试主函数的宏
//QTEST_MAIN(TestWidget)

// 包含自动生成的 moc 文件
#include "moc_tst_widget.cpp"
