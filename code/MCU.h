#ifndef _MCU_H_
#define _MCU_H_

#include <vector>
#include "JPEGCodec.h"
#include "SOS.h"
#include "SOF.h"
using namespace std;

class SOSComponent;

class DataUnit{
public:
	int dc;
	int ac[63];
};

class MCU{
public:
	MCU(SOFComponent*, SOSComponent*);
	void Decode(JPEGCodec*, vector<unsigned char>&, int&, int&, int, int*); // image, pos, bitIndex, numberOfComponents
	vector<vector<vector<DataUnit>>>* GetDataUnits();
private:
	// MCU index
	int xIndex;
	int yIndex;
	// Data unit
	vector<vector<vector<DataUnit>>> dataUnits; // componentID, V, H
	// Components
	SOFComponent* sofComponents;
	SOSComponent* sosComponents;
};

#endif