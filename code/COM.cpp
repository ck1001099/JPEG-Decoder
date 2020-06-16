#include <iostream>
#include "COM.h"

unsigned char COM::marker[2] = { 0xFF, 0xFE };

COM::COM(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

COM::~COM(){
	if (comment != NULL){
		delete[] comment;
	}
}

bool COM::Decode(vector<unsigned char>& image, int& pos){
	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	// Get Comment
	int commentLength = length-2;
	comment = new char[commentLength+1];
	for (int i = 0 ; i < commentLength ; i++, pos++){
		comment[i] = image[pos];
	}
	comment[commentLength] = '\0';
	//Display();
	return true;
}

void COM::Display(){
	cout << "COM segment" << endl;
	cout << "Length: " << length << endl;
	cout << "Comment: " << comment << endl;
}