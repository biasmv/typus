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
#include <iostream>
#include <fstream>
#include <vector>
#include <typus/small_vector.hh>

namespace ty = typus;

template <typename C>
std::size_t test(const std::vector<std::string> &data) {
    C result;
    for (const auto & s: data) {
        for (char c : s) {
            if (c <= 'z' && c >= 'a') {
                result.push_back(c);
            }
        }
    }
    return result.size();
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        std::cerr << "usage small-vector-benchmark <data-file>\n";
        return -1;
    }
    std::vector<std::string> data;
    std::ifstream in_stream(argv[1]);
    std::string line;
    while (std::getline(in_stream, line)) {
        data.push_back(line);
    }
    std::size_t count = 0;
    for (int i = 0; i < 100000; ++i) {
        count += test<std::vector<char>>(data);
        //count += test<ty::small_vector_n<char, 8>>(data);
    }
    std::cout << count << "\n";
    return 0;
}
