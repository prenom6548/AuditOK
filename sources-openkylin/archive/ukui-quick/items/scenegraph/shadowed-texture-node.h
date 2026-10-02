/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  SPDX-FileCopyrightText: 2025 iaom <zhangpengfei@kylinos.cn>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QPointer>
#include <QSGTextureProvider>

#include "shadowed-rectangle-node.h"
#include "shadowed-texture-material.h"

/**
 * Scene graph node for a shadowed texture source.
 *
 * This node will set up the geometry and materials for a shadowed rectangle,
 * optionally with rounded corners, using a supplied texture source as the color
 * for the rectangle.
 *
 * \note You must call updateGeometry() after setting properties of this node,
 * otherwise the node's state will not correctly reflect all the properties.
 *
 * \sa ShadowedTexture
 */
class ShadowedTextureNode : public ShadowedRectangleNode
{
public:
    ShadowedTextureNode();

    void setTextureSource(QSGTextureProvider *source);
    void preprocess() override;

private:
    ShadowedRectangleMaterial *createBorderlessMaterial() override;
    ShadowedBorderRectangleMaterial *createBorderMaterial() override;
    QSGMaterialType *borderlessMaterialType() override;
    QSGMaterialType *borderMaterialType() override;

    QPointer<QSGTextureProvider> m_textureSource;
};
