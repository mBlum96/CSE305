#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "CoarseBST.cpp"
#include "td6.cpp"


int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: ./tree_benchmarker num_thread num_insertions" << std::endl;
        return 0;
    }

    int num_threads = std::stoi(argv[1]);
    int num_insertions = std::stoi(argv[2]);

}

/*

SPACE TO REPORT AND ANALYZE THE RUNTIMES

 */
