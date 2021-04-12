#include <iostream>
#include <cmath>

#include "printer.h"
#include "watcard.h"
#include "train.h"
#include "trainstop.h"
#include "nameserver.h"
#include "MPRNG.h"
using namespace std;

extern MPRNG mprng;

struct SeatInfo {
  uCondition wait;
  bool freerider = false;       // am I caught by the conductor

  // communication from student to train
  bool inUse = false;           // if this seat is occupied
  WATCard *card = nullptr;      // my watcard
  uBaseTask *student;           // myself, can be _Resumed _At

  // communication from train to student
  TrainStop **dest;             // The train tells me when I get off.
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
  struct SeatInfo * seats;

  unsigned int studentId;

  PImpl(Printer & prt, NameServer & nameServer, unsigned int id, unsigned int maxNumStudents, unsigned int numStops) : 
    printer(prt), nameServer(nameServer), id(id), maxNumStudents(maxNumStudents), numStops(numStops) {
    if (id == 0) {
      direction = Direction::Clockwise;
      curStop = 0;
    } else {
      direction = Direction::CounterClockwise;
      curStop = ceil(numStops / 2);
    }

    seats = new SeatInfo[maxNumStudents];
    curSize = 0;
  }

  ~PImpl() {
    delete [] seats;
  }
};

Train::Train(Printer & prt, NameServer & nameServer, unsigned int id, unsigned int maxNumStudents,
             unsigned int numStops):
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
  TrainStop *myStop;
  pimpl->studentId = studentId;

  // find myself a seat
  int index = -1;
  for (int i = 0; i < (int) pimpl->maxNumStudents; ++i) {
    if (!(pimpl->seats[i].inUse)) {
      pimpl->seats[i].student = &uThisTask();
      pimpl->seats[i].dest = &myStop;
      pimpl->seats[i].card = &card;
      pimpl->seats[i].inUse = true;
      index = i;
      break;
    }
  }

  // wait until I am ejected/at my deststop
  pimpl->seats[index].wait.wait(destStop);

  return myStop;
}

void Train::scanPassengers() {}

void Train::main() {
  TrainStop ** stopList = pimpl->nameServer.getStopList(pimpl->id);
  for (;;) {
    _Accept (~Train) {
      break;

    // accepting conductors' call to scan, mark student as freerider when not paid
    } or _Accept (scanPassengers) {       
      for (int i = 0; i < (int) pimpl->maxNumStudents; ++i) {
        if (pimpl->seats[i].inUse) {
          if (!pimpl->seats[i].card->paidForTicket()) pimpl->seats[i].freerider = true;
        } 
      }
    } _Else {
      TrainStop * stop = stopList[pimpl->curStop];
      unsigned int cur = pimpl->curSize;
      
      // let students get off the train
      for (int i = 0; i < (int) pimpl->maxNumStudents; ++i) {
        if (!pimpl->seats[i].wait.empty()) {
          // reset seat, POP, throw exception for free riders
          if (pimpl->seats[i].freerider) {
            pimpl->seats[i].inUse = false;
            pimpl->seats[i].freerider = false;
            pimpl->seats[i].card = nullptr;
            --pimpl->curSize;
            _Resume Ejected() _At *(pimpl->seats[i].student);
            pimpl->seats[i].student = nullptr;
            pimpl->seats[i].wait.signalBlock();

          // inform other students they are at their dest and reset seat.
          } else if (pimpl->seats[i].wait.front() == pimpl->curStop) {
            pimpl->seats[i].inUse = false;
            pimpl->seats[i].freerider = false;
            pimpl->seats[i].card->resetPOP();
            pimpl->seats[i].card = nullptr;
            pimpl->seats[i].student = nullptr;
            --pimpl->curSize;
            *(pimpl->seats[i].dest) = stop;
            pimpl->seats[i].wait.signalBlock();
          } 
        }
      }
      
      // arrive at the station.
      unsigned int avail = pimpl->maxNumStudents - pimpl->curSize;
      pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'A', pimpl->curStop, avail, cur);
      unsigned int taken = stop->arrive(pimpl->id, pimpl->direction, avail);

      // Let students embark
      for (unsigned int i = 0; i < taken; ++i) {
        _Accept (embark) {
          pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'E', pimpl->studentId, pimpl->curStop);
          ++pimpl->curSize;
        }
      }

      // go to my next stop
      if (pimpl->direction == Direction::Clockwise) {
        if (pimpl->curStop == pimpl->numStops - 1) {
          pimpl->curStop = 0;
        } else {
          ++pimpl->curStop;
        }
      } else {
        if (pimpl->curStop == 0) {
          pimpl->curStop = pimpl->numStops - 1;
        } else {
          --pimpl->curStop;
        }                
      }
    }
  }
  
  pimpl->printer.print(Printer::Kind::Train, pimpl->id, 'F');
}
