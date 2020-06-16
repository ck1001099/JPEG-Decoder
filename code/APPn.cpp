#include <iostream>
#include <vector>
#include <map>
#include "APPn.h"

unsigned char APPn::marker[2] = { 0xFF, 0xE0 };

APPn::APPn(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

APPn::~APPn(){
	
}

bool APPn::Decode(vector<unsigned char>& image, int& pos){
	if (image[pos] != marker[0] || (image[pos+1] & 0xF0) != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	pos = pos + length - 2;

	return true;
}

void APPn::Display(){
	cout << "APPn segment" << endl;
	cout << "Length: " << length << endl;
}