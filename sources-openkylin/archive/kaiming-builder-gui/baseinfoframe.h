#ifndef BASEINFOFRAME_H
#define BASEINFOFRAME_H

#include <globalinfo.h>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QMap>
#include <QVariant>

class BaseInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BaseInfoFrame(QWidget *parent = nullptr);
    void getData();
    void clear();

    void load(const QString &base, const QString &runtime, const QVector<QString> &depends);
    QString base() const {
        return m_baseInput->text();
    }
    void setBaseFocus() const {
        m_baseInput->setFocus();
    }

private slots:
    void addDependency(const QString& ref = "");
    // 删除依赖项
    void removeDependency(QWidget *dependRow);
    void openSearchDialog(const QString &kind, const QString &base, const QString &runtime);

protected:
    void updateGeometry();

private:
    QLineEdit *m_baseInput;
    QPushButton *m_baseSearchButton;
    QLineEdit *m_runtimeInput;
    QPushButton *m_runtimeSearchButton;
    QPushButton *m_addDependencyButton;
    QWidget *m_dependencyContainer;
    QVBoxLayout *m_dependencyLayout;

    QStringList m_selectedDepends;
};

#endif // BASEINFOFRAME_H
