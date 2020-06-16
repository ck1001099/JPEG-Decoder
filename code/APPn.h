#ifndef _APPn_H_
#define _APPn_H_

#include <vector>
#include <map>
#include <string>
#include "Segment.h"
using namespace std;

class APPn: public Segment{
public:
	APPn(JPEGCodec*);
	~APPn();
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	int length;
};

#endif