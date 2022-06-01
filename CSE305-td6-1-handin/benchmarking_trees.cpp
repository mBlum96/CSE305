#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "CoarseBST.cpp"
#include "td6.cpp"

template <typename T>
int benchmark_single_thread(T& set, int count) {
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i) {
        set.add(rand());
    }    
    auto finish = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
    return elapsed;
}

template<typename T>
void add_caller(T& set, int count){
    for(int i=0; i<count; i++){
        set.add(rand());
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
        std::cout << "Usage: ./tree_benchmarker num_thread num_insertions" << std::endl;
        return 0;
    }

    int num_threads = std::stoi(argv[1]);
    int num_insertions = std::stoi(argv[2]);

    if (num_threads == 1) {
        // Timing for coarse-grained
        CoarseBST CL;
	int elapsed = benchmark_single_thread(CL, num_insertions);
        std::cout << "Time for coare-grained version is " << elapsed << " microseconds" << std::endl;
    
        // Timing for fine-grained
        FineBST L;
	elapsed = benchmark_single_thread(L, num_insertions);        
        std::cout << "Time for fine-grained version is " << elapsed << " microseconds" << std::endl;
    } else {
        CoarseBST CL;
        int elapsed = benchmark_multi_thread(CL, num_insertions, num_threads);
        std::cout << "Time for multi thread coare-grained version is " << elapsed << " microseconds" << std::endl;
        FineBST L;
        elapsed = benchmark_multi_thread(L, num_insertions, num_threads);
        std::cout << "Time for multi thread fine-grained version is " << elapsed << " microseconds" << std::endl;

    }
}


/*

SPACE TO REPORT AND ANALYZE THE RUNTIMES

./tree_benchmarker 40 99


Time for multi thread coare-grained version is 788 microseconds
Time for multi thread fine-grained version is 454 microseconds


./set_benchmarker 40 9999
Time for multi thread coare-grained version is 6197 microseconds
Time for multi thread fine-grained version is 21676 microseconds


We see that when we increase the number of isertions the coarse grain
version performs better, that is because there is less wait for a lock.
This makes sense because when we do the fine grained way, we switch locks
a lot more often than in the coarse grained one.
If we think about it, it is similar to comparing exp(x) and x functions, the exp(x)
will have smaller values in the beggining but soon enough will have values
a lot larger than the linear one.

 */