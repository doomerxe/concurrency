#include "trainstop.h"
#include "printer.h"
#include "nameserver.h"

#include <iostream>

using namespace std;

class TrainStop::PImpl {
 public:
  Printer &prt;
  NameServer &server;
  unsigned int id;
  unsigned int stopCost;
  unsigned int numWaitingCW = 0;
  unsigned int numWaitingCCW = 0;

  uCondition wBuying;             // caller of buy wait on this to get watcard back
  uCondition wWaitingCW;          // wait for the clockwise train
  uCondition wWaitingCCW;         // wait for the counter-clockwise train
  uCondition wTrain;              // train wait for timer to tick

  // communication variables from caller to trainstop
  uBaseTask *caller;
  WATCard *card;
  unsigned int numStops;
  unsigned int studentId;
  unsigned int trainId;
  Train::Direction direction;
  unsigned int maxNumStudents;
  Train *train;
  unsigned int *numEmbarked;

  // communication variable from trainstop to caller
  Train *trainCW;
  Train *trainCCW;

  PImpl( Printer & prt, NameServer & nameServer, unsigned int id, unsigned int stopCost ):
    prt{prt}, server{nameServer}, id{id}, stopCost{stopCost} {}
};

TrainStop::Funds::Funds( unsigned int amt): amount{amt} {}

TrainStop::TrainStop( Printer & prt, NameServer & nameServer, unsigned int id, unsigned int stopCost ):
  pimpl{new PImpl{ prt, nameServer, id, stopCost}} {
  pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 'S');
}

TrainStop::~TrainStop() {
  pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 'F');
  delete pimpl;
}

_Nomutex unsigned int TrainStop::getId() const {
  return pimpl->id;
}

void TrainStop::main() {
  pimpl->server.registerStop(pimpl->id);

  while (true) {
    _Accept( ~TrainStop ) {
      break;

    // update watcard or throw exception for student buying ticket
    } or _Accept( buy ) {
      unsigned int balance = pimpl->card->getBalance();
      unsigned int cost = pimpl->stopCost * pimpl->numStops;
      if ( balance >= cost ) {    // has enough balance?
        pimpl->card->withdraw( cost );
        pimpl->card->markPaid();
        pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 'B', cost);
      } else {
        _Resume Funds(cost - balance) _At *(pimpl->caller);
      }
      pimpl->wBuying.signalBlock();

    // increase waiting count according to students' direction
    } or _Accept( wait ) {
      char dirChar = pimpl->direction == Train::Direction::Clockwise ? '<' : '>';
      pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 'W', pimpl->studentId, dirChar);
      if (pimpl->direction == Train::Direction::Clockwise) {
        pimpl->numWaitingCW++;
      } else {
        pimpl->numWaitingCCW++;
      }

    // wake students up for the arriving train, return the # of student embarked to the train
    }  or _Accept( arrive ) {
      bool isClockwise = (Train::Direction::Clockwise == pimpl->direction);
      unsigned int &numWaiting = isClockwise ? pimpl->numWaitingCW : pimpl->numWaitingCCW;
      uCondition &wWaiting = isClockwise ? pimpl->wWaitingCW : pimpl->wWaitingCCW;

      if (isClockwise) {
        pimpl->trainCW = pimpl->train;
      } else {
        pimpl->trainCCW = pimpl->train;
      }

      pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 'A', pimpl->trainId, pimpl->maxNumStudents, numWaiting);
      unsigned int canTake = pimpl->maxNumStudents > numWaiting ? numWaiting : pimpl->maxNumStudents;

      for (unsigned int i = 0; i < canTake; ++i) {
        wWaiting.signalBlock();
        numWaiting--;
      }

      *(pimpl->numEmbarked) = canTake;

    // print disembarking
    } or _Accept( disembark ) {
      pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 'D', pimpl->studentId);

    // wake trains up, and advance the system.
    } or _Accept( tick ) {
      pimpl->prt.print(Printer::Kind::TrainStop, pimpl->id, 't');

      while (!pimpl->wTrain.empty()) {
        pimpl->wTrain.signalBlock();
      }
    }
  }
}

void TrainStop::buy( unsigned int numStops, WATCard &card ) {
  pimpl->caller = &uThisTask();
  pimpl->card = &card;
  pimpl->numStops = numStops;

  // wait for the paying process
  pimpl->wBuying.wait();
}

void TrainStop::disembark( unsigned int studentId ) {
  pimpl->studentId = studentId;
}

Train *TrainStop::wait(unsigned int studentId, Train::Direction direction) {
  pimpl->direction = direction;
  pimpl->studentId = studentId;

  // wait for the train for my direction
  if (direction == Train::Direction::Clockwise) {
    pimpl->wWaitingCW.wait();
    return pimpl->trainCW;
  } else {
    pimpl->wWaitingCCW.wait();
    return pimpl->trainCCW;
  }
}

unsigned int TrainStop::arrive( unsigned int trainId, Train::Direction direction,
                                unsigned int maxNumStudents ) {
  pimpl->trainId = trainId;
  pimpl->direction = direction;
  pimpl->maxNumStudents = maxNumStudents;
  pimpl->train = static_cast<Train*>(&uThisTask());

  unsigned int numEmbarked;
  pimpl->numEmbarked = &numEmbarked;

  // wait for timer to tick
  pimpl->wTrain.wait();

  return numEmbarked;
}

void TrainStop::tick() { }