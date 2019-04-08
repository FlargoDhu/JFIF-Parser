// JFIF_Parser.cpp: definiuje punkt wejścia dla aplikacji konsolowej.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <memory>

#define start_of_frame 0xFF
#define start_of_image 0xD8
#define start_of_size 0xC0
#define start_of_progressive_size 0xC2 //not used
#define start_of_human_tables 0xC4
#define start_of_quantization_table 0xDB
#define start_of_scan 0xDA
#define start_of_restart 0xD7
#define start_of_comment 0xFE
#define end_of_image 0xD9
#define zeroes 0xF0
#define block_end 0x00
#define app 0xE0 //till 0xEF
unsigned long jfif[] = { 0x4a, 0x46, 0x49, 0x46, 0x00 };
typedef std::bitset<8> BYTE;

class HuffmanTable {
	public:
		int index;
		std::bitset<8> HTI;
		bool HTT; //false DC table, true AC table
		std::vector<unsigned long> HT[16];
		std::vector<unsigned long> HTS;
};

static std::vector<char> ReadAllBytes(char const* filename)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();

	std::vector<char>  result(pos);

	ifs.seekg(0, std::ios::beg);
	ifs.read(&result[0], pos);

	return result;
}

int main()
{
	std::vector<char> bytes;
	std::vector<unsigned long> QT[15];
	std::vector<std::shared_ptr<HuffmanTable>> HuffmanTables;
	std::bitset<8> QTI;
	std::shared_ptr<HuffmanTable> actual;
	int height;
	int width;
	int precision;
	int qti_precision;
	int long components[10][3];
	int x_y_density; //0 - x/y aspect ratio, 1 - x/y dost/inch, 2 - x/y dots/cm
	int X_density;
	int Y_density;
	unsigned long k;
	unsigned long n;
	unsigned long z;
	bytes = ReadAllBytes("test.jpg");

	for (std::vector<char>::const_iterator i = bytes.begin(); i != bytes.end()-1; ++i){
		//here we cast on BYTE for safety, since when casting 0xFF value to int it's equal -1
		if (((BYTE)*i).to_ulong() == start_of_frame) {
			switch (((BYTE)*(i+1)).to_ulong()) {
			case start_of_image:
				//nothing to do here sir, look further, should be an image though ;)
				break;
			case start_of_size:
				//while it could work without cast on BYTE it's still reccomended since value 0xFF can apear
				precision = (((BYTE)*(i + 4)).to_ulong());
				height = ((((BYTE)*(i + 5)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 6)).to_ulong() & 0xFF);
				width = ((((BYTE)*(i + 7)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 8)).to_ulong() & 0xFF);
				std::cout << "precision: " << precision << " height:" << height << " width:" << width;
				k = ((BYTE)*(i + 9)).to_ulong();
				for (n = 0; n < k; n++) {
					components[n][1] = ((BYTE)*(i + 10+(n*3))).to_ulong();
					components[n][2] = ((BYTE)*(i + 11+(n*3))).to_ulong();
					components[n][3] = ((BYTE)*(i + 12+(n*3))).to_ulong();
					std::cout << std::endl;
					std::cout << "component: " << components[n][1] << " " << components[n][2] << " " << components[n][3] << std::endl;;
				}
				break;
			case app:
				k = ((((BYTE)*(i + 2)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 3)).to_ulong() & 0xFF);
				for (int j = 0; j < 5; j++) {
					if (((BYTE)*(i + 4 + j)).to_ulong() != jfif[j]){
						std::cerr << "it's not jfif file";
						return -1;
					}
				}
				if (((BYTE)*(i + 9)).to_ulong() != 1) {
					std::cerr << "file error";
					return -2;
				}
				if (((BYTE)*(i + 10)).to_ulong() < 0 || ((BYTE)*(i + 10)).to_ulong() > 2) {
					std::cerr << "this shouldn't have happened but will continue anyway";
				}
				x_y_density = ((BYTE)*(i + 11)).to_ulong();
				X_density = ((((BYTE)*(i + 12)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 13)).to_ulong() & 0xFF);
				Y_density = ((((BYTE)*(i + 14)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 15)).to_ulong() & 0xFF);
				std::cout << "x/y density type: " << x_y_density << " X_density:" << X_density << " Y_density:" << Y_density << std::endl;
				//16,17 is thumbnail then n bytes for thumbnail
				break;
			case start_of_quantization_table:
				k = ((((BYTE)*(i + 2)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 3)).to_ulong() & 0xFF);
				QTI = (BYTE)*(i + 4);
				z = ((QTI << 4) >> 4).to_ulong();
				if (z > 15) {
					return -3;
				}
				qti_precision = ((QTI) >> 4).to_ulong();
				if (qti_precision > 0) {
					for (n = 0; n < (64 * (qti_precision + 1)); n++) {
						k = ((((BYTE)*(i + 5+(n*2))).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 6+(n*2))).to_ulong() & 0xFF);
						QT[z].push_back(k);
					}
				}
				else {
					for (n = 0; n < (64 * (qti_precision + 1)); n++) {
						QT[z].push_back(((BYTE)*(i + 5 + n)).to_ulong());
					}
				}
				std::cout << "QT precison: " << qti_precision << std::endl;
				std::cout << "QT content: ";
				for (std::vector<unsigned long>::const_iterator i = QT[z].begin(); i != QT[z].end() - 1; ++i) {
					std::cout << *i << " ";
				}
				std::cout << std::endl;
				break;
			case start_of_human_tables:
				k = ((((BYTE)*(i + 2)).to_ulong() & 0xff) << 8) | (((BYTE)*(i + 3)).to_ulong() & 0xFF);
				HuffmanTables.push_back(std::make_shared<HuffmanTable>());
				actual = *(--(HuffmanTables.end()));
				actual->HTI = (BYTE)*(i + 4);
				actual->index = ((QTI << 4) >> 4).to_ulong();
				if (actual->HTI.test(4)) {
					actual->HTT = true;
					std::cout << "Huffman Table Type: AC" << std::endl;
					}
				else {
					actual->HTT = false;
					std::cout << "Huffman Table Type: DC" << std::endl;
					}
				if (((actual->HTI) >> 5).to_ulong() != 0) {
					std::cerr << "error hufman tables wrong";
					return -5;
				}
				for (n = 0; n < 16; n++) {
					actual->HTS.push_back(((BYTE)*(i + 5 + n)).to_ulong());
				}
				k = 20;
				z = 0;
				
				for (auto next_hts = actual->HTS.begin(); next_hts != actual->HTS.end(); next_hts++)
				{
					std::cout << "Huffman Symbol:" << " " << (*next_hts) << std::endl;
					for (n = 1; n <= (*next_hts); n++) {
						actual->HT[z].push_back(((BYTE)*(i + n + k)).to_ulong());
					}
					std::cout << "Content for current symbol: ";
					for (auto next_htt = actual->HT[z].begin(); next_htt != actual->HT[z].end(); next_htt++) {
						std:: cout << (*next_htt) << " ";
					}
					std::cout << std::endl;
					k+=(*next_hts);
					z++;
				}
				std::cout << std::endl;

				break;
			default:
				break;
			}
		}
	}
	getchar();
	return 0;
	
}
