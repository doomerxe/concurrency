#include "student.h"
#include "groupoff.h"
#include "printer.h"
#include "watcard.h"
#include "cardoffice.h"
#include "train.h"
#include "trainstop.h"
#include "nameserver.h"
#include "MPRNG.h"
using namespace std;

extern MPRNG mprng;

class Student::PImpl {
    public:
        // communicate
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
    // prepare for the trips to start
    unsigned int numStudentTrips = mprng(1, pimpl->maxStudentTrips);
    unsigned int numStops = pimpl->numStops;
    unsigned int maxTripCost = pimpl->stopCost * numStops / 2;
    WATCard::FWATCard giftcard = pimpl->groupoff.giftCard();
    WATCard::FWATCard watcard = pimpl->cardOffice.create(pimpl->id, maxTripCost);
    
    pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'S', numStudentTrips);

    // array to help calculate start & end destinations
    unsigned int stops[numStops];
    for (unsigned int i = 0; i < numStops; ++i) {
        stops[i] = i;
    }

    unsigned int startIndex;
    unsigned int start;
    bool usedGift = false;
    
    _Enable {
    for (unsigned int i = 0; i < numStudentTrips; ++i) {

        unsigned int delay = mprng(0, pimpl->maxStudentDelay);
        yield(delay);

        if (i == 0) {   // first time needs to generate the start index
            startIndex = mprng(0, numStops - 1);
            start = stops[startIndex];
            stops[startIndex] = stops[numStops - 1];
            stops[numStops - 1] = start;        
        }

        // generate a distinct end destination
        unsigned int endIndex = mprng(0, numStops - 2);
        unsigned int end = stops[endIndex];
        TrainStop * startlocation = pimpl->nameServer.getStop(pimpl->id ,stops[numStops - 1]);

        // find the direction
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

        // see if a student wants to avoid the fee
        bool freeride = false;
        if (numTripStops == 1) {
            if (mprng(1, 10) <= 5) freeride = true;
        } else {
            if (mprng(1, 10) <= 3) freeride = true;
        }

        freeride = false;

        // buy ticket
        unsigned int fee = pimpl->stopCost * numTripStops;
        if (!freeride) {
            for (;;) {
                try {
                    // first try to use giftcard
                    _Select(giftcard) {
                        WATCard * card = giftcard();
                        startlocation->buy(numTripStops, *card);
                        // set usedGift to true
                        usedGift = true;
                        pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'G', fee, card->getBalance());
                        break;
                    } or _Select(watcard) {
                        WATCard * card = watcard();
                        startlocation->buy(numTripStops, *card);
                        pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'B', fee, card->getBalance());
                        break;                        
                    }
                } catch(WATCardOffice::Lost &) {    // if card lost when first created ask a new one
                    pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'L');
                    watcard = pimpl->cardOffice.create(pimpl->id, maxTripCost);
                    continue;
                } catch (TrainStop::Funds & f) {    // if not enough money, do a transfer and try again
                    WATCard * card = watcard();
                    pimpl->cardOffice.transfer(pimpl->id, f.amount, card);
                    continue;
                }
            }            
        } else {    // avoid the fee
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
            // determine the card to check
            WATCard * current_card;
            if (usedGift) current_card = giftcard();
            else current_card = watcard();

            // wait for train and start trip
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

        // giftcard cannot be used again
        if (usedGift) giftcard.reset();
        // next trip start from the end station
        start = end;
        stops[endIndex] = stops[numStops - 1];
        stops[numStops - 1] = start;        
    }
    }

    delete watcard;
    pimpl->printer.print(Printer::Kind::Student, pimpl->id, 'F');
}
