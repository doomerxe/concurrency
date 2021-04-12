#include <iostream>

#include "watcard.h"
using namespace std;

class WATCard::PImpl {
    public:
        unsigned int balance;   
        bool pop;               // proof of purchase
    
        PImpl() {
            balance = 0;
            pop = false;
        }

        ~PImpl() {}
};

void WATCard::markPaid() {
    pimpl->pop = true;
}

WATCard::WATCard() : pimpl (new PImpl()) {}

WATCard::~WATCard() {
    delete pimpl;
}

void WATCard::deposit(unsigned int amount) {
    pimpl->balance += amount;
}


void WATCard::withdraw(unsigned int amount) {
    pimpl->balance -= amount;
}

unsigned int WATCard::getBalance() {
    return pimpl->balance;
}

bool WATCard::paidForTicket() {
    return pimpl->pop;
}

void WATCard::resetPOP() {
    pimpl->pop = false;
}
