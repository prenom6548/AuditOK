#ifndef OUTPUTEDIT_H
#define OUTPUTEDIT_H
#include <QPlainTextEdit>

class OutputEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit OutputEdit(QWidget *parent = nullptr);

    void appendText(const QString &text);

public:
    bool m_isScrollBarInteracting = false;
    int m_currentScrollValue = 0;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // OUTPUTEDIT_H
