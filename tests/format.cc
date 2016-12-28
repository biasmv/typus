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
#include <typus/format.hh>

#include <gtest/gtest.h>

using namespace typus;

TEST(Format, two_simple_args) {
    ASSERT_EQ("abc1defXYZ", $("abc{}def{}").to_str(1, "XYZ"));
}

TEST(Format, allows_to_escape_curly_braces) {
    ASSERT_EQ("abc{}defXYZ", $("abc{{}}def{}").to_str("XYZ"));
    ASSERT_EQ("abcXYZdef{}", $("abc{}def{{}}").to_str("XYZ"));
    ASSERT_EQ("abc}defXYZ", $("abc}}def{}").to_str("XYZ"));
    ASSERT_EQ("abc{defXYZ", $("abc{{def{}").to_str("XYZ"));
}

TEST(Format, format_modifier) {
    //ASSERT_EQ("a: 4.25", $("a: {:.2}").to_str(4.25f));
}

struct NonCopyableAndMovable {
    NonCopyableAndMovable(const NonCopyableAndMovable &rhs) = delete;
    NonCopyableAndMovable(const NonCopyableAndMovable &&rhs) = delete;
    explicit NonCopyableAndMovable(int a): value(a) {
    }
    int value;
};

std::ostream &operator<<(std::ostream &os, const NonCopyableAndMovable& v) {
    return os << v.value;
}

TEST(Format, does_not_move_or_copy_arguments) {
    ASSERT_EQ("3", $("{}").to_str(NonCopyableAndMovable(3)));
}

