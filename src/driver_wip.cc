#include <iostream>
#include <sstream>

#include "headers/config.h"
#include "headers/MPRNG.h"
#include "headers/printer.h"
#include "headers/bank.h"
#include "headers/parent.h"
#include "headers/cardoffice.h"
#include "headers/groupoff.h"
#include "headers/nameserver.h"
#include "headers/timer.h"
#include "headers/trainstop.h"
#include "headers/train.h"
#include "headers/conductor.h"
#include "headers/student.h"

using namespace std;

// convert C string to integer
static bool convert( int & val, char * buffer ) {
  stringstream ss( buffer );
  string temp;
  ss >> dec >> val;
  return !ss.fail() && !( ss >> temp );
} // convert

// print the usage and exit
void usage( char *argv[] ) {
    cerr << "Usage: " << argv[0] << " [ config-file [ seed ] ] ";
    exit( EXIT_FAILURE ); 
} // usage

int main( int argc, char *argv[] ) {
  int seed = getpid();
  char *configFile = NULL;

  try {
    switch ( argc ) {
      case 3:
        if ( !convert( seed, argv[2] ) || seed < 1) throw 1;
      case 2:
        configFile = argv[1];
      case 1:
        break;
    } // switch
  } catch ( ... ) {
    usage( argv );
  } // try-catch

  ConfigParms cparms;
  processConfigFile( configFile?configFile:"lrt.config" , cparms );
  mprng.set_seed(seed);

  const unsigned int numTrains = 2;
  const unsigned int maxTripCost = cparms.stopCost * (cparms.numStops / 2);

  Printer printer{ cparms.numStudents, numTrains, cparms.numStops, cparms.numCouriers };
  Bank bank{ cparms.numStudents };
  Parent *parent = new Parent{ printer, bank, cparms.numStudents, cparms.parentalDelay, maxTripCost };
  WATCardOffice office{ printer, bank, cparms.numCouriers };
  Groupoff groupoff{ printer, cparms.numStudents, maxTripCost, cparms.groupoffDelay };
  NameServer nameServer{ printer, cparms.numStops, cparms.numStudents };
  Timer * timer = new Timer{ printer, nameServer, cparms.timerDelay };
  
  TrainStop *trainStops[cparms.numStops];
  Train *trains[numTrains];
  Conductor *conductors[numTrains];
  Student *students[cparms.numStudents];

  for ( unsigned int i = 0; i < cparms.numStops; ++i ) {
    trainStops[i] = new TrainStop{ printer, nameServer, i, cparms.stopCost };
  }

  for ( unsigned int i = 0; i< numTrains; ++i ) {
    trains[i] = new Train{ printer, nameServer, i, cparms.maxNumStudents, cparms.numStops };
  }

  for ( unsigned int i = 0; i < numTrains; ++i ) {
    conductors[i] = new Conductor{ printer, i, trains[i], cparms.conductorDelay };
  }

  for ( unsigned int i = 0; i < cparms.numStudents; ++i ) {
    students[i] = new Student{ printer, nameServer, office, groupoff, i, cparms.numStops,
                               cparms.stopCost, cparms.maxStudentDelay, cparms.maxStudentTrips};
  }

  for ( unsigned int i = 0; i < cparms.numStudents; ++i ) {
    delete students[i];
  }

  for ( unsigned int i = 0; i < numTrains; ++i ) {
    delete conductors[i];
  }
  
  for ( unsigned int i = 0; i< numTrains; ++i ) {
    delete trains[i];
  }

  delete timer;

  for ( unsigned int i = 0; i < cparms.numStops; ++i ) {
    delete trainStops[i];
  }

  delete parent;
  return 0;
} // main