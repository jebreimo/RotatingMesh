#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include "GouraudShaderProgram.hpp"
#include "Debug.hpp"

struct Point
{
    Xyz::Vector3F coords;
    Xyz::Vector3F normal;
};

std::vector<Xyz::Vector2F> make_polygon(unsigned n)
{
    constexpr auto PI = Xyz::Constants<float>::PI;
    std::vector<Xyz::Vector2F> result;
    auto angle0 = 1.5f * PI - PI / float(n);
    for (unsigned i = 0; i < n; ++i)
    {
        auto angle = angle0 + float(i) * 2 * PI / float(n);
        result.push_back({cos(angle), sin(angle)});
    }
    return result;
}

std::vector<Xyz::Vector2F> make_transition_polygon(unsigned n, float fraction)
{
    if (fraction <= 0)
        return make_polygon(n);
    if (fraction >= 1)
        return make_polygon(n + 1);
    const auto points0 = make_polygon(n);
    const auto points1 = make_polygon(n + 1);
    std::vector<Xyz::Vector2F> result;
    for (unsigned i = 0; i < n; ++i)
        result.push_back(points0[i] + (points1[i] - points0[i]) * fraction);
    result.push_back(points0[0] + (points1.back() - points0[0]) * fraction);
    return result;
}

Xyz::Mesh<float> make_polygon_mesh(unsigned n, float fraction)
{
    const auto RADIUS = sqrt(2.0f);
    auto points = make_transition_polygon(n, fraction);
    Xyz::Mesh<float> mesh;
    for (auto p : points)
    {
        p *= RADIUS;
        mesh.add_vertex({p[0], p[1], -1});
        mesh.add_vertex({p[0], p[1], 1});
    }
    if (fraction > 0)
        n += 1;
    for (unsigned i = 0; i < n - 1; ++i)
    {
        auto j = i * 2;
        mesh.add_face({j, j + 2, j + 1});
        mesh.add_face({j + 2, j + 3, j + 1});
    }
    auto bottom_center = uint32_t(mesh.vertexes().size());
    mesh.add_vertex({0, 0, -1});
    auto top_center = uint32_t(mesh.vertexes().size());
    mesh.add_vertex({0, 0, 1});
    mesh.add_face({2 * n - 2, 0, 2 * n - 1});
    mesh.add_face({0, 1, 2 * n - 1});
    for (unsigned i = 0; i < n - 1; ++i)
    {
        auto j = i * 2;
        mesh.add_face({j + 1, j + 3, top_center});
        mesh.add_face({j + 2, j, bottom_center});
    }

    mesh.add_face({2 * n - 1, 1, top_center});
    mesh.add_face({0, 2 * n - 2, bottom_center});

    return mesh;
}

void add_mesh(Tungsten::ArrayBuffer<Point>& buffer,
              Xyz::Mesh<float>& mesh)
{
    Tungsten::ArrayBufferBuilder builder(buffer);
    builder.reserve_vertexes(mesh.faces().size() * 3);
    builder.reserve_indexes(mesh.faces().size() * 3);
    int n = 0;
    for (const auto& face : mesh.faces())
    {
        auto normal = mesh.normal(face);
        builder.add_vertex({mesh.vertexes()[face[0]], normal});
        builder.add_vertex({mesh.vertexes()[face[1]], normal});
        builder.add_vertex({mesh.vertexes()[face[2]], normal});
        builder.add_indexes(n, n + 1, n + 2);
        n += 3;
    }
}

struct Foo
{
    uint32_t start_timestamp = 0;
    float start_value = 0;
    float end_value = 1.0;
    float factor = 0;

    [[nodiscard]]
    constexpr float value(uint32_t timestamp) const
    {
        auto delta = float(timestamp - this->start_timestamp);
        auto value = delta * factor + start_value;
        return Xyz::clamp(value, std::min(start_value, end_value),
                          std::max(start_value, end_value));
    }
};

