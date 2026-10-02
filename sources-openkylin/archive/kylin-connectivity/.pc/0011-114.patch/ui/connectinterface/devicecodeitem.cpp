#include "devicecodeitem.h"
#include <QDebug>
#include <QRegExpValidator>
#include <QVariant>
#include <QFont>

const int LINE_WIDTH = 56;
const int LINE_HEIGHT = 56;
const QString PLACEHOLDER = " ";

DeviceCodeItem::DeviceCodeItem(QWidget *parent) : QLineEdit(parent)
{
    QFont font;
    font.setPointSizeF(26);
    setFont(font);
    setFixedSize(LINE_WIDTH, LINE_HEIGHT);
    setFocusPolicy(Qt::NoFocus);
    setAlignment(Qt::AlignCenter);
    setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]+$")));
    setCursor(Qt::ArrowCursor);
    setReadOnly(true);
    setText(PLACEHOLDER);
    connect(this, &DeviceCodeItem::textChanged, this, &DeviceCodeItem::slotTextChanged);
    connect(this, &DeviceCodeItem::cursorPositionChanged, this, &DeviceCodeItem::slotCursorPositionChanged);
}

void DeviceCodeItem::setInput()
{
    m_isInput = true;
    setFocus();
}

void DeviceCodeItem::exitInput()
{
    m_isInput = false;
    setFocusPolicy(Qt::NoFocus);
}

void DeviceCodeItem::clearText()
{
    m_isActiveDelete = true;
    setText(PLACEHOLDER);
}

void DeviceCodeItem::setHead()
{
    m_isHead = true;
}

void DeviceCodeItem::setEnd()
{
    m_isEnd = true;
}

void DeviceCodeItem::mouseMoveEvent(QMouseEvent *)
{
    return;
}

// 同上，鼠标双击什么也不干
void DeviceCodeItem::mouseDoubleClickEvent(QMouseEvent *)
{
    return;
}

// 同上，鼠标双击什么也不干
void DeviceCodeItem::mousePressEvent(QMouseEvent *)
{
    if (m_isInput) {
        setFocus();
    }
    return;
}

void DeviceCodeItem::keyPressEvent(QKeyEvent *e)
{
    setReadOnly(false);
    QLineEdit::keyPressEvent(e);
    setReadOnly(true);
}

void DeviceCodeItem::slotTextChanged()
{
    QString str = text();
    if (str.size() > 2) {
        m_isActiveDelete = true;
        str.chop(1);
        setText(str);
        return;
    }
    if (!str.isEmpty() && str != PLACEHOLDER) {
        Q_EMIT sigTextInput(str.remove(PLACEHOLDER));
    }
    if ((m_isHead || m_isEnd) && str.size() == 0) {
        if (m_isEnd) {
            Q_EMIT sigBackspace();
        }
        m_isActiveDelete = true;
        setText(PLACEHOLDER);
    }
}

void DeviceCodeItem::slotCursorPositionChanged()
{
    if (0 == cursorPosition() && !m_isActiveDelete) {
        Q_EMIT sigBackspace();
    }
    if (m_isActiveDelete) {
        m_isActiveDelete = false;
    }
}
