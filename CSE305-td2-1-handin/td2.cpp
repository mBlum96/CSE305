#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#pragma once
#include <climits>
#include <thread>
#include <numeric>
#include <iterator>
#include <optional>
#include <iostream>
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
int FindTarget(Iter begin, Iter end, T target, bool &found, std::atomic<int> &helper_cnt,int count){
    Iter it = begin;
    int count_val = helper_cnt.load();
    while(count_val!=helper_cnt.load()){
            count_val = helper_cnt.load();
    }
    while(it!=end && count!=count_val){
        while(count_val!=helper_cnt.load()){
            count_val = helper_cnt.load();
        }
        if(*it==target){
                helper_cnt.fetch_add(1);
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
bool FindParallel(Iter begin, Iter end, T target, int count, size_t num_threads) {
    // YOUR CODE HERE (AND MAYBE AN AUXILIARY FUNCTION OUTSIDE)
    bool found = false;
    unsigned long const length = std::distance(begin,end);//length of the vector
    if(!length) return false;
    // std::cout<<"the count is "<<count << std::endl;

    unsigned long const block_size = length/num_threads;

    std::atomic<int> helper_cnt;
    helper_cnt.store(0);
    
    // std::vector<Num> results(num_threads, 0.);
    std::vector<std::thread> workers(num_threads - 1);
    Iter start_block = begin;
    for(size_t i=0; i<num_threads-1; ++i){
        Iter end_block = start_block + block_size; 
        workers[i] = std::thread(&FindTarget<Iter,T>, start_block, end_block,target,
        std::ref(found), std::ref(helper_cnt), count);
        start_block = end_block;
    }
    FindTarget(start_block, end,target,std::ref(found),std::ref(helper_cnt),count);

    int count_val = helper_cnt.load();
    for (size_t i = 0; i < num_threads - 1; ++i) {
        workers[i].join();
    }
    while(count_val!=helper_cnt.load()){
            count_val = helper_cnt.load();
    }
    if(count_val==count)return 1;
    return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

class MyLock {
        std::atomic<bool> locked;
        std::thread::id owner;
    public:
        MyLock() {
            std::atomic_init(&locked, false);
            owner = std::thread::id();
        }

        // specification as https://en.cppreference.com/w/cpp/thread/mutex/lock
        // but no need to worry about exceptions
        void lock() {
            bool current_lock_sit = this->locked.load();
            while(current_lock_sit!=this->locked.load()){
                current_lock_sit=this->locked.load();
            }
            while(this->locked.compare_exchange_weak(current_lock_sit,true)){}
            this->locked.store(true);
        }

        // specification as https://en.cppreference.com/w/cpp/thread/mutex/try_lock
        bool try_lock() {
            bool current_lock_sit = this->locked.load();
            while(current_lock_sit!=this->locked.load()){
                current_lock_sit=this->locked.load();
            }
            while(this->locked.compare_exchange_weak(current_lock_sit,true)){
                return false;
            }
            this->lock();
            return true;
        }

        // specification as https://en.cppreference.com/w/cpp/thread/mutex/unlock
        // but if called by a nonowner thread, nothing should happen
        void unlock() {
            if(std::thread::id()==owner){
                locked.store(false);
            }
        }
};

//-----------------------------------------------------------------------------

class OrderedVec {
        std::vector<int> data;
        std::mutex insertLock;
        std::mutex queueLock;
        std::mutex criticalLock;
        int nonInsertActions;
        // YOU CAN ADD NEW ATTRIBUTES HERE
    public:
        OrderedVec() {
            nonInsertActions = 0;
            insertLock.unlock();
            queueLock.unlock();
            criticalLock.unlock();
        }

        void enterReader(){
            queueLock.lock();
            criticalLock.lock();
            nonInsertActions ++;
            if(nonInsertActions == 1){
                insertLock.lock();
            }
            criticalLock.unlock();
            queueLock.unlock();
        }
        void enterWriter(){
            queueLock.lock();
            insertLock.lock();
            queueLock.unlock();
        }
        void leaveReader(){
            criticalLock.lock();
            nonInsertActions--;
            if(nonInsertActions == 0){
                insertLock.unlock();
            }
            criticalLock.unlock();
        }
        void leaveWriter(){
            insertLock.unlock();
        }

        // insertion in O(n)
        void insert(int val) {
            enterWriter();
            auto it = std::upper_bound(data.cbegin(), data.cend(), val);
            data.insert(it, val);
            leaveWriter();
        }

        // search in O(log n)
        bool search(int val) {
            enterReader();
            bool search_res = std::binary_search(data.begin(), data.end(), val);
            leaveReader();
            return search_res;
        }

        // prints to std::cout, whitespace separated
        void print() {
            enterReader();
            for (char i:data){
                std::cout << i << ' ';
            }
            leaveReader();
        }

        std::vector<int> get_data() {
            return data;
        }
};
