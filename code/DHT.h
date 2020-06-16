#ifndef _DHT_H_
#define _DHT_H_

#include <vector>
#include <map>
#include <queue>
#include <string>
#include "Segment.h"
using namespace std;

class HuffmanTable{
public:
	int id; // Th (number of HT)
	int type; // Tc (type of HT, 0 = DC table, 1 = AC table)
	int numberOfSymbols[16];
	int sumOfSymbols;
	vector<int> symbols;
	void BuildHuffmanTable(int, int);
	map<string, int> huffmanTable; // code word -> symbol
	queue<string> qBefore;
	queue<string> qAfter;
};

class DHT: public Segment{
public:
	DHT(JPEGCodec*);
	bool Decode(vector<unsigned char>&, int&);
	void Display();
private:
	static unsigned char marker[2];
	int length;
	vector<HuffmanTable> huffmanTables;
};

#endif