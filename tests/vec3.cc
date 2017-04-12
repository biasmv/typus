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
#include <typus/vec3.hh>

#include <gtest/gtest.h>

using namespace typus;

TEST(Vec3, default_construction) {
    vec3_f v{};

    ASSERT_EQ(0.0f, v.x);
    ASSERT_EQ(0.0f, v.y);
    ASSERT_EQ(0.0f, v.z);
}

TEST(Vec3, construction) {
    vec3_f v{1.0f, 2.0f, 3.0f};

    ASSERT_EQ(1.0f, v.x);
    ASSERT_EQ(2.0f, v.y);
    ASSERT_EQ(3.0f, v.z);
}

TEST(Vec3, normalize) {
    vec3_f v{2.0f, 3.0f, 6.0f};
    f32 length = v.normalize();
    ASSERT_NEAR(7.0f, length, 0.001f);

    ASSERT_NEAR(2.0f/7.0f, v.x, 0.001f);
    ASSERT_NEAR(3.0f/7.0f, v.y, 0.001f);
    ASSERT_NEAR(6.0f/7.0f, v.z, 0.001f);
}

TEST(Vec3, equality) {
    ASSERT_TRUE(vec3_f(1.0f, 2.0f, 3.0f) == vec3_f(1.0f, 2.0f, 3.0f));
    ASSERT_TRUE(vec3_f(1.0f, 2.0f, 3.0f) != vec3_f(2.0f, 2.0f, 3.0f));
    ASSERT_TRUE(vec3_f(1.0f, 2.0f, 3.0f) != vec3_f(1.0f, 3.0f, 3.0f));
    ASSERT_TRUE(vec3_f(1.0f, 2.0f, 3.0f) != vec3_f(1.0f, 2.0f, 4.0f));
}

TEST(Vec3, normalized) {
    vec3_f v{2.0f, 3.0f, 6.0f};
    vec3_f n = v.normalized();
    ASSERT_EQ(2.0f, v.x);
    ASSERT_EQ(3.0f, v.y);
    ASSERT_EQ(6.0f, v.z);

    ASSERT_NEAR(2.0f/7.0f, n.x, 0.001f);
    ASSERT_NEAR(3.0f/7.0f, n.y, 0.001f);
    ASSERT_NEAR(6.0f/7.0f, n.z, 0.001f);
}

TEST(Vec3, swizzle) {
    ASSERT_EQ(vec3_f(1.0f, 1.0f, 1.0f), vec3_f(1.0f, 2.0f, 3.0f).xxx());
    ASSERT_EQ(vec3_f(2.0f, 2.0f, 2.0f), vec3_f(1.0f, 2.0f, 3.0f).yyy());
    ASSERT_EQ(vec3_f(3.0f, 3.0f, 3.0f), vec3_f(1.0f, 2.0f, 3.0f).zzz());
    ASSERT_EQ(vec3_f(3.0f, 2.0f, 1.0f), vec3_f(1.0f, 2.0f, 3.0f).zyx());
}

