#include "headers/nameserver.h"

class NameServer::PImpl {
  Printer &prt;
  unsigned int numStops;
  unsigned int numStudents;
  TrainStop **stopList;

  // communication variables
  unsigned int studentId;
  unsigned int trainStopId;

  PImpl(Printer &prt, unsigned int numStops, unsigned int numStudents):
    prt{prt}, numStops{numStops}, numStudents{numStudents}, stopList{new TrainStop*[numStops]} {}
};

void NameServer::main() {
  for (unsigned int i = 0; i < numStops; ++i) {
    _Accept( registerStop ) {

    }
  }
}

NameServer::NameServer( Printer & prt, unsigned int numStops, unsigned int numStudents ):
  pimpl{new PImpl{prt, numStops, numStudents}} {
}

NameServer::~NameServer() {
  delete pimpl;
}

void NameServer::registerStop( unsigned int trainStopId ) {

}

TrainStop *NameServer::getStop( unsigned int studentId, unsigned int trainStopId ) {

}

TrainStop **NameServer::getStopList() {
  return pimpl->stopList;
}

TrainStop **NameServer::getStopList( unsigned int trainId ) {
}

unsigned int NameServer::getNumStops() {
  return pimpl->numStops;
}