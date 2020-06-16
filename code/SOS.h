#ifndef _SOS_H_
#define _SOS_H_

#include <vector>
#include <map>
#include <string>
#include "Segment.h"
#include "MCU.h"

class SOSComponent{
public:
	int componentID; // 1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q
	int huffmanDCID;
	int huffmanACID;
};

class SOS: public Segment{
public:
	SOS(JPEGCodec*);
	bool Decode(vector<unsigned char>&, int&);
	void Display();
	void DecodeCompressedData(vector<unsigned char>&, int&);
private:
	static unsigned char marker[2];
	static map<int, string> numberOfComponentsMap;
	int length;
	int numberOfComponents; // Usually 1 = grey scaled, 3 = color YCrCb or YIQ, 4 = color CMYK
	SOSComponent* components = NULL;
	vector<vector<MCU>> MCUs;
};

#endif