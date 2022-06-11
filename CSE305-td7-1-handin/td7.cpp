#include <atomic>
#include <climits>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

//--------- Slightly modified lazy version of SetList from the lecture --------

class Node {
public:
    std::mutex lock;
    std::string item;
    unsigned long key;
    std::shared_ptr<Node> next;
    bool marked;
    Node(const std::string& s, std::shared_ptr<Node> n = NULL) {
        item = s;
        key = std::hash<std::string>{}(s);
        next = n;
        marked = false;
    }
    Node(unsigned long k) {
        item = "";
        key = k;
        next = NULL; 
        marked = false;
    }
};

class SetList{
    static const unsigned long LOWEST_KEY;
    static const unsigned long LARGEST_KEY;
    std::shared_ptr<Node> head;

    static bool validate(const std::shared_ptr<const Node> pred, const std::shared_ptr<const Node> curr);
public:
    SetList() { // constructor
        head = std::make_shared<Node>(SetList::LOWEST_KEY);
        head->next = std::make_shared<Node>(SetList::LARGEST_KEY);
    }
    ~SetList() {}; // destructor
    bool add(const std::string& val);
    bool remove(const std::string& val);
    bool contains(const std::string& val) const;
};

const unsigned long SetList::LOWEST_KEY = 0;
const unsigned long SetList::LARGEST_KEY = ULONG_MAX;
 
bool SetList::validate(std::shared_ptr<const Node> pred, std::shared_ptr<const Node> curr) {
  return (!pred->marked) && (!curr->marked) && (pred->next == curr);
}

bool SetList::add(const std::string& val) {
    unsigned long key = std::hash<std::string>{}(val);
    while (true) {
        std::shared_ptr<Node> pred = this->head;
        std::shared_ptr<Node> curr = pred->next;
        while (curr->key < key) {
            pred = curr;
            curr = curr->next;
        }
        std::lock_guard<std::mutex> pred_lk(pred->lock);
        std::lock_guard<std::mutex> curr_lk(curr->lock);
        if (SetList::validate(pred, curr)) {
            if (key == curr->key) {
                return false;
            }
            pred->next = std::make_shared<Node>(val, curr);
            return true;
        }
    }
}
    
bool SetList::remove(const std::string& val) {
    unsigned long key = std::hash<std::string>{}(val);
    while (true) {
        std::shared_ptr<Node> pred = this->head;
        std::shared_ptr<Node> curr = pred->next;
        while (curr->key < key) {
            pred = curr;
            curr = curr->next;
        }
        std::lock_guard<std::mutex> pred_lk(pred->lock);
        std::lock_guard<std::mutex> curr_lk(curr->lock);
        if (SetList::validate(pred, curr)) {
            if (key == curr->key) {
                curr->marked = true;
                
            	pred->next = curr->next;
	            return true;
            }
	        return false;
        }
    }
}

bool SetList::contains(const std::string& val) const {
    unsigned long key = std::hash<std::string>{}(val);
    std::shared_ptr<Node> curr = this->head;
    while(curr->key < key)
        curr = curr->next;
    return (curr->key == key) && (!curr->marked);
}

//-----------------------------------------------------------------------------

class MultiNode {
public:
    std::mutex lock;
    std::string item;
    unsigned long key;
    std::shared_ptr<MultiNode> next;
    unsigned int multiplicity;

    MultiNode(const std::string& s, std::shared_ptr<MultiNode> n = NULL) {
        item = s;
        key = std::hash<std::string>{}(s);
        next = n;
        multiplicity = 1;
    }
    MultiNode(unsigned long k) {
        item = "";
        key = k;
        next = NULL; 
        multiplicity = 1;
    }
};

class MultiSetList{
    static const unsigned long LOWEST_KEY;
    static const unsigned long LARGEST_KEY;
    std::shared_ptr<MultiNode> head;

    static bool validate(std::shared_ptr<const MultiNode> pred, std::shared_ptr<const MultiNode> curr);
public:
    MultiSetList() { // constructor
        head = std::make_shared<MultiNode>(MultiSetList::LOWEST_KEY);
        head->next = std::make_shared<MultiNode>(MultiSetList::LARGEST_KEY);
    }
    void add(const std::string& val);
    bool remove(const std::string& val);
    unsigned int contains(const std::string& val) const;
};

const unsigned long MultiSetList::LOWEST_KEY = 0;
const unsigned long MultiSetList::LARGEST_KEY = ULONG_MAX;
 
bool MultiSetList::validate(std::shared_ptr<const MultiNode> pred, std::shared_ptr<const MultiNode> curr) {
  return (pred->multiplicity > 0) && (curr->multiplicity > 0) && (pred->next == curr);
}