class RotatingMeshLoop : public Tungsten::EventLoop
{
public:
    void on_startup(Tungsten::SdlApplication& app) override
    {
        mesh_ = make_polygon_mesh(10, 0);
        Tungsten::ArrayBuffer<Point> buffer;
        add_mesh(buffer, mesh_);

        vertex_array_ = Tungsten::generate_vertex_array();
        Tungsten::bind_vertex_array(vertex_array_);

        buffers_ = Tungsten::generate_buffers(2);
        Tungsten::set_buffers(buffers_[0], buffers_[1], buffer,
                              GL_DYNAMIC_DRAW);
        element_count_ = GLsizei(buffer.indexes.size());
        program_.setup();

        GLsizei row_size = sizeof(Point);
        Tungsten::enable_vertex_attribute(program_.position_attr);
        Tungsten::define_vertex_attribute_pointer(program_.position_attr, 3,
                                                  GL_FLOAT, false, row_size, 0);
        Tungsten::enable_vertex_attribute(program_.normal_attr);
        Tungsten::define_vertex_attribute_pointer(program_.normal_attr, 3,
                                                  GL_FLOAT, false, row_size, 3 * sizeof(GLfloat));

        auto proj_mat = Xyz::scale4<float>(1.0f, app.aspect_ratio(), 1.0f)
                        * Xyz::make_frustum_matrix<float>(-2, 2, -2, 2, 2, 20)
                        * Xyz::make_look_at_matrix(Xyz::make_vector3<float>(-4, -4, 2.5),
                                                   Xyz::make_vector3<float>(0, 0, 0),
                                                   Xyz::make_vector3<float>(0, 0, 1));
        program_.proj_matrix.set(proj_mat);

        app.set_swap_interval(1);
        glEnable(GL_DEPTH_TEST);
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
            return false;

        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_p)
        {
            draw_wireframe_ = !draw_wireframe_;
            glPolygonMode(GL_FRONT_AND_BACK, draw_wireframe_ ? GL_LINE : GL_FILL);
            return true;
        }

        if (event.key.keysym.sym != SDLK_SPACE)
            return false;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            foo_ = {event.key.timestamp,
                    foo_.value(event.key.timestamp),
                    3,
                    -0.0005f};
        }
        else if (event.type == SDL_KEYUP)
        {
            foo_ = {event.key.timestamp,
                    foo_.value(event.key.timestamp),
                    10,
                    0.0006f};
        }

        return true;
    }

    void on_update(Tungsten::SdlApplication& app) override
    {
        auto value = foo_.value(SDL_GetTicks());
        if (value == prev_value_)
            return;

        float int_part;
        float fraction = modf(value, &int_part);
        mesh_ = make_polygon_mesh(unsigned(int_part), fraction);
        update_buffer_ = true;
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        try
        {
            if (update_buffer_)
            {
                Tungsten::ArrayBuffer<Point> buffer;
                add_mesh(buffer, mesh_);

                auto [v_buf, v_size] = buffer.array_buffer();
                Tungsten::set_buffer_subdata(GL_ARRAY_BUFFER, 0,
                                             GLsizeiptr(v_size), v_buf);
                auto [i_buf, i_size] = buffer.index_buffer();
                Tungsten::set_buffer_subdata(GL_ELEMENT_ARRAY_BUFFER, 0,
                                             GLsizeiptr(i_size), i_buf);
                element_count_ = GLsizei(buffer.indexes.size());
                update_buffer_ = false;
            }
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto angle = Xyz::to_radians(float(SDL_GetTicks() / 50.0));
            auto model_mat = Xyz::rotate_z(angle);
            program_.mv_matrix.set(model_mat);

            glDrawElements(GL_TRIANGLES, element_count_, GL_UNSIGNED_SHORT, nullptr);
        }
        catch (Tungsten::TungstenException& ex)
        {
            std::cerr << ex.what() << "\n";
        }
    }

private:
    std::vector<Tungsten::BufferHandle> buffers_;
    Tungsten::VertexArrayHandle vertex_array_;
    GouraudShaderProgram program_;
    GLsizei element_count_ = 0;
    Xyz::Mesh<float> mesh_;
    bool update_buffer_ = false;
    Foo foo_ = {0, 10, 3, 0};
    float prev_value_ = 10;
    bool draw_wireframe_ = false;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("RotatingMesh",
                                     std::make_unique<RotatingMeshLoop>());
        app.parse_command_line_options(argc, argv);
        auto params = app.window_parameters();
        params.gl_parameters.multi_sampling = {1, 2};
        app.set_window_parameters(params);
        app.run();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }

    return 0;
}
