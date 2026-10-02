#include "HistoryManager.h"
#include <QFile>
#include <QTextStream>
#include <QDir>

// 构造函数
HistoryManager::HistoryManager() {
    // 确保缓存目录存在
    QDir cacheDir(QDir::homePath() + "/.cache/kaiming-builder-gui");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }

    // 设置历史记录文件路径
    historyFilePath = cacheDir.filePath("history.txt");

    // 加载历史记录
    loadHistory();
}

void HistoryManager::addHistory(const QString& projectID, const QString& yamlPath) {
    // 构造新记录
    QString newRecord = projectID + "," + yamlPath;

    // 检查是否已经存在相同的记录
    for (int i = 0; i < history.size(); ++i) {
        if (history[i] == newRecord) {
            // 如果存在，删除旧记录
            history.removeAt(i);
            break;
        }
    }

    // 将新记录添加到最前面
    history.prepend(newRecord);

    // 如果历史记录超过10行，移除最旧的一条
    if (history.size() > 10) {
        history.removeLast();
    }

    // 保存历史记录
    saveHistory();
}

void HistoryManager::moveRecordToEnd(const QString& yamlPath) {
    // 遍历历史记录查找匹配的记录
    for (int i = 0; i < history.size(); ++i) {
        // 分割记录，获取路径部分
        QStringList parts = history[i].split(',');
        if (parts.size() >= 2 && parts[1] == yamlPath) {
            // 移动记录到最后
            QString record = history.takeAt(i);
            history.append(record);
            saveHistory();
            break; // 找到匹配项后退出循环
        }
    }
}


// 获取历史记录
QVector<QString> HistoryManager::getHistory() {
    history.clear();       // 清空当前数据
    loadHistory();         // 重新从文件加载
    return history;
}

// 加载历史记录
void HistoryManager::loadHistory() {
    QFile file(historyFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                history.append(line);
            }
        }
        file.close();
    }
}

// 保存历史记录
void HistoryManager::saveHistory() {
    QFile file(historyFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString& line : history) {
            out << line << "\n";
        }
        file.close();
    }
}

