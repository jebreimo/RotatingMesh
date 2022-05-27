//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 26.01.2016
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>

class SpaceBarsShaderProgram
{
public:
    void setup();

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix4F> model_view_projection_matrix;
    Tungsten::Uniform<Xyz::Matrix4F> model_matrix;

    GLuint position_attribute;
    GLuint normal_attribute;
    GLuint color_attribute;
};
