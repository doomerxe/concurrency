#include <iostream>

#include "headers/bank.h"
#include "headers/printer.h"
#include "headers/watcard.h"
#include "headers/cardoffice.h"
#include "headers/MPRNG.h"
using namespace std;

extern MPRNG mprng;

struct Job {
    WATCard * card;
    unsigned int sid;
    unsigned int amount;
    WATCard::FWATCard futureCard;
            
    Job(WATCard * card, unsigned int sid, unsigned int amount): card(card), sid(sid), amount(amount) {}
};

class WATCardOffice::PImpl {
    public:
        _Task Courier {
            unsigned int cid;
            Printer & printer;
            Bank & bank;
            WATCardOffice * cardoffice;

            void main() {
                for(;;) {
                    Job * current_job = cardoffice->requestWork();
                    if (cardoffice->pimpl->finish) break;
                    unsigned int sid = current_job->sid;
                    unsigned int amount = current_job->amount;
                    WATCard * card = current_job->card;
                    // start job
                    printer.print(Printer::Kind::WATCardOfficeCourier, cid, 't', sid, amount);
                    bank.withdraw(sid, amount);
                    card->deposit(amount);
                    // check lost
                    if (mprng(1, 6) == 1) {
                        current_job->futureCard.exception(new Lost());
                        printer.print(Printer::Kind::WATCardOfficeCourier, cid, 'L', sid);
                        delete card;
                    } else {
                        current_job->futureCard.delivery(card);
                        printer.print(Printer::Kind::WATCardOfficeCourier, cid, 'T', sid, amount);
                    }
                    delete current_job;
                }
                printer.print(Printer::Kind::WATCardOfficeCourier, cid, 'F');
            }

            public:
            Courier(unsigned int cid, Printer & prt, Bank & bank, WATCardOffice * cardoffice) : 
                cid(cid), printer(prt), bank(bank), cardoffice(cardoffice) {
                    printer.print(Printer::Kind::WATCardOfficeCourier, cid, 'S');
                }

            ~Courier() {
            }
        };

        Printer & printer;
        Bank & bank;
        unsigned int numCouriers;
        list<Job *> joblist;
        bool finish;
    
        PImpl(Printer & prt, Bank & bank, unsigned int numCouriers): 
            printer(prt), bank(bank), numCouriers(numCouriers) {
                finish = false;
            }

        ~PImpl() {}
};

WATCardOffice::WATCardOffice(Printer & prt, Bank & bank, unsigned int numCouriers) : 
    pimpl (new PImpl(prt, bank, numCouriers)) {
        pimpl->printer.print(Printer::Kind::WATCardOffice, 'S');
    }

WATCardOffice::~WATCardOffice() {
    delete pimpl;
}

WATCard::FWATCard WATCardOffice::create(unsigned int sid, unsigned int amount) {
    // watcard should be destroyed by student, used print to figure that out....
    WATCard * card = new WATCard();
    Job * card_job = new Job(card, sid, amount);
    pimpl->joblist.push_back(card_job);
    return card_job->futureCard;    
}

WATCard::FWATCard WATCardOffice::transfer(unsigned int sid, unsigned int amount, WATCard * card) {
    Job * card_job = new Job(card, sid, amount);
    pimpl->joblist.push_back(card_job);
    return card_job->futureCard; 
}

Job * WATCardOffice::requestWork() {
    Job * current_job = pimpl->joblist.front();
    pimpl->joblist.pop_front();
    return current_job;
}

void WATCardOffice::main() {
    // courier list should be initialized here, used print to find this...
    list<PImpl::Courier *> couriers;
    for (unsigned int i = 0; i < pimpl->numCouriers; ++i) {
        PImpl::Courier * courier = new PImpl::Courier(i, pimpl->printer, pimpl->bank, this);
        couriers.push_back(courier);
    }

    for (;;) {
        _Accept (~WATCardOffice) {
            pimpl->finish = true;
            pimpl->joblist.clear();
            for (unsigned int i = 0; i < pimpl->numCouriers; ++i) {
                pimpl->joblist.push_back(NULL);
                _Accept(requestWork);
            }
            break;
        } or _When(!pimpl->joblist.empty()) _Accept(requestWork) {
            pimpl->printer.print(Printer::Kind::WATCardOffice, 'W');
        } or _Accept(create) {
            pimpl->printer.print(Printer::Kind::WATCardOffice, 'C', pimpl->joblist.back()->sid, pimpl->joblist.back()->amount);
        } or _Accept(transfer) {
            pimpl->printer.print(Printer::Kind::WATCardOffice, 'T', pimpl->joblist.back()->sid, pimpl->joblist.back()->amount);
        } 
    }

    while(!couriers.empty()) {
        PImpl::Courier * courier = couriers.back();
        couriers.pop_back();
        delete courier;
    }

    pimpl->printer.print(Printer::Kind::WATCardOffice, 'F');
}
