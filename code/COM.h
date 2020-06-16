#ifndef _COM_H_
#define _COM_H_

#include <vector>
#include <map>
#include <string>
#include "Segment.h"
using namespace std;

class COM: public Segment{
public:
	COM(JPEGCodec*);
	~COM();
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	int length;
	char* comment;
};

#endif