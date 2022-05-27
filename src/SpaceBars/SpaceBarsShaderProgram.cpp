//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 26.01.2016
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SpaceBarsShaderProgram.hpp"

#include "SpaceBars-frag.glsl.hpp"
#include "SpaceBars-vert.glsl.hpp"

void SpaceBarsShaderProgram::setup()
{
    program = Tungsten::create_program();
    auto vertexShader = Tungsten::create_shader(GL_VERTEX_SHADER,
                                               SpaceBars_vert);
    Tungsten::attach_shader(program, vertexShader);

    auto fragmentShader = Tungsten::create_shader(GL_FRAGMENT_SHADER,
                                                 SpaceBars_frag);
    Tungsten::attach_shader(program, fragmentShader);
    Tungsten::link_program(program);
    Tungsten::use_program(program);

    position_attribute = Tungsten::get_vertex_attribute(program, "position");
    normal_attribute = Tungsten::get_vertex_attribute(program, "normal");
    color_attribute = Tungsten::get_vertex_attribute(program, "color");

    model_view_projection_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(
            program, "mvpMatrix");
    model_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(
            program, "mMatrix");
}
