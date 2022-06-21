#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "CoarseSetList.cpp"
#include "SetList.cpp"

template <typename T>
int benchmark_single_thread(T& set, int count) {
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i) {
        set.add(std::to_string(rand()));
    }    
    auto finish = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
    return elapsed;
}

template<typename T>
void add_caller(T& set, int count){
    for(int i=0; i<count; i++){
        set.add(std::to_string(rand()));
    }
}

template <typename T>
int benchmark_multi_thread(T& set, int count, int num_threads){
    std::vector<std::thread> workers(num_threads);
    auto start = std::chrono::steady_clock::now();
    int count_for_each = count/num_threads;
    for(int i=0; i<num_threads; i++){
        workers[i] = std::thread(add_caller<T>, std::ref(set), count_for_each);
    }
    for (int i=0; i<num_threads;i++){
        workers[i].join();
    }



    auto finish = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
    return elapsed;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: ./set_benchmarker num_thread num_insertions" << std::endl;
        return 0;
    }

    int num_threads = std::stoi(argv[1]);
    int num_insertions = std::stoi(argv[2]);

    if (num_threads == 1) {
        // Timing for coarse-grained
        CoarseSetList CL;
	int elapsed = benchmark_single_thread(CL, num_insertions);
        std::cout << "Time for coare-grained version is " << elapsed << " microseconds" << std::endl;
    
        // Timing for fine-grained
        SetList L;
	elapsed = benchmark_single_thread(L, num_insertions);        
        std::cout << "Time for fine-grained version is " << elapsed << " microseconds" << std::endl;
    } else {
        CoarseSetList CL;
        int elapsed = benchmark_multi_thread(CL, num_insertions, num_threads);
        std::cout << "Time for multi thread coare-grained version is " << elapsed << " microseconds" << std::endl;
        SetList L;
        elapsed = benchmark_multi_thread(L, num_insertions, num_threads);
        std::cout << "Time for multi thread fine-grained version is " << elapsed << " microseconds" << std::endl;

    }
}

/*

SPACE TO REPORT AND ANALYZE THE RUNTIMES

./set_benchmarker 34 1000


Time for multi thread coare-grained version is 3586 microseconds
Time for multi thread fine-grained version is 3613 microseconds

./set_benchmarker 34 9999
Time for multi thread coare-grained version is 408371 microseconds
Time for multi thread fine-grained version is 151543 microseconds

We can see that for coarse grained compared to fine grained version we get better results when the number
of insertions is relativly low
We can see that for fine grained cpm[ared to coarse grained version we get better results when the number
of insertions is relativly high

This co-ensides with what we saw in the lecture

 */
