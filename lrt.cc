_Monitor Bank {
  public:
	Bank( unsigned int numStudents );
	~Bank();
	void deposit( unsigned int id, unsigned int amount );
	void withdraw( unsigned int id, unsigned int amount );
}; 

_Task WATCardOffice {
	void main();
  public:
	struct Job {										// marshalled arguments and return future
		Args args;
		WATCard::FWATCard result;

		Job( Args args ) : args( args ) {}
	}; 

	_Task Courier {
	  public:
		Courier( ... );
	}; 

	_Event Lost {};										// lost WATCard
	WATCardOffice( Printer & prt, Bank & bank, unsigned int numCouriers );
	WATCard::FWATCard create( unsigned int sid, unsigned int amount ); 
	WATCard::FWATCard transfer( unsigned int sid, unsigned int amount, WATCard * card ); 
	Job * requestWork();
}; // WATCardOffice

_Task Conductor {
	void main();
  public:
  	Conductor( Printer & prt, unsigned int id, Train * train, unsigned int delay );
};

struct ConfigParms {
	unsigned int stopCost;								// amount to charge per train stop
	unsigned int numStudents;							// number of students to create
	unsigned int numStops;								// number of train stops; minimum of 2
	unsigned int maxNumStudents;						// maximum students each train can carry
	unsigned int timerDelay;							// length of time between each tick of the timer
	unsigned int maxStudentDelay;						// maximum random student delay between trips
	unsigned int maxStudentTrips;						// maximum number of train trips each student takes
	unsigned int groupoffDelay;							// length of time between initializing gift cards
	unsigned int conductorDelay;						// length of time between checking on passenger POPs
	unsigned int parentalDelay;							// length of time between cash deposits
	unsigned int numCouriers;							// number of WATCard office couriers in the pool
}; // ConfigParms

void processConfigFile( const char * configFile, ConfigParms & cparms );

static bool comments( ifstream & in, string & name ) {
	for ( ;; ) {
		in >> name;
	  if ( in.fail() ) return true;
	  if ( name.substr(0,1) != "#" ) break;
		in.ignore( numeric_limits<int>::max(), '\n' );	// ignore remainder of line
	} // for
	return false;
} // comments

// Process the configuration file to set the simulation parameters.
void processConfigFile( const char *configFile, ConfigParms & cparms ) {
	enum { Parmnum = 11 };
	struct {
		const char * name;								// configuration name
		bool used;										// already supplied ?
		unsigned int & value;							// location to put configuration value
	} static parms[Parmnum] = {
		{ "StopCost", false, cparms.stopCost },
		{ "NumStudents", false, cparms.numStudents },
		{ "NumStops", false, cparms.numStops },
		{ "MaxNumStudents", false, cparms.maxNumStudents },
		{ "TimerDelay", false, cparms.timerDelay },
		{ "MaxStudentDelay", false, cparms.maxStudentDelay },
		{ "MaxStudentTrips", false, cparms.maxStudentTrips },
		{ "GroupoffDelay", false, cparms.groupoffDelay },
		{ "ConductorDelay", false, cparms.conductorDelay },
		{ "ParentalDelay", false, cparms.parentalDelay },
		{ "NumCouriers", false, cparms.numCouriers },
	};
	string name;
	int value;
	unsigned int cnt, posn, numOfParm = 0;

	try {
		ifstream in( configFile );						// open the configuration file for input

		for ( cnt = 0 ; cnt < Parmnum; cnt += 1 ) {		// parameter names can appear in any order
		  if ( comments( in, name ) ) break;			// eof ?
			for ( posn = 0; posn < Parmnum && name != parms[posn].name; posn += 1 ); // linear search
		  if ( posn == Parmnum ) break;					// configuration not found ?
		  if ( parms[posn].used ) break;				// duplicate configuration ?
			in >> value;
			if ( value < 0 ) {
				cerr << "Error: file \"" << configFile << "\" parameter " << name
					 << " value " << value << " must be non-negative." << endl;
				exit( EXIT_FAILURE );
			} // if
		  if ( in.fail() ) break;
			in.ignore( numeric_limits<int>::max(), '\n' ); // ignore remainder of line
			numOfParm += 1;
			parms[posn].used = true;
			parms[posn].value = value;
		} // for

		if ( numOfParm != Parmnum ) {
			cerr << "Error: file \"" << configFile << "\" is corrupt." << endl;
			exit( EXIT_FAILURE );
		} // if
		if ( ! comments( in, name ) ) {					// ! eof ?
			cerr << "Error: file \"" << configFile << "\" has extraneous data." << endl;
			exit( EXIT_FAILURE );
		} // if
	} catch( uFile::Failure & ) {
		cerr << "Error: could not open input file \"" << configFile << "\"" << endl;
		exit( EXIT_FAILURE );	
	} // try
} // processConfigFile

