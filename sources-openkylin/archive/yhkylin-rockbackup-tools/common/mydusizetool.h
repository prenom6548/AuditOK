#ifndef MYDUSIZETOOL_H
#define MYDUSIZETOOL_H

#include <QObject>
#include <QProcess>

class MyDuSizeTool : public QObject {
    Q_OBJECT
public:
    MyDuSizeTool(QObject* parent = nullptr);
    ~MyDuSizeTool();

    qint64 Do(QStringList paths, QStringList excludePaths, bool block = true);
    qint64 Do(QStringList paths, bool block = true);
    qint64 size() { return m_size; }

private:
    void Init(QStringList paths, QStringList excludePaths);
    qint64 _Do(QStringList paths, QStringList excludePaths, bool needExclude, bool block = true);

signals:
    void duFinished(qint64 size);
private slots:
    void finished(int, QProcess::ExitStatus);

private:
    QStringList m_paths;
    QStringList m_excludePaths;
    qint64 m_size = 0;

    QProcess* m_p;
};

#endif // !MYDUSIZETOOL_H
