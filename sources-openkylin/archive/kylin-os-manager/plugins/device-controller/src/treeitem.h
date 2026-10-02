#ifndef TREEITEM_H
#define TREEITEM_H

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include "include/text_label.h"
#include "QGSettings/qgsettings.h"

class TreeItem : public QWidget {
Q_OBJECT
public:
    TreeItem(QString name, QString content, QWidget *parent = nullptr);

    void setLabelText(QString text);
    QGSettings *m_themeData = nullptr;

protected:
    void mousePressEvent(QMouseEvent *mouseEvent) override;

private:
    QString m_name;
    QString m_content;

    QLabel *m_nameLabel = nullptr;
    K::TextLabel *m_contentLabel = nullptr;

    QHBoxLayout *m_layout = nullptr;

    void initGui();

    void initConnect();

    QWidget *m_parent = nullptr;
private slots:

    void setLabelFont();

};


#endif //TREEITEM_H
