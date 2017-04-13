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
#ifndef TYPUS_ASSERT_HH
#define TYPUS_ASSERT_HH

#include <cassert>
#include <cmath>
#include <iostream>


namespace typus {

inline void fail(const char* file, int line,
                 const std::string &message) {
    std::cerr << file << ":" << line << ": " << message << "\n";
    std::abort();
}

} // namespace


#if defined(TYPUS_DISABLE_INVARIANT_CHECKS) && TYPUS_DISABLE_INVARIANT_CHECKS != 0
#   define TYPUS_INVARIANT(type, cond)
#   define TYPUS_REQUIRES(type, cond)
#   define TYPUS_GUARANTEES(type, cond)
#else
#   define TYPUS_INVARIANT(type, cond) \
        if (!(cond)) { typus::fail(__FILE__, __LINE__, type " failed: " #cond); }
#   define TYPUS_REQUIRES(cond) TYPUS_INVARIANT("precondition", cond)
#   define TYPUS_GUARANTEES(cond) TYPUS_INVARIANT("postcondition", cond)
#endif


    
#endif // TYPUS_ASSERT_HH
