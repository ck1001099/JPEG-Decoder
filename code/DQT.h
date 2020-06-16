#ifndef _DQT_H_
#define _DQT_H_

#include <vector>
#include "Segment.h"
using namespace std;

class QuantizationTable{
public:
	int id; // Tq (number of QT)
	int precision; // Pq (precision of QT)
	int table[64];
};

class DQT: public Segment{
public:
	DQT(JPEGCodec*);
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	int length;
	vector<QuantizationTable> quantizationTables;
};

#endif