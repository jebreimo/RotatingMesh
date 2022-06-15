//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>

class GouraudShaderProgram
{
public:
    void setup();

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix4F> mv_matrix;
    Tungsten::Uniform<Xyz::Matrix4F> proj_matrix;

    Tungsten::Uniform<Xyz::Vector3F> light_pos;
    Tungsten::Uniform<Xyz::Vector3F> diffuse_albedo;
    Tungsten::Uniform<Xyz::Vector3F> specular_albedo;
    Tungsten::Uniform<float> specular_power;
    Tungsten::Uniform<Xyz::Vector3F> ambient;

    GLuint position_attr;
    GLuint normal_attr;
};
