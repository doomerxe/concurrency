#include <iostream>
#include <string>

#include "printer.h"
using namespace std;

const int totalType = 10;

struct Info {
    Printer::Kind kind;
    char state;
    unsigned int lid;
    unsigned int oid;
    unsigned int value1;
    unsigned int value2;
    char c;

    bool nameTimer;
    bool isFlushed;
};

class Printer::PImpl {
    public:
        // communicate
        unsigned int numStudents;
        unsigned int numTrains;
        unsigned int numStops;
        unsigned int numCouriers;

        // fields to help calculate index
        int total;
        int trainOffset;
        int condOffset;
        int stopOffset;
        int studOffset;

        // info list
        struct Info * printerInfo;

        PImpl(unsigned int numStudents, unsigned int numTrains, unsigned int numStops, unsigned int numCouriers):
            numStudents(numStudents), numTrains(numTrains), numStops(numStops), numCouriers(numCouriers) {
                //calculate total size & offsets
                total = totalType + (int) numTrains * 2 + (int) numStops + (int) numStudents + (int) numCouriers - 5;
                trainOffset = (int) numTrains - 1;
                condOffset = trainOffset + (int) numTrains - 1;
                stopOffset = condOffset + (int) numStops - 1;
                studOffset = stopOffset + (int) numStudents - 1;

                printerInfo = new Info[total];
                cout << "Parent\t" << "Gropoff\t" << "WATOff\t" << "Names\t" << "Timer\t";

                for (int i = 0; i < totalType; ++i) {
                    // single instance objects
                    if (i <= Kind::Timer) printerInfo[i].isFlushed = true;
                    
                    // multiple instance objects
                    else if (i == Kind::Train) {
                        for (int j = 0; j < (int) numTrains; ++j) {
                            printerInfo[i + j].isFlushed = true;
                            cout << "Train" << j << "\t";
                        }
                    }
                    else if (i == Kind::Conductor) {
                        for (int j = 0; j < (int) numTrains; ++j) {
                            printerInfo[i + trainOffset + j].isFlushed = true;
                            cout << "Cond" << j << "\t";
                        }
                    }
                    else if (i == Kind::TrainStop) {
                        for (int j = 0; j < (int) numStops; ++j) {
                            printerInfo[i + condOffset + j].isFlushed = true;
                            cout << "Stop" << j << "\t";
                        }
                    }
                    else if (i == Kind::Student) {
                        for (int j = 0; j < (int) numStudents; ++j) {
                            printerInfo[i + stopOffset + j].isFlushed = true;
                            cout << "Stud" << j << "\t";
                        }
                    }
                    else {
                        for (int j = 0; j < (int) numCouriers; ++j) {
                            printerInfo[i + studOffset + j].isFlushed = true;
                            cout << "WCour" << j;
                            if (j < (int) numCouriers - 1) cout << '\t';
                            else cout << endl;
                        }
                    }
                }

                for (int i = 0; i < total; ++i) {
                    cout << "*******";
                    if (i < total - 1) cout << "\t";
                    else cout << endl;
                }
            }
            
        ~PImpl() {
            flush();
            cout << "***********************" << endl;
            delete [] printerInfo;
        }

        
        int checkOffset(Kind kind) {
            if (kind == Kind::Conductor) return trainOffset;
            else if (kind == Kind::TrainStop) return condOffset;
            else if (kind == Kind::Student) return stopOffset;
            else if (kind == Kind::WATCardOfficeCourier) return studOffset;
            else return 0;
        }

