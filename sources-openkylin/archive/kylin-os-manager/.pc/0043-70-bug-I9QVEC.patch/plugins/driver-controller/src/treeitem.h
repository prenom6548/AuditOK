#ifndef TREEITEM_H
#define TREEITEM_H

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>

class TreeItem : public QWidget {
Q_OBJECT
public:
    TreeItem(QString name, QString content, QWidget *parent = nullptr);

    void setLabelText(QString text);

private:
    QString m_name;
    QString m_content;

    QLabel *m_nameLabel = nullptr;
    QLabel *m_contentLabel = nullptr;

    QHBoxLayout *m_layout = nullptr;

    void initGui();
};


#endif //TREEITEM_H
