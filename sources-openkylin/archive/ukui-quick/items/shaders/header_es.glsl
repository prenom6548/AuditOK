/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

// This file contains common directives needed for the shaders to work.
// It is included as the very first bit in the shader.
// Important: If a specific GLSL version is needed, it should be set in this
// file.

// This file is intended for OpenGLES version 2.0 or greater.

#version 100
#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif
#extension GL_OES_standard_derivatives : enable


