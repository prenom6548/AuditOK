#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QVector>
#include <QString>

class HistoryManager {
public:
    // 构造函数
    HistoryManager();

    // 添加历史记录
    void addHistory(const QString& projectName, const QString& yamlPath);

    void moveRecordToEnd(const QString& yamlPath);

    // 获取历史记录
    QVector<QString> getHistory();

private:
    // 加载历史记录
    void loadHistory();

    // 保存历史记录
    void saveHistory();

    // 历史记录存储容器
    QVector<QString> history;

    // 历史记录文件路径
    QString historyFilePath;
};

#endif // HISTORYMANAGER_H
