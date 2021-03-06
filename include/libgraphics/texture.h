#pragma once

#include <map>
#include <stdexcept>
#include <vector>

#include "libgraphics/color.h"
#include "libgraphics/vector.h"

namespace LibGraphics
{

class Texture
{
public:
    using SizeVector = Vector<size_t>;
    using PositionVector = SizeVector;

    size_t constexpr static bytesPerPixel{3};

    enum class Filtering { Nearest, Linear };

    Texture() = delete;

    template <typename TData>
    Texture(TData data, SizeVector const &size, Filtering filtering = Filtering::Nearest) :
        m_data{std::forward<TData>(data)},
        m_size{size},
        m_valid{false},
        m_filtering{filtering}
    {
        if (m_size.getX() * m_size.getY() * bytesPerPixel != m_data.size())
        {
            throw std::invalid_argument{"LibGraphics::Texture Error. Invalid size information."};
        }
    }

    std::vector<uint8_t> const &getData() const;
    SizeVector getSize() const;
    Filtering getFiltering() const;
    bool isValid() const;

    bool setPixel(PositionVector const &position, Color color);
    void validate();

private:
    std::vector<uint8_t> m_data;
    SizeVector m_size;
    bool m_valid;
    Filtering m_filtering;
};

};
