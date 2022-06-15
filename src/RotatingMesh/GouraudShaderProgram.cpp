//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GouraudShaderProgram.hpp"

#include "Gouraud-frag.glsl.hpp"
#include "Gouraud-vert.glsl.hpp"

void GouraudShaderProgram::setup()
{
    program = Tungsten::create_program();
    auto vertexShader = Tungsten::create_shader(GL_VERTEX_SHADER,
                                                Gouraud_vert);
    Tungsten::attach_shader(program, vertexShader);

    auto fragmentShader = Tungsten::create_shader(GL_FRAGMENT_SHADER,
                                                  Gouraud_frag);
    Tungsten::attach_shader(program, fragmentShader);
    Tungsten::link_program(program);
    Tungsten::use_program(program);

    position_attr = Tungsten::get_vertex_attribute(program, "a_position");
    normal_attr = Tungsten::get_vertex_attribute(program, "a_normal");

    mv_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(program, "u_mv_matrix");
    proj_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(program, "u_proj_matrix");

    light_pos = Tungsten::get_uniform<Xyz::Vector3F>(program, "u_light_pos");
    diffuse_albedo = Tungsten::get_uniform<Xyz::Vector3F>(program, "u_diffuse_albedo");
    specular_albedo = Tungsten::get_uniform<Xyz::Vector3F>(program, "u_specular_albedo");
    specular_power = Tungsten::get_uniform<float>(program, "u_specular_power");
    ambient = Tungsten::get_uniform<Xyz::Vector3F>(program, "u_ambient");
}
