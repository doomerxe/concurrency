#include <iostream>

#include "headers/conductor.h"
#include "headers/printer.h"
#include "headers/train.h"
using namespace std;

class Conductor::PImpl {
    public:
        Printer & printer;
        unsigned int id;
        Train * train;
        unsigned int delay;
    
        PImpl(Printer & prt, unsigned int id, Train * train, unsigned int delay): 
            printer(prt), id(id), train(train), delay(delay) {
        }

        ~PImpl() {
        }
};

Conductor::Conductor(Printer & prt, unsigned int id, Train * train, unsigned int delay) : 
    pimpl (new PImpl(prt, id, train, delay)) {
        pimpl->printer.print(Printer::Kind::Conductor, id, 'S');
    }

Conductor::~Conductor() {
    delete pimpl;
}

void Conductor::main() {
    for (;;) {
        _Accept(~Conductor) {
            break;
        } _Else {
            yield(pimpl->delay);
            pimpl->printer.print(Printer::Kind::Conductor, pimpl->id, 'c');
            pimpl->train->scanPassengers();
        }

    }

    pimpl->printer.print(Printer::Kind::Conductor, pimpl->id, 'F');
}
