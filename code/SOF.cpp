#include <string>
#include <map>
#include "JPEGCodec.h"
#include "SOF.h"

unsigned char SOF::marker[2] = { 0xFF, 0xC0 };
map<int, string> SOF::numberOfComponentsMap = {
	{1, "grey scaled"},
	{3, "color YCrCb or YIQ"},
	{4, "color CMYK"}
};

SOF::SOF(JPEGCodec* jpegCodec): Segment(jpegCodec){
	
}

SOF::~SOF(){
	if (components != NULL){
		delete[] components;
	}
}

bool SOF::Decode(vector<unsigned char>& image, int& pos){
	if (image[pos] != marker[0] || image[pos+1] != marker[1]){
		return false;
	}
	pos = pos + 2;

	// Get Length
	length = image[pos] * 256 + image[pos+1];
	pos = pos + 2;

	// Get Data precision
	dataPrecision = image[pos];
	pos = pos + 1;

	// Get Height and Width
	imageHeight = image[pos] * 256 + image[pos+1];
	imageWidth = image[pos+2] * 256 + image[pos+3];
	pos = pos + 4;

	jpegCodec->SetImageWidth(imageWidth);
	jpegCodec->SetImageHeight(imageHeight);

	// Get Number of components
	numberOfComponents = image[pos];
	pos = pos + 1;

	// Get Components
	if (components != NULL){
		delete[] components;
	}
	components = new SOFComponent[numberOfComponents];
	for (int i = 0 ; i < numberOfComponents ; i++, pos=pos+3){
		components[i].componentID = image[pos];
		components[i].verticalSamplingFactor = (image[pos+1] & 0x0F);
		components[i].horizontalSamplingFactor = (image[pos+1] & 0xF0) >> 4;
		components[i].quantizationTableNumber = image[pos+2];
	}

	jpegCodec->SetSOFComponent(components);
	
	return true;
}

void SOF::Display(){
	cout << "SOF segment" << endl;
	cout << "Length: " << length << endl;
	cout << "Data precision: " << dataPrecision << endl;
	cout << "Image size: " << imageWidth << " x " << imageHeight << endl;
	cout << "Number of component: " << numberOfComponents << " (" << numberOfComponentsMap[numberOfComponents] << ")" << endl;
	for (int i = 0 ; i < numberOfComponents ; i++){
		cout << "  Component " << components[i].componentID << endl;
		cout << "     Sampling factor (Vertical)  : " << components[i].verticalSamplingFactor << endl;
		cout << "     Sampling factor (Horizontal): " << components[i].horizontalSamplingFactor << endl;
		cout << "     Quantization table number   : " << components[i].quantizationTableNumber << endl;
	}
}