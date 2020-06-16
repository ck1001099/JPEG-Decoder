#include <iostream>
#include <fstream>
#include <math.h>
#include "JPEGCodec.h"
#include "MCU.h"
using namespace std;

unsigned char JPEGCodec::SOI[2] = { 0xFF, 0xD8 };
unsigned char JPEGCodec::EOI[2] = { 0xFF, 0xD9 };

JPEGCodec::JPEGCodec(){
	for (int i = 0 ; i < 16 ; i++){
		vector<int> v;
		if (i == 0){
			v.push_back(0);
		} else {
			int boundOut = pow(2, i);
			int boundIn = pow(2, i-1);
			for (int j = -boundOut+1 ; j < boundOut ; j++){
				v.push_back(j);
				if (j == -boundIn){
					j = boundIn - 1;
				}
			}
		}
		huffmanCoefficients.push_back(v);
	}

	// for (int i = 0 ; i < huffmanCoefficients.size() ; i++){
	// 	for (int j = 0 ; j < huffmanCoefficients[i].size() ; j++){
	// 		cout << huffmanCoefficients[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }

	for (int i = 0 ; i < 8 ; i++){
		vector<double> v;
		for (int j = 0 ; j < 8 ; j++){
			double c = cos((2.0 * j + 1.0) * i * M_PI / 16.0) / 2.0;
			if (i == 0){
				c = c / sqrt(2.0);
			}
			v.push_back(c);
		}
		cosineLookUpTable.push_back(v);
	}

	// for (int i = 0 ; i < cosineLookUpTable.size() ; i++){
	// 	for (int j = 0 ; j < cosineLookUpTable[i].size() ; j++){
	// 		cout << cosineLookUpTable[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }
}

JPEGCodec::~JPEGCodec(){
	for (int i = 0 ; i < segments.size() ; i++){
		delete segments[i];	
	}

	if (image != NULL){
		for (int i = 0 ; i < imageHeight ; i++){
			if (image[i] != NULL){
				delete[] image[i];
			}
		}
		delete[] image;
	}
}

void JPEGCodec::Read(string imagePath, bool display){
	// Open image
	ifs.open(imagePath, ios::in | ios::binary);
	if (!ifs){
		cout << "[System] \"" << imagePath << "\" cannot be opened." << endl;
		return;
	}
	// Real all bytes to buffer
	buffer.assign(istreambuf_iterator<char>(ifs), {});

	// Display file info
	cout << "[System] Image path: \"" << imagePath << "\" -> size = " << buffer.size() << " bytes." << endl;
	size_t imagePathSplit = imagePath.find_last_of("/\\");
	imageName = imagePath.substr(imagePathSplit+1);
	imageSize = buffer.size();

	// Check SOI and EOI
	if (!CheckSOI() || !CheckEOI()){
		Finish();
		return;
	}

	pos = 2;
	while (pos < imageSize-2){
		Log("Position: " + to_string(pos));
		if (buffer[pos] == 0xFF){
			switch (buffer[pos+1]){
				case 0xE0:
					Log("APP0 segment");
					segments.push_back(new APP0(this));
					break;
				case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6:
				case 0xE7: case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC:
				case 0xED: case 0xEE: case 0xEF:
					Log("APPn segment");
					segments.push_back(new APPn(this));
					break;
				case 0xFE:
					Log("COM segment");
					segments.push_back(new COM(this));
					break;
				case 0xDB:
					Log("DQT segment");
					segments.push_back(new DQT(this));
					break;
				case 0xDD:
					Log("DRI segment");
					segments.push_back(new DRI(this));
					break;
				case 0xC0:
					Log("SOF segment");
					segments.push_back(new SOF(this));
					break;
				case 0xC4:
					Log("DHT segment");
					segments.push_back(new DHT(this));
					break;
				case 0xDA:
					Log("SOS segment");
					segments.push_back(new SOS(this));
					break;
				default:
					Log("Unrecognizable marker: " + int_to_hex(buffer[pos]) + " " + int_to_hex(buffer[pos+1]));
					break;
			}
			if (!segments[segments.size()-1]->Decode(buffer, pos)){
				Log("Decode error");
				break;
			}
			if (display){
				segments[segments.size()-1]->Display();	
			}
		} else {
			Log("Unrecognizable marker: " + int_to_hex(buffer[pos]) + " " + int_to_hex(buffer[pos+1]));
			break;
		}
	}

	// DisplayMCUBlock(0, 0);

	// Dequantization
	Log("Dequantization");
	Dequantization();

	// IDCT
	Log("IDCT");
	IDCT();

	// Shift
	Log("Shift");
	Shift();

	// Upsampling
	Log("Upsampling");
	Upsampling();

	// YCbCr to RGB
	Log("YCbCr to RGB");
	ToRGB();

	// Save to BMP
	SaveToBMP(imagePath);
	
	Finish();
	return;
}

