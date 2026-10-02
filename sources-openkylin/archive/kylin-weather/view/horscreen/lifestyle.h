/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef LIFESTYLE_H
#define LIFESTYLE_H

#include <QWidget>
#include <QHBoxLayout>
#include "ultravioletrays.h"
#include "air.h"
#include "sport.h"
#include "washcar.h"
#include "flu.h"
#include "clothe.h"
#include "global/corevar.h"

class lifestyle : public QWidget
{
    Q_OBJECT
public:
    explicit lifestyle(QWidget *parent = nullptr);
    void getLifeStyleInfor(LifeStyle m_lifeStyle, int m_isFullScreen);

private:
    QHBoxLayout *m_lifestyleLayout;
    ultravioletrays *m_ultravioletrays = nullptr;
    air *m_air = nullptr;
    sport *m_sport = nullptr;
    washcar *m_washcar = nullptr;
    flu *m_flu = nullptr;
    clothe *m_clothe = nullptr;
    void initLayout();
    void initQss();
    void initConnect();

    void paintEvent(QPaintEvent *event);


signals:
    void sendIsFullScreen(int isFullScreen);
};

#endif // LIFESTYLE_H
