#include <iostream>

#include "headers/bank.h"
using namespace std;

struct bankAccount {
    unsigned int balance;
    uCondition wait;
};

class Bank::PImpl {
    public:
        // communicate
        unsigned int numStudents;

        struct bankAccount * accounts;     //  one account per student
    
        PImpl(unsigned int numStudents): numStudents(numStudents) {
            accounts = new bankAccount[numStudents];
            for (int i = 0; i < (int) numStudents; ++i) {
                accounts[i].balance = 0;    // initially 0 balance
            }
        }

        ~PImpl() {
            delete [] accounts;
        }
};

Bank::Bank(unsigned int numStudents) : pimpl (new PImpl(numStudents)) {}

Bank::~Bank() {
    delete pimpl;
}

void Bank::deposit(unsigned int id, unsigned int amount) {
    pimpl->accounts[id].balance += amount;
    // signal when there is enough money (balance) in the account.
    if (!pimpl->accounts[id].wait.empty() && 
        pimpl->accounts[id].wait.front() <= pimpl->accounts[id].balance) {
            pimpl->accounts[id].wait.signal(); 
        }
}


void Bank::withdraw(unsigned int id, unsigned int amount) {
    // blocked when not enought money (balance)
    if (pimpl->accounts[id].balance < amount) {
        pimpl->accounts[id].wait.wait(amount);
    }
    pimpl->accounts[id].balance -= amount;
}
