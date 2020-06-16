#ifndef _DRI_H_
#define _DRI_H_

#include <vector>
#include <map>
#include <string>
#include "Segment.h"
using namespace std;

class DRI: public Segment{
public:
	DRI(JPEGCodec*);
	~DRI();
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	int length;
	int restartInterval;
};

#endif