void JPEGCodec::SaveToBMP(string imagePath){
	size_t imagePathSplit = imagePath.find_last_of(".");
	imagePath = imagePath.substr(0, imagePathSplit+1).append("bmp");

	Log("Start to convert to BMP file...");
	Log("Save to " + imagePath);
	ofs.open(imagePath, ios::out | ios::binary);
	if (!ofs){
		Log("\"" + imagePath + "\" cannot be opened.");
		return;
	}

	char buffer[4];

	/*********************************
	 * Bitmap File Header (14 bytes) *
	 *********************************/
	// bfType
	buffer[0] = 'B'; buffer[1] = 'M';
	ofs.write(buffer, 2);
	// bfSize
	unsigned int bfSize = 54 + ((imageWidth * 3 + 4)/ 4) * 4 * imageHeight;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = bfSize & 0xFF;
		bfSize = bfSize >> 8;
	}
	ofs.write(buffer, 4);
	// bfReserved1
	buffer[0] = 0; buffer[1] = 0;
	ofs.write(buffer, 2);
	// bfReserved2
	buffer[0] = 0; buffer[1] = 0;
	ofs.write(buffer, 2);
	// bfOffBits
	unsigned int bfOffBits = 54;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = bfOffBits & 0xFF;
		bfOffBits = bfOffBits >> 8;
	}
	ofs.write(buffer, 4);
	//*/

	/*********************************
	 * Bitmap Info Header (40 bytes) *
	 *********************************/
	// biSize
	unsigned int biSize = 40;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biSize & 0xFF;
		biSize = biSize >> 8;
	}
	ofs.write(buffer, 4);
	// biWidth
	int biWidth = imageWidth;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biWidth & 0xFF;
		biWidth = biWidth >> 8;
	}
	ofs.write(buffer, 4);
	// biHeight
	int biHeight = -imageHeight;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biHeight & 0xFF;
		biHeight = biHeight >> 8;
	}
	ofs.write(buffer, 4);
	// biPlane
	int biPlane = 1;
	for (int i = 0 ; i < 2 ; i++){
		buffer[i] = biPlane & 0xFF;
		biPlane = biPlane >> 8;
	}
	ofs.write(buffer, 2);
	// biBitCount
	int biBitCount = 24;
	for (int i = 0 ; i < 2 ; i++){
		buffer[i] = biBitCount & 0xFF;
		biBitCount = biBitCount >> 8;
	}
	ofs.write(buffer, 2);
	// biCompression
	int biCompression = 0;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biCompression & 0xFF;
		biCompression = biCompression >> 8;
	}
	ofs.write(buffer, 4);
	// biSizeImage
	int biSizeImage = 0;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biSizeImage & 0xFF;
		biSizeImage = biSizeImage >> 8;
	}
	ofs.write(buffer, 4);
	// biXPelsPerMeter
	int biXPelsPerMeter = 0;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biXPelsPerMeter & 0xFF;
		biXPelsPerMeter = biXPelsPerMeter >> 8;
	}
	ofs.write(buffer, 4);
	// biYPelsPerMeter
	int biYPelsPerMeter = 0;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biXPelsPerMeter & 0xFF;
		biXPelsPerMeter = biXPelsPerMeter >> 8;
	}
	ofs.write(buffer, 4);
	// biClrUsed
	int biClrUsed = 0;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biClrUsed & 0xFF;
		biClrUsed = biClrUsed >> 8;
	}
	ofs.write(buffer, 4);
	// biClrImportant
	int biClrImportant = 0;
	for (int i = 0 ; i < 4 ; i++){
		buffer[i] = biClrImportant & 0xFF;
		biClrImportant = biClrImportant >> 8;
	}
	ofs.write(buffer, 4);
	//*/

	/***************
	 * Bitmap data *
	 ***************/
	int padding = 0;
	for (int px = 0 ; px < imageHeight ; px++){
		padding = 0;
		for (int py = 0 ; py < imageWidth ; py++){
			ofs.put((char)image[px][py].B);
			ofs.put((char)image[px][py].G);
			ofs.put((char)image[px][py].R);
			padding = padding + 3;
		}
		while (padding % 4 != 0){
			ofs.put(0);
			padding = padding + 1;
		}
	}
	//*/

	ofs.close();
}

