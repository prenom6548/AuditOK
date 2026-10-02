#include "loaddialog.h"

#include <QDebug>
#define LOAD_W 60
#define LOAD_H 60

LoadDialog::LoadDialog(QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::ApplicationModal);  // 模态窗口
    setFixedSize(LOAD_W, LOAD_H);
    m_loadAnimation = new LoadAnimation(this);
}

void LoadDialog::setHighLight()
{
    m_loadAnimation->setHighLight();
}
