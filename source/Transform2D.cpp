#include "Transform2D.hpp"

#include <cmath>

Transform2D Transform2D::translation(float tx, float ty) noexcept {
    Transform2D result;
    result.m[2][0] = tx;
    result.m[2][1] = ty;
    return result;
}

Transform2D Transform2D::rotation(float angle_rad) noexcept {
    const float c = std::cos(angle_rad);
    const float s = std::sin(angle_rad);

    Transform2D result;
    result.m[0][0] = c;
    result.m[0][1] = s;
    result.m[1][0] = -s;
    result.m[1][1] = c;
    return result;
}

Transform2D Transform2D::scale(float sx, float sy) noexcept {
    Transform2D result;
    result.m[0][0] = sx;
    result.m[1][1] = sy;
    return result;
}

Transform2D Transform2D::operator*(const Transform2D& rhs) const noexcept {
    Transform2D result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            result.m[row][col] =
                m[row][0] * rhs.m[0][col] +
                m[row][1] * rhs.m[1][col] +
                m[row][2] * rhs.m[2][col];
        }
    }
    return result;
}

Transform2D& Transform2D::operator*=(const Transform2D& rhs) noexcept {
    *this = *this * rhs;
    return *this;
}

Vector2D Transform2D::transform_point(const Vector2D& point) const noexcept {
    return Vector2D{
        point.x * m[0][0] + point.y * m[1][0] + m[2][0],
        point.x * m[0][1] + point.y * m[1][1] + m[2][1]
    };
}

Vector2D Transform2D::transform_vector(const Vector2D& direction) const noexcept {
    return Vector2D{
        direction.x * m[0][0] + direction.y * m[1][0],
        direction.x * m[0][1] + direction.y * m[1][1]
    };
}
