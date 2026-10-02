#include "programmodel.h"

ProgramModel::ProgramModel(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    m_display = new ProgramDisplay();
    m_display->setAccessibleName("kylin-calculator_ProgramDisplay_m_display");
    m_toolbar = new ToolBar();
    m_toolbar->setAccessibleName("kylin-calculator_ToolBar_m_toolbar");
    m_binWin = new BinaryKeyboary();
    m_binWin->setAccessibleName("kylin-calculator_BinaryKeyboary_m_binWin");
    m_keyboary = new ProgramKeyboary();
    m_keyboary->setAccessibleName("kylin-calculator_ProgramKeyboary_m_keyboary");

    m_display->setMinimumHeight(PROGRAM_DISPLAY_HEIGHT);
    m_toolbar->setMinimumHeight(PROGRAM_TOOL_HEIGHT);
    m_binWin->setMinimumHeight(PROGRAM_BIN_HEIGHT);
    m_keyboary->setMinimumHeight(PROGRAM_KEYBOARY_HEIGHT);


    // 连接槽
    connect(m_keyboary, &ProgramKeyboary::sigBtnClicked, this, &ProgramModel::slotKeyBtnClicked);
    connect(m_toolbar, &ToolBar::sigBoxValueChanged, this, &ProgramModel::slotBoxValueChanged);
    connect(m_toolbar, &ToolBar::sigBtnClicked, this, &ProgramModel::slotToolBtnClicked);

    // 布局
    m_layout = new QVBoxLayout();
    m_layout->setSpacing(0);
    m_layout->addWidget(m_display, 228);
    m_layout->addSpacing(2);
    m_layout->addWidget(m_toolbar, 60);
    m_layout->addSpacing(4);
    m_layout->addWidget(m_binWin, 168);
    m_layout->addSpacing(4);
    m_layout->addWidget(m_keyboary, 572);
    m_layout->setContentsMargins(4, 0, 4, 4);
    this->setLayout(m_layout);

    // 初始隐藏二进制显示
    m_binWin->hide();
}

void ProgramModel::setLightUI()
{
    // 浅色模式
    m_display->setLightUI();
}

void ProgramModel::setDarkUI()
{
    // 深色模式
    m_display->setDarkUI();
}

void ProgramModel::handleAC()
{
    m_display->setIsInput(true);
    m_toolbar->setBaseEnabled(true);

    m_display->setCurLab("0");
    m_display->setBudLab("");
    m_display->setHisLab("");
    m_display->setCodeLab("");

    m_isClearResult = QString("TRUE");

    m_result = {"0", "0", "0", "0=0", "0", "0", "", "TRUE", "TRUE"};

    m_binWin->clear();
}

void ProgramModel::handlePasteError(void)
{
    handleAC();
    m_display->setBudLab(tr("Input error!"));
}

bool ProgramModel::isBinWinShow()
{
    if (QString("TRUE") == m_binWinShowState) {
        return true;
    } else {
        return false;
    }
}

QString ProgramModel::getFormulaNow()
{
    return m_display->data().at(LAB_CURRENT_EXPRESSION);
}

void ProgramModel::setFormulaNow(QString formula)
{
    for (int i = 0; i < formula.size(); i++) {
        this->slotKeyBtnClicked(QString(formula[i]));
    }
}

void ProgramModel::handleDel()
{
    if (1 == m_result[MARK_CURRENT_EXPRESSION].size()) {
        if (QString("0") == m_result[MARK_CURRENT_EXPRESSION]) {
            m_display->setBudLab("0");
            m_toolbar->setBaseEnabled(true);
            return;
        }

        m_result[MARK_CURRENT_EXPRESSION] = QString("0");
        m_toolbar->setBaseEnabled(true);
    } else {
        m_result[MARK_CURRENT_EXPRESSION].chop(1);
        if (0 == ProcessFormula::getInstance()->opNum(m_result[MARK_CURRENT_EXPRESSION])) {
            m_toolbar->setBaseEnabled(true);
        }
    }

    m_result = ProcessFormula::getInstance()->process(m_result[MARK_CURRENT_EXPRESSION]);
    m_display->setIsInput(true);
    m_display->setCurLab(m_result[MARK_DISPLY_EXPRESSION]);
    m_display->setBudLab(m_result[MARK_BUDGET_RESULTS]);

    if (QString("FALSE") == m_result[MARK_IS_RIGHT]) {
        // 计算错误
        m_binWin->clear();
        m_display->setCodeLab("");
        return;
    }

    // 计算正确
    setBinCodeData();
}

