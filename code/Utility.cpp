#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include "Utility.h"
using namespace std;

string int_to_hex(int i){
	stringstream stream;
	stream << "0x" << std::setfill ('0') << std::setw(2) << hex << i;
	return stream.str();
}

void ZicZac(int* array, int num, int xBound, int yBound, int xInit, int yInit, bool dir){
	int* arrayZicZac = new int[xBound*yBound];

	int x = xInit;
	int y = yInit;

	for (int i = 0 ; i < num ; i++){
		arrayZicZac[yBound * x + y] = array[i];
		// cout << x << " " << y << " " << array[i] << " " << yBound * x + y << endl;
		if (dir){
			if (y == 0 || x == xBound - 1){
				dir = false;
			} else {
				y = y - 1;
			}
			if (x == 7){
				y = y + 1;
			} else {
				x = x + 1;	
			}
		} else {
			if (x == 0 || y == yBound - 1){
				dir = true;
			} else {
				x = x - 1;
			}
			if (y == 7){
				x = x + 1;	
			} else {
				y = y + 1;
			}
		}
	}
	for (int i = 0 ; i < num ; i++){
		array[i] = arrayZicZac[xBound * yBound - num + i];
	}

	delete[] arrayZicZac;
}