        void flush() {
            //find the flushIndex
            int flushIndex = total - 1;
            for (int i = total - 1; i >=0; --i) {
                if (printerInfo[i].isFlushed == false) {
                    flushIndex = i;
                    break;
                }
            }

            //print msg
            for (int i = 0; i <= flushIndex; ++i) {
                if (printerInfo[i].isFlushed == false) {
                    if (printerInfo[i].state == 'S') {
                        if (printerInfo[i].kind == Kind::Train) 
                            cout << "S" << printerInfo[i].value1 << "," << printerInfo[i].c;
                        else if (printerInfo[i].kind == Kind::Student) 
                            cout << "S" << printerInfo[i].value1;
                        else cout << "S";
                    }
                    else if (printerInfo[i].state == 'F') cout << "F";
                    else if (printerInfo[i].state == 'D') {
                        if (printerInfo[i].kind == Kind::Parent) 
                            cout << "D" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                        else cout << "D" << printerInfo[i].value1;
                    }
                    else if (printerInfo[i].state == 'W') {
                        if (printerInfo[i].kind == Kind::WATCardOffice) cout << "W";
                        else if (printerInfo[i].kind == Kind::TrainStop)
                            cout << "W" << printerInfo[i].value1 << "," << printerInfo[i].c;
                        else if (printerInfo[i].kind == Kind::Student)
                            cout << "W" << printerInfo[i].value1;
                    }
                    else if (printerInfo[i].state == 'C')
                        cout << "C" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                    else if (printerInfo[i].state == 'T') {
                        if (printerInfo[i].kind == Kind::Student)
                            cout << "T" << printerInfo[i].value1 << "," << printerInfo[i].value2 << "," << printerInfo[i].c;
                        else 
                            cout << "T" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                    }
                    else if (printerInfo[i].state == 'R') 
                        cout << "R" << printerInfo[i].value1;
                    else if (printerInfo[i].state == 'L') {
                        if ((printerInfo[i].kind == Kind::NameServer && printerInfo[i].nameTimer == false) ||
                            printerInfo[i].kind == Kind::WATCardOfficeCourier)
                                cout << "L" << printerInfo[i].value1;
                        else cout << "L";
                    }
                    else if (printerInfo[i].state == 'A') 
                        cout << "A" << printerInfo[i].oid << "," << printerInfo[i].value1 << "," << printerInfo[i].value2;
                    else if (printerInfo[i].state == 'E'){
                        if (printerInfo[i].kind == Kind::Student)
                            cout << "E" << printerInfo[i].value1;
                        else cout << "E" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                    }
                    else if(printerInfo[i].state == 'B') {
                        if (printerInfo[i].kind == Kind::TrainStop)
                            cout << "B" << printerInfo[i].value1;
                        else cout << "B" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                    }
                    else if (printerInfo[i].state == 't') {
                        if (printerInfo[i].kind == Kind::TrainStop)
                            cout << "t";
                        else if (printerInfo[i].kind == Kind::WATCardOfficeCourier)
                            cout << "t" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                        else cout << "t" << printerInfo[i].value1;
                    }
                    else if (printerInfo[i].state == 'c') cout << "c";
                    else if (printerInfo[i].state == 'e') {
                        if (printerInfo[i].kind == Kind::Conductor) cout << "e" << printerInfo[i].value1;
                        else cout << "e";
                    }
                    else if (printerInfo[i].state == 'G') cout << "G" << printerInfo[i].value1 << "," << printerInfo[i].value2;
                    else if (printerInfo[i].state == 'f') cout << "f";

                    printerInfo[i].isFlushed = true;
                }
                if (i != flushIndex) cout << '\t';
            } 
            cout << endl;   
        }
};

Printer::Printer(unsigned int numStudents, unsigned int numTrains, unsigned int numStops, unsigned int numCouriers):
    pimpl(new PImpl(numStudents, numTrains, numStops, numCouriers)) {}

Printer::~Printer() {
    delete pimpl;
}

void Printer::print(Kind kind, char state) {
    if (!pimpl->printerInfo[kind].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind].kind = kind;
    pimpl->printerInfo[kind].state = state;
    pimpl->printerInfo[kind].isFlushed = false;

    if (kind == Kind::NameServer && state == 'L') pimpl->printerInfo[kind].nameTimer = true;
}

void Printer::print(Kind kind, char state, unsigned int value1) {
    if (!pimpl->printerInfo[kind].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind].kind = kind;
    pimpl->printerInfo[kind].state = state;
    pimpl->printerInfo[kind].value1 = value1;
    pimpl->printerInfo[kind].isFlushed = false;

    if (kind == Kind::NameServer && state == 'L') pimpl->printerInfo[kind].nameTimer = false;
}

void Printer::print(Kind kind, char state, unsigned int value1, unsigned int value2) {
    if (!pimpl->printerInfo[kind].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind].kind = kind;
    pimpl->printerInfo[kind].state = state;
    pimpl->printerInfo[kind].value1 = value1;
    pimpl->printerInfo[kind].value2 = value2;
    pimpl->printerInfo[kind].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state, unsigned int value1) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].value1 = value1;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state, unsigned int value1, unsigned int value2) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].value1 = value1;
    pimpl->printerInfo[kind + offset + lid].value2 = value2;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state, unsigned int oid, unsigned int value1, unsigned int value2) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].oid = oid;
    pimpl->printerInfo[kind + offset + lid].value1 = value1;
    pimpl->printerInfo[kind + offset + lid].value2 = value2;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state, char c) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].c = c;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state, unsigned int value1, char c) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].value1 = value1;
    pimpl->printerInfo[kind + offset + lid].c = c;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}

void Printer::print(Kind kind, unsigned int lid, char state, unsigned int value1, unsigned int value2, char c) {
    int offset = pimpl->checkOffset(kind);
    if (!pimpl->printerInfo[kind + offset + lid].isFlushed) pimpl->flush();
    pimpl->printerInfo[kind + offset + lid].kind = kind;
    pimpl->printerInfo[kind + offset + lid].state = state;
    pimpl->printerInfo[kind + offset + lid].value1 = value1;
    pimpl->printerInfo[kind + offset + lid].value2 = value2;
    pimpl->printerInfo[kind + offset + lid].c = c;
    pimpl->printerInfo[kind + offset + lid].isFlushed = false;
}
