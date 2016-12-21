#include <iostream>

#include <typus/format.hh>

int main(int argc, char ** argv)  {
#if 1
    auto format = $("executable name: {}\n");
    
    for (int i = 0; i < 100000; ++i) {
        format.into_stream(std::cout, argv[0]);
    }
#else
    for (int i = 0; i < 100000; ++i) {
        std::cout << "executable name: " << argv[0] << "\n";
    }
#endif
}
