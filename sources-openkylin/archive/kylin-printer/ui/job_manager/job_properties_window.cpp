/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "job_properties_window.h"
#include "ukui_printer.h"

#include <QHBoxLayout>

JobPropertiesWindow::JobPropertiesWindow(int jobId, QWidget *parent)
    : CustomWidget(parent, true),
      m_jobId(jobId)
{
    this->initUi();
}

void JobPropertiesWindow::initUi()
{
    this->setWidgetName(tr("Job properties"));
    this->setMinimumSize(820,420);
    this->initView();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_view.get());
    layout->setContentsMargins(24,16,24,24);
    this->baseBar()->setLayout(layout);
}

void JobPropertiesWindow::initView()
{
    m_view.reset(new CustomListView());
    m_model.reset(new QStandardItemModel());

    QMap<QString, QVariant> jobProperties;
    UkuiPrinterManager::getInstance().getJobAttributes(m_jobId, nullptr, &jobProperties);
    m_model->setRowCount(jobProperties.size());
    m_model->setColumnCount(2);
    qDebug() << jobProperties.size();
    m_model->setHeaderData(0, Qt::Horizontal, tr("name"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("value"));
    int cnt = 0;
    for (auto innerIterator = jobProperties.begin(); innerIterator != jobProperties.end(); ++innerIterator) {
        auto innerKey = innerIterator.key();
        auto value = innerIterator.value();
        m_model->setData(m_model->index(cnt, 0), innerKey);
        m_model->setData(m_model->index(cnt, 1), value);
        cnt++;
    }
    m_view->setModel(m_model.get());
    m_view->setColumnWidth(0, 200);
}
