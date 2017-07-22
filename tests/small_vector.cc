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
#include <typus/small_vector.hh>

#include <gtest/gtest.h>

using namespace typus;

TEST(SmallVector, construction) {
    small_vector_n<int, 8> v;
    EXPECT_EQ(0u, v.end() - v.begin());
    EXPECT_EQ(8u, v.capacity());

    small_vector_n<uint16_t, 1> v2;
    EXPECT_EQ(0u, v2.end() - v2.begin());
    EXPECT_EQ(1u, v2.capacity());
}

struct T {
    T(int &value): v(value) {}
    T(T &&) = default;
    T(const T &) = default;
    ~T() {
        ++v;
    }
    int &v;
};

TEST(SmallVector, clear_small) {
    int dtor = 0;
    {
        small_vector_n<T, 2> v{};
        v.emplace_back(dtor);
        v.emplace_back(dtor);
        v.clear();
        ASSERT_EQ(2, dtor);
    }
    ASSERT_EQ(2, dtor);

}

TEST(SmallVector, copy_constructor_small) {
    auto values = {
        4, 3
    };
    small_vector_n<int, 2> v(values.begin(), values.end());
    small_vector_n<int, 2> v2 = v;
    ASSERT_EQ(2u, v2.size());
    ASSERT_EQ(4u, v2.capacity());
    ASSERT_EQ(4, v2[0]);
    ASSERT_EQ(3, v2[1]);
}

TEST(SmallVector, copy_constructor_big) {
    auto values = {
        4, 3, 2, 1
    };
    small_vector_n<int, 2> v(values.begin(), values.end());
    small_vector_n<int, 2> v2 = v;
    ASSERT_EQ(4u, v2.size());
    ASSERT_EQ(8u, v2.capacity());
    ASSERT_EQ(4, v2[0]);
    ASSERT_EQ(3, v2[1]);
    ASSERT_EQ(2, v2[2]);
    ASSERT_EQ(1, v2[3]);
}

TEST(SmallVector, destructor_of_elements_gets_called) {
    int counter = 0;
    T value { counter };
    {
        small_vector_n<T, 2> v;
        v.push_back(value);
        v.push_back(value);
    }
    ASSERT_EQ(2, counter);
}

TEST(SmallVector, push_back) {
    small_vector_n<int, 2> v;
    v.push_back(33);
    ASSERT_EQ(1u, v.size());
    ASSERT_EQ(33, v[0]);
    v.push_back(34);
    ASSERT_EQ(34, v[1]);
    ASSERT_EQ(2u, v.capacity());
    v.push_back(35);
    ASSERT_EQ(4u, v.capacity());
    ASSERT_EQ(3u, v.size());
    ASSERT_EQ(33,  v[0]);
    ASSERT_EQ(34,  v[1]);
    ASSERT_EQ(35,  v[2]);
}

TEST(SmallVector, emplace_back) {
    int value = 0;
    {
        small_vector_n<T, 3> v;
        v.emplace_back(value);
        v.emplace_back(value);
        v.emplace_back(value);
        v.emplace_back(value);
        ASSERT_EQ(8u, v.capacity());
        ASSERT_EQ(4u, v.size());
        // resize required before 4th push-back. Values are moved
        ASSERT_EQ(3, value);
    }
    ASSERT_EQ(7, value);
}


TEST(SmallVector, pop_back) {
    int value = 0;
    {
        small_vector_n<T, 3> v;
        v.emplace_back(value);
        v.pop_back();
        ASSERT_EQ(1, value);
    }
    // d'tor should not get called a second time
    ASSERT_EQ(1, value);
}

TEST(SmallVector, resize_to_larger_size) {
    small_vector_n<int, 8> v;
    v.push_back(456);
    v.resize(4);
    ASSERT_EQ(0, v[1]);
    ASSERT_EQ(0, v[2]);
    ASSERT_EQ(0, v[3]);
}

struct MoveCount {
    int &ctor;
    int &assign;
    int &dtor;

    MoveCount(const MoveCount&) = delete;
    MoveCount &operator=(const MoveCount&) = delete;

    ~MoveCount() {
        ++dtor;
    }

    MoveCount(int &c, int &a, int &d): ctor(c), assign(a), dtor(d) {
    }
    MoveCount(MoveCount &&rhs): 
        ctor(rhs.ctor), assign(rhs.assign), dtor(rhs.dtor) {
        ctor++;

    }
    MoveCount && operator=(MoveCount &&rhs) {
        ctor = rhs.ctor;
        assign = rhs.assign;
        dtor = rhs.dtor;
        assign++;
        return std::move(*this);
    }
};

