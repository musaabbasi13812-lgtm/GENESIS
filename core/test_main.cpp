
#include <iostream>

extern "C" {
    const char* genesis_version();
    const char* genesis_run();
}

int main() {
    std::cout << "=== GENESIS ENGINE TEST ===\n\n";

    std::cout << "Version:\n";
    std::cout << genesis_version() << "\n\n";

    std::cout << "Scientific Benchmark Result:\n";
    std::cout << genesis_run() << "\n";

    return 0;
}
