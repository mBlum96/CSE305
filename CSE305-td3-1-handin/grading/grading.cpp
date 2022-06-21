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
#include "td3.cpp"
#include <limits>

namespace tdgrading {

using namespace testlib;
using namespace std;


//-----------------------------------------------------------------------------

// Auxiliary functions for testing the Account class
void withdraw(Account& a) {
    for (size_t i = 0; i < 1000000; ++i) {
        a.withdraw(1);
    }
}

void add(Account& a) {
    for (size_t i = 0; i < 1000000; ++i) {
        a.add(1);
    }
}

void transfer(Account& a, Account& b) {
    for (size_t i = 0; i < 1000; ++i) {
        Account::transfer(1, a, b);
    }
}

void generate_accounts1(std::vector<unsigned int>& result) {
    for (size_t i = 0; i < 100000; ++i) {
        Account a;
        result.push_back(a.get_id());
    }
}

void generate_accounts2(std::vector<unsigned int>& result) {
    for (size_t i = 0; i < 100000; ++i) {
        Account a(0);
        result.push_back(a.get_id());
    }
}

int check_deadlock() {
    Account B(1000);
    Account C(1000);
    std::thread t1(&transfer, std::ref(B), std::ref(C));
    std::thread t2(&transfer, std::ref(C), std::ref(B));
    t1.join();
    t2.join();
    return 1;
}

int test_account(std::ostream &out, const std::string test_name) {
    std::string fun_name = "Account";

    start_test_suite(out, test_name);
    std::vector<int> res;
  
    Account A(2000000);
    std::thread t1(&withdraw, std::ref(A));
    std::thread t2(&withdraw, std::ref(A));
    t1.join();
    t2.join();
    if (A.get_amount() != 0 ) {
        print(out, "Parallel withdrawals from an account interleave, consider using lock");
        res.push_back(0);
    } else {
        res.push_back(1);
    }
 
    std::thread t3(&add, std::ref(A));
    std::thread t4(&add, std::ref(A));
    t3.join();
    t4.join();
    if (A.get_amount() != 2000000 ) {
        print(out, "Parallel additions from an account interleave, consider using lock");
        res.push_back(0);
    } else {
        res.push_back(1);
    }
    
    std::future<int> f = std::async(std::launch::async, &check_deadlock);
    std::future_status status = f.wait_for(std::chrono::seconds(5));
    if (status != std::future_status::ready) {
        print(out, "Dealocks in parallel transfer");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    std::vector<unsigned int> ids1;
    std::vector<unsigned int> ids2;
    std::thread t5(&generate_accounts1, std::ref(ids1));
    std::thread t6(&generate_accounts2, std::ref(ids2));
    t5.join();
    t6.join();
    ids1.insert(ids1.end(), ids2.begin(), ids2.end());
    std::sort(ids1.begin(), ids1.end());
    for (auto it = ids1.begin(); it != ids1.end() - 1; ++it) {
        if (*it == *(it + 1)) {
            print(out, "Parallel creation of accounts yields equal ids\n");
            res.push_back(0);
        } else {
            res.push_back(1);
        }
    }

    return end_test_suite(out, test_name,
                          accumulate(res.begin(), res.end(), 0), res.size());
}

//-----------------------------------------------------------------------------

void aux_massiv_withdraw(std::vector<Account*> accounts, int count) {
    for (size_t i = 0; i < count; ++i) {
        Account::massiv_withdraw(accounts, 1);
    }
}

int test_massiv_withdraw(std::ostream &out, const std::string test_name) {
    std::string fun_name = "massiv_withdraw";

    start_test_suite(out, test_name);
    std::vector<int> res;
  
    Account A(1000);
    Account B(800);
    Account C(799);
    std::vector<Account*> abc = {&A, &B, &C};
    std::vector<Account*> acb = {&A, &C, &B};
    std::vector<Account*> bac = {&B, &A, &C};
    std::vector<Account*> bca = {&B, &C, &A};
    std::vector<Account*> cab = {&C, &A, &B};
    std::vector<Account*> cba = {&C, &B, &A};

    std::thread t1(&aux_massiv_withdraw, abc, 100);
    std::thread t2(&aux_massiv_withdraw, acb, 100);
    std::thread t3(&aux_massiv_withdraw, bac, 100);
    std::thread t4(&aux_massiv_withdraw, bca, 100);
    std::thread t5(&aux_massiv_withdraw, cab, 100);
    std::thread t6(&aux_massiv_withdraw, cba, 100);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    res.push_back(test_eq(out, "account A", A.get_amount(), 400));
    res.push_back(test_eq(out, "account B", B.get_amount(), 200));
    res.push_back(test_eq(out, "account C", C.get_amount(), 199));

    Account::massiv_withdraw(abc, 199);
    bool overdraft = Account::massiv_withdraw(cab, 1);
    res.push_back(test_eq(out, "No removing", overdraft, false));
    res.push_back(test_eq(out, "account A", A.get_amount(), 201));
    res.push_back(test_eq(out, "account B", B.get_amount(), 1));
    res.push_back(test_eq(out, "account C", C.get_amount(), 0));

    return end_test_suite(out, test_name,
                          accumulate(res.begin(), res.end(), 0), res.size());
}



//-----------------------------------------------------------------------------

// Auxiliary functions

int check_dealock_set() {
    ChunkedThreadArray<10, 10> a;
    a.set(0, 0);
    try {
        a.set(0, 0, [](int x) -> bool {throw 1;});
    } catch (int i) {
    }
    //should not go into deadlock
    a.set(0, 0);

    return 1;
}

bool fs(int a, int b) {
    return true;
}

void swap_a_lot(ChunkedThreadArray<10, 10>& arr, size_t i, size_t j) {
    for (size_t k = 0; k < 100000; ++k) {
        arr.swap(i, j, &fs);
    }
}

int check_dealock_swap() {
    ChunkedThreadArray<10, 10> a;
    std::thread t1(&swap_a_lot, std::ref(a), 0, 20);
    std::thread t2(&swap_a_lot, std::ref(a), 0, 20);
    t1.join();
    t2.join();

    return 1;
}

void inc_a_lot(ChunkedThreadArray<10, 10>& arr, size_t i) {
    for (size_t j = 0; j < 100000; ++j) {
        bool done = false;
        while (!done) {
            int current = arr.get(i);
            done = arr.set(i, current + 1, [current](int a) -> bool {return a == current;});
        }
    }
}

void swap_a_lot_race(ChunkedThreadArray<2, 1>& arr) {
    for (size_t j = 0; j < 1000000; ++j) {
        arr.swap(0, 1, [](int a, int b) -> bool {return true;});
    }
}

int test_chunked_thread_array(std::ostream &out, const std::string test_name) {
    std::string fun_name = "ChunkedThreadArray";

    start_test_suite(out, test_name);
    std::vector<int> res;

    std::future<int> f = std::async(std::launch::async, &check_dealock_set);
    std::future_status status = f.wait_for(std::chrono::milliseconds(100));
    if (status != std::future_status::ready) {
        print(out, "Dealocks in set after throwing exception");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    std::future<int> f_swap = std::async(std::launch::async, &check_dealock_swap);
    std::future_status status_swap = f_swap.wait_for(std::chrono::milliseconds(2000));
    if (status_swap != std::future_status::ready) {
        print(out, "Dealocks in swap");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    ChunkedThreadArray<10, 10> a;
    a.set(0, 0);
    std::thread t1(&inc_a_lot, std::ref(a), 0);
    std::thread t2(&inc_a_lot, std::ref(a), 0);
    t1.join();
    t2.join();
    if (a.get(0) != 200000) {
        print(out, a.get(0));
        print(out, "Interleaving in concurrent incrementing via set");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    ChunkedThreadArray<2, 1> b;
    b.set(0, 1);
    b.set(1, 2);
    std::thread t3(&swap_a_lot_race, std::ref(b));
    std::thread t4(&swap_a_lot_race, std::ref(b));
    t3.join();
    t4.join();
    if (b.get(0) == b.get(1)) {
        print(out, "Race condition in swap");
        res.push_back(0);
    } else {
        res.push_back(1);
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
  "names" : ["td3.cpp::Account", "td3.cpp::massiv_withdraw", "td3.cpp::ChunkedThreadArray"],
  "points" : [6, 7, 7]
}
[END-AUTOGRADER-ANNOTATION]
*/

    int const total_test_cases = 3;
    std::string const test_names[total_test_cases] = {"Account", "massiv_withdraw", "ChunkedThreadArray"};
    int const points[total_test_cases] = {6, 7, 7};
    int (*test_functions[total_test_cases]) (std::ostream &, const std::string) = {
      test_account, test_massiv_withdraw, test_chunked_thread_array
    };

    return run_grading(out, test_case_number, total_test_cases,
                       test_names, points,
                       test_functions);
}

} // End of namepsace tdgrading
