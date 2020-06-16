#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "JPEGCodec.h"
#include "SOS.h"
using namespace std;

unsigned char SOS::marker[2] = { 0xFF, 0xDA };
map<int, string> SOS::numberOfComponentsMap = {
	{1, "grey scaled"},
	{3, "color YCrCb or YIQ"},
	{4, "color CMYK"}
};

SOS::SOS(JPEGCodec* jpegCodec): Segment(jpegCodec){

}

bool SOS::Decode(vector<unsigned char>& image, int& pos){
	// Get length
	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	// Get Number of components
	numberOfComponents = image[pos];
	pos = pos + 1;

	// Get Components
	if (components != NULL){
		delete[] components;
	}
	components = new SOSComponent[numberOfComponents];
	for (int i = 0 ; i < numberOfComponents ; i++, pos=pos+2){
		components[i].componentID = image[pos];
		components[i].huffmanDCID = (image[pos+1] & 0xF0) >> 4;
		components[i].huffmanACID = (image[pos+1] & 0x0F);
	}
	
	// Skip 3 bytes
	pos = pos + 3;
	
	DecodeCompressedData(image, pos);
	
	return true;
}

void SOS::Display(){
	cout << "SOS segment" << endl;
	cout << "Length: " << length << endl;
	cout << "Number of component: " << numberOfComponents << " (" << numberOfComponentsMap[numberOfComponents] << ")" << endl;
	for (int i = 0 ; i < numberOfComponents ; i++){
		cout << "  Component " << components[i].componentID << endl;
		cout << "     Huffman DC table ID: " << components[i].huffmanDCID << endl;
		cout << "     Huffman AC table ID: " << components[i].huffmanACID << endl;
	}
}

void SOS::DecodeCompressedData(vector<unsigned char>& image, int& pos){
	int bitIndex = 0;
	// Calculate the number of MCUs
	const int imageWidth = jpegCodec->GetImageWidth();
	const int imageHeight = jpegCodec->GetImageHeight();
	SOFComponent* sofComponents = jpegCodec->GetSOFComponent();

	int mcuNumX = imageHeight / (8 * sofComponents[0].verticalSamplingFactor) + (imageHeight % (8 * sofComponents[0].verticalSamplingFactor) != 0 ? 1 : 0);
	int mcuNumY = imageWidth / (8 * sofComponents[0].horizontalSamplingFactor) + (imageWidth % (8 * sofComponents[0].horizontalSamplingFactor) != 0 ? 1 : 0);
	// cout << mcuNumX << " x " << mcuNumY << endl;
	
	int* dcPredictor = new int[numberOfComponents]{0};

	for (int i = 0 ; i < mcuNumX ; i++){
		vector<MCU> m;
		for (int j = 0 ; j < mcuNumY ; j++){
			// cout << "MCU (" << i << "," << j << ")" << endl;
			MCU mcu(sofComponents, components);
			mcu.Decode(jpegCodec, image, pos, bitIndex, numberOfComponents, dcPredictor);
			m.push_back(mcu);
		}
		MCUs.push_back(m);
	}
	jpegCodec->SetMCUs(&MCUs);

	delete[] dcPredictor;

	if (bitIndex != 8){
		pos = pos + 1;
	}
}