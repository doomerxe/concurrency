#include <iostream>

#include "headers/parent.h"
#include "headers/printer.h"
#include "headers/bank.h"
#include "headers/MPRNG.h"
using namespace std;

extern MPRNG mprng;

class Parent::PImpl {
    public:
        // communicate
        Printer & printer;
        Bank & bank;
        unsigned int numStudents;
        unsigned int parentalDelay;
        unsigned int maxTripCost;
    
        PImpl(Printer & prt, Bank & bank, unsigned int numStudents, unsigned int parentalDelay, unsigned int maxTripCost): 
            printer(prt), bank(bank), numStudents(numStudents), parentalDelay(parentalDelay), maxTripCost(maxTripCost) {}

        ~PImpl() {}
};

Parent::Parent(Printer & prt, Bank & bank, unsigned int numStudents, unsigned int parentalDelay, unsigned int maxTripCost) : 
    pimpl (new PImpl(prt, bank, numStudents, parentalDelay, maxTripCost)) {
        pimpl->printer.print(Printer::Kind::Parent, 'S');
    }

Parent::~Parent() {
    pimpl->printer.print(Printer::Kind::Parent, 'F');
    delete pimpl;
}

void Parent::main() {
    for (;;) {
        // yield here (instead of inside _Else) to match input
        yield(pimpl->parentalDelay);
        _Accept(~Parent) {
            break;
        } _Else {
            // parent deposits money into students' accounts
            unsigned int gift = mprng(1, 3);
            unsigned int studentIndex = mprng(0 , pimpl->numStudents - 1);
            unsigned int amount = max((unsigned int) 1, pimpl->maxTripCost * gift / 3);
            pimpl->printer.print(Printer::Kind::Parent, 'D', studentIndex, amount);
            pimpl->bank.deposit(studentIndex, amount);
        }
    }
}
