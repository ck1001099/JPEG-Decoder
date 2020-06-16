#include <vector>
#include "JPEGCodec.h"
#include "SOF.h"
#include "SOS.h"
#include "MCU.h"
using namespace std;

MCU::MCU(SOFComponent* sofComponents, SOSComponent* sosComponents){
	this->sofComponents = sofComponents;
	this->sosComponents = sosComponents;
}

void MCU::Decode(JPEGCodec* jpegCodec, vector<unsigned char>& image, int& pos, int& bitIndex, int numberOfComponents, int* dcPredictor){
	for (int componentIndex = 0 ; componentIndex < numberOfComponents ; componentIndex++){
		vector<vector<DataUnit>> d1;
		for (int vIndex = 0 ; vIndex < sofComponents[componentIndex].verticalSamplingFactor ; vIndex++){
			vector<DataUnit> d2;
			for (int hIndex = 0 ; hIndex < sofComponents[componentIndex].horizontalSamplingFactor ; hIndex++){
				DataUnit dataUnit;

				// cout << componentIndex << " " << vIndex << " " << hIndex << endl << endl;

				// variables
				int tableType = 0;
				string codeWord = "";
				int symbol = -1;
				int valueSize = 0;
				int valueIndex = 0;
				int value = 0;
				int run = 0;
				int acCount = 0;
				bool isEnd = false;

				// Get DC/AC data of data unit
				while (true){
					// Check 0xFF
					if (image[pos] == 0xFF){
						if ((image[pos+1] & 0xF8) == 0xD0){
							//cout << "RST: " << pos << " " << int_to_hex(image[pos+1]) << endl;
							pos = pos + 2;
							bitIndex = 0;
							codeWord = "";
							for (int i = 0 ; i < numberOfComponents ; i++){
								dcPredictor[i] = 0;
							}
							continue;
						} else if (image[pos+1] != 0x00){
							break;
						}
					}
					// Next byte
					if (bitIndex == 8){
						bitIndex = 0;
						if (image[pos] == 0xFF){
							// cout << pos << " " << int_to_hex(image[pos]) << endl;
							pos = pos + 2;
						} else {
							pos = pos + 1;
						}
					}
					// cout << pos << " " << int_to_hex(image[pos]) << endl;

					// Get code word while the symbol hasn't been found
					if (symbol == -1){
						while (bitIndex < 8){
							codeWord = codeWord.append(1, '0' + ((unsigned char)(image[pos] << bitIndex) >> 7));
							bitIndex = bitIndex + 1;
							// cout << codeWord << " ";

							// Check whether codeWord exists in Huffman table
							symbol = jpegCodec->GetSymbolFromHuffmanTable(tableType == 0 ? sosComponents[componentIndex].huffmanDCID : sosComponents[componentIndex].huffmanACID, tableType, codeWord);
							// cout << int_to_hex(symbol) << endl;

							if (symbol != -1){
								if (tableType == 0){
									valueSize = symbol;
								} else if (tableType == 1){
									valueSize = symbol % 16;
									run = symbol / 16;
								}
								valueIndex = 0;
								value = 0;
								codeWord = "";
								break;
							}
						}
					}
					if (symbol != -1){
						while (bitIndex < 8){
							if (valueIndex == valueSize){
								symbol = -1;
								//cout << "Value: " << value << endl << endl;
								if (valueSize != 0){
									value = jpegCodec->GetHuffmanCoefficient(valueSize, value);
								}
								// cout << "Value: " << value << endl << endl;

								if (tableType == 0){
									dcPredictor[componentIndex] = dcPredictor[componentIndex] + value;
									dataUnit.dc = dcPredictor[componentIndex];
									tableType = 1;
								} else {
									for (int i = 0 ; i < run ; i++, acCount++){
										dataUnit.ac[acCount] = 0;
									}
									if (valueSize != 0){
										dataUnit.ac[acCount] = value;
										acCount = acCount + 1;
									} else {
										if (run == 0){
											for ( ; acCount < 63 ; acCount++){
												dataUnit.ac[acCount] = 0;	
											}
										} else {
											dataUnit.ac[acCount] = 0;
											acCount = acCount + 1;
										}
									}
									if (acCount == 63){
										isEnd = true;
									}
								}
								break;
							}

							value = (value << 1) + ((unsigned char)(image[pos] << bitIndex) >> 7);
							bitIndex = bitIndex + 1;
							valueIndex = valueIndex + 1;
						}
					}
					if (isEnd){
						break;
					}
				}

				// cout << dataUnit.dc << " ";
				// for (int i = 0 ; i < 63 ; i++){
				// 	cout << dataUnit.ac[i] << " ";
				// }
				// cout << endl;

				ZicZac(dataUnit.ac, 63, 8, 8, 0, 1, true);

				d2.push_back(dataUnit);
			}
			d1.push_back(d2);
		}
		dataUnits.push_back(d1);
	}
}

vector<vector<vector<DataUnit>>>* MCU::GetDataUnits(){
	return &dataUnits;
}
