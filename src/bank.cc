#include "iostream"
#include "headers/bank.h"
using namespace std;

class Bank::PImpl {
    public:
        unsigned int numStudents;
        unsigned int * balance;
        uCondition * wait;
    
        PImpl(unsigned int numStudents): numStudents(numStudents) {
            balance = new unsigned int[numStudents];
            wait = new uCondition[numStudents];
            for (int i = 0; i < (int) numStudents; ++i) {
                balance[i] = 0;
            }
        }

        ~PImpl() {
            delete [] balance;
            delete [] wait;
        }
};

Bank::Bank(unsigned int numStudents) : pimpl (new PImpl(numStudents)) {}

Bank::~Bank() {
    delete pimpl;
}

void Bank::deposit(unsigned int id, unsigned int amount) {
    pimpl->balance[(int) id] += amount;
    if (!pimpl->wait[(int) id].empty() && pimpl->wait[(int) id].front() <= pimpl->balance[(int) id]) pimpl->wait[(int) id].signal();
}


void Bank::withdraw(unsigned int id, unsigned int amount) {
    if (pimpl->balance[(int) id] < amount) pimpl->wait[(int) id].wait(amount);
    pimpl->balance[(int) id] -= amount;
}