void JPEGCodec::InsertHuffmanTable(int id, int type, map<string, int> huffmanTable){
	huffmanTables[id][type] = huffmanTable;
	// for (map<string, int>::iterator iter = huffmanTables[id][type]->begin() ; iter != huffmanTables[id][type]->end() ; iter++){
	// 	cout << iter->first << " " << iter->second << endl;
	// }
}

void JPEGCodec::InsertQuantizationTable(int id, int* quantizationTable){
	vector<int> qt;
	for (int i = 0 ; i < 64 ; i++){
		qt.push_back(quantizationTable[i]);
	}
	quantizationTables[id] = qt;
}

int JPEGCodec::GetSymbolFromHuffmanTable(int id, int type, string codeWord){
	if (huffmanTables[id][type].find(codeWord) == huffmanTables[id][type].end()){
		return -1;
	} else {
		return huffmanTables[id][type][codeWord];
	}
}

void JPEGCodec::SetSOFComponent(SOFComponent* sofComponents){
	this->sofComponents = sofComponents;
}

SOFComponent* JPEGCodec::GetSOFComponent(){
	return sofComponents;
}

void JPEGCodec::SetImageWidth(int imageWidth){
	this->imageWidth = imageWidth;
}

int JPEGCodec::GetImageWidth(){
	return imageWidth;
}

void JPEGCodec::SetImageHeight(int imageHeight){
	this->imageHeight = imageHeight;
}

int JPEGCodec::GetImageHeight(){
	return imageHeight;
}

int JPEGCodec::GetHuffmanCoefficient(int SSSS, int index){
	return huffmanCoefficients[SSSS][index];
}

void JPEGCodec::SetMCUs(vector<vector<MCU>>* MCUs){
	this->MCUs = MCUs;
}

void JPEGCodec::Log(string message){
	cout << "[" << imageName << "] " << message << endl;
	return;
}

void JPEGCodec::DisplayMCUBlock(int mcuIndexX, int mcuIndexY){
	if (mcuIndexX >= (*MCUs).size() || mcuIndexY >= ((*MCUs)[mcuIndexX]).size()){
		cout << "MCU (" << mcuIndexX << "," << mcuIndexY << ") doesn't exist" << endl;
		return;
	}
	for (int componentIndex = 0 ; componentIndex < (*(*MCUs)[mcuIndexX][mcuIndexY].GetDataUnits()).size() ; componentIndex++){
		for (int vIndex = 0 ; vIndex < (*(*MCUs)[mcuIndexX][mcuIndexY].GetDataUnits())[componentIndex].size() ; vIndex++){
			for (int hIndex = 0 ; hIndex < (*(*MCUs)[mcuIndexX][mcuIndexY].GetDataUnits())[componentIndex][vIndex].size() ; hIndex++){
				cout << "(" << componentIndex << "," << vIndex << "," << hIndex << ")" << endl;
				cout << (*(*MCUs)[mcuIndexX][mcuIndexY].GetDataUnits())[componentIndex][vIndex][hIndex].dc << " ";
				for (int i = 0 ; i < 63 ; i++){
					cout << (*(*MCUs)[mcuIndexX][mcuIndexY].GetDataUnits())[componentIndex][vIndex][hIndex].ac[i] << " ";
					if (i % 8 == 6){
						cout << endl;
					}
				}
				cout << endl;
			}
		}
	}
}

