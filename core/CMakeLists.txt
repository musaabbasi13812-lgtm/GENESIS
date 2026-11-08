
add_library(genesis_core STATIC genesis.cpp)

target_compile_features(genesis_core PUBLIC cxx_std_20)

set_target_properties(genesis_core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

add_executable(genesis_test test_main.cpp)

target_link_libraries(genesis_test PRIVATE genesis_core)

if(MSVC)
    target_compile_options(genesis_core PRIVATE /W4)
    target_compile_options(genesis_test PRIVATE /W4)
else()
    target_compile_options(genesis_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(genesis_test PRIVATE -Wall -Wextra -Wpedantic)
endif()
