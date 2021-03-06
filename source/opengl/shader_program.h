#pragma once

#include <string>
#include <stdexcept>
#include <vector>

#include "libutilities/non_null.h"

#include "opengl.h"
#include "shader.h"

namespace LibGraphics
{

class ShaderProgram
{
public:
    template <typename ...ShaderT>
    ShaderProgram(NonNullSharedPtr<OpenGl> openGl, ShaderT const &...shaders) :
        m_openGl{openGl}
    {
        m_id = m_openGl->glCreateProgram()();
        (m_openGl->glAttachShader()(m_id, shaders->getId()), ...);
        m_openGl->glLinkProgram()(m_id);
        int success;
        m_openGl->glGetProgramiv()(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            std::vector<char> infoLog;
            GLint infoLogLength;
            m_openGl->glGetProgramiv()(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
            infoLog.resize(infoLogLength + 1);
            m_openGl->glGetProgramInfoLog()(m_id, infoLogLength, nullptr, &*infoLog.begin());
            throw std::runtime_error{
                std::string{"LibGraphics::Window Error. Failed to link shader program.\n"}
                .append(infoLog.data())
            };
        }
    }

    ~ShaderProgram()
    {
        m_openGl->glDeleteProgram()(m_id);
    }

    GLuint getId() const
    {
        return m_id;
    }

private:
    NonNullSharedPtr<OpenGl> m_openGl;
    GLuint m_id;
};

}