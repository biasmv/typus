#ifndef TYPUS_STR_CONST_HH
#define TYPUS_STR_CONST_HH

#include <string>

namespace typus {

class str_const {
public:
    template <size_t N>
    constexpr str_const(const char(&a)[N]): d_(a), size_(N-1) {
    }

    constexpr size_t size() const { return size_; }

    constexpr char operator[](size_t index) const {
        return d_[index];
    }
private:
    const char* const d_;
    const size_t size_;
};

}
#endif
