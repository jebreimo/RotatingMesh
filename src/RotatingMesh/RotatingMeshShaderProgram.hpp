//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 26.01.2016
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>

class RotatingMeshShaderProgram
{
public:
    void setup();

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix4F> mv_matrix;
    Tungsten::Uniform<Xyz::Matrix4F> proj_matrix;
    Tungsten::Uniform<Xyz::Vector3F> light_vector;

    GLuint position_attribute;
    GLuint normal_attribute;
};
