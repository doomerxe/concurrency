#include "headers/nameserver.h"
#include "headers/printer.h"
#include "headers/trainstop.h"

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

  for (unsigned int i = 0; i < pimpl->numStops; ++i) {
    _Accept( registerStop ) {
      pimpl->prt.print( Printer::Kind::NameServer, 'R', pimpl->trainStopId);
      pimpl->stopList[pimpl->trainStopId] = pimpl->trainStopAddress;
    } // _Accept
  } // for

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
      }
    } or _Accept( getNumStops )
  } // while

  pimpl->prt.print( Printer::NameServer, 'F');
}

NameServer::NameServer( Printer & prt, unsigned int numStops, unsigned int numStudents ):
  pimpl{new PImpl{prt, numStops, numStudents}} {
}

NameServer::~NameServer() {
  delete pimpl;
}

void NameServer::registerStop( unsigned int trainStopId ) {
  pimpl->trainStopId = trainStopId;
  pimpl->trainStopAddress = static_cast<TrainStop*>(&uThisTask());
}

TrainStop *NameServer::getStop( unsigned int studentId, unsigned int trainStopId ) {
  pimpl->studentId = studentId;
  pimpl->trainStopId = trainStopId;
  return pimpl->stopList[trainStopId];
}

TrainStop **NameServer::getStopList() {
  pimpl->getStopListCaller = Printer::Kind::Timer;
  return pimpl->stopList;
}

TrainStop **NameServer::getStopList( unsigned int trainId ) {
  pimpl->getStopListCaller = Printer::Kind::Train;
  pimpl->trainId = trainId;
  return pimpl->stopList;
}

unsigned int NameServer::getNumStops() {
  return pimpl->numStops;
}