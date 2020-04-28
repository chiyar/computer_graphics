#pragma once

#include <vector>
#include <cassert>
#include <stdexcept>
#include <iostream>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Triangle {
    std::vector<glm::vec3> points;

public:
    Triangle(const std::vector<GLfloat>& data) {
        assert(data.size() == 9);
        for (size_t i = 0; i < 3; ++i) {
            auto iter = data.begin() + 3 * i;
            points.emplace_back(
                    glm::vec3(*iter, *(iter+1), *(iter+2))
            );
        }
    }

    Triangle(const std::vector<glm::vec3>& points) : points(points) {
        assert(points.size() == 3);
    }

    void move(const glm::vec3& shift) {
        for (auto& point : points) {
            point += shift;
        }
    }

    void stretch(GLfloat alpha) {
        for (auto& point : points) {
            point *= alpha;
        }
    }

    void turn(const glm::vec3& angle) {
        GLfloat sin1 = sin(angle.x);
        GLfloat cos1 = cos(angle.x);
        GLfloat sin2 = sin(angle.y);
        GLfloat cos2 = cos(angle.y);
        GLfloat sin3 = sin(angle.z);
        GLfloat cos3 = cos(angle.z);

        for (auto& point : points) {
            glm::vec3 new_point;
            new_point.x = point.x * cos1 - point.y * sin1;
            new_point.y = point.x * sin1 + point.y * cos1;
            new_point.z = point.z;
            point = new_point;
            new_point.x = point.x * cos2 - point.z * sin2;
            new_point.z = point.x * sin2 + point.z * cos2;
            new_point.y = point.y;
            point = new_point;
            new_point.x = point.x * cos3 - point.y * sin3;
            new_point.y = point.x * sin3 + point.y * cos3;
            new_point.z = point.z;
            point = new_point;
        }
    }

    const std::vector<glm::vec3>& get_points() const {
        return points;
    }
};


class Buffer {
    std::vector<GLfloat> _vertex_data;
    std::vector<GLfloat> _color_data;
    std::vector<GLfloat> _texture_data;
public:
    Buffer() {}

    void clear() {
        _vertex_data.clear();
        _color_data.clear();
    }

    const void* vertex_data() {
        return _vertex_data.data();
    }

    const void* color_data() {
        return _color_data.data();
    }

    const void* texture_data() {
        return _texture_data.data();
    }

    size_t size() const {
        assert(_vertex_data.size() == _color_data.size());
        return _vertex_data.size();
    }

    size_t texture_size() const {
        return _texture_data.size();
    }

    void add(const std::vector<Triangle>& triangles, const std::vector<GLfloat>& colors,
        const std::vector<glm::vec2>& texcoords) {
        assert(colors.size() == 3);

        for (auto& triangle: triangles) {
            for (const auto& point : triangle.get_points()) {
                _vertex_data.emplace_back(point.x);
                _vertex_data.emplace_back(point.y);
                _vertex_data.emplace_back(point.z);
            }

            for (int i = 0; i < 3; ++i) {
                for (auto comp : colors) {
                    _color_data.emplace_back(comp);
                }
            }
        }

        for (auto& coords: texcoords) {
            _texture_data.emplace_back(coords.x);
            _texture_data.emplace_back(coords.y);
        }
    }
};


class Object {
protected:
    std::vector<Triangle> triangles;
    std::vector<GLfloat> colors;
    std::vector<glm::vec2> texcoords;

    Object() : center(0, 0, 0) {}
public:
    glm::vec3 center;
    void draw(Buffer& buffer) const {
        buffer.add(triangles, colors, texcoords);
    }

    void move(const glm::vec3& shift) {
        center += shift;
        for (auto& triangle : triangles) {
            triangle.move(shift);
        }
    }
};

class Floor : public Object {
    static constexpr GLfloat FIELD_SIZE = 10.0f;
public:
    Floor() {
        auto t1 = Triangle({
                   -FIELD_SIZE, 0.0f, -FIELD_SIZE,
                   FIELD_SIZE, 0.0f,  FIELD_SIZE,
                   FIELD_SIZE, 0.0f, -FIELD_SIZE,
        });
        auto t2 = Triangle({
                   FIELD_SIZE, 0.0f,  FIELD_SIZE,
                   -FIELD_SIZE, 0.0f, -FIELD_SIZE,
                   -FIELD_SIZE, 0.0f,  FIELD_SIZE,
        });
        triangles = {t1, t2};
        colors = {0.7, 0.5, 0.2};
    }
};


