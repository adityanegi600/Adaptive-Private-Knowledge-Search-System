#include "Agent.h"
#include "Authentication.h"
#include <exception>
#include <iostream>

int main() {
    if (!rar::authenticate()) return 2;
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
