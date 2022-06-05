#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include "SpaceBarsShaderProgram.hpp"
#include "Debug.hpp"

struct Point
{
    Xyz::Vector3F coords;
    Xyz::Vector3F normal;
    Xyz::Vector3F color;
};

Xyz::Mesh<float> make_cube_mesh()
{
    return Xyz::Mesh<float>(
        {
            {-1, -1, -1},
            {-1, -1, 1},
            {1, -1, -1},
            {1, -1, 1},
            {1, 1, -1},
            {1, 1, 1},
            {-1, 1, -1},
            {-1, 1, 1}},
        {
            {0, 2, 1},
            {2, 3, 1},
            {2, 4, 3},
            {4, 5, 3},
            {4, 6, 5},
            {6, 7, 5},
            {6, 0, 7},
            {0, 1, 7},
            {0, 6, 2},
            {6, 4, 2},
            {1, 3, 7},
            {3, 5, 7}});
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
        builder.add_vertex({mesh.vertexes()[face[0]], normal, {1, 1, 1}});
        builder.add_vertex({mesh.vertexes()[face[1]], normal, {1, 1, 1}});
        builder.add_vertex({mesh.vertexes()[face[2]], normal, {1, 1, 1}});
        builder.add_indexes(n, n + 1, n + 2);
        n += 3;
    }
}

struct Foo
{
    uint32_t timestamp = 0;
    float base = 0;
    float factor = 0;

    [[nodiscard]]
    constexpr float value(uint32_t timestamp) const
    {
        auto delta = float(timestamp - this->timestamp);
        auto value = delta * factor + base;
        return std::max(0.0f, std::min(1.0f, value));
    }
};

class SpaceBarLoop : public Tungsten::EventLoop
{
public:
    void on_startup(Tungsten::SdlApplication& app) override
    {
        mesh_ = make_cube_mesh();
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
        Tungsten::enable_vertex_attribute(program_.position_attribute);
        Tungsten::define_vertex_attribute_pointer(program_.position_attribute, 3,
                                                  GL_FLOAT, false, row_size, 0);
        Tungsten::enable_vertex_attribute(program_.normal_attribute);
        Tungsten::define_vertex_attribute_pointer(program_.normal_attribute, 3,
                                                  GL_FLOAT, false, row_size, 3 * sizeof(GLfloat));
        Tungsten::enable_vertex_attribute(program_.color_attribute);
        Tungsten::define_vertex_attribute_pointer(program_.color_attribute, 3,
                                                  GL_FLOAT, false, row_size, 6 * sizeof(GLfloat));

        app.set_swap_interval(1);
        glEnable(GL_DEPTH_TEST);
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
            return false;
        if (event.key.keysym.sym != SDLK_SPACE)
            return false;

        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            foo_ = {event.key.timestamp,
                    foo_.value(event.key.timestamp),
                    0.0002f};
        }
        else if (event.type == SDL_KEYUP)
        {
            foo_ = {event.key.timestamp,
                    foo_.value(event.key.timestamp),
                    -0.0006f};
        }

        return EventLoop::on_event(app, event);
    }

    void on_update(Tungsten::SdlApplication& app) override
    {
        auto value = foo_.value(SDL_GetTicks());
        if (value == prev_value_)
            return;

        mesh_.set_vertex(0, {-1 + value, -1 + value, -1 + value});
        update_buffer_ = true;
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        auto view_mat = Xyz::scale4<float>(1.0f, app.aspect_ratio(), 1.0f)
                        * Xyz::make_frustum_matrix<float>(-2, 2, -2, 2, 2, 20)
                        * Xyz::make_look_at_matrix(Xyz::make_vector3<float>(-4, -4, 2.5),
                                                   Xyz::make_vector3<float>(0, 0, 0),
                                                   Xyz::make_vector3<float>(0, 0, 1));

        auto angle = Xyz::to_radians(float(SDL_GetTicks() / 10.0));

        try
        {
            if (update_buffer_)
            {
                Tungsten::ArrayBuffer<Point> buffer;
                add_mesh(buffer, mesh_);
                auto [v_buf, v_size] = buffer.array_buffer();
                Tungsten::set_buffer_subdata(GL_ARRAY_BUFFER, 0,
                                             GLuint(v_size), v_buf);
                update_buffer_ = false;
            }
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto scale = 0.5f + 2.0f * foo_.value(SDL_GetTicks());
            auto modelMatrix = Xyz::rotate_z(angle)
                               * Xyz::scale4(scale, scale, scale);
            auto mat = view_mat * modelMatrix;
            program_.model_view_projection_matrix.set(mat);
            program_.model_matrix.set(modelMatrix);

            // Draw a triangle from the 3 vertices
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
    SpaceBarsShaderProgram program_;
    GLsizei element_count_ = 0;
    Xyz::Mesh<float> mesh_;
    bool update_buffer_ = false;
    Foo foo_;
    float prev_value_ = 0;
};

int main(int argc, char* argv[])
{
    try
    {
        Tungsten::SdlApplication app("PolygonFountain",
                                     std::make_unique<SpaceBarLoop>());
        app.parse_command_line_options(argc, argv);
        app.run();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }

    return 0;
}