class Fireball : public Object {
public:
    GLfloat radius;

    Fireball(GLfloat radius, size_t triangles_count, const std::vector<GLfloat>& colors={0.0, 0.0, 0.0})
    : radius(radius) {
        this->colors = colors;
        const size_t squares_count = triangles_count / 2;
        for (size_t i = 0; i < squares_count; ++i) {
            double theta = (double)glm::pi<double>() * i / squares_count;
            double theta1 = (double)glm::pi<double>() * (i + 1) / squares_count;

            for (size_t j = 0; j < triangles_count; ++j) {
                double phi = 2.0f * glm::pi<double>() * j / triangles_count + glm::pi<double>();
                double phi1 = 2.0f * glm::pi<double>() * (j + 1) / triangles_count + glm::pi<double>();

                //Первый треугольник
                triangles.emplace_back(Triangle(std::vector<glm::vec3>{
                    glm::vec3(
                        cos(phi) * sin(theta) * radius,
                        sin(phi) * sin(theta) * radius,
                        cos(theta) * radius
                    ),
                    glm::vec3(
                        cos(phi1) * sin(theta1) * radius,
                        sin(phi1) * sin(theta1) * radius,
                        cos(theta1) * radius
                    ),
                    glm::vec3(
                        cos(phi1) * sin(theta) * radius,
                        sin(phi1) * sin(theta) * radius,
                        cos(theta) * radius
                    )
                }));

                texcoords.emplace_back(glm::vec2((float)j / triangles_count, 1.0 - (float)i / squares_count));
                texcoords.emplace_back(glm::vec2((float)(j + 1) / triangles_count, 1.0 - (float)(i + 1) / squares_count));
                texcoords.emplace_back(glm::vec2((float)(j + 1) / triangles_count, 1.0 - (float)i / squares_count));


                //Второй треугольник
                triangles.emplace_back(Triangle(std::vector<glm::vec3>{
                    glm::vec3(
                        cos(phi) * sin(theta) * radius,
                        sin(phi) * sin(theta) * radius,
                        cos(theta) * radius
                    ),
                    glm::vec3(
                        cos(phi) * sin(theta1) * radius,
                        sin(phi) * sin(theta1) * radius,
                        cos(theta1) * radius
                    ),
                    glm::vec3(
                        cos(phi1) * sin(theta1) * radius,
                        sin(phi1) * sin(theta1) * radius,
                        cos(theta1) * radius
                    )
                }));

                texcoords.emplace_back(glm::vec2((float)j / triangles_count, 1.0f - (float)i / squares_count));
                texcoords.emplace_back(glm::vec2((float)j / triangles_count, 1.0f - (float)(i + 1) / squares_count));
                texcoords.emplace_back(glm::vec2((float)(j + 1) / triangles_count, 1.0f - (float)(i + 1) / squares_count));
            }
        }
    }
};

const std::vector<Triangle> CUBE_TRIANGLES = {
        Triangle({-1.0f, -1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f}),
        Triangle({1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f}),
        Triangle({1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f}),
        Triangle({1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f}),
        Triangle({-1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f}),
        Triangle({1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f}),
        Triangle({-1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f}),
        Triangle({1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f}),
        Triangle({1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f}),
        Triangle({1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f}),
        Triangle({1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f}),
        Triangle({1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f})
};


class Target : public Object {
    int lifetime;
public:
    GLfloat radius;

    Target(const glm::vec3& icenter,
            GLfloat radius,
            const glm::vec3& angle,
            const std::vector<GLfloat>& icolor,
            int lifetime
            ) : lifetime(lifetime), radius(radius) {
        triangles = std::vector<Triangle>(CUBE_TRIANGLES);
        colors = icolor;
        center = icenter;
        for (auto& t : triangles) {
            t.stretch(radius);
            t.turn(angle);
            t.move(icenter);
        }
    }
    bool expired(int timestamp) const {
        return timestamp >= lifetime;
    }
};
