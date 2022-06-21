#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdarg>
#include <iterator>
#include <string>
#include <regex>
#include <numeric>
#include <cmath>


#include "../gradinglib/gradinglib.hpp"
#include "td2.cpp"
#include <limits>

namespace tdgrading {

using namespace testlib;
using namespace std;

int test_find_parallel(std::ostream &out, const std::string test_name) {
    std::string fun_name = "FindParallel";

    start_test_suite(out, test_name);

    std::vector<int> res;
   
    for (size_t i = 0; i < 150; ++i) {
        size_t len = (rand() % 30000) + 10;
        if (i < 2) {
            len = i + 1;
        }
        std::vector<int> test;
        for (size_t j = 0; j < len; ++j) {
            test.push_back(rand() % (len / 10 + 1));
        }
        int count = std::count(test.begin(), test.end(), test[0]);
        bool correct = (count >= 10);
        bool student_result = FindParallel(test.begin(), test.end(), test[0], 10, (rand() % 5) + 1);
        res.push_back(test_eq(
           out, fun_name, student_result, correct
        ));
    }

    std::vector<int> long_vec;
    for (size_t i = 0; i < 10000000; ++i) {
        long_vec.push_back(rand() % 100);
    }
    for (size_t i = 0; i < 5; ++i) {
        long_vec[i] = 101;
        long_vec[5000000 + i] = 101;
    }
    auto start = std::chrono::steady_clock::now();
    FindParallel(long_vec.begin(), long_vec.end(), 101, 10, 2);
    auto end = std::chrono::steady_clock::now();
    auto rt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if (rt > 10) {
        print(out, "It seems that you do not terminate after finding the necessary number of occurences, too slow");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    long_vec = std::vector<int>();
    for (size_t i = 0; i < 10000000; ++i) {
        long_vec.push_back(rand() % 100);
    }
    for (size_t i = 0; i < 5; ++i) {
        long_vec[i] = 101;
        long_vec[3333333 + i] = 101;
        long_vec[6666666 + i] = 101;
    }
    start = std::chrono::steady_clock::now();
    FindParallel(long_vec.begin(), long_vec.end(), 101, 10, 3);
    end = std::chrono::steady_clock::now();
    rt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if (rt > 10) {
        print(out, "It seems that you do not terminate after finding the necessary number of occurences, too slow");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    return end_test_suite(out, test_name,
                          accumulate(res.begin(), res.end(), 0), res.size());
}

//-----------------------------------------------------------------------------

template <typename T>
void SlowInc(int& target, T& l) {
    l.lock();
    int tmp = target + 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    target = tmp;
    l.unlock();
}


void TryUnlock(MyLock& l, bool& result) {
    l.unlock();
    result = l.try_lock();
}

void TryLock(MyLock& l, bool& result) {
    for (size_t i = 0; i < 1000; ++i) {
        if (l.try_lock()) {
            result = true;
            return;
        }
    }
}

int test_my_lock(std::ostream &out, const std::string test_name) {
    std::string fun_name = "MyLock";

    start_test_suite(out, test_name);

    std::vector<int> res;
   
    int x = 1;
    MyLock l;
    std::thread C(&SlowInc<MyLock>, std::ref(x), std::ref(l));
    std::thread D(&SlowInc<MyLock>, std::ref(x), std::ref(l));
    C.join();
    D.join();
    if (x != 3) {
        print(out, "Looks like the lock does not block");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    MyLock looo;
    looo.lock();
    bool unlocked = false;
    std::thread A(&TryUnlock, std::ref(looo), std::ref(unlocked));
    A.join();
    if (unlocked) {
        print(out, "A non-owner thread can release a lock, this should not be so");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    MyLock a;
    a.lock();
    bool got_lock = false;
    std::thread B(&TryLock, std::ref(a), std::ref(got_lock));
    B.join();
    if (got_lock) {
        print(out, "Lock can be got by two distinct threads");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    return end_test_suite(out, test_name,
                          accumulate(res.begin(), res.end(), 0), res.size());
}

//-----------------------------------------------------------------------------

int test_ordered_vec(std::ostream &out, const std::string test_name) {
    std::string fun_name = "OrderedVec";

    start_test_suite(out, test_name);

    std::vector<int> res;
   
    for (size_t i = 0; i < 1000; ++i) {
        OrderedVec v;
        for (size_t j = 0; j < 1000; ++j) {
            v.insert(rand() % 100);
        }
        std::vector<int> data = v.get_data();
        if (!std::is_sorted(data.begin(), data.end())) {
            print(out, "The data is not maintained in the sorted order");
            res.push_back(0);
        } else {
           res.push_back(1);
        }

        if (v.search(101)) {
            print(out, "Finds nonexisting element");
            res.push_back(0);
        } else {
           res.push_back(1);
        }

        if (!v.search(data[0])) {
            print(out, "Does not find an element which is present");
            res.push_back(0);
        } else {
           res.push_back(1);
        }
    }

    return end_test_suite(out, test_name,
                          accumulate(res.begin(), res.end(), 0), res.size());
}


//-----------------------------------------------------------------------------

int grading(std::ostream &out, const int test_case_number)
{
/**

Annotations used for the autograder.

[START-AUTOGRADER-ANNOTATION]
{
  "total" : 3,
  "names" : ["td2.cpp::FindParallel", "td2.cpp::MyLock", "td2.cpp::OrderedVec"],
  "points" : [6, 7, 7]
}
[END-AUTOGRADER-ANNOTATION]
*/

    int const total_test_cases = 3;
    std::string const test_names[total_test_cases] = {"FindParallel", "MyLock", "OrderedVec"};
    int const points[total_test_cases] = {6, 7, 7};
    int (*test_functions[total_test_cases]) (std::ostream &, const std::string) = {
      test_find_parallel, test_my_lock, test_ordered_vec
    };

    return run_grading(out, test_case_number, total_test_cases,
                       test_names, points,
                       test_functions);
}

} // End of namepsace tdgrading