void JPEGCodec::DisplayImageBlock(int xInit, int yInit, int mode){
	if (image == NULL){
		return;
	}
	if (mode == 0){
		for (int px = xInit ; px < xInit + 8 ; px++){
			for (int py = yInit ; py < yInit + 8 ; py++){
				if (px >= imageHeight || py >= imageWidth){
					continue;
				}
				cout << image[px][py].R << " ";
			}
			cout << endl;
		}
		cout << endl;
		for (int px = xInit ; px < xInit + 8 ; px++){
			for (int py = yInit ; py < yInit + 8 ; py++){
				if (px >= imageHeight || py >= imageWidth){
					continue;
				}
				cout << image[px][py].G << " ";
			}
			cout << endl;
		}
		cout << endl;
		for (int px = xInit ; px < xInit + 8 ; px++){
			for (int py = yInit ; py < yInit + 8 ; py++){
				if (px >= imageHeight || py >= imageWidth){
					continue;
				}
				cout << image[px][py].B << " ";
			}
			cout << endl;
		}
		cout << endl;
	} else if (mode == 1){
		for (int px = xInit ; px < xInit + 8 ; px++){
			for (int py = yInit ; py < yInit + 8 ; py++){
				if (px >= imageHeight || py >= imageWidth){
					continue;
				}
				cout << image[px][py].Y << " ";
			}
			cout << endl;
		}
		cout << endl;
		for (int px = xInit ; px < xInit + 8 ; px++){
			for (int py = yInit ; py < yInit + 8 ; py++){
				if (px >= imageHeight || py >= imageWidth){
					continue;
				}
				cout << image[px][py].Cr << " ";
			}
			cout << endl;
		}
		cout << endl;
		for (int px = xInit ; px < xInit + 8 ; px++){
			for (int py = yInit ; py < yInit + 8 ; py++){
				if (px >= imageHeight || py >= imageWidth){
					continue;
				}
				cout << image[px][py].Cb << " ";
			}
			cout << endl;
		}
		cout << endl;
	}	
}

void JPEGCodec::Dequantization(){
	for (int i = 0 ; i < (*MCUs).size() ; i++){
		for (int j = 0 ; j < (*MCUs)[i].size() ; j++){
			vector<vector<vector<DataUnit>>>* dataUnits = (*MCUs)[i][j].GetDataUnits();
			for (int componentIndex = 0 ; componentIndex < (*dataUnits).size() ; componentIndex++){
				for (int vIndex = 0 ; vIndex < (*dataUnits)[componentIndex].size() ; vIndex++){
					for (int hIndex = 0 ; hIndex < (*dataUnits)[componentIndex][vIndex].size() ; hIndex++){
						(*dataUnits)[componentIndex][vIndex][hIndex].dc = (*dataUnits)[componentIndex][vIndex][hIndex].dc * quantizationTables[sofComponents[componentIndex].quantizationTableNumber][0];
						for (int k = 0 ; k < 63 ; k++){
							(*dataUnits)[componentIndex][vIndex][hIndex].ac[k] = (*dataUnits)[componentIndex][vIndex][hIndex].ac[k] * quantizationTables[sofComponents[componentIndex].quantizationTableNumber][k+1];
						}
					}
				}
			}
		}
	}
	// DisplayMCUBlock(0, 0);
	// DisplayMCUBlock(29, 65);
}

