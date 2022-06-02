#include <iostream>
#include <Tungsten/Tungsten.hpp>
#include "SpaceBarsShaderProgram.hpp"

struct Point
{
    Xyz::Vector3F coords;
    Xyz::Vector3F normal;
    Xyz::Vector3F color;
};

void add_face(Tungsten::ArrayBuffer<Point>& buffer,
              const Xyz::Vector3F& major,
              const Xyz::Vector3F& minor,
              const Xyz::Vector3F& normal,
              const Xyz::Vector3F& color)
{
    Tungsten::ArrayBufferBuilder(buffer)
        .reserve_vertexes(4)
        .add_vertex({-major - minor + normal, normal, color})
        .add_vertex({major - minor + normal, normal, color})
        .add_vertex({major + minor + normal, normal, color})
        .add_vertex({-major + minor + normal, normal, color})
        .reserve_indexes(6)
        .add_indexes(0, 1, 3)
        .add_indexes(1, 2, 3);
}

void add_cube(Tungsten::ArrayBuffer<Point>& buffer)
{
    add_face(buffer, {1, 0, 0}, {0, 1, 0}, {0, 0, -1}, {0.4, 0.7, 0.9});
    add_face(buffer, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {0.7, 0.9, 0.4});
    add_face(buffer, {-1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.9, 0.4, 0.7});
    add_face(buffer, {0, 0, -1}, {0, 1, 0}, {-1, 0, 0}, {0.4, 0.9, 0.7});
    add_face(buffer, {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0.7, 0.4, 0.9});
    add_face(buffer, {-1, 0, 0}, {0, 0, 1}, {0, -1, 0}, {0.9, 0.7, 0.4});
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
        Tungsten::ArrayBuffer<Point> buffer;
        add_cube(buffer);

        vertex_array_ = Tungsten::generate_vertex_array();
        Tungsten::bind_vertex_array(vertex_array_);

        buffers_ = Tungsten::generate_buffers(2);
        Tungsten::set_buffers(buffers_[0], buffers_[1], buffer);
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
    Foo foo_;
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
