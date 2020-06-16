#ifndef _APP0_H_
#define _APP0_H_

#include <vector>
#include <map>
#include <string>
#include "Segment.h"
using namespace std;

//TODO: change name?
class APP0: public Segment{
public:
	APP0(JPEGCodec*);
	~APP0();
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	static map<int, string> densityUnitsMap;
	int length;
	char identifier[5];
	string JFIFversion;
	int densityUnits;
	int xDensity;
	int yDensity;
	int xThumbnail;
	int yThumbnail;
	int* thumbnailData = NULL;
	int thumbnailDataCount;
};

#endif