#pragma once

#include <vector>

#include "lib_graphics/vertex.h"

namespace LibGraphics
{

class VertexBuffer
{
public:
    enum class Primitive { Triangle, Quad };

    VertexBuffer(std::vector<Vertex> const &vertices, Primitive primitive, bool haveTexture = false);
    VertexBuffer(std::vector<Vertex> &&vertices, Primitive primitive, bool haveTexture = false);

    void move(Vector<float> const &delta);    
    std::vector<float> const &getData() const;
    Primitive getPrimitive() const;
    size_t getCount() const;

private:
    void updateData();

    bool m_haveTexture;
    std::vector<Vertex> m_vertices;
    std::vector<float> m_data;
    Primitive m_primitive;
};

}