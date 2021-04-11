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
        struct Card {
            WATCard * card;
            WATCard::FWATCard futureCard;
            
            Card(WATCard * card): card(card) {}
        };

        Printer & printer;
        unsigned int numStudents;
        unsigned int maxTripCost;
        unsigned int groupoffDelay;
        unsigned int callByStudent;
        vector<Card *> watcards;
        vector<WATCard *> deletecards;
    
        PImpl(Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay): 
            printer(prt), numStudents(numStudents), maxTripCost(maxTripCost), groupoffDelay(groupoffDelay) {
                callByStudent = 0;
            }

        ~PImpl() {}
};

Groupoff::Groupoff(Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay) : 
    pimpl (new PImpl(prt, numStudents, maxTripCost, groupoffDelay)) {
        pimpl->printer.print(Printer::Kind::Groupoff, 'S');
    }

Groupoff::~Groupoff() {
    while (!pimpl->watcards.empty()) {
        PImpl::Card * current_card = pimpl->watcards.back();
        pimpl->watcards.pop_back();
        delete current_card;
    }
    while (!pimpl->deletecards.empty()) {
        WATCard * current_card = pimpl->deletecards.back();
        pimpl->deletecards.pop_back();
        delete current_card;
    }
    delete pimpl;
}

WATCard::FWATCard Groupoff::giftCard() {
    WATCard * card = new WATCard();
    pimpl->deletecards.push_back(card);
    PImpl::Card * card_ip = new PImpl::Card(card);
    pimpl->watcards.push_back(card_ip);
    return card_ip->futureCard;
}

void Groupoff::main() {
    for (;pimpl->callByStudent < pimpl->numStudents;) {
        _Accept(giftCard) {
            ++pimpl->callByStudent;
        } 
    }

    for (;pimpl->callByStudent > 0;) {
        yield(pimpl->groupoffDelay);
        _Accept(~Groupoff) {
            break;
        } _Else {
            unsigned int studentIndex = mprng(0 , pimpl->watcards.size() - 1);
            unsigned int amount = pimpl->maxTripCost;
            PImpl::Card * current_card = pimpl->watcards[studentIndex];
            pimpl->watcards.erase(pimpl->watcards.begin() + studentIndex);
            WATCard * card = current_card->card;
            card->deposit(amount);
            current_card->futureCard.delivery(card);
            delete current_card;
            pimpl->printer.print(Printer::Kind::Groupoff, 'D', amount);
            --pimpl->callByStudent;
        }
    }
    // put it here to match the output
    pimpl->printer.print(Printer::Kind::Groupoff, 'F');
}
