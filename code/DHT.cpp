#include <iostream>
#include <iomanip>
#include <string>
#include "JPEGCodec.h"
#include "DHT.h"
using namespace std;

void HuffmanTable::BuildHuffmanTable(int level, int indexOfSymbols){
	//cout << level << " " << indexOfSymbols << endl;
	if (level == 0){
		qBefore.push("0");
		qBefore.push("1");
		BuildHuffmanTable(1, 0);
	} else {
		for (int i = 0 ; i < numberOfSymbols[level-1] ; i++){
			string codeWord = qBefore.front();
			qBefore.pop();
			huffmanTable[codeWord] = symbols[indexOfSymbols];
			indexOfSymbols = indexOfSymbols + 1;
		}
		while (!qBefore.empty()){
			string codeWord = qBefore.front();
			qBefore.pop();
			qAfter.push(codeWord + "0");
			qAfter.push(codeWord + "1");
		}
		if (indexOfSymbols < sumOfSymbols){
			qBefore.swap(qAfter);
			BuildHuffmanTable(level+1, indexOfSymbols);	
		}
	}
}

unsigned char DHT::marker[2] = { 0xFF, 0xC4 };

DHT::DHT(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

bool DHT::Decode(vector<unsigned char>& image, int& pos){
	int posInit = pos;

	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	while (pos != posInit + length + 2){
		HuffmanTable ht;

		// Get HT information
		ht.id = (image[pos] & 0x0F);
		ht.type = (image[pos] & 0xF0) >> 4;
		pos = pos + 1;

		// Get Number of symbols
		for (int i = 0 ; i < 16 ; i++, pos++){
			ht.numberOfSymbols[i] = image[pos];
		}

		// Get Symbols
		ht.sumOfSymbols = 0;
		for (int i = 0 ; i < 16 ; i++){
			ht.sumOfSymbols = ht.sumOfSymbols + ht.numberOfSymbols[i];
		}
		for (int i = 0 ; i < ht.sumOfSymbols ; i++, pos++){
			ht.symbols.push_back((int)image[pos]);
			// cout << ht.symbols[i] << " " << (int)image[pos] << endl;
		}

		ht.BuildHuffmanTable(0, 0);

		huffmanTables.push_back(ht);

		jpegCodec->InsertHuffmanTable(ht.id, ht.type, ht.huffmanTable);
	}

	return true;
}

void DHT::Display(){
	cout << "DHT segment" << endl;
	cout << "Length: " << length << endl;
	for (int t = 0 ; t < huffmanTables.size() ; t++){
		cout << "Th: " << huffmanTables[t].id << ", Tc: " << huffmanTables[t].type << endl;
		cout << "Number of symbols: ";
		for (int i = 0 ; i < 16 ; i++){
			cout << setw(3) << huffmanTables[t].numberOfSymbols[i] << " ";
		}
		cout << endl;
		cout << "Symbols: ";
		for (int i = 0 ; i < huffmanTables[t].sumOfSymbols ; i++){
			cout << setw(3) << huffmanTables[t].symbols[i] << " ";
		}
		cout << endl;
		cout << "Table: " << endl;
		for (map<string, int>::iterator iter = huffmanTables[t].huffmanTable.begin() ; iter != huffmanTables[t].huffmanTable.end() ; iter++){
			cout << iter->first << " " << iter->second << endl;
		}
		cout << endl;
	}
}