void JPEGCodec::IDCT(){
	for (int i = 0 ; i < (*MCUs).size() ; i++){
		for (int j = 0 ; j < (*MCUs)[i].size() ; j++){
			vector<vector<vector<DataUnit>>>* dataUnits = (*MCUs)[i][j].GetDataUnits();
			for (int componentIndex = 0 ; componentIndex < (*dataUnits).size() ; componentIndex++){
				for (int vIndex = 0 ; vIndex < (*dataUnits)[componentIndex].size() ; vIndex++){
					for (int hIndex = 0 ; hIndex < (*dataUnits)[componentIndex][vIndex].size() ; hIndex++){
						//cout << i << " " << j << " " << componentIndex << " " << vIndex << " " << hIndex << endl;
						double tmp1[64] = {0};
						for (int v = 0 ; v < 8 ; v++){
							for (int n2 = 0 ; n2 < 8 ; n2++){
								for (int k2 = 0 ; k2 < 8 ; k2++){
									if (v == 0 && k2 == 0){
										tmp1[v * 8 + n2] = tmp1[v * 8 + n2] + (*dataUnits)[componentIndex][vIndex][hIndex].dc * cosineLookUpTable[k2][n2];
									} else {
										tmp1[v * 8 + n2] = tmp1[v * 8 + n2] + (*dataUnits)[componentIndex][vIndex][hIndex].ac[v * 8 + k2 - 1] * cosineLookUpTable[k2][n2];
									}
								}
							}
						}
						double tmp2[64] = {0};
						for (int n1 = 0 ; n1 < 8 ; n1++){
							for (int n2 = 0 ; n2 < 8 ; n2++){
								for (int k1 = 0 ; k1 < 8 ; k1++){
									tmp2[n1 * 8 + n2] = tmp2[n1 * 8 + n2] + tmp1[k1 * 8 + n2] * cosineLookUpTable[k1][n1];
								}
							}
						}
						(*dataUnits)[componentIndex][vIndex][hIndex].dc = tmp2[0];
						for (int k = 0 ; k < 63 ; k++){
							(*dataUnits)[componentIndex][vIndex][hIndex].ac[k] = tmp2[k+1];
						}
					}
				}
			}
		}
	}
	// DisplayMCUBlock(0, 0);
	// DisplayMCUBlock(29, 65);
}

void JPEGCodec::Shift(){
	for (int i = 0 ; i < (*MCUs).size() ; i++){
		for (int j = 0 ; j < (*MCUs)[i].size() ; j++){
			vector<vector<vector<DataUnit>>>* dataUnits = (*MCUs)[i][j].GetDataUnits();
			for (int componentIndex = 0 ; componentIndex < (*dataUnits).size() ; componentIndex++){
				for (int vIndex = 0 ; vIndex < (*dataUnits)[componentIndex].size() ; vIndex++){
					for (int hIndex = 0 ; hIndex < (*dataUnits)[componentIndex][vIndex].size() ; hIndex++){
						(*dataUnits)[componentIndex][vIndex][hIndex].dc = (*dataUnits)[componentIndex][vIndex][hIndex].dc + 128;
						for (int k = 0 ; k < 63 ; k++){
							(*dataUnits)[componentIndex][vIndex][hIndex].ac[k] = (*dataUnits)[componentIndex][vIndex][hIndex].ac[k] + 128;
						}
					}
				}
			}
		}
	}
	// DisplayMCUBlock(0, 0);
	// DisplayMCUBlock(29, 65);
}

