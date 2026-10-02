#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QLabel>
#include <logic/HistoryManager.h>

class StartWidget : public QWidget {
    Q_OBJECT

public:
    explicit StartWidget(QWidget *parent = nullptr);
    void loadAndDisplayHistory();

protected:
    void paintEvent(QPaintEvent *) override;

signals:
    void fileSelected(const QString &filePath); // 选择文件时发射的信号
    void folderSelected(const QString &folderPath, const QString &projectID); // 选择路径时发射的信号

private slots:
    void onNewClicked(); // New 按钮点击事件
    void onOpenClicked(); // Open 按钮点击事件
    void onDebianConvertClicked(); //Debian源码转换
    void loadProject(const QString& yamlPath);

    bool isValidAppName(const QString &appName, QString &errMsg);
    void handleFileSelected(const QString &filePath);

private:
    QLabel *m_titleBig;
    QLabel *m_description;
    QLabel * m_labelProject;
    QPushButton *m_newButton;
    QPushButton *m_openButton;
    QPushButton *m_debianConvertButton;
    QString m_selectedPath; // 存储用户选择的路径
    QVBoxLayout *m_fileListLayout;
    void clearLayout(QLayout *layout);

    HistoryManager m_historyManager;
};

#endif // STARTWIDGET_H
