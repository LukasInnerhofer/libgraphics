#include "libgraphics/drawable_vertex_buffer.h"

namespace LibGraphics
{

DrawableVertexBuffer::DrawableVertexBuffer() :
    m_vertexBuffer{std::vector<Vertex>{}, VertexBuffer::Primitive::Triangle, std::optional<NonNullSharedPtr<Texture>>{}}
{

}

void DrawableVertexBuffer::setPosition(const Vector<float> &position)
{
    m_vertexBuffer.move(position - m_position);
    Drawable::setPosition(position);
}

void DrawableVertexBuffer::draw(Window &window)
{
    window.draw(m_vertexBuffer);
}

}
