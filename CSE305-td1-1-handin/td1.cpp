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

void SumMapThread(NumIter begin, NumIter end, Num f(Num), long double& result) {
    // YOUR CODE HERE
    Num local_result = 0;
    while (begin!=end){
        local_result+= f(*begin);
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
Num SumParallel(NumIter begin, NumIter end, Num f(Num), size_t num_threads) {
    // YOUR CODE HERE
    unsigned long const length = std::distance(begin,end);//length of the vector
    if(!length) return 0.;

    unsigned long const block_size = length/num_threads;

    std::vector<Num> results(num_threads, 0.);
    std::vector<std::thread> workers(num_threads - 1);
    NumIter start_block = begin;
    for(size_t i=0; i<num_threads-1; ++i){
        NumIter end_block = start_block + block_size; 
        workers[i] = std::thread(&SumMapThread, start_block, end_block,f,
        std::ref(results[i]));
        start_block = end_block;
    }
    SumMapThread(start_block, end,f, results[num_threads - 1]);
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

Num retItself(Num num){
    return num;
}

/**
 * @brief Computes the mean of the numbers in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param num_threads The number of threads to use
 * @return The mean in the range
*/
Num MeanParallel(NumIter begin, NumIter end, size_t num_threads) {
    // YOUR CODE HERE
    unsigned long const length = std::distance(begin,end);//length of the vector
    Num sum = SumParallel(begin,end,retItself,num_threads);
    return sum/length;
    return 0;
}

//-----------------------------------------------------------------------------

Num retSquared(Num num){
    return num*num;
}

/**
 * @brief Computes the variance of the numbers in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param num_threads The number of threads to use
 * @return The variance in the range
*/
Num VarianceParallel(NumIter begin, NumIter end, size_t num_threads) {
    // YOUR CODE HERE
    unsigned long const length = std::distance(begin,end);//length of the vector
    Num mean = MeanParallel(begin, end, num_threads);
    Num meanOnSquared = SumParallel(begin,end, retSquared, num_threads)/length;
    return (meanOnSquared - (mean*mean));
    return 0;
}

//-----------------------------------------------------------------------------

void FindMinInBlock(std::vector<int>::const_iterator begin,
std::vector<int>::const_iterator end, std::pair<int,int>& result){
    std::pair<int,int> local_result = {*begin,0};
    auto it = begin;
    while (it!=end){
        if(local_result.first>*it){
            local_result.first = *it;
            local_result.second = 1;
        }
        else if(local_result.first==*it){
            local_result.second += 1;
        }
        it++;
    }
    result = local_result;
    return;
}

/**
 * @brief Computs the occurences of the minimal value in [begin, end)
 * @param begin Start iterator
 * @param end End iterator
 * @param num_threads The number of threads to use
 * @return the number of occurences of the minimal value in [begin, end)
*/
int CountMinsParallel(std::vector<int>::const_iterator begin,
std::vector<int>::const_iterator end, size_t num_threads) {
    // YOUR CODE HERE
    unsigned long const length = std::distance(begin,end);//length of the vector
    if(!length) return 0.;

    unsigned long const block_size = length/num_threads;

    std::vector<std::pair<int,int>> results(num_threads);
    std::vector<std::thread> workers(num_threads - 1);
    auto start_block = begin;
    for(size_t i=0; i<num_threads-1; ++i){
        auto end_block = start_block + block_size; 
        workers[i] = std::thread(&FindMinInBlock, start_block, end_block,
        std::ref(results[i]));
        start_block = end_block;
    }
    FindMinInBlock(start_block, end, results[num_threads - 1]);
    for (size_t i = 0; i < num_threads - 1; ++i) {
        workers[i].join();
    }

    int total_result = 0;
    int total_min = *begin;
    for (size_t i = 0; i < results.size(); ++i) {
        if(total_min>results[i].first){
            total_min = results[i].first;
            total_result=results[i].second;
        }
        else if(total_min==results[i].first){
            total_result+=results[i].second;
        }
    }
    return total_result;
    return 0;
}

//-----------------------------------------------------------------------------
template <typename Iter, typename T>
int FindTarget(Iter begin, Iter end, T target, bool &found){
    Iter it = begin;
    while(it!=end){
        if (found==true){
            return 0;
        }
        else{
            if(*it==target){
                found = true;
            }
        }
        it++;
    }
    return 0;
}

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
    bool found = false;
    unsigned long const length = std::distance(begin,end);//length of the vector
    if(!length) return false;

    unsigned long const block_size = length/num_threads;

    // std::vector<Num> results(num_threads, 0.);
    std::vector<std::thread> workers(num_threads - 1);
    Iter start_block = begin;
    for(size_t i=0; i<num_threads-1; ++i){
        Iter end_block = start_block + block_size; 
        workers[i] = std::thread(&FindTarget<Iter,T>, start_block, end_block,target,
        std::ref(found));
        start_block = end_block;
    }
    FindTarget(start_block, end,target,std::ref(found));
    for (size_t i = 0; i < num_threads - 1; ++i) {
        workers[i].join();
    }
    return found;
    return false;
}

//-----------------------------------------------------------------------------

template <typename ArgType, typename ReturnType>
int CheckRetType(ReturnType f(ArgType), ArgType arg,
std::shared_ptr<ReturnType> retType,
std::shared_ptr<int> monitoringVar){
    *retType = f(arg);
    *monitoringVar = 1;
    return 0;
}

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
    std::optional<ReturnType> defaultRet;
    std::shared_ptr<ReturnType> retType = std::make_shared<ReturnType>();
    std::shared_ptr<int> monitoringVar = std::make_shared<int>();
    *monitoringVar = 0;
    std::thread worker(&CheckRetType<ArgType, ReturnType>, f, arg,
    retType, monitoringVar);
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    worker.detach();
    if(*monitoringVar==1){
        return *retType;
    };
    return defaultRet;
    return {};
}

//-----------------------------------------------------------------------------
