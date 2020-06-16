#ifndef _JPEGCodec_H_
#define _JPEGCodec_H_

#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include "Utility.h"
#include "Segment.h"
#include "APP0.h"
#include "APPn.h"
#include "DRI.h"
#include "COM.h"
#include "DQT.h"
#include "SOF.h"
#include "DHT.h"
#include "SOS.h"
#include "MCU.h"
using namespace std;

class Pixel{
public:
	int R;
	int G;
	int B;
	int Y;
	int Cb;
	int Cr;
};

class JPEGCodec{
public:
	JPEGCodec();
	~JPEGCodec();
	void Read(string, bool display = false);
	void SaveToBMP(string);
	void InsertHuffmanTable(int, int, map<string, int>);
	void InsertQuantizationTable(int, int*);
	int GetSymbolFromHuffmanTable(int, int, string);
	void SetSOFComponent(SOFComponent*);
	SOFComponent* GetSOFComponent();
	void SetImageWidth(int);
	int GetImageWidth();
	void SetImageHeight(int);
	int GetImageHeight();
	int GetHuffmanCoefficient(int, int);
	void SetMCUs(vector<vector<MCU>>*);
	void Log(string);
	void DisplayMCUBlock(int, int);
	void DisplayImageBlock(int, int, int);
private:
	ifstream ifs;
	ofstream ofs;
	vector<unsigned char> buffer;
	int pos;

	string imageName;
	int imageSize;
	int imageWidth;
	int imageHeight;

	vector<Segment*> segments;
	map<int, map<int, map<string, int>>> huffmanTables;
	map<int, vector<int>> quantizationTables;

	SOFComponent* sofComponents;

	vector<vector<int>> huffmanCoefficients;
	vector<vector<double>> cosineLookUpTable;

	vector<vector<MCU>>* MCUs;
	Pixel** image = NULL;

	void Dequantization();
	void IDCT();
	void Shift();
	void Upsampling();
	void ToRGB();

	static unsigned char SOI[2];
	static unsigned char EOI[2];

	bool CheckSOI();
	bool CheckEOI();
	void Finish();
};

#endif