void ProgramModel::handleClear()
{
    m_display->setIsInput(true);
    m_toolbar->setBaseEnabled(true);

    m_display->setCurLab("0");
    m_display->setBudLab("");
    m_display->setCodeLab("");

    m_isClearResult = QString("TRUE");

    m_result = {"0", "0", "0", "0=0", "0", "0", "", "TRUE", "TRUE"};

    m_binWin->clear();
}

void ProgramModel::handleEqual()
{
    if (!m_display->isInput()) {
        m_result[MARK_CURRENT_EXPRESSION].chop(1);
    }
    m_display->setIsInput(true);
    m_result = ProcessFormula::getInstance()->process(m_result[MARK_CURRENT_EXPRESSION]);
    if ("FALSE" == m_result[MARK_IS_RIGHT]) {
        m_display->setBudLab(m_result[MARK_CAL_RESULTS]);
        // 当前计算结果错误, 清空二进制显示
        m_binWin->clear();
        m_display->setCodeLab("");
        return;
    }

    // 计算正确
    m_isClearResult = QString("TRUE");

    m_display->setCurLab(m_result[MARK_CAL_RESULTS]);
    m_display->setBudLab(m_result[MARK_BUDGET_RESULTS]);
    m_display->setHisLab(m_result[MARK_HISTOR_RECORDS]);

    // 启用进制转换和字符转换
    m_toolbar->setBaseEnabled(true);
    // 计算表达式设置成计算结果
    m_result[MARK_CURRENT_EXPRESSION] = m_result[MARK_CAL_RESULTS];

    setBinCodeData();
}

void ProgramModel::handleNum(QString value)
{
    int formulaSize = m_result[MARK_CURRENT_EXPRESSION].size();
    if (formulaSize >= 2) {
        if (QString("0") == m_result[MARK_CURRENT_EXPRESSION].right(1)
            && (ProcessFormula::getInstance()->containsBinarry(
                    m_result[MARK_CURRENT_EXPRESSION].mid(formulaSize - 2, 1))
                || ProcessFormula::getInstance()->containsUnary(
                    m_result[MARK_CURRENT_EXPRESSION].mid(formulaSize - 2, 1))
                || QString("(") == m_result[MARK_CURRENT_EXPRESSION].mid(formulaSize - 2, 1))) {
            m_result[MARK_CURRENT_EXPRESSION].chop(1);
        }
    }
    if (QString("0") == m_result[MARK_CURRENT_EXPRESSION]) {
        // 处理表达式
        m_result[MARK_CURRENT_EXPRESSION].clear();
    }
    // 数字
    if (QString("TRUE") == m_isClearResult) {
        // 需要清空前面的表达式
        m_isClearResult = QString("FALSE");

        // 处理表达式
        m_result[MARK_CURRENT_EXPRESSION].clear();
    }

    if (QString(")") == value && m_result[MARK_CURRENT_EXPRESSION].isEmpty()) {
        m_result[MARK_CURRENT_EXPRESSION] = "0";
    }

    // 处理表达式
    m_result = ProcessFormula::getInstance()->process(m_result[MARK_CURRENT_EXPRESSION] + value);
    m_display->setCurLab(m_result[MARK_DISPLY_EXPRESSION]);
    m_display->setBudLab(m_result[MARK_BUDGET_RESULTS]);

    if (QString("FALSE") == m_result[MARK_IS_RIGHT]) {
        // 计算错误
        m_binWin->clear();
        m_display->setCodeLab("");
        m_toolbar->setBaseEnabled(false);
        return;
    }

    if (0 != ProcessFormula::getInstance()->opNum(m_result[MARK_CURRENT_EXPRESSION])) {
        // 表达式有操作符，不能进行进制转换和字符转换
        m_toolbar->setBaseEnabled(false);
    } else {
        m_toolbar->setBaseEnabled(true);
    }

    // 计算正确
    setBinCodeData();
}

