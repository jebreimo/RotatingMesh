//****************************************************************************
// Copyright © 2016 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 26.01.2016
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "RotatingMeshShaderProgram.hpp"

#include "RotatingMesh-frag.glsl.hpp"
#include "RotatingMesh-vert.glsl.hpp"

void RotatingMeshShaderProgram::setup()
{
    program = Tungsten::create_program();
    auto vertexShader = Tungsten::create_shader(GL_VERTEX_SHADER,
                                               RotatingMesh_vert);
    Tungsten::attach_shader(program, vertexShader);

    auto fragmentShader = Tungsten::create_shader(GL_FRAGMENT_SHADER,
                                                 RotatingMesh_frag);
    Tungsten::attach_shader(program, fragmentShader);
    Tungsten::link_program(program);
    Tungsten::use_program(program);

    position_attr = Tungsten::get_vertex_attribute(program, "a_position");
    normal_attr = Tungsten::get_vertex_attribute(program, "a_normal");

    mv_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(
        program, "u_mv_matrix");
    proj_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(
        program, "u_proj_matrix");
    light_vector = Tungsten::get_uniform<Xyz::Vector3F>(
        program, "u_light_vec");
}
