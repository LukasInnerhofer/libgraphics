#include <cassert>
#include <fstream>
#include <map>
#include <memory>
#include <queue>
#include <stdexcept>

#include "libgraphics/window.h"
#include "libgraphics/string.h"
#include "opengl_drawable.h"
#include "window_impl.h"
#include "shader.h"
#include "shader_program.h"

namespace LibGraphics
{

static std::map<VertexBuffer::Primitive, GLenum> const primitiveMap {
    { VertexBuffer::Primitive::Triangle, GL_TRIANGLES },
    { VertexBuffer::Primitive::Quad, GL_QUADS }
};

static std::map<Texture::Filtering, GLint> const textureFilteringMap {
    { Texture::Filtering::Nearest, GL_NEAREST },
    { Texture::Filtering::Linear, GL_LINEAR }
};

class Window::Impl
{
public:
    Impl(String const &title) :
        events{std::make_shared<std::queue<Window::Event>>()},
        osImpl{title, events},
        openGl{osImpl.getOpenGl()},
        vertexShader{
            std::make_shared<Shader>(
                openGl,
                LibUtilities::InStream<GLchar>{std::make_shared<std::basic_ifstream<GLchar>>("vertex.glsl")},
                GL_VERTEX_SHADER)},
        textureVertexShader{
            std::make_shared<Shader>(
                openGl,
                LibUtilities::InStream<GLchar>{std::make_shared<std::basic_ifstream<GLchar>>("vertex_texture.glsl")},
                GL_VERTEX_SHADER)},
        fragmentShader{
            std::make_shared<Shader>(
                openGl,
                LibUtilities::InStream<GLchar>{std::make_shared<std::basic_ifstream<GLchar>>("fragment.glsl")},
                GL_FRAGMENT_SHADER)},
        textureFragmentShader{
            std::make_shared<Shader>(
                openGl,
                LibUtilities::InStream<GLchar>{std::make_shared<std::basic_ifstream<GLchar>>("fragment_texture.glsl")},
                GL_FRAGMENT_SHADER)},
        shaderProgram{std::make_unique<ShaderProgram>(openGl, vertexShader, fragmentShader)},
        textureShaderProgram{std::make_unique<ShaderProgram>(openGl, textureVertexShader, textureFragmentShader)}
    {
        
    }

    NonNullSharedPtr<std::queue<Window::Event>> events;
    WindowImpl osImpl;
    NonNullSharedPtr<OpenGl> openGl;
    std::map<VertexBuffer const *, NonNullUniquePtr<OpenGlDrawable>> openGlDrawables;

    NonNullSharedPtr<Shader> vertexShader;
    NonNullSharedPtr<Shader> textureVertexShader;
    NonNullSharedPtr<Shader> fragmentShader;
    NonNullSharedPtr<Shader> textureFragmentShader;
    NonNullUniquePtr<ShaderProgram> shaderProgram;
    NonNullUniquePtr<ShaderProgram> textureShaderProgram;

    bool clearSupported;
};

Window::Window(String const &title) :
    m_pImpl{new Impl{title}}
{

}

Window::~Window()
{

}

bool Window::pollEvent(Window::Event &event)
{
    m_pImpl->osImpl.pollEvents();

    if (m_pImpl->events->size() > 0)
    {
        event = m_pImpl->events->front();
        return true;
    }
    return false;
}

void Window::clear(Color const &color)
{
    if (!m_pImpl->openGl->glClearColorSupported() ||
        !m_pImpl->openGl->glClearSupported())
    {
        throw std::runtime_error{"LibGraphics::Window::clear Error. Your system is missing support for required OpenGL functions."};
    }

    const Color::Float floatColor{color.toFloat()};
    if (!m_pImpl->openGl->isCurrent())
    {
        m_pImpl->openGl->makeCurrent();
    }
    m_pImpl->openGl->glClearColor()(floatColor.r, floatColor.g, floatColor.b, floatColor.a);
    m_pImpl->openGl->glClear()(GL_COLOR_BUFFER_BIT);
}

void Window::draw(Drawable &drawable)
{
    drawable.draw(*this);
}

void Window::draw(VertexBuffer const &vertexBuffer)
{
    std::optional<std::reference_wrapper<Texture>> texture{vertexBuffer.getTexture(LibUtilities::Badge<Window>{})};

    if (texture)
    {
        m_pImpl->openGl->glUseProgram()(m_pImpl->textureShaderProgram->getId());
    }
    else
    {
        m_pImpl->openGl->glUseProgram()(m_pImpl->shaderProgram->getId());
    }

    if (!m_pImpl->openGlDrawables.contains(&vertexBuffer))
    {
        //m_pImpl->openGlDrawables.insert({&vertexBuffer, std::make_unique<OpenGlDrawable>(m_pImpl->openGl, static_cast<bool>(texture))});
        m_pImpl->openGlDrawables.emplace(&vertexBuffer, std::make_unique<OpenGlDrawable>(m_pImpl->openGl, static_cast<bool>(texture)));
        m_pImpl->openGlDrawables.at(&vertexBuffer)->bind();
        if (texture)
        {
            m_pImpl->openGl->glVertexAttribPointer()(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(0));
            m_pImpl->openGl->glEnableVertexAttribArray()(0);
            m_pImpl->openGl->glVertexAttribPointer()(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
            m_pImpl->openGl->glEnableVertexAttribArray()(1);
            m_pImpl->openGl->glVertexAttribPointer()(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(6 * sizeof(float)));
            m_pImpl->openGl->glEnableVertexAttribArray()(2);

            // TODO: make these configurable
            m_pImpl->openGl->glTexParameteri()(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            m_pImpl->openGl->glTexParameteri()(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            m_pImpl->openGl->glTexParameteri()(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            m_pImpl->openGl->glTexParameteri()(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilteringMap.at(texture->get().getFiltering()));
        }
        else
        {
            m_pImpl->openGl->glVertexAttribPointer()(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(0));
            m_pImpl->openGl->glEnableVertexAttribArray()(0);
            m_pImpl->openGl->glVertexAttribPointer()(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
            m_pImpl->openGl->glEnableVertexAttribArray()(1);
        }
    }
    else
    {
        m_pImpl->openGlDrawables.at(&vertexBuffer)->bind();
    }

    if (texture)
    {
        if (!texture->get().isValid())
        {
            m_pImpl->openGl->glTexImage2D()(
                GL_TEXTURE_2D,
                0,
                GL_RGB,
                texture->get().getSize().getX(),
                texture->get().getSize().getY(),
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                texture->get().getData().data());
            m_pImpl->openGl->glGenerateMipmap()(GL_TEXTURE_2D);
            texture->get().validate();
        }
    }

    std::vector<float> const &vertexData{vertexBuffer.getData()};
    m_pImpl->openGl->glBufferData()(
        GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(float),
        vertexData.data(),
        GL_STATIC_DRAW); // TODO: Determine draw type from drawables vertex buffer

    m_pImpl->openGl->glDrawArrays()(
        primitiveMap.at(vertexBuffer.getPrimitive()),
        0,
        static_cast<GLsizei>(vertexBuffer.getCount()));   
}

void Window::display() const
{
    m_pImpl->osImpl.display();
}

}
