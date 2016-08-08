#pragma once

namespace paiv {

typedef struct vector3d {
  linalg::aliases::float3 v;
} vector3d;

typedef struct matrix3d {
  linalg::aliases::float3x3 m;

  matrix3d inverse() const {
    return { linalg::inverse(m) };
  }

  matrix3d mul(const matrix3d& other) const {
    return { linalg::mul(m, other.m) };
  }

  vector3d mul(const vector3d& other) const {
    return { linalg::mul(m, other.v) };
  }

} matrix3d;

}
