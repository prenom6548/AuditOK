/*
 * Qt5-UKUI's Library
 *
 * KDragWidget 的原生文件选择器桥接。
 */

#include "kdragfiledialog.h"

#include <QFileDialog>
#include <QStandardPaths>

using namespace UKUISDKStyle;

KDragFileDialog::KDragFileDialog(QQuickItem *parent)
    : QQuickItem(parent)
{
}

QStringList KDragFileDialog::nameFilters() const
{
    return m_nameFilters;
}

void KDragFileDialog::setNameFilters(const QStringList &filters)
{
    if (m_nameFilters == filters)
        return;

    m_nameFilters = filters;
    emit nameFiltersChanged();
}

void KDragFileDialog::open()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setWindowTitle(tr("Please select file"));
    if (!m_nameFilters.isEmpty())
        dialog.setNameFilters(m_nameFilters);

    if (dialog.exec() == QDialog::Accepted)
        emit accepted(dialog.selectedFiles());
}
