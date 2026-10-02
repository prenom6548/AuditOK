#include "outputedit.h"
#include <QScrollBar>

OutputEdit::OutputEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QScrollBar *scrollbar = verticalScrollBar();
    connect(scrollbar, &QScrollBar::sliderPressed, this, [=]() {
        this->m_isScrollBarInteracting = true;
    });
    connect(scrollbar, &QScrollBar::sliderReleased, this, [=]() {
        this->m_isScrollBarInteracting = false;
    });
}

void OutputEdit::appendText(const QString &text)
{
    if (m_isScrollBarInteracting) {
        m_currentScrollValue = verticalScrollBar()->value();
        appendPlainText(text);
        verticalScrollBar()->setValue(m_currentScrollValue);
    } else {
        appendPlainText(text);
        QScrollBar *scrollbar = verticalScrollBar();
        scrollbar->setValue(scrollbar->maximum());
    }
}

void OutputEdit::mousePressEvent(QMouseEvent *event)
{
    m_isScrollBarInteracting = true;
    QPlainTextEdit::mousePressEvent(event);
}

void OutputEdit::mouseReleaseEvent(QMouseEvent *event)
{
    m_isScrollBarInteracting = false;
    QPlainTextEdit::mouseReleaseEvent(event);
}
