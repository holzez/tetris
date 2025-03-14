#if !defined(TETRIS_MATH_H)

#include "tetris_typedefs.h"

struct vec2i_t
{
    union
    {
        int32 x;
        int32 w;
    };

    union
    {
        int32 y;
        int32 h;
    };

    vec2i_t operator+(const vec2i_t &other) const
    {
        return {x + other.x, y + other.y};
    }

    vec2i_t operator-(const vec2i_t &other) const
    {
        return {x - other.x, y - other.y};
    }

    vec2i_t operator*(int32 scalar) const
    {
        return {x * scalar, y * scalar};
    }

    vec2i_t operator/(int32 scalar) const
    {
        return {x / scalar, y / scalar};
    }

    bool operator==(const vec2i_t &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const vec2i_t &other) const
    {
        return x != other.x || y != other.y;
    }

    vec2i_t &operator+=(const vec2i_t &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    vec2i_t &operator-=(const vec2i_t &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    vec2i_t &operator*=(int32 scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    vec2i_t &operator/=(int32 scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    vec2i_t operator-() const
    {
        return {-x, -y};
    }

    int32 &operator[](int32 index)
    {
        return (&x)[index];
    }

    const int32 &operator[](int32 index) const
    {
        return (&x)[index];
    }
};

struct vec2_t
{
    union
    {
        real32 x;
        real32 w;
    };

    union
    {
        real32 y;
        real32 h;
    };

    vec2_t operator+(const vec2_t &other) const
    {
        return {x + other.x, y + other.y};
    }

    vec2_t operator-(const vec2_t &other) const
    {
        return {x - other.x, y - other.y};
    }

    vec2_t operator*(int32 scalar) const
    {
        return {x * scalar, y * scalar};
    }

    vec2_t operator/(int32 scalar) const
    {
        return {x / scalar, y / scalar};
    }

    bool operator==(const vec2_t &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const vec2_t &other) const
    {
        return x != other.x || y != other.y;
    }

    vec2_t &operator+=(const vec2_t &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    vec2_t &operator-=(const vec2_t &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    vec2_t &operator*=(real32 scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    vec2_t &operator/=(real32 scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    vec2_t operator-() const
    {
        return {-x, -y};
    }

    real32 &operator[](int32 index)
    {
        return (&x)[index];
    }

    const real32 &operator[](int32 index) const
    {
        return (&x)[index];
    }
};

struct color_t
{
    uint8 r;
    uint8 g;
    uint8 b;
};

#define TETRIS_MATH_H
#endif
