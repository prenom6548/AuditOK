#ifndef LOGSETTINGSWIDGET_H
#define LOGSETTINGSWIDGET_H

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class LogSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit LogSettingsWidget(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

private:
    void createLogGroup();
    void refreshLogList();
    void refreshLogListWithFeedback();
    void loadLogContent(const QString &filePath);
    QString runtimeHome() const;

    QVBoxLayout *m_mainLayout;
    QPushButton *m_refreshButton;
    QListWidget *m_fileList;
    QTextEdit *m_logContent;
};

#endif
