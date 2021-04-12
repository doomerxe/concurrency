#include "nameserver.h"
#include "printer.h"
#include "trainstop.h"

class NameServer::PImpl {
 public:
  // members
  Printer &prt;
  unsigned int numStops;
  unsigned int numStudents;
  TrainStop **stopList;

  // communication variables
  unsigned int studentId;
  unsigned int trainStopId;
  unsigned int trainId;
  TrainStop *trainStopAddress;
  Printer::Kind getStopListCaller;

  PImpl(Printer &prt, unsigned int numStops, unsigned int numStudents):
    prt{prt}, numStops{numStops}, numStudents{numStudents}, stopList{new TrainStop*[numStops]} {}
  
  ~PImpl() {
    delete [] stopList;
  }
};

void NameServer::main() {
  pimpl->prt.print( Printer::Kind::NameServer, 'S');

  // wait for all stops to register themselves before the main loop
  for (unsigned int i = 0; i < pimpl->numStops; ++i) {
    _Accept( registerStop ) {
      pimpl->prt.print( Printer::Kind::NameServer, 'R', pimpl->trainStopId);
      pimpl->stopList[pimpl->trainStopId] = pimpl->trainStopAddress;
    } // _Accept
  } // for

  // wait for incomings and do the work
  while (true) {
    _Accept( ~NameServer ) {
      break;
    } or _Accept( getStop ) {
      pimpl->prt.print( Printer::Kind::NameServer, 'T', pimpl->studentId, pimpl->trainStopId );
    } or _Accept( getStopList ) {
      if ( pimpl->getStopListCaller == Printer::Kind::Train ) {
        pimpl->prt.print( Printer::Kind::NameServer, 'L', pimpl->trainId );
      } else {
        pimpl->prt.print( Printer::Kind::NameServer, 'L');
      } // if-else
    } or _Accept( getNumStops ) {} // _Accept
  } // while

  pimpl->prt.print( Printer::NameServer, 'F');
} // main

NameServer::NameServer( Printer & prt, unsigned int numStops, unsigned int numStudents ):
  pimpl{new PImpl{prt, numStops, numStudents}} {
} // NameServer

NameServer::~NameServer() {
  delete pimpl;
} // ~NameServer

void NameServer::registerStop( unsigned int trainStopId ) {
  pimpl->trainStopId = trainStopId;
  pimpl->trainStopAddress = static_cast<TrainStop*>(&uThisTask());
} // registerStop

TrainStop *NameServer::getStop( unsigned int studentId, unsigned int trainStopId ) {
  pimpl->studentId = studentId;
  pimpl->trainStopId = trainStopId;
  return pimpl->stopList[trainStopId];
} // getStop

TrainStop **NameServer::getStopList() {
  pimpl->getStopListCaller = Printer::Kind::Timer;
  return pimpl->stopList;
} // getStopList

TrainStop **NameServer::getStopList( unsigned int trainId ) {
  pimpl->getStopListCaller = Printer::Kind::Train;
  pimpl->trainId = trainId;
  return pimpl->stopList;
} // getStopList

unsigned int NameServer::getNumStops() {
  return pimpl->numStops;
} // getNumStops