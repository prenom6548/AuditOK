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
#ifndef JOB_PROPERTIES_WINDOW_H
#define JOB_PROPERTIES_WINDOW_H

#include "custom_widget.h"
#include "custom_list_view.h"

#include <QStandardItemModel>
#include <memory>

class JobPropertiesWindow : public CustomWidget
{
    Q_OBJECT
public:
    explicit JobPropertiesWindow(int jobId = 0, QWidget *parent = nullptr);
private:
    int m_jobId = 0;
    std::unique_ptr<CustomListView> m_view{nullptr};
    std::unique_ptr<QStandardItemModel> m_model{nullptr};
    void initUi();
    void initView();
};

#endif // JOB_PROPERTIES_WINDOW_H
