#include <iostream>
#include <vector>
#include <map>
#include "DRI.h"

unsigned char DRI::marker[2] = { 0xFF, 0xDD };

DRI::DRI(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

DRI::~DRI(){
	
}

bool DRI::Decode(vector<unsigned char>& image, int& pos){
	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	// Get Restart Interval
	restartInterval	= image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	return true;
}

void DRI::Display(){
	cout << "DRI segment" << endl;
	cout << "Length: " << length << endl;
	cout << "Restart interval: " << restartInterval << endl;
}