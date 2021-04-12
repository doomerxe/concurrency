#include <iostream>
#include <vector>

#include "headers/groupoff.h"
#include "headers/printer.h"
#include "headers/watcard.h"
#include "headers/MPRNG.h"
using namespace std;

extern MPRNG mprng;

class Groupoff::PImpl {
    public:
        // similar structure as job
        struct Card {
            WATCard * card;
            WATCard::FWATCard futureCard;
            
            Card(WATCard * card): card(card) {}
        };

        // communicate
        Printer & printer;
        unsigned int numStudents;
        unsigned int maxTripCost;
        unsigned int groupoffDelay;

        unsigned int callByStudent;     // track # of giftCard() calls by students
        vector<Card *> watcards;        // card (job) list
        vector<WATCard *> deletecards;  // actual watcard list
    
        PImpl(Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay): 
            printer(prt), numStudents(numStudents), maxTripCost(maxTripCost), groupoffDelay(groupoffDelay) {
                callByStudent = 0;
            }

        ~PImpl() {
            // destroy card (job)
            while (!watcards.empty()) {
                Card * current_card = watcards.back();
                watcards.pop_back();
                delete current_card;
            }
            // destroy unfinished watcard
            while (!deletecards.empty()) {
                WATCard * current_card = deletecards.back();
                deletecards.pop_back();
                delete current_card;
            }
        }
};

Groupoff::Groupoff(Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay) : 
    pimpl (new PImpl(prt, numStudents, maxTripCost, groupoffDelay)) {
        pimpl->printer.print(Printer::Kind::Groupoff, 'S');
    }

Groupoff::~Groupoff() {
    delete pimpl;
}

WATCard::FWATCard Groupoff::giftCard() {
    // create new giftcard
    WATCard * card = new WATCard();
    pimpl->deletecards.push_back(card);
    // create new card (job)
    PImpl::Card * card_ip = new PImpl::Card(card);
    pimpl->watcards.push_back(card_ip);

    return card_ip->futureCard;
}

void Groupoff::main() {
    // wait until all students call giftcard()
    for (;pimpl->callByStudent < pimpl->numStudents;) {
        _Accept(giftCard) {
            ++pimpl->callByStudent;
        } 
    }

    // distribute gitfcards
    for (;pimpl->callByStudent > 0;) {
        yield(pimpl->groupoffDelay);
        _Accept(~Groupoff) {
            break;
        } _Else {
            // find a random student to give the "real" giftcard
            unsigned int studentIndex = mprng(0 , pimpl->watcards.size() - 1);
            unsigned int amount = pimpl->maxTripCost;
            PImpl::Card * current_card = pimpl->watcards[studentIndex];
            // reduce the number of job 
            pimpl->watcards.erase(pimpl->watcards.begin() + studentIndex);
            // do the job
            WATCard * card = current_card->card;
            card->deposit(amount);
            current_card->futureCard.delivery(card);
            // finish the job
            delete current_card;

            pimpl->printer.print(Printer::Kind::Groupoff, 'D', amount);
            --pimpl->callByStudent;
        }
    }

    // put it here to match the output
    pimpl->printer.print(Printer::Kind::Groupoff, 'F');
}
