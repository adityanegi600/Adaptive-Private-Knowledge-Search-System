#include "Agent [Arshveer Singh].h"
#include <exception>
#include <iostream>

int main() {
    try {
        rar::Agent agent;
        std::cout << "Private local-first prototype: no external API calls are made.\n";
        agent.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
