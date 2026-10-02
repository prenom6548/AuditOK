#ifndef SETUPINFOFRAME_H
#define SETUPINFOFRAME_H

#include <globalinfo.h>
#include <QLineEdit>
#include <QFrame>
#include <QComboBox>
#include <QTextEdit>
#include <QListView>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QTableWidgetItem>
#include <QScrollArea>
#include <QFormLayout>

class SetupInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SetupInfoFrame(QWidget *parent = nullptr);
    ~SetupInfoFrame();
    void getData();
    void clear();
    void load(const QString &channel, const QVector<QString> &commands, const QVector<QString> &cleanups, const QMap<QString, QString> &modules, const QMap<QString, QString> &envs);

    QString command() const{
        return m_commandLineEdit->text();
    }

    void setCommandFocus() const{
        m_commandLineEdit->setFocus();
    }


private slots:
    void addModule();  // Create a new row for modules   
    void removeModule(QWidget *moduleRow);


protected:
    void updateGeometry();

private:
    // UI Elements
    QLineEdit *m_channelLineEdit;  // channel input
    QPushButton *m_addModuleButton;  // Button to add new module row
    QWidget *m_modulesWidget;
    QVBoxLayout *m_modulesLayout;
    QLineEdit *m_commandLineEdit;
    QTextEdit *m_cleanupTextEdit;


    QWidget *m_envContainer;
    QTableWidget *envTableWidget;  // Environment variables table
    QPushButton *addEnvButton;  // Add environment variable button


    // Private functions
    void setupUI();  // Setup UI elements
    void createEnvRow();  // Create a new row for environment variables

};


#endif // SETUPINFOFRAME_H