TEST(SmallVector, move_assignment_small_to_small) {
    int ctor = 0;
    int assign = 0;
    int dtor = 0;
    {
        small_vector_n<MoveCount, 2> v;
        v.emplace_back(ctor, assign, dtor);
        v.emplace_back(ctor, assign, dtor);
        ASSERT_EQ(2u, v.size());
        {
            small_vector_n<MoveCount, 2> v2;
            v2.emplace_back(ctor, assign, dtor);
            v2 = std::move(v);
        }
        EXPECT_EQ(1, assign);
        EXPECT_EQ(1, ctor);
        EXPECT_EQ(3, dtor);
    }
    EXPECT_EQ(3, dtor);
}

TEST(SmallVector, move_assignment_small_to_large) {
    small_vector_n<MoveCount, 2> v;
    int ctor = 0;
    int assign = 0;
    int dtor = 0;
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    ASSERT_EQ(2u, v.size());
    {
        small_vector_n<MoveCount, 2> v2;
        v2.emplace_back(ctor, assign, dtor);
        v2.emplace_back(ctor, assign, dtor);
        v2.emplace_back(ctor, assign, dtor);
        EXPECT_EQ(2, ctor);
        EXPECT_EQ(2, dtor);
        v2 = std::move(v);
    }
    EXPECT_EQ(0, assign);
    EXPECT_EQ(4, ctor);
    EXPECT_EQ(9, dtor);
}

TEST(SmallVector, move_assignment_large_to_small) {
    small_vector_n<MoveCount, 2> v;
    int ctor = 0;
    int assign = 0;
    int dtor = 0;
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    EXPECT_EQ(2, dtor);

    {
        small_vector_n<MoveCount, 2> v2;
        v2.emplace_back(ctor, assign, dtor);
        v2 = std::move(v);
    }
    EXPECT_EQ(0, assign);
    EXPECT_EQ(2, ctor);
    EXPECT_EQ(6, dtor);
}

TEST(SmallVector, move_assignment_large_to_large) {
    small_vector_n<MoveCount, 2> v;
    int ctor = 0;
    int assign = 0;
    int dtor = 0;
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    {
        small_vector_n<MoveCount, 2> v2;
        v2.emplace_back(ctor, assign, dtor);
        v2.emplace_back(ctor, assign, dtor);
        v2.emplace_back(ctor, assign, dtor);
        v2 = std::move(v);
    }
    EXPECT_EQ(0, assign);
    EXPECT_EQ(4, ctor);
    EXPECT_EQ(10, dtor);
}

TEST(SmallVector, copy_assignment) {
    small_vector_n<int, 2> v;
    v.push_back(1);
    v.push_back(2);
    {
        small_vector_n<int, 2> v2;
        v2.push_back(3);
        v2 = v;
        ASSERT_EQ(2u, v2.size());
        ASSERT_EQ(1, v2[0]);
        ASSERT_EQ(2, v2[1]);
    }
}

TEST(SmallVector, move_construction_small) {
    small_vector_n<MoveCount, 2> v;
    int ctor = 0;
    int assign = 0;
    int dtor = 0;
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    ASSERT_EQ(2u, v.size());
    {
        small_vector_n<MoveCount, 2> v2(std::move(v));
    }
    EXPECT_EQ(0, assign);
    EXPECT_EQ(2, ctor);
    EXPECT_EQ(4, dtor);
}

TEST(SmallVector, move_construction_large) {
    small_vector_n<MoveCount, 2> v;
    int ctor = 0;
    int assign = 0;
    int dtor = 0;
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    v.emplace_back(ctor, assign, dtor);
    {
        small_vector_n<MoveCount, 2> v2(std::move(v));
    }
    EXPECT_EQ(0, assign);
    EXPECT_EQ(2, ctor);
    EXPECT_EQ(5, dtor);
}

TEST(SmallVector, resize_to_smaller_size) {
    small_vector_n<int, 8> v;
    v.push_back(456);
    v.push_back(300);
    v.push_back(200);
    v.resize(2);
    ASSERT_EQ(2u, v.size());
    ASSERT_EQ(456, v[0]);
    ASSERT_EQ(300, v[1]);
}

