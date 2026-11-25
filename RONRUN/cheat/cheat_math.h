#pragma once
// ============================================================================
//  Math Utilities — vectors, matrices, angle calculations
// ============================================================================

#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD(x) ((x) * (float)(M_PI / 180.0))
#define RAD2DEG(x) ((x) * (float)(180.0 / M_PI))

// ---- Vector Types ----

struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
    Vec2 operator-(const Vec2& o) const { return { x - o.x, y - o.y }; }
    Vec2 operator*(float s)       const { return { x * s, y * s }; }

    float Length() const { return sqrtf(x * x + y * y); }
    float Dot(const Vec2& o) const { return x * o.x + y * o.y; }
};

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s)       const { return { x * s, y * s, z * s }; }

    float Length()  const { return sqrtf(x * x + y * y + z * z); }
    float Length2D() const { return sqrtf(x * x + z * z); }

    float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }

    Vec3 Normalized() const {
        float len = Length();
        if (len < 0.0001f) return { 0, 0, 0 };
        return { x / len, y / len, z / len };
    }

    float Distance(const Vec3& o) const {
        return (*this - o).Length();
    }
};

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

// ---- Matrix ----

struct Matrix4x4 {
    float m[4][4];

    Matrix4x4() { memset(m, 0, sizeof(m)); }

    Vec4 Row(int i) const { return { m[i][0], m[i][1], m[i][2], m[i][3] }; }
};

// ---- World to Screen ----
// Uses Unity's view-projection matrix to convert 3D world position to 2D screen
// Returns false if the point is behind the camera

inline bool WorldToScreen(
    const Vec3& world, const Matrix4x4& vpm,
    float screen_w, float screen_h,
    Vec2& out
) {
    float w = vpm.m[3][0] * world.x + vpm.m[3][1] * world.y +
              vpm.m[3][2] * world.z + vpm.m[3][3];

    if (w < 0.001f)
        return false;

    float x = vpm.m[0][0] * world.x + vpm.m[0][1] * world.y +
              vpm.m[0][2] * world.z + vpm.m[0][3];
    float y = vpm.m[1][0] * world.x + vpm.m[1][1] * world.y +
              vpm.m[1][2] * world.z + vpm.m[1][3];

    float inv_w = 1.0f / w;
    x *= inv_w;
    y *= inv_w;

    out.x = (screen_w * 0.5f) + (x * screen_w * 0.5f);
    out.y = (screen_h * 0.5f) - (y * screen_h * 0.5f);

    return true;
}

// ---- Angle Calculations ----

// Calculate aim angles from source to destination (pitch, yaw)
inline Vec2 CalcAngle(const Vec3& src, const Vec3& dst) {
    Vec3 delta = dst - src;
    float hyp = delta.Length2D();

    float pitch = -RAD2DEG(atan2f(delta.y, hyp));
    float yaw   =  RAD2DEG(atan2f(delta.x, delta.z));

    return { pitch, yaw };
}

// Normalize angles to [-180, 180]
inline void NormalizeAngles(Vec2& angles) {
    while (angles.x >  89.0f) angles.x -= 180.0f;
    while (angles.x < -89.0f) angles.x += 180.0f;
    while (angles.y >  180.0f) angles.y -= 360.0f;
    while (angles.y < -180.0f) angles.y += 360.0f;
}

// Get FOV (angle difference) between current aim and target angle
inline float GetFOV(const Vec2& current, const Vec2& target) {
    Vec2 delta = { target.x - current.x, target.y - current.y };
    NormalizeAngles(delta);
    return delta.Length();
}

// Smooth aim towards target
inline Vec2 SmoothAngle(const Vec2& current, const Vec2& target, float smoothing) {
    Vec2 delta = { target.x - current.x, target.y - current.y };
    NormalizeAngles(delta);
    return {
        current.x + delta.x / smoothing,
        current.y + delta.y / smoothing
    };
}

// Get angle for Unity (euler angles, Y-up coordinate system)
inline Vec3 CalcAngleUnity(const Vec3& src, const Vec3& dst) {
    Vec3 dir = (dst - src).Normalized();

    float pitch = -RAD2DEG(asinf(dir.y));
    float yaw   =  RAD2DEG(atan2f(dir.x, dir.z));

    return { pitch, yaw, 0.0f };
}
