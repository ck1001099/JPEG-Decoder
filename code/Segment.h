#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include <iostream>
#include <vector>
using namespace std;

class JPEGCodec;

class Segment{
public:
	Segment(JPEGCodec* jpegCodec){
		this->jpegCodec = jpegCodec;
	}
	virtual ~Segment() = default;
	virtual bool Decode(vector<unsigned char>&, int&) = 0;
	virtual void Display() = 0;

protected:
	JPEGCodec* jpegCodec;
private:

};

#endif