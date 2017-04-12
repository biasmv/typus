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
#include <typus/mem_view.hh>

#include <gtest/gtest.h>

using namespace typus;

TEST(MemView, construction) {
    int data[] = {
        0, 1, 2, 3, 4, 5
    };

    mem_view<int> view{data, data + 6};
    ASSERT_EQ(6, view.size());
    ASSERT_EQ(data, view.begin());
    ASSERT_EQ(data + 6, view.end());
    ASSERT_TRUE(!view.empty());
}

TEST(MemView, indexing) {
    int data[] = {
        0, 1, 2, 3, 4, 5
    };

    mem_view<int> view{data, data + 6};
    ASSERT_EQ(0, view[0]);
    ASSERT_EQ(1, view[1]);
    ASSERT_EQ(2, view[2]);
    ASSERT_EQ(3, view[3]);
    ASSERT_EQ(4, view[4]);
    ASSERT_EQ(5, view[5]);
}

TEST(MemView, comparison) {
    int data1[] = {
        0, 1, 2, 3, 4, 5
    };
    int data2[] = {
        1, 1, 2, 3, 4, 5
    };

    mem_view<int> view1{data1, data1 + 6};
    mem_view<int> view2{data2, data2 + 6};

    ASSERT_TRUE(view1 == view1);

    ASSERT_FALSE(view1 == view2);
    ASSERT_TRUE(view1 != view2);

    mem_view<int> view3{data1+1, data1 + 6};
    mem_view<int> view4{data2+1, data2 + 6};
    ASSERT_TRUE(view3 == view3);
    ASSERT_FALSE(view3 != view4);
    ASSERT_TRUE(view1 != view3);
}

