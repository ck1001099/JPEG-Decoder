#include <iostream>
#include <string>
#include "JPEGCodec.h"
using namespace std;

// [REF] JPEG File Layout and Format: http://vip.sugovica.hu/Sardi/kepnezo/JPEG%20File%20Layout%20and%20Format.htm
// [REF] Huffman decoding: https://www.impulseadventure.com/photo/jpeg-huffman-coding.html
// [REF] JPEG Image Compression Systems: https://www.ece.ucdavis.edu/cerl/reliablejpeg/compression/
// [REF] Bitmap format: https://crazycat1130.pixnet.net/blog/post/1345538
// [REF] http://read.pudn.com/downloads63/doc/220135/JPEG.pdf
// [REF] https://www.itread01.com/content/1549504280.html

int main(int argc, char** argv){

	JPEGCodec jpegCodec;

	// Input
	if (argc > 1){
		cout << "[System] Start decoding/encoding multi-images automatically..." << endl;
		for (int i = 1 ; i < argc ; i++){
			string path = argv[i];
			//cout << path << endl;
			jpegCodec.Read(path);
		}
	} else {
		cout << "[System] Start JPEG codec program." << endl;
		cout << " <-- Command List --> " << endl;
		cout << " - read IMG_PATH  : decode jpeg file from IMG_PATH." << endl;
		cout << " - read2 IMG_PATH : decode jpeg file from IMG_PATH and display the information of each segment." << endl;
		cout << " - displayMCU X Y : display (X, Y) MCU block." << endl;
		cout << " - exit           : exit this program." << endl;

		cout << "> ";
		string command;
		while (cin >> command){
			if (command == "exit"){
				cout << "[System] Finish program." << endl;
				break;
			} else if (command == "read"){
				string path;
				cin >> path;
				jpegCodec.Read(path);
			} else if (command == "read2"){
				string path;
				cin >> path;
				jpegCodec.Read(path, true);
			} else if (command == "displayMCU"){
				int mcuIndexX, mcuIndexY;
				cin >> mcuIndexX >> mcuIndexY;
				jpegCodec.DisplayMCUBlock(mcuIndexX, mcuIndexY);
			}
			cout << "> ";
		}
	}

	return 0;
}