_Task Groupoff {
	void main();
  public:
	Groupoff( Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay );
	~Groupoff();
	WATCard::FWATCard giftCard();
};

_Task NameServer {
	void main();
  public:
	NameServer( Printer & prt, unsigned int numStops, unsigned int numStudents );
	~NameServer();
	void registerStop( unsigned int trainStopId );
	TrainStop * getStop( unsigned int studentId, unsigned int trainStopId );
	TrainStop ** getStopList();
	TrainStop ** getStopList( unsigned int trainId );
	unsigned int getNumStops();
};

_Task Parent {
	void main();
  public:
	Parent( Printer & prt, Bank & bank, unsigned int numStudents, unsigned int parentalDelay, unsigned int maxTripCost );
}; 

_Monitor Printer {
  public:
	enum Kind { Parent, Groupoff, WATCardOffice, NameServer, Timer, Train, Conductor, TrainStop, Student, WATCardOfficeCourier };
	Printer( unsigned int numStudents, unsigned int numTrains, unsigned int numStops, unsigned int numCouriers );
	~Printer();
	void print( Kind kind, char state );
	void print( Kind kind, char state, unsigned int value1 );
	void print( Kind kind, char state, unsigned int value1, unsigned int value2 );
	void print( Kind kind, unsigned int lid, char state );
	void print( Kind kind, unsigned int lid, char state, unsigned int value1 );
	void print( Kind kind, unsigned int lid, char state, unsigned int value1, unsigned int value2 );
	void print( Kind kind, unsigned int lid, char state, unsigned int oid, unsigned int value1, unsigned int value2 );
	void print( Kind kind, unsigned int lid, char state, char c );
	void print( Kind kind, unsigned int lid, char state, unsigned int value1, char c );
	void print( Kind kind, unsigned int lid, char state, unsigned int value1, unsigned int value2, char c );
}; 

_Task Student {
    void main();
  public:
    Student( Printer & prt, NameServer & nameServer, WATCardOffice & cardOffice, Groupoff & groupoff, 
        unsigned int id, unsigned int numStops, unsigned int stopCost, unsigned int maxStudentDelay, unsigned int maxStudentTrips );
};

_Task Timer {
    void main();
  public:
    Timer( Printer & prt, NameServer & nameServer, unsigned int timerDelay );
};

_Task Train {
	void main();
  public:
	enum Direction { Clockwise, CounterClockwise };
	_Event Ejected {};						// Exception raised at student without ticket
	Train( Printer & prt, NameServer & nameServer, unsigned int id, unsigned int maxNumStudents, unsigned int numStops );
	~Train();
	_Nomutex unsigned int getId() const;
	TrainStop * embark( unsigned int studentId, unsigned int destStop, WATCard& card );
	void scanPassengers();
};

_Task TrainStop {
	void main();
  public:
	_Event Funds {										// Thrown when there are insufficient funds on the card to buy a ticket.
	  public:
		unsigned int amount;
		Funds( unsigned int amt );
	};

	TrainStop( Printer & prt, NameServer & nameServer, unsigned int id, unsigned int stopCost );
	_Nomutex unsigned int getId() const;
	void buy( unsigned int numStops, WATCard & card );
	Train * wait( unsigned int studentId, Train::Direction direction );
	void disembark( unsigned int studentId );
	void tick();
	unsigned int arrive( unsigned int trainId, Train::Direction direction, unsigned int maxNumStudents );
};

#include <uFuture.h>

class WATCard {
	WATCard( const WATCard & ) = delete;				// prevent copying
	WATCard & operator=( const WATCard & ) = delete;

  public:
	typedef Future_ISM<WATCard *> FWATCard;				// future watcard pointer
	WATCard();
	void deposit( unsigned int amount );
	void withdraw( unsigned int amount );
	unsigned int getBalance();
	bool paidForTicket();
  	void resetPOP();
};