void JPEGCodec::Upsampling(){
	if (image != NULL){
		for (int i = 0 ; i < imageHeight ; i++){
			if (image[i] != NULL){
				delete[] image[i];
			}
		}
		delete[] image;
	}

	image = new Pixel*[imageHeight];
	for (int i = 0 ; i < imageHeight ; i++){
		image[i] = new Pixel[imageWidth];
	}

	for (int i = 0 ; i < (*MCUs).size() ; i++){
		for (int j = 0 ; j < (*MCUs)[i].size() ; j++){
			vector<vector<vector<DataUnit>>>* dataUnits = (*MCUs)[i][j].GetDataUnits();
			int xInit = i * (8 * sofComponents[0].verticalSamplingFactor);
			int yInit = j * (8 * sofComponents[0].horizontalSamplingFactor);
			for (int componentIndex = 0 ; componentIndex < (*dataUnits).size() ; componentIndex++){
				int vUpsamplingRate = sofComponents[0].verticalSamplingFactor / sofComponents[componentIndex].verticalSamplingFactor;
				int hUpsamplingRate = sofComponents[0].horizontalSamplingFactor / sofComponents[componentIndex].horizontalSamplingFactor;
				for (int vIndex = 0 ; vIndex < (*dataUnits)[componentIndex].size() ; vIndex++){
					for (int hIndex = 0 ; hIndex < (*dataUnits)[componentIndex][vIndex].size() ; hIndex++){
						for (int x = 0 ; x < 8 ; x++){
							for (int y = 0 ; y < 8 ; y++){
								for (int v = 0 ; v < vUpsamplingRate ; v++){
									for (int h = 0 ; h < hUpsamplingRate ; h++){
										int px = xInit + vIndex * 8 + x * vUpsamplingRate + v;
										int py = yInit + hIndex * 8 + y * hUpsamplingRate + h;
										// cout << px << " " << py << endl;
										if (px >= imageHeight || py >= imageWidth){
											continue;
										}
										if (x == 0 && y == 0){
											if (componentIndex == 0){
												image[px][py].Y = (*dataUnits)[componentIndex][vIndex][hIndex].dc;
											} else if (componentIndex == 1){
												image[px][py].Cb = (*dataUnits)[componentIndex][vIndex][hIndex].dc;
											} else if (componentIndex == 2){
												image[px][py].Cr = (*dataUnits)[componentIndex][vIndex][hIndex].dc;
											}
										} else {
											if (componentIndex == 0){
												image[px][py].Y = (*dataUnits)[componentIndex][vIndex][hIndex].ac[x * 8 + y - 1];
											} else if (componentIndex == 1){
												image[px][py].Cb = (*dataUnits)[componentIndex][vIndex][hIndex].ac[x * 8 + y - 1];
											} else if (componentIndex == 2){
												image[px][py].Cr = (*dataUnits)[componentIndex][vIndex][hIndex].ac[x * 8 + y - 1];
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	// DisplayImageBlock(0, 0, 1);
	// DisplayImageBlock(29, 65, 1);
}

void JPEGCodec::ToRGB(){
	// Y =  0.299  R + 0.587  G + 0.114  B
	// U = -0.1687 R - 0.3313 G + 0.5    B + 128    --> Cb
	// V =  0.5    R - 0.4187 G - 0.0813 B + 128    --> Cr

	// R =         Y + 1.402  (V-128)
	// G =         Y - 0.34414(U-128) - 0.71414(V-128)
	// B =         Y + 1.772  (U-128)
	for (int px = 0 ; px < imageHeight ; px++){
		for (int py = 0 ; py < imageWidth ; py++){
			image[px][py].R = image[px][py].Y + 1.402 * (image[px][py].Cr-128);
			image[px][py].G = image[px][py].Y - 0.34414 * (image[px][py].Cb-128) - 0.71414 * (image[px][py].Cr-128);
			image[px][py].B = image[px][py].Y + 1.772 * (image[px][py].Cb-128);

			if (image[px][py].R > 255){
				image[px][py].R = 255;
			} else if (image[px][py].R < 0){
				image[px][py].R = 0;
			}
			if (image[px][py].G > 255){
				image[px][py].G = 255;
			} else if (image[px][py].G < 0){
				image[px][py].G = 0;
			}
			if (image[px][py].B > 255){
				image[px][py].B = 255;
			} else if (image[px][py].B < 0){
				image[px][py].B = 0;
			}
		}
	}
	// DisplayImageBlock(0, 0, 0);
}

bool JPEGCodec::CheckSOI(){
	Log("Check SOI...");
	if (buffer[0] == SOI[0] && buffer[1] == SOI[1]){
		Log("Success!");
		return true;
	} else {
		Log("SOI is not correct!");
		return false;
	}
}

bool JPEGCodec::CheckEOI(){
	Log("Check EOI...");
	if (buffer[imageSize-2] == EOI[0] && buffer[imageSize-1] == EOI[1]){
		Log("Success!");
		return true;
	} else {
		Log("EOI is not correct!");
		return false;
	}
}

void JPEGCodec::Finish(){
	ifs.close();
	cout << "[System] Finish converting \"" << imageName << "\"" << endl;
}