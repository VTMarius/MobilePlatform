#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include "orientation.h"
using namespace orientation;
bool near(float a,float b,float eps=0.001f) { return std::fabs(a-b)<eps; }
int main() {
  Quaternion out;
  assert(!normalize({0,0,0,0},out));
  assert(!normalize({std::numeric_limits<float>::quiet_NaN(),0,0,0},out));
  assert(normalize({2,0,0,0},out) && near(out.w,1));
  constexpr float h=0.70710678118f;
  const Quaternion z90(h,0,0,h), x90(h,h,0,0);
  auto e=eulerDegrees(z90); assert(near(e.yaw,90) && near(e.roll,0) && near(e.pitch,0));
  auto v=rotate(z90,{1,0,0}); assert(near(v.x,0) && near(v.y,1) && near(v.z,0));
  auto relative=relativeTo(z90,z90); assert(near(relative.w,1) && near(relative.z,0));
  relative=relativeTo(z90,Quaternion{}); assert(near(eulerDegrees(relative).yaw,-90));
  e=eulerDegrees({-h,0,0,-h}); assert(near(e.yaw,90));
  // Noncommuting rotations catch a reversed mounting multiplication order.
  const auto body=bodyOrientation(z90,x90);
  v=rotate(body,{0,1,0}); assert(near(v.x,0) && near(v.y,0) && near(v.z,1));
  const auto wrong=rotate(multiply(x90,z90),{0,1,0}); assert(near(wrong.x,-1));
  v=rotate(conjugate(z90),{0,1,0}); assert(near(v.x,1) && near(v.y,0));
  Quaternion pitch90(h,0,h,0); normalize(pitch90,pitch90);
  e=eulerDegrees(pitch90); assert(std::isfinite(e.pitch) && near(e.pitch,90,0.04f));
  std::cout << "Orientation checks passed\n";
}
