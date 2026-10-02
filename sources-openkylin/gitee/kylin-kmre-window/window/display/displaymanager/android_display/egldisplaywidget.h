/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EGLDISPLAYWIDGET
#define EGLDISPLAYWIDGET

#include "displaywidget.h"
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QString>

#define VERTICE_DATA_SIZE 20
#define GAUSS_KERNEL_SIZE 9
#define GAUSS_KERNEL_SIGMA 10

class EglDisplayWorkManager;

class EglDisplayWidget : public DisplayWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum RotateDegree {
        eDegree_0 = 0,
        eDegree_90,
        eDegree_180,
        eDegree_270,
        eDegree_Max
    };

public:
    EglDisplayWidget(KmreWindow* window, int id, int width, int height, int orientation, const QString& displayName);
    virtual ~EglDisplayWidget();

    void setOrientation(int orientation);

protected:
    void showEvent(QShowEvent *event) override;
    void initializeGL() override;
    void paintGL() override;

private:
    void makeObject();
    void drawDisplay();
    void updateVbo(const QSize& framebufferSize, bool enableBlur = false);
    void updateBlurShaderData(const QSize& framebufferSize);

    QOpenGLShaderProgram *mDefaultShaderProgram;
    QOpenGLShaderProgram *mBlurShaderProgram;
    QOpenGLShaderProgram *mCurrentShaderProgram;

    int mCurrentOrientation;
    QOpenGLBuffer vbo;
    QOpenGLBuffer ebo;
    GLfloat mVertices[eDegree_Max][VERTICE_DATA_SIZE];
    GLfloat mKernel[GAUSS_KERNEL_SIZE];

    bool mIsFirstShow = true;

};

#endif // EGLDISPLAYWIDGET
