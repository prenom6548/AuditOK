/*
 * Qt5-UKUI's Library
 *
 * KDragWidget 的原生文件选择器桥接。
 */

#ifndef KDRAGFILEDIALOG_H
#define KDRAGFILEDIALOG_H

#include <QQuickItem>
#include <QStringList>

namespace UKUISDKStyle {

class KDragFileDialog : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)

public:
    explicit KDragFileDialog(QQuickItem *parent = nullptr);

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &filters);

    // 使用 QtWidgets 文件对话框，保持与 KDragWidget 一致的多选行为。
    Q_INVOKABLE void open();

signals:
    void nameFiltersChanged();
    void accepted(const QStringList &paths);

private:
    QStringList m_nameFilters;
};

}

#endif // KDRAGFILEDIALOG_H
