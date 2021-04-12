#include <iostream>
#include <cmath>

#include "headers/printer.h"
#include "headers/watcard.h"
#include "headers/train.h"
#include "headers/trainstop.h"
#include "headers/nameserver.h"
#include "headers/MPRNG.h"
using namespace std;

extern MPRNG mprng;

struct studentInfo {
    bool paid;
    bool freerider;
    uCondition wait;
};

class Train::PImpl {
    public:

        Printer & printer;
        NameServer & nameServer;
        unsigned int id;
        unsigned int maxNumStudents;
        unsigned int numStops;

        unsigned int curStop;
        unsigned int curSize;
        Direction direction;
        struct studentInfo * seats;
        bool * inUse;

        PImpl(Printer & prt, NameServer & nameServer, unsigned int id, unsigned int maxNumStudents, unsigned int numStops) : 
            printer(prt), nameServer(nameServer), id(id), maxNumStudents(maxNumStudents), numStops(numStops) {
                if (id == 0) {
                    direction = Direction::Clockwise;
                    curStop = 0;
                } else {
                    direction = Direction::CounterClockwise;
                    curStop = ceil(numStops / 2);
                }

                seats = new studentInfo[maxNumStudents];
                inUse = new bool[maxNumStudents];
                for (unsigned int i = 0; i < maxNumStudents; ++i) {
                    inUse[i] = false;
                    seats[i].paid = false;
                    seats[i].freerider = false;
                }
                curSize = 0;
            }

        ~PImpl() {
            delete [] seats;
            delete [] inUse;
        }
};

Train::Train(Printer & prt, NameServer & nameServer, unsigned int id, unsigned int maxNumStudents, unsigned int numStops) :
    pimpl(new PImpl(prt, nameServer, id, maxNumStudents, numStops)) {
        char d;
        if (pimpl->direction == Direction::Clockwise) d = '<';
        else d = '>';
        pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'S', pimpl->curStop, d);
    }

Train::~Train() {
    delete pimpl;
}

unsigned int Train::getId() const {
    return pimpl->id;
}

TrainStop * Train::embark(unsigned int studentId, unsigned int destStop, WATCard& card) {
    TrainStop ** stopList = pimpl->nameServer.getStopList(pimpl->id);
    pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'E', studentId, pimpl->curStop);
    ++pimpl->curSize;
    int index = -1;

    for (int i = 0; i < (int) pimpl->maxNumStudents; ++i) {
        if (!pimpl->inUse[i]) {
            index = i;
            pimpl->inUse[i] = true;
            break;
        }
    }

    if (card.paidForTicket()) pimpl->seats[index].paid = true;
    pimpl->seats[index].wait.wait(destStop);

    bool eject = false;
    if (pimpl->seats[index].freerider) eject = true;
    pimpl->seats[index].paid = false;
    pimpl->inUse[index] = false;
    pimpl->seats[index].freerider = false;

    --pimpl->curSize;

    if (eject) _Throw Ejected();
    return stopList[pimpl->curStop];
}

void Train::scanPassengers() {
    for (int i = 0; i < (int) pimpl->maxNumStudents; ++i) {
        if (pimpl->inUse[i]) {
            if (!pimpl->seats[i].paid) pimpl->seats[i].freerider = true;
        } 
    }
}

void Train::main() {
    TrainStop ** stopList = pimpl->nameServer.getStopList(pimpl->id);
    for (;;) {
        _Accept (~Train) {
            break;
        } or _Accept (scanPassengers) {           
        } _Else {
            TrainStop * stop = stopList[pimpl->curStop];
            unsigned int cur = pimpl->curSize;
            for (int i = 0; i < (int) pimpl->maxNumStudents; ++i) {
                if (!pimpl->seats[i].wait.empty()) {
                    if (pimpl->seats[i].wait.front() == pimpl->curStop ||
                        pimpl->seats[i].freerider) {
                            pimpl->seats[i].wait.signalBlock();
                        } 
                }
            }
            unsigned int avail = pimpl->maxNumStudents - pimpl->curSize;
            pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'A', pimpl->curStop, avail, cur);
            unsigned int taken = stop->arrive(pimpl->id, pimpl->direction, avail);
            for (unsigned int i = 0; i < taken; ++i) {
                _Accept (embark);
            }
            if (pimpl->direction == Direction::Clockwise) {
                if (pimpl->curStop == pimpl->numStops - 1) {
                    pimpl->curStop = 0;
                } else {
                    ++pimpl->curStop;
                }
            } else {
                if (pimpl->curStop == 0) {
                    pimpl->curStop = ceil(pimpl->numStops / 2);
                } else {
                    --pimpl->curStop;
                }                
            }
        }
    }
    pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'F');
}
