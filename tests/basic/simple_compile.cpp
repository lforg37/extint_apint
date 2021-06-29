#include "apintext.hpp"
#include <cstdint>

int main() {
    apintext::Value<32, false>(uint32_t{18});
    return 0;
}
