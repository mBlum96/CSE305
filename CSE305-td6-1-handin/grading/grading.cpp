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
#include "td6.cpp"
#include "CoarseBST.cpp"
#include <limits>

namespace tdgrading {

using namespace testlib;
using namespace std;

//-----------------------------------------------------------------------------

template <typename Tree>
void inserter(Tree& BST, int offset, int upper, int step, bool& failure) {
    for (int i = offset; i < upper; i += step) {
        if (!BST.add(i)) {
            failure = true;
        }
    }
}

template <typename Tree>
int test_tree(std::ostream &out, const std::string test_name) {
    std::string fun_name = "FineBST";

    start_test_suite(out, test_name);
    std::vector<int> res;

    Tree BST;
    int num_threads = 5;
    int upper = 10000;
    bool failed_addition = false;
    std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; ++i) {
        workers.push_back(std::thread(inserter<Tree>, std::ref(BST), i, upper, num_threads, std::ref(failed_addition)));
    }
    for (int i = 0; i < num_threads; ++i) {
        workers[i].join();
    }

    if (failed_addition) {
        print(out, "Addition failed unexpectedly");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    for (int i = 0; i < upper; ++i) {
        if (!BST.contains(i)) {
            print(out, "Insertion did not happen or search is working incorrectly\n");
            res.push_back(0);
        } else {
            res.push_back(1);
        }
    }

    return end_test_suite(out, test_name,
                          accumulate(res.begin(), res.end(), 0), res.size());
}

//-----------------------------------------------------------------------------

template <typename Tree>
void remover(Tree& BST, int offset, int upper, int step, bool& failure) {
    for (int i = offset; i < upper; i += step) {
        if (!BST.remove(i)) {
            std::cout<<"the offender is "<< i<<std::endl;
            failure = true;
        }
    }
}

template <typename Tree>
int test_remove(std::ostream &out, const std::string test_name) {
    std::string fun_name = "FineBST::remove";

    start_test_suite(out, test_name);
    std::vector<int> res;

    Tree BST;
    int num_threads = 5;
    int upper = 10000;

    for (int i = 0; i < upper; ++i) {
         BST.add(i);
    }

    bool failed_removal = false;
    std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; ++i) {
        workers.push_back(std::thread(remover<Tree>, std::ref(BST), i, upper, num_threads, std::ref(failed_removal)));
    }
    for (int i = 0; i < num_threads; ++i) {
        workers[i].join();
    }

    if (failed_removal) {
        print(out, "Removal failed unexpectedly\n");
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    for (int i = 0; i < upper; ++i) {
        if (BST.contains(i)) {
            print(out, "Removal did not happen or search is working incorrectly\n");
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
  "total" : 2,
  "names" : ["td6.cpp::FineBST", "td6.cpp::remove"],
  "points" : [8, 0]
}
[END-AUTOGRADER-ANNOTATION]
*/

    int const total_test_cases = 2;
    std::string const test_names[total_test_cases] = {"FineBST", "remove"};
    int const points[total_test_cases] = {8, 0};
    int (*test_functions[total_test_cases]) (std::ostream &, const std::string) = {
        test_tree<FineBST>, test_remove<FineBST>
    };

    return run_grading(out, test_case_number, total_test_cases,
                       test_names, points,
                       test_functions);
}

} // End of namepsace tdgrading
