#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include "DQT.h"
#include "JPEGCodec.h"
#include "Utility.h"
using namespace std;

unsigned char DQT::marker[2] = { 0xFF, 0xDB };

DQT::DQT(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

bool DQT::Decode(vector<unsigned char>& image, int& pos){
	int posInit = pos;

	// Check marker
	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	while (pos != posInit + length + 2){
		QuantizationTable qt;

		// Get QT information
		qt.id = (image[pos] & 0x0F);
		qt.precision = (image[pos] & 0xF0) >> 4;
		pos = pos + 1;

		// Get table
		if (qt.precision == 0){
			for (int i = 0 ; i < 64 ; i++, pos++){
				qt.table[i] = image[pos];
			}	
		} else {
			for (int i = 0 ; i < 64 ; i++, pos=pos+2){
				qt.table[i] = image[pos] * 256 + image[pos + 1];
			}
		}
		
		ZicZac(qt.table, 64, 8, 8, 0, 0, false);

		quantizationTables.push_back(qt);

		jpegCodec->InsertQuantizationTable(qt.id, qt.table);
	}

	return true;
}

void DQT::Display(){
	cout << "DQT segment" << endl;
	cout << "Length: " << length << endl;
	for (int t = 0 ; t < quantizationTables.size() ; t++){
		cout << "Tq: " << quantizationTables[t].id << ", Pq: " << quantizationTables[t].precision << endl;
		cout << "Table" << endl;
		for (int i = 0 ; i < 8 ; i++){
			for (int j = 0 ; j < 8 ; j++){
				cout << setw(3) << quantizationTables[t].table[i*8+j] << " ";
			}
			cout << endl;
		}
	}
}