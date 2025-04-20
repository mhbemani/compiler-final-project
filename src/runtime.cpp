#include <stdexcept>

extern "C" void throwTypeError() {
    throw std::runtime_error("Type error");
}