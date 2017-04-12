// -----------------------------------------------------------------------------
// Copyright 2016 Marco Biasini
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// -----------------------------------------------------------------------------
#ifndef TYPUS_VEC3_HH
#define TYPUS_VEC3_HH

#include <cmath>
#include <ostream>

#include <typus/numbers.hh>

namespace typus {

template <typename T>
struct vec3 {
    vec3() = default;

    vec3(T px, T py, T pz): x(px), y(py), z(pz) {
    }

    vec3(const vec3 &rhs) = default;

    vec3& operator=(const vec3 &rhs) = default;

    vec3 &operator-=(const vec3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    vec3 operator-(const vec3 &rhs) const {
        vec3 r(*this);
        r-= rhs;
        return r;
    }

    vec3 &operator+=(const vec3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    vec3 operator+(const vec3 &rhs) const {
        vec3 r(*this);
        r+= rhs;
        return r;
    }

    inline T normSquared() const {
        return x * x + y * y + z * z;
    }

    inline T norm() const {
        return std::sqrt(this->normSquared());
    }

    vec3<T> normalized() const {
        vec3 r(*this);
        r.normalize();
        return r;
    }

    f32 normalize() {
        f32 n = this->norm();
        f32 f = 1.0f/n;
        x *= f;
        y *= f;
        z *= f;
        return n;
    }

    vec3 & operator /=(T f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    vec3 & operator *=(T f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    vec3 operator *(T f) const {
        vec3 r(*this);
        r *= f;
        return r;
    }

    vec3 xxx() const {
        return vec3(x, x, x);
    }

    vec3 yyy() const {
        return vec3(y, y, y);
    }

    vec3 zzz() const {
        return vec3(z, z, z);
    }

    vec3 zyx() const {
        return vec3(z, y, x);
    }

    T x = 0;
    T y = 0;
    T z = 0;
};

template <typename T>
vec3<T> operator*(T lhs, const vec3<T>& rhs) {
    return rhs * lhs;
}

template <typename T>
inline float dot(const vec3<T>& a, const vec3<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
inline vec3<T> reflect(const vec3<T>& v, const vec3<T>& n) {
    float d = 2.0f * dot(v, n);
    return (d * n - v).normalized();
}

template <typename T>
std::ostream &operator <<(std::ostream &s, const vec3<T> &v) {
    return (s << "{" << v.x << ", " << v.y << ", " << v.z << "}");
}


template <typename T>
bool operator == (const vec3<T> &lhs, const vec3<T> &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template <typename T>
bool operator != (const vec3<T> &lhs, const vec3<T> &rhs) {
    return !operator==(lhs, rhs);
}

using vec3_f = vec3<f32>;

} // namespace

#endif // TYPUS_VEC3_HH