void MultiSetList::add(const std::string& val) {
    unsigned long key = std::hash<std::string>{}(val);
    std::shared_ptr<MultiNode> curr = this->head;
    if(contains(val)>0){
        std::lock_guard<std::mutex> curr_lk(curr->lock);

        while (curr->key < key)
            curr = curr->next;
        if(curr->key==key){
            curr->multiplicity += 1;
        }
    }
    else{
        while (true) {
            std::shared_ptr<MultiNode> pred = this->head;
            std::shared_ptr<MultiNode> curr = pred->next;
            while (curr->key < key) {
                pred = curr;
                curr = curr->next;
            }
            std::lock_guard<std::mutex> pred_lk(pred->lock);
            std::lock_guard<std::mutex> curr_lk(curr->lock);
            if (MultiSetList::validate(pred, curr)) {
                if (key == curr->key) {
                    curr->multiplicity +=1;
                    break;
                }
                else{
                    pred->next = std::make_shared<MultiNode>(val, curr);
                    break;
                }
            }
        }
    }
}
    
bool MultiSetList::remove(const std::string& val) {
    unsigned long key = std::hash<std::string>{}(val);
    while(contains(val)>0){
        std::shared_ptr<MultiNode> pred = this->head;
        std::shared_ptr<MultiNode> curr = pred->next;
        while (curr->key < key){
            pred = curr;
            curr = curr->next;
        }
        std::lock_guard<std::mutex> pred_lk(pred->lock);
        std::lock_guard<std::mutex> curr_lk(curr->lock);
        if (MultiSetList::validate(pred, curr)) {
            if(curr->key==key){
                curr->multiplicity -= 1;
                if(curr->multiplicity == 0){
                    pred->next = curr->next;
                }
            }
            return true;
        }
    }
    return false;
}

unsigned int MultiSetList::contains(const std::string& val) const {
    unsigned long key = std::hash<std::string>{}(val);
    std::shared_ptr<MultiNode> curr = this->head;
    while (curr->key < key)
        curr = curr->next;
    if (curr->key == key) {
        return curr->multiplicity;
    }
    return 0;
}

//-----------------------------------------------------------------------------

class AtomicNode {
public:
    std::string item;
    unsigned long key;
    std::atomic<AtomicNode*> next;
    
    AtomicNode(const std::string& s, AtomicNode* n = NULL) {
        item = s;
        key = std::hash<std::string>{}(s);
        std::atomic_init<AtomicNode*>(&next, n);
    }
    AtomicNode(unsigned long k) {
        item = "";
        key = k;
        next = NULL; 
    }
};


class MonotonicLockFreeSet {
    AtomicNode* head;

    static const unsigned long LOWEST_KEY;
    static const unsigned long LARGEST_KEY;
public:
    MonotonicLockFreeSet() {
        this->head = new AtomicNode(MonotonicLockFreeSet::LOWEST_KEY);
        this->head->next = new AtomicNode(MonotonicLockFreeSet::LARGEST_KEY);
    }
    ~MonotonicLockFreeSet();
    bool add(const std::string& val);
    bool contains(const std::string& val) const;
};

const unsigned long MonotonicLockFreeSet::LOWEST_KEY = 0;
const unsigned long MonotonicLockFreeSet::LARGEST_KEY = ULONG_MAX;

bool MonotonicLockFreeSet::add(const std::string& val) {
    unsigned long key = std::hash<std::string>{}(val);
    if(contains(val)){
        return false;
    }
    while(true){
        std::atomic<AtomicNode*> pred = this->head;
        std::atomic<AtomicNode*> curr = pred.load()->next.load();
        while(curr.load()->key<key){
            pred.exchange(curr);
            curr.exchange(curr.load()->next);
        }
        if(key==curr.load()->key){
            return false;
        }
        AtomicNode* new_node = new AtomicNode(val);
        new_node->next = curr.load();
        AtomicNode* validate_node = curr.load();
        if(pred.load()->next.compare_exchange_weak(validate_node,new_node)){
            return true;
        }
    }
}

bool MonotonicLockFreeSet::contains(const std::string& val) const {
    unsigned long key = std::hash<std::string>{}(val);
    AtomicNode* curr = this->head;
    while (curr->key < key) {
        curr = curr->next;
    }
    return curr->key == key;
}

MonotonicLockFreeSet::~MonotonicLockFreeSet() {
    AtomicNode* pred = this->head;
    AtomicNode* curr = pred->next;
    while (curr != NULL) {
        delete pred;
        pred = curr;
        curr = curr->next;
    }
    delete pred;
}

//-----------------------------------------------------------------------------

template <typename T>
struct RefAndMark {
    T* ref;
    bool mark;
    RefAndMark(T* _ref, bool _mark) {
        this->ref = _ref;
        this->mark = _mark;
    }
};

template <typename T>
class AtomicMarkable {
    std::atomic<RefAndMark<T> > data;
public:
    AtomicMarkable(T* _ref = nullptr, bool mark = false): data(RefAndMark<T>(_ref, mark)) {}

    bool compare_and_set(T*& expected_ref, bool& expected_mark, T* desired_ref, bool desired_mark) {
        return false;
    }

    bool attempt_mark(T* expected_ref, bool new_mark) {
        return true;
    }

    T* get(bool& mark) const {
    }
};


