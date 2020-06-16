#ifndef _SOF_H_
#define _SOF_H_

#include <string>
#include <map>
#include "Segment.h"

class SOFComponent{
public:
	int componentID; // 1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q
	int verticalSamplingFactor;
	int horizontalSamplingFactor;
	int quantizationTableNumber;
};

class SOF: public Segment{
public:
	SOF(JPEGCodec*);
	~SOF();
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	static map<int, string> numberOfComponentsMap;
	int length;
	int dataPrecision;
	int imageHeight;
	int imageWidth;
	int numberOfComponents; // Usually 1 = grey scaled, 3 = color YCrCb or YIQ, 4 = color CMYK
	SOFComponent* components = NULL;
};

#endif