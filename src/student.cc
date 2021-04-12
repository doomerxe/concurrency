#include <iostream>
#include <vector>

#include "headers/student.h"
#include "headers/groupoff.h"
#include "headers/printer.h"
#include "headers/watcard.h"
#include "headers/cardoffice.h"
#include "headers/train.h"
#include "headers/trainstop.h"
#include "headers/nameserver.h"
#include "headers/MPRNG.h"
using namespace std;

extern MPRNG mprng;

class Student::PImpl {
    public:
        Printer & printer;
        NameServer & nameServer;
        WATCardOffice & cardOffice;
        Groupoff & groupoff;
        unsigned int id;
        unsigned int numStops;
        unsigned int stopCost;
        unsigned int maxStudentDelay;
        unsigned int maxStudentTrips;

        PImpl(Printer & prt, NameServer & nameServer, WATCardOffice & cardOffice, Groupoff & groupoff, 
        unsigned int id, unsigned int numStops, unsigned int stopCost, 
        unsigned int maxStudentDelay, unsigned int maxStudentTrips) : 
            printer(prt), nameServer(nameServer), cardOffice(cardOffice), groupoff(groupoff),
            id(id), numStops(numStops), stopCost(stopCost), maxStudentDelay(maxStudentDelay), maxStudentTrips(maxStudentTrips) {}

        ~PImpl() {}
};

Student::Student(Printer & prt, NameServer & nameServer, WATCardOffice & cardOffice, Groupoff & groupoff, 
    unsigned int id, unsigned int numStops, unsigned int stopCost, 
    unsigned int maxStudentDelay, unsigned int maxStudentTrips) :
    pimpl(new PImpl(prt, nameServer, cardOffice, groupoff, id, numStops, stopCost, maxStudentDelay, maxStudentTrips)) {}

Student::~Student() {
    delete pimpl;
}

void Student::main() {
    vector<unsigned int>stops;

    unsigned int numStudentTrips = mprng(1, pimpl->maxStudentTrips);
    unsigned int numStops = pimpl->numStops;
    unsigned int maxTripCost = pimpl->stopCost * numStops / 2;
    WATCard::FWATCard gitfcard = pimpl->groupoff.giftCard();
    WATCard::FWATCard watcard = pimpl->cardOffice.create(pimpl->id, maxTripCost);
    
    pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'S', numStudentTrips);

    for (unsigned int i = 0; i < numStops; ++i) {
        stops.push_back(i);
    }

    unsigned int startIndex;
    unsigned int start;
    
    for (unsigned int i = 0; i < numStudentTrips; ++i) {

        unsigned int delay = mprng(0, pimpl->maxStudentDelay);
        yield(delay);

        if (i == 0) {
            startIndex = mprng(0, numStops - 1);
            start = stops[startIndex];
            stops[startIndex] = stops[numStops - 1];
            stops[numStops - 1] = start;        
        }

        unsigned int endIndex = mprng(0, numStops - 2);
        unsigned int end = stops[endIndex];
        TrainStop * startlocation = pimpl->nameServer.getStop(pimpl->id ,stops[numStops - 1]);

        unsigned int numTripStops;
        char direction;
        Train::Direction trainDir;
        if (start > end) {
            if (start - end >= end + numStops - start) {
                numTripStops = end + numStops - start;
                direction = '<';
                trainDir = Train::Direction::Clockwise;    
            } else {
                numTripStops = start - end;
                direction = '>';
                trainDir = Train::Direction::CounterClockwise;
            }
        } else {
            if (end - start > start + numStops - end) {
                numTripStops = start + numStops - end;
                direction = '>';
                trainDir = Train::Direction::CounterClockwise;     
            } else {
                numTripStops = end - start;
                direction = '<';
                trainDir = Train::Direction::Clockwise;    
            }            
        }
        pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'T', start, end, direction);

        bool freeride = false;
        if (numTripStops == 1) {
            if (mprng(1, 10) <= 5) freeride = true;
        } else {
            if (mprng(1, 10) <= 3) freeride = true;
        }

        unsigned int fee = pimpl->stopCost * numTripStops;
        if (!freeride) {
            for (;;) {
                try {
                    _Select(gitfcard) {
                        WATCard * card = gitfcard();
                        startlocation->buy(numTripStops, *card);
                        gitfcard.reset();
                        pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'G', fee, card->getBalance());
                        break;
                    } or _Select(watcard) {
                        WATCard * card = watcard();
                        startlocation->buy(numTripStops, *card);
                        pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'B', fee, card->getBalance());
                        break;                        
                    }
                } catch(WATCardOffice::Lost &) {
                    pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'L');
                    watcard = pimpl->cardOffice.create(pimpl->id, maxTripCost);
                    continue;
                } catch (TrainStop::Funds & f) {
                    WATCard * card = watcard();
                    pimpl->cardOffice.transfer(pimpl->id, f.amount, card);
                    continue;
                }
            }            
        } else {
            pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'f');
        }

        //before wait, the current student must have a watcard
        for (;;) {
            try {
                watcard();
                break;
            } catch (WATCardOffice::Lost &) {
                watcard = pimpl->cardOffice.create(pimpl->id, maxTripCost);
                continue;
            }
        }

        try {
            WATCard * current_card = watcard();
            pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'W', start);
            Train * train = startlocation->wait(pimpl->id, trainDir);
            pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'E', train->getId());
            TrainStop * endlocation = train->embark(pimpl->id, end, *current_card);
            pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'D', end);
            endlocation->disembark(pimpl->id);
        } catch (Train::Ejected &) {
            pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'e');
            break;
        }

        start = end;
        stops[endIndex] = stops[numStops - 1];
        stops[numStops - 1] = start;        
    }

    delete watcard;
    pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'F');
}
