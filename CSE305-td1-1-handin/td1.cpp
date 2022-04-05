#pragma once
#include <climits>
#include <thread>
#include <numeric>
#include <iterator>
#include <optional>
#include <vector>

typedef long double Num;
typedef std::vector<long double>::const_iterator NumIter;

//-----------------------------------------------------------------------------

void SumMapThread(NumIter begin, NumIter end,/* Num f(Num),*/ long double& result) {
    // YOUR CODE HERE
    Num local_result = 0;
    while (begin!=end){
        local_result+= *begin;
        begin++;
    }
    result = local_result;
}

/**
 * @brief Sums f(x) for x in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param f Function to apply
 * @param result Reference to the result variable
 */
Num SumParallel(NumIter begin, NumIter end,/* Num f(Num),*/ size_t num_threads) {
    // YOUR CODE HERE
    unsigned long const length = std::distance(begin,end);//length of the vector
    if(!len) return 0.;

    unsigned long const block_size = length/num_threads;

    std::vector<Num> results(num_threads, 0.);
    std::vector<std::thread> workers(num_threads - 1);
    NumIter start_block = begin;
    for(size_t i=0; i<num_threads-1; ++i){
        NumIter end_block = start_block + block_size; 
        workers[i] = std::thread(&SumMapThread, start_block, end_block, std::ref(results[i]));
        start_block = end_block;
    }
    SumMapThread(start_block, end, results[num_threads - 1]);
    for (size_t i = 0; i < num_threads - 1; ++i) {
        workers[i].join();
    }

    Num total_result = 0.;
    for (size_t i = 0; i < results.size(); ++i) {
        total_result += results[i];
    }
    return total_result;
    return 0;
}

//-----------------------------------------------------------------------------

/**
 * @brief Computes the mean of the numbers in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param num_threads The number of threads to use
 * @return The mean in the range
*/
Num MeanParallel(NumIter begin, NumIter end, size_t num_threads) {
    // YOUR CODE HERE
    return 0;
}

//-----------------------------------------------------------------------------

/**
 * @brief Computes the variance of the numbers in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param num_threads The number of threads to use
 * @return The variance in the range
*/
Num VarianceParallel(NumIter begin, NumIter end, size_t num_threads) {
    // YOUR CODE HERE
    return 0;
}

//-----------------------------------------------------------------------------



/**
 * @brief Computs the occurences of the minimal value in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param num_threads The number of threads to use
 * @return the number of occurences of the minimal value in [begin, end)
*/
int CountMinsParallel(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end, size_t num_threads) {
    // YOUR CODE HERE
    return 0;
}

//-----------------------------------------------------------------------------


/**
 * @brief Finds target in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param target The target to search for
 * @param num_threads The number of threads to use
 * @return The sum in the range
*/
template <typename Iter, typename T>
bool FindParallel(Iter begin, Iter end, T target, size_t num_threads) {
    // YOUR CODE HERE (AND MAYBE AN AUXILIARY FUNCTION OUTSIDE)
    return false;
}

//-----------------------------------------------------------------------------


/**
 * @brief Runs a function and checks whether it finishes within a timeout
 * @param f Function to run
 * @param arg Arguments to pass to the function 
 * @param timeout The timeout
 * @return std::optional with result (if the function finishes) and empty (if timeout)
 */
template <typename ArgType, typename ReturnType>
std::optional<ReturnType> RunWithTimeout(ReturnType f(ArgType), ArgType arg, size_t timeout) {
    // YOUR CODE HERE (AND MAYBE AN AUXILIARY FUNCTION OUTSIDE)
    return {};
}

//-----------------------------------------------------------------------------
