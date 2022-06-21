#include <chrono>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <queue>


//-----------------------------------------------------------------------------

static std::atomic<unsigned int> max_id = std::atomic<unsigned int>(0);
static std::mutex global_lock;
static int last_id = -1;
class Account {
        unsigned int amount;
        unsigned int account_id;
        std::mutex lock;

        static std::atomic<unsigned int> max_account_id;
    public:
        Account() {
            global_lock.lock();
            max_id.fetch_add(1);
            account_id = max_id;
            this->amount = 0;
            global_lock.unlock();
        }

        Account(unsigned int amount) {
            global_lock.lock();
            max_id.fetch_add(1);
            account_id = max_id;
            this->amount = amount;
            global_lock.unlock();
        }

        // copy-contructor and assignment are deleted to make the id's really unique
        Account(const Account& other) = delete;

        Account& operator = (const Account& other) = delete;
        
        unsigned int get_amount() const {
            return this->amount;
        }

        unsigned int get_id() const {
            return this->account_id;
        }

        // withdrwas deduction if the current amount is at least deduction
        // returns whether the withdrawal took place
        bool withdraw(unsigned int deduction) {
            this->lock.lock();
            if(get_amount()<deduction){
                this->lock.unlock();
                return false;
            }
            this->amount -= deduction;
            this->lock.unlock();
            return true;
        }

        // adds the prescribed amount of money to the account
        void add(unsigned int to_add) {
            this->lock.lock();
            this->amount += to_add;
            this->lock.unlock();
        }

        // transfers amount from from to to if there are enough money on from
        // returns whether the transfer happened
        static bool transfer(unsigned int amount, Account& from, Account& to) {
            // from.lock.lock();
            // to.lock.lock();
            // while(last_id>-1 && from.get_id()>last_id){}
            // global_lock.lock();
            // last_id = from.get_id();
            if (from.withdraw(amount)){
                to.add(amount);
            }
            else{
                // to.lock.unlock();
                // from.lock.unlock();
                return false;
            }
            // to.lock.unlock();
            // from.lock.unlock();
            // global_lock.unlock();
            return true;
        }
    
        static bool massiv_withdraw(std::vector<Account*>& accounts, unsigned int amount) {
            for (Account *account : accounts){
                // std::lock_guard s_lock{global_lock};
                if(!account->withdraw(amount)){
                    return false;
                }
            }
            return true;
        }
};

std::atomic<unsigned int> Account::max_account_id(0);

//-----------------------------------------------------------------------------
// static int max_index = 0;

template <unsigned int n, unsigned int M>

class ChunkedThreadArray {
        int data[n * M];
        std::mutex m[n];
        std::mutex swap_lock;
    public:
        int get(size_t i) const {
            return this->data[i];
        }

        bool set(size_t i, int val) {
            return set(i, val, [](int) {return true;});
        }

        template <typename Func>
        bool set(size_t i, int val, Func f) {
            std::lock_guard<std::mutex> lg(m[i/M]);
            if(f(get(i))){
                data[i] = val;
                return true;
            }
            return false;

        }

        template <typename Func>
        bool swap(size_t i, size_t j, Func f) {
            std::lock_guard<std::mutex> slock(swap_lock);
            int temp = get(i);
            if(f(temp,get(j))){
                set(i,get(j));      
                set(j,temp);
                return true;
            }
            return false;

        }
};

//-----------------------------------------------------------------------------