void ProgramModel::handleOp(QString value)
{
    int formulaSize = m_result[MARK_CURRENT_EXPRESSION].size();
    if (formulaSize >= 2) {
        if (ProcessFormula::getInstance()->containsBinarry(value)
            && ProcessFormula::getInstance()->containsBinarry(m_result[MARK_CURRENT_EXPRESSION].right(1))) {
            if (value == m_result[MARK_CURRENT_EXPRESSION].right(1)) {
                m_display->setBudLab(tr("Input error!"));
                return;
            }
            m_result[MARK_CURRENT_EXPRESSION].chop(1);
        }
    }

    // 表达式有操作符，不能进行进制转换和字符转换
    m_toolbar->setBaseEnabled(false);

    m_isClearResult = QString("FALSE");
    // 处理表达式
    m_result = ProcessFormula::getInstance()->process(m_result[MARK_CURRENT_EXPRESSION] + value);
    m_display->setCurLab(m_result[MARK_DISPLY_EXPRESSION]);
    m_display->setBudLab(m_result[MARK_BUDGET_RESULTS]);

    if (QString("FALSE") == m_result[MARK_IS_RIGHT]) {
        // 计算错误
        m_binWin->clear();
        m_display->setCodeLab("");
        return;
    }

    setBinCodeData();

    return;
}

void ProgramModel::handleBase(int base)
{
    m_isRepeat = QString("FALSE");
    // 进制转换
    QString baseValue = ProcessFormula::getInstance()->baseHandle(m_result[MARK_CAL_RESULTS], base);

    // 处理进制转换的结果
    m_result[MARK_CURRENT_EXPRESSION] = baseValue;
    m_result[MARK_DISPLY_EXPRESSION] = baseValue;
    m_result[MARK_BUDGET_RESULTS] = baseValue;
    m_result[MARK_CAL_RESULTS] = baseValue;
    m_result[MARK_HISTOR_RECORDS] = m_result[MARK_DISPLY_EXPRESSION] + QString("=") + baseValue;

    m_display->setCurLab(m_result[MARK_CAL_RESULTS]);
    m_display->setBudLab(m_result[MARK_BUDGET_RESULTS]);

    // 设置进制转换后禁用按钮
    m_keyboary->setBtnEnable(base);

    return;
}

void ProgramModel::setBinCodeData()
{
    // 设置二进制显示
    if (m_display->isInput()) {
        m_binWin->clear();
        m_binWin->setData(m_result[MARK_BIN_RESULTS]);
        // 设置字符显示
        m_display->setCodeLab(m_result[MARK_CODE_RESULTS]);
    } else {
        m_result[MARK_CURRENT_EXPRESSION].chop(1);
    }
}

void ProgramModel::slotKeyBtnClicked(QString value)
{
    if (QString("AC") == value) {
        m_isRepeat = QString("FALSE");
        // 处理按键AC
        handleAC();
        return;
    }
    if (QString("Clear") == value) {
        m_isRepeat = QString("FALSE");
        // 处理按键Clear
        handleClear();
        return;
    }
    if (QString("Delete") == value) {
        m_isRepeat = QString("FALSE");
        handleDel();
        return;
    }
    if (QString("Equal") == value) {
        if (QString("TRUE") == m_isRepeat) {
            m_display->setBudLab(tr("Input error!"));
            return;
        }
        m_isRepeat = QString("TRUE");
        // 处理等号
        handleEqual();
        return;
    }
    if (!m_display->isInput()) {
        m_isRepeat = QString("FALSE");
        // 显示屏不能继续输入
        return;
    }

    if (m_keyboary->containsNum(value) || QString("(") == value || QString(")") == value) {
        m_isRepeat = QString("FALSE");
        // 如果是操作数， 或者是左括号（左括号更操作数的处理逻辑一样）
        handleNum(value);
        return;
    }
    m_isRepeat = QString("FALSE");
    // 操作符
    handleOp(value);
    return;
}

