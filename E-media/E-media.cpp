// JFIF_Parser.cpp: definiuje punkt wejścia dla aplikacji konsolowej.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <memory>
#include <cmath>




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

static std::vector<long> ReadAllBytes(char const* filename)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();

	std::vector<long>  result;
	std::vector<char>  resultchar(pos);

	ifs.seekg(0, std::ios::beg);
	ifs.read(&resultchar[0], pos);

	for (std::vector<char>::iterator i = resultchar.begin(); i != resultchar.end(); ++i) {

		result.push_back(((BYTE) * (i)).to_ulong());
		int k = 0;

	}
	return result;
}

double long modulo(double long x, int y) {
	y = std::abs(y);
	return (x < y) ? x : modulo(x - y, y);
};

template <typename Number>
Number GCD(Number u, Number v) {
	while (v != 0) {
		Number r = u % v;
		u = v;
		v = r;
	}
	return u;
}


int main()
{
	std::vector<long> bytes;
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
	bytes = ReadAllBytes("pobrane.jpg");
	std::vector<long>::iterator Data_begin = bytes.begin();
	unsigned long scan_lenght = 0;
	unsigned long component_lenght = 0;

	for (std::vector<long>::iterator i = bytes.begin(); i != bytes.end() - 1; ++i) {
		//here we cast on BYTE for safety, since when casting 0xFF value to int it's equal -1
		if (((BYTE)* i).to_ulong() == start_of_frame) {
			switch (((BYTE) * (i + 1)).to_ulong()) {
			case start_of_image:
				//nothing to do here sir, look further, should be an image though ;)
				break;
			case start_of_size:
				//while it could work without cast on BYTE it's still reccomended since value 0xFF can apear
				precision = (((BYTE) * (i + 4)).to_ulong());
				height = ((((BYTE) * (i + 5)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 6)).to_ulong() & 0xFF);
				width = ((((BYTE) * (i + 7)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 8)).to_ulong() & 0xFF);
				std::cout << "precision: " << precision << " height:" << height << " width:" << width;
				k = ((BYTE) * (i + 9)).to_ulong();
				for (n = 0; n < k; n++) {
					components[n][1] = ((BYTE) * (i + 10 + (n * 3))).to_ulong();
					components[n][2] = ((BYTE) * (i + 11 + (n * 3))).to_ulong();
					components[n][3] = ((BYTE) * (i + 12 + (n * 3))).to_ulong();
					std::cout << std::endl;
					std::cout << "component: " << components[n][1] << " " << components[n][2] << " " << components[n][3] << std::endl;;
				}
				break;
			case app:
				k = ((((BYTE) * (i + 2)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 3)).to_ulong() & 0xFF);
				for (int j = 0; j < 5; j++) {
					if (((BYTE) * (i + 4 + j)).to_ulong() != jfif[j]) {
						std::cerr << "it's not jfif file";
						return -1;
					}
				}
				if (((BYTE) * (i + 9)).to_ulong() != 1) {
					std::cerr << "file error";
					return -2;
				}
				if (((BYTE) * (i + 10)).to_ulong() < 0 || ((BYTE) * (i + 10)).to_ulong() > 2) {
					std::cerr << "this shouldn't have happened but will continue anyway";
				}
				x_y_density = ((BYTE) * (i + 11)).to_ulong();
				X_density = ((((BYTE) * (i + 12)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 13)).to_ulong() & 0xFF);
				Y_density = ((((BYTE) * (i + 14)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 15)).to_ulong() & 0xFF);
				std::cout << "x/y density type: " << x_y_density << " X_density:" << X_density << " Y_density:" << Y_density << std::endl;
				//16,17 is thumbnail then n bytes for thumbnail
				break;
			case start_of_quantization_table:
				k = ((((BYTE) * (i + 2)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 3)).to_ulong() & 0xFF);
				QTI = (BYTE) * (i + 4);
				z = ((QTI << 4) >> 4).to_ulong();
				if (z > 15) {
					return -3;
				}
				qti_precision = ((QTI) >> 4).to_ulong();
				if (qti_precision > 0) {
					for (n = 0; n < (64 * (qti_precision + 1)); n++) {
						k = ((((BYTE) * (i + 5 + (n * 2))).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 6 + (n * 2))).to_ulong() & 0xFF);
						QT[z].push_back(k);
					}
				}
				else {
					for (n = 0; n < (64 * (qti_precision + 1)); n++) {
						QT[z].push_back(((BYTE) * (i + 5 + n)).to_ulong());
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
				k = ((((BYTE) * (i + 2)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 3)).to_ulong() & 0xFF);
				HuffmanTables.push_back(std::make_shared<HuffmanTable>());
				actual = *(--(HuffmanTables.end()));
				actual->HTI = (BYTE) * (i + 4);
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
					actual->HTS.push_back(((BYTE) * (i + 5 + n)).to_ulong());
				}
				k = 20;
				z = 0;

				for (auto next_hts = actual->HTS.begin(); next_hts != actual->HTS.end(); next_hts++)
				{
					std::cout << "Huffman Symbol:" << " " << (*next_hts) << std::endl;
					for (n = 1; n <= (*next_hts); n++) {
						actual->HT[z].push_back(((BYTE) * (i + n + k)).to_ulong());
					}
					std::cout << "Content for current symbol: ";
					for (auto next_htt = actual->HT[z].begin(); next_htt != actual->HT[z].end(); next_htt++) {
						std::cout << (*next_htt) << " ";
					}
					std::cout << std::endl;
					k += (*next_hts);
					z++;
				}
				std::cout << std::endl;

				break;
			case start_of_scan:
				//tu są przechowywane informacje o przechowywanym obrazku, jak dekodowac
				scan_lenght = ((((BYTE) * (i + 2)).to_ulong() & 0xff) << 8) | (((BYTE) * (i + 3)).to_ulong() & 0xFF);
				component_lenght = (((BYTE) * (i + 4)).to_ulong() & 0xFF);
				if (component_lenght > 4 || component_lenght < 1) {
					std::cerr << "Error bad SOS component lenght " << component_lenght << std::endl;
				}
				if (scan_lenght != ((component_lenght * 2) + 6)) {
					std::cerr << "Error bad SOS lenght " << scan_lenght << std::endl;
					return -1;
				}
				Data_begin = i + 2 + scan_lenght; // w tym miejscu zaczynają się dane aż do image end FF D9
			default:
				break;
			}
		}
	}
	int koniec = 0;
	std::ofstream ofs;
	ofs.open("testfile.jpg", std::ofstream::out | std::ofstream::trunc);
	ofs << "HELLO";
	ofs.close();
	int zr = 0; int zl = 1;
	int p = 13, q = 23;// values for RSA
	int count;
	int rsan = p * q, phi = (p - 1) * (q - 1), e = 1, zk = 1;
	double d = 100.5;
	while (d > 100) {
		e++;
		while (GCD(e, phi) != 1) {
			e++;
		}
		while ((d - (int)d) > 0) {
			d = ((double)1 + (float)(zk++ * phi)) / (double)e;
		}
		zk = 1;
		//std::cout << "e: " << e << std::endl;
		if (e > 30) return -10;
	}
	while (koniec != 1) {

		//std::cout << "e: " << e;
		//std::cout << " d: " << d;
		//std::cout << " n: " << rsan;
		//std::cout << " " << d * e;
		//std::cout << " " << 1 + zk * phi;
		char menu = -1;
		std::cout << "MENU" << std::endl;
		std::cout << "1 - XOR" << std::endl;
		std::cout << "2 - XOR DECODE" << std::endl;
		std::cout << "3 - RSA Simple" << std::endl;
		std::cout << "4 - RSA Simple DECODE" << std::endl;
		std::cout << "5 - RSA 128" << std::endl;
		std::cout << "6 - RSA 128 DECODE" << std::endl;
		std::cout << "7 - Primal Numbers" << std::endl;
		std::cout << "8 - PN DECODE" << std::endl;
		std::cout << "9 - SAVE CURRENT MEMORY TO FILE" << std::endl;
		std::cout << "0 - END" << std::endl;
		std::cin >> menu;
		switch (menu) {
		case '1':
			for (std::vector<long>::iterator i = Data_begin; ; i++) {
				char key = 'x';
				if ((((char)* i) & 0xff) == 0xFF) {
					if ((((char) * (i + 1)) & 0xff) == 0xD9)
						break;
				}
				*i = ((char)* i) ^ key;
			}
			break;
		case '2':
			for (std::vector<long>::iterator i = Data_begin; ; i++) {
				char key = 'x';
				if ((((char)* i) & 0xff) == 0xFF) {
					if ((((char) * (i + 1)) & 0xff) == 0xD9)
						break;
				}
				*i = ((char)* i) ^ key;
			}
			break;
		case '3':
			for (std::vector<long>::iterator i = Data_begin; ; i++) {
				if ((((char)* i) & 0xff) == 0xFF) {
					if ((((char) * (i + 1)) & 0xff) == 0xD9)
						break;
				}
				long double m = 1;
				for (int loop = 0; loop < e; loop++) {
					m = m * (*i);
					m = fmod(m, rsan);
				}
				*i = m;
			}
			break;
		case '4':
			for (std::vector<long>::iterator i = Data_begin; ; i++) {
				if ((((char)* i) & 0xff) == 0xFF) {
					if ((((char) * (i + 1)) & 0xff) == 0xD9)
						break;
				}
				long double m = 1;
				for (int loop = 0; loop < d; loop++) {
					m = m * (*i);
					m = fmod(m, rsan);
				}
				*i = m;
				if (*i > 255) {
					std::cout << "ree";
					break;
				}
			}
			break;
		case '5':
			break;
		case '6':
			break;
		case '7':
			break;
		case '8':
			break;
		case '9':
			ofs.open("testfile.jpg", std::ofstream::out | std::ofstream::trunc | std::ios::binary | std::ios::ate);
			for (std::vector<long>::iterator i = bytes.begin(); i != bytes.end(); i++) {
				ofs << (char)* i;
			}
			ofs.close();
			break;
		case '0':
			koniec = 1;
			break;
		default:
			std::cout << "NOTHING HERE BUT PENGUINS";
		}
	}

	return 0;

}
