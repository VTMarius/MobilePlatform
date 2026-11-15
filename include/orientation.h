#pragma once

#include <algorithm>
#include <cmath>

namespace orientation {
struct Quaternion {
  float w = 1, x = 0, y = 0, z = 0;
  Quaternion() = default;
  Quaternion(float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) {}
};
struct Vector3 { float x, y, z; };
struct Euler { float roll, pitch, yaw; };

inline bool normalize(const Quaternion& q, Quaternion& result) {
  const double n2 = double(q.w)*q.w + double(q.x)*q.x + double(q.y)*q.y + double(q.z)*q.z;
  if (!std::isfinite(n2) || n2 < 1e-12) return false;
  const float scale = static_cast<float>(1.0 / std::sqrt(n2));
  result = Quaternion(q.w*scale, q.x*scale, q.y*scale, q.z*scale);
  return true;
}
inline Quaternion conjugate(const Quaternion& q) { return Quaternion(q.w,-q.x,-q.y,-q.z); }
inline Quaternion multiply(const Quaternion& a, const Quaternion& b) {
  return Quaternion(a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z,
                    a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
                    a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
                    a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w);
}
// Active Hamilton rotations: q_world_from_body = q_world_from_sensor * q_sensor_from_body.
inline Quaternion bodyOrientation(const Quaternion& sensor, const Quaternion& sensorFromBody) {
  return multiply(sensor, sensorFromBody);
}
inline Quaternion relativeTo(const Quaternion& reference, const Quaternion& current) {
  return multiply(conjugate(reference), current);
}
inline Vector3 rotate(const Quaternion& q, const Vector3& v) {
  const Quaternion r = multiply(multiply(q, Quaternion(0,v.x,v.y,v.z)), conjugate(q));
  return {r.x,r.y,r.z};
}
inline Euler eulerDegrees(const Quaternion& q) {
  constexpr float radToDeg = 57.29577951308232f;
  const float s = std::max(-1.0f,std::min(1.0f,2.0f*(q.w*q.y-q.z*q.x)));
  return {std::atan2(2*(q.w*q.x+q.y*q.z),1-2*(q.x*q.x+q.y*q.y))*radToDeg,
          std::asin(s)*radToDeg,
          std::atan2(2*(q.w*q.z+q.x*q.y),1-2*(q.y*q.y+q.z*q.z))*radToDeg};
}
}  // namespace orientation
