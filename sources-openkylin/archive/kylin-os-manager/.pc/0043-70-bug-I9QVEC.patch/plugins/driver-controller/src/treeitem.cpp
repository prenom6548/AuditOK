#include "treeitem.h"

TreeItem::TreeItem(QString name, QString content, QWidget *parent)
        : m_name(name),
          m_content(content),
          QWidget(parent),
          m_nameLabel(new QLabel()),
          m_contentLabel(new QLabel()),
          m_layout(new QHBoxLayout()){
    initGui();
}

void TreeItem::initGui() {
    m_nameLabel->setText(m_name);
    m_contentLabel->setText(m_content);
    m_nameLabel->setMinimumWidth(148);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addSpacing(28);
    m_layout->addWidget(m_nameLabel);
    m_layout->addWidget(m_contentLabel);
    m_layout->addStretch();
    this->setLayout(m_layout);
    this->setFixedHeight(40);
}

void TreeItem::setLabelText(QString text){
    m_contentLabel->setText(text);
}