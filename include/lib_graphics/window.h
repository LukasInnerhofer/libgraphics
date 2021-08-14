#pragma once

#include "lib_graphics/canvas.h"
#include "lib_graphics/color.h"
#include "lib_graphics/string.h"

namespace LibGraphics
{

class Window : public Canvas
{
public:
    enum class EventType { Closed };
    typedef struct EventTag
    {
        EventType type;

        EventTag() : type{EventType::Closed}
        {

        }

        EventTag(EventType type) : type{type}
        {

        }
    } Event;

    Window(String const &title);
    ~Window();
    
    bool pollEvent(Event &event);
    void clear(Color const &color);
    using Canvas::draw;
    void draw(VertexBuffer const &vertexBuffer) override;
    void display() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};

}

