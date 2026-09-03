// math.h - Minimal matrix/vector math for OpenGL (column-major)
#pragma once
#include <cmath>
#include <cstring>

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float len = length();
        return len > 0 ? Vec3(x / len, y / len, z / len) : Vec3();
    }
};

// 4x4 matrix, column-major (OpenGL style)
struct Mat4 {
    float m[16];

    Mat4(bool identity = false) {
        std::memset(m, 0, sizeof(m));
        if (identity) m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    float& operator()(int row, int col) { return m[col * 4 + row]; }
    const float& operator()(int row, int col) const { return m[col * 4 + row]; }

    const float* ptr() const { return m; }

    static Mat4 perspective(float fovY, float aspect, float near, float far) {
        Mat4 result;
        float tanHalf = std::tan(fovY * 0.5f);
        result(0, 0) = 1.0f / (aspect * tanHalf);
        result(1, 1) = 1.0f / tanHalf;
        result(2, 2) = (far + near) / (near - far);
        result(2, 3) = (2.0f * far * near) / (near - far);
        result(3, 2) = -1.0f;
        return result;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);

        Mat4 result(true);
        result(0, 0) = s.x; result(0, 1) = u.x; result(0, 2) = -f.x;
        result(1, 0) = s.y; result(1, 1) = u.y; result(1, 2) = -f.y;
        result(2, 0) = s.z; result(2, 1) = u.z; result(2, 2) = -f.z;
        result(0, 3) = -s.dot(eye);
        result(1, 3) = -u.dot(eye);
        result(2, 3) = f.dot(eye);
        return result;
    }

    static Mat4 translate(const Vec3& t) {
        Mat4 result(true);
        result(0, 3) = t.x;
        result(1, 3) = t.y;
        result(2, 3) = t.z;
        return result;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 result(true);
        result(0, 0) = s.x;
        result(1, 1) = s.y;
        result(2, 2) = s.z;
        return result;
    }
};
