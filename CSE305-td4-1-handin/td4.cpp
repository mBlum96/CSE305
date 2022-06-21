#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <cmath>
#include <mutex>
#include <numeric>
#include <thread>
#include <queue>

void DivideOnceEven(std::condition_variable& iseven, std::mutex& m, int& n) {
    // std::lock_guard<std::mutex> lk(m);
    std::unique_lock<std::mutex> lk(m);
    iseven.wait(lk);
    n /= 2;
    iseven.notify_all();
    return;
}

//-----------------------------------------------------------------------------

template <class E> 
class SafeUnboundedQueue {
        std::queue<E> elements;
        std::mutex lock;
        std::condition_variable not_empty;
    public: 
        SafeUnboundedQueue<E>(){}
        void push(const E& element);
        E pop ();
        bool is_empty() const {return this->elements.empty();}
};

template <class E>
void SafeUnboundedQueue<E>::push(const E& element) {
    std::unique_lock<std::mutex> lk(lock);
    if(this->is_empty()){
        this->not_empty.notify_all();
    }
    elements.push(element);
    return;
}

template <class E> 
E SafeUnboundedQueue<E>::pop() {
    std::unique_lock<std::mutex> lk(lock);
    while(this->is_empty()){
        this->not_empty.wait(lk);
    }
    auto popped_element = elements.front();
    elements.pop();
    return popped_element;
}

//-----------------------------------------------------------------------------

class SimplePool {
        unsigned int num_workers;
        std::vector<std::thread> workers;
        SafeUnboundedQueue<std::function<bool()> > tasks;
        // the suggestion is to use the returned bool value to distinguish
        // between usuall tasks and stopper tasks
        //
        // std::optional<std::function<void()> >
        // struct task {std::function<void()>, bool}
        // ...
        // stopper
        // push([]() -> bool {return false;})
        // usual task
        // push([]() -> bool {do some work with f and args; return true;})
        

        // void do_work();
        // create workers: workers[i] = std::thread(&do_work)
        // std::optional<std::function<bool()>> task;

        void do_work();
    public:
        SimplePool(unsigned int num_workers = 0);
        ~SimplePool();
        template <class F, class... Args>
        void push(F f, Args ... args);
        void stop();
};

void SimplePool::do_work() {
    std::function<bool()> task;
    bool condition = true;
    while(condition){
        /*std::function<bool()>*/task = this->tasks.pop();
        // std::cout<<"hey ho"<<std::endl;
        condition = task();
    }
}

// void SimplePool::do_work() {
//     bool res;
//     do {
//         std::function<bool()> task = tasks.pop();
//         res = task();
//     }
//     while (res);
// }

SimplePool::SimplePool(unsigned int num_workers) {
    
    this->num_workers = num_workers;

    for(unsigned int i=0;i<num_workers;i++){
        workers.push_back(std::thread(&SimplePool::do_work, this));
    }

}

SimplePool::~SimplePool() {
    this->stop();
    for (unsigned int i = 0; i < num_workers; i++){
        if (workers[i].joinable()){
            workers[i].join();
        }
    }
}

template <class F, class... Args>
void SimplePool::push(F f, Args ... args) {
    std::function<bool()> task = [f, args...]() -> bool {f (args ...); return true;};
    tasks.push(task);
}

void SimplePool::stop() {
    for (unsigned int i = 0; i<num_workers; i++){
        tasks.push([]() -> bool {return false;});
    }
}

//-----------------------------------------------------------------------------
void determine_prime(unsigned long range, std::vector<char> &prime_vec){
    for(int i=2;i<range;i++){
        for( int j=0; j<range; j++){
            if(prime_vec[i]==0){
                prime_vec[i*j]=0;
            }
            if(prime_vec[i]%prime_vec[j]==0){
                prime_vec[i]=0;
            }
        }
    }
}

std::vector<unsigned long> Eratosthene(unsigned long N, size_t num_workers) {
    std::vector<char> is_prime(N + 1, 1);
    is_prime[0]=1;
    is_prime[1]=1;
    std::vector<unsigned long> primes;
    // determine_prime(sqrt(N),ref(is_prime));
    std::vector<std::thread> workers(workers - 1);
    auto start_block = begin;
    for(size_t i=0; i<num_threads-1; ++i){
        auto end_block = start_block + block_size; 
        workers[i] = std::thread(&determine_prime(sqrt(N),ref(is_prime)), start_block, end_block,
        std::ref(results[i]));
        start_block = end_block;
    }
    FindMinInBlock(start_block, end, results[num_threads - 1]);
    for (size_t i = 0; i < num_threads - 1; ++i) {
        workers[i].join();
    }
    return primes;
}
