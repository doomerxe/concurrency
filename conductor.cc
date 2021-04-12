#include "conductor.h"
#include "printer.h"
#include "train.h"
using namespace std;

class Conductor::PImpl {
    public:
        // communicate
        Printer & printer;
        unsigned int id;
        Train * train;
        unsigned int delay;
    
        PImpl(Printer & prt, unsigned int id, Train * train, unsigned int delay): 
            printer(prt), id(id), train(train), delay(delay) {
        }

        ~PImpl() {}
};

Conductor::Conductor(Printer & prt, unsigned int id, Train * train, unsigned int delay) : 
    pimpl (new PImpl(prt, id, train, delay)) {
        pimpl->printer.print(Printer::Kind::Conductor, id, 'S');
    }

Conductor::~Conductor() {
    pimpl->printer.print(Printer::Kind::Conductor, pimpl->id, 'F');
    delete pimpl;
}

void Conductor::main() {
    for (;;) {
        _Accept(~Conductor) {
            break;
        } _Else {
            // yield and call scanPassengers() on the train to check freeriders
            yield(pimpl->delay);
            pimpl->printer.print(Printer::Kind::Conductor, pimpl->id, 'c');
            pimpl->train->scanPassengers();
        }
    }

}
