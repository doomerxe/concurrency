#include "timer.h"
#include "trainstop.h"
#include "nameserver.h"
#include "printer.h"

class Timer::PImpl {
 public:
  Printer &prt;
  NameServer &server;
  unsigned int delay;
  unsigned int tickNumber = 0;

  PImpl(Printer &prt, NameServer &server, unsigned int delay):
    prt{prt}, server{server}, delay{delay} {} // PImpl
}; // Timer::PImpl

void Timer::main() {
  pimpl->prt.print(Printer::Kind::Timer, 'S');
  TrainStop **trainStops = pimpl->server.getStopList();
  unsigned int numStops = pimpl->server.getNumStops();

  while (true) {
    _Accept( ~Timer ) {
      break;
    } _Else {
      yield(pimpl->delay);
      pimpl->prt.print(Printer::Kind::Timer, 't', pimpl->tickNumber);
      pimpl->tickNumber++;
      for (unsigned int i = 0; i < numStops; ++i) {
        trainStops[i]->tick();
      } // for
    } // _Accept-_Else
  } // while

  pimpl->prt.print(Printer::Kind::Timer, 'F');
} // main

Timer::Timer(Printer &prt, NameServer &nameServer, unsigned int timerDelay):
  pimpl{new PImpl{prt, nameServer, timerDelay}} {}

Timer::~Timer() {
  delete pimpl;
}