void ProgramModel::slotBoxValueChanged(int index)
{
    m_isRepeat = QString("FALSE");
    switch (index) {
    case 0:
        // 64位
        ProcessFormula::getInstance()->digitHandle(DIGIT_64);
        break;
    case 1:
        // 32位
        ProcessFormula::getInstance()->digitHandle(DIGIT_32);
        break;
    case 2:
        // 16位
        ProcessFormula::getInstance()->digitHandle(DIGIT_16);
        break;
    case 3:
        // 8位
        ProcessFormula::getInstance()->digitHandle(DIGIT_8);
        break;
    default:
        break;
    }

    if (QString("TRUE") == m_result[MARK_IS_RIGHT]) {
        // 当前表达式计算正确，将结果进行合法化
        QString result = ProcessFormula::getInstance()->legal(m_result[MARK_CAL_RESULTS]);
        m_result[MARK_CURRENT_EXPRESSION] = result;
        m_result[MARK_BUDGET_RESULTS] = result;
        m_result[MARK_CAL_RESULTS] = result;
        m_result[MARK_HISTOR_RECORDS] = m_result[MARK_DISPLY_EXPRESSION] + QString("=") + result;
        m_result[MARK_BIN_RESULTS] = ProcessFormula::getInstance()->bin(result);
        m_result[MARK_CODE_RESULTS] = ProcessFormula::getInstance()->code(result);
        m_display->setCurLab(m_result[MARK_CURRENT_EXPRESSION]);
        m_display->setBudLab(m_result[MARK_BUDGET_RESULTS]);

        setBinCodeData();
    }
}

void ProgramModel::slotToolBtnClicked(QString value)
{
    if (QString("ASCII") == value || QString("Unicode") == value) {
        ProcessFormula::getInstance()->setCode(value);
        if (QString("TRUE") == m_result[MARK_IS_RIGHT]) {
            m_display->setCodeLab(ProcessFormula::getInstance()->code(m_result[MARK_CAL_RESULTS]));
            return;
        }
        m_display->setCodeLab("");
        return;
    }
    if (QString("cancel") == value) {
        ProcessFormula::getInstance()->setCode("");
        m_display->setCodeLab("");
        return;
    }
    if (QString(tr("ShowBinary")) == value) {
        this->parentWidget()->window()->setFixedSize(QSize(PROGRAM_WIN_WIDTH, PROGRAM_WIN_HEIGHT + PROGRAM_BIN_HEIGHT));
        m_binWinShowState = QString("TRUE");
        m_binWin->show();
        return;
    }
    if (QString(tr("HideBinary")) == value) {
        m_binWin->hide();
        this->parentWidget()->window()->setFixedSize(QSize(PROGRAM_WIN_WIDTH, PROGRAM_WIN_HEIGHT));
        m_binWinShowState = QString("FALSE");
        return;
    }
    if (QString("MS") == value) {
        m_display->setHisLab(m_result[MARK_HISTOR_RECORDS]);
        return;
    }
    if (QString("8") == value) {
        handleBase(BASE_OCT);
        return;
    }
    if (QString("10") == value) {
        handleBase(BASE_DEC);
        return;
    }
    if (QString("16") == value) {
        handleBase(BASE_HEX);
        return;
    }
}

void ProgramModel::keyPressEvent(QKeyEvent *event)
{
    m_keyboary->keyPressEvent(event);

    QWidget::keyPressEvent(event);
}
