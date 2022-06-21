#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <experimental/filesystem>
using namespace std;
using json = nlohmann::json;

/*server_base*/
ENetHost* server;
string SERVER_PORT = "17091", SERVER_IP = "127.0.0.1", SERVER_META = "127.0.0.1", SERVER_CDN = "0098/63368/cache/", SERVER_GAME_VERSION = "3.91", SERVER_ANDROID_GAME_VERSION = "3.90";
bool server_on_freeze = false, maintenance = false;
int itemdathash = 0, itemsDatSize = 0, totaluserids = 0, cId = 1, maxItems = 0;
BYTE* itemsDat = 0;
/*server_base*/

/*back_end*/
inline long long GetCurrentTimeInternalSeconds() {
	using namespace chrono;
	return (duration_cast<seconds>(system_clock::now().time_since_epoch())).count();
}
inline void server_alert(string cmd) {
	cout << "[INFO] " + cmd + "" << endl;
}
inline int char2(char x) {
	switch (x) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'B': return 11;
		case 'C': return 12;
		case 'D': return 13;
		case 'E': return 14;
		case 'F': return 15;
	}
	return -1;
}
unsigned char* getA(string fileName, int* pSizeOut, bool bAddBasePath, bool bAutoDecompress) {
	unsigned char* pData = NULL;
	FILE* fp = fopen(fileName.c_str(), "rb");
	if (not fp) {
		server_alert("File not found");
		if (not fp) return NULL;
	}
	fseek(fp, 0, SEEK_END);
	*pSizeOut = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	pData = (unsigned char*)new unsigned char[((*pSizeOut) + 1)];
	if (not pData) {
		printf("Out of memory opening %s?", fileName.c_str());
		return 0;
	}
	pData[*pSizeOut] = 0;
	fread(pData, *pSizeOut, 1, fp);
	fclose(fp);
	return pData;
}
inline ifstream::pos_type filesize(const char* filename) {
	ifstream in(filename, ifstream::ate | ifstream::binary);
	return in.tellg();
}
inline uint32_t HashString(unsigned char* str, int len) {
	if (not str) return 0;
	unsigned char* n = (unsigned char*)str;
	uint32_t acc = 0x55555555;
	if (len == 0) {
		while (*n) acc = (acc >> 27) + (acc << 5) + *n++;
	} else {
		for (int i = 0; i < len; i++) {
			acc = (acc >> 27) + (acc << 5) + *n++;
		}
	}
	return acc;
}
struct vec2 {
	int m_x, m_y;
	vec2(int x, int y) : m_x(x), m_y(y) { }
	bool operator==(const vec2& other) {
		return m_x == other.m_x and m_y == other.m_y;
	}
};
inline vector<string> explode(const string& delimiter, const string& str) {
	vector<string> arr;
	int strleng = str.length();
	int delleng = delimiter.length();
	if (delleng == 0) return arr;
	int i = 0;
	int k = 0;
	while (i < strleng) {
		int j = 0;
		while (i + j < strleng and j < delleng and str[i + j] == delimiter[j]) j++;
		if (j == delleng) {
			arr.push_back(str.substr(k, i - k));
			i += delleng;
			k = i;
		} else {
			i++;
		}
	}
	arr.push_back(str.substr(k, i - k));
	return arr;
}
inline string replace_str(string& str, const string& from, const string& to) {
	while (str.find(from) != string::npos)
		str.replace(str.find(from), from.length(), to);
	return str;
}
inline string toUpperText(string txt) {
	string ret;
	for (char c : txt) ret += toupper(c);
	return ret;
}
inline string toLowerText(string name) {
	string newS;
	for (char c : name) newS += (c >= 'A' and c <= 'Z') ? c - ('A' - 'a') : c;
	string ret;
	for (int i = 0; i < (int)newS.length(); i++) {
		if (newS[i] == '`') i++; else ret += newS[i];
	}
	string ret2;
	for (char c : ret) if ((c >= 'a' and c <= 'z') || (c >= '0' and c <= '9')) ret2 += c;
	return ret2;
}
inline void rm_space_(string& usrStr) {
	char cha1, cha2;
	for (int i = 0; i < (int)usrStr.size() - 1; ++i) {
		cha1 = usrStr.at(i), cha2 = usrStr.at(i + 1);
		if ((cha1 == ' ') and (cha2 == ' ')) {
			usrStr.erase(usrStr.begin() + 1 + i);
			--i;
		}
	}
	char cha, chas;
	for (int i = 0; i < (int)usrStr.size() - 1; ++i) {
		cha = usrStr.at(i), chas = usrStr.at(i + 1);
		if ((cha == '`') and (chas == '`')) {
			usrStr.erase(usrStr.begin() + 1 + i);
			--i;
		}
	}
}
inline BYTE* struct_ptr(ENetPacket* packet) {
	unsigned int packetLenght = packet->dataLength;
	BYTE* result = NULL;
	if (packetLenght >= 0x3C) {
		BYTE* packetData = packet->data;
		result = packetData + 4;
		if (*(BYTE*)(packetData + 16) & 8) {
			if (packetLenght < *(unsigned int*)(packetData + 56) + 60) {
				server_alert("Packet too small for extended packet to be valid");
				server_alert("Sizeof float is 4.  TankUpdatePacket size: 56");
				result = 0;
			}
		} else {
			int zero = 0;
			memcpy(packetData + 56, &zero, 4);
		}
	}
	return result;
}
inline void send_(ENetPeer* p_, string t_, string l_ = "", string l_p = "") {
	if (l_ != "") t_ = "action|log\nmsg|" + t_;
	int y_ = 3;
	BYTE z_ = 0;
	BYTE* const d_ = new BYTE[5 + t_.length()];
	memcpy(d_, &y_, 4), memcpy(d_ + 4, t_.c_str(), t_.length()), memcpy(d_ + 4 + t_.length(), &z_, 1);
	ENetPacket* const p = enet_packet_create(d_, 5 + t_.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(p_, 0, p);
	delete[]d_;
	if (l_ != "") {
		string w_ = "https://youtube.com/";
		if (l_p != "") {
			w_ = l_p;
		}
		l_ = "action|set_url\nurl|" + w_ + "\nlabel|" + l_ + "\n";
		BYTE* const u_ = new BYTE[5 + l_.length()];
		memcpy(u_, &y_, 4), memcpy(u_ + 4, l_.c_str(), l_.length()), memcpy(u_ + 4 + l_.length(), &z_, 1);
		ENetPacket* const p3 = enet_packet_create(u_, 5 + l_.length(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(p_, 0, p3);
		delete[] u_;
	}
}
inline void send_p(ENetPeer* peer, int num, char* data, const int len) {
	const auto packet = enet_packet_create(nullptr, len + 5, ENET_PACKET_FLAG_RELIABLE);
	memcpy(packet->data, &num, 4);
	if (data != nullptr) {
		memcpy(packet->data + 2, data, len);
	}
	char zero = 0;
	memcpy(packet->data + 2 + len, &zero, 1);
	enet_peer_send(peer, 0, packet);
}
inline int message_ptr(ENetPacket* packet) {
	if (packet->dataLength > 3u) {
		return *(packet->data);
	}
	return 0;
}
inline char* text_ptr(ENetPacket* packet) {
	char zero = 0;
	memcpy(packet->data + packet->dataLength - 1, &zero, 1);
	return (char*)(packet->data + 4);
}
inline void send_raw(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag) {
	ENetPacket* p;
	if (peer != NULL) {
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8) {
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			memcpy(p->data, &four, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			memcpy((char*)p->data + packetDataSize + 4, a4, *((DWORD*)packetData + 13));
			enet_peer_send(peer, 0, p);
		} else {
			p = enet_packet_create(0, packetDataSize + 5, packetFlag);
			memcpy(p->data, &a1, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			enet_peer_send(peer, 0, p);
		}
	}
	delete (char*)packetData;
}
inline void send_raw3(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag) {
	ENetPacket* p;
	if (peer) {
		p = enet_packet_create(0, packetDataSize + 5, packetFlag);
		memcpy(p->data, &a1, 4);
		memcpy((char*)p->data + 4, packetData, packetDataSize);
		enet_peer_send(peer, 0, p);
	}
	delete (char*)packetData;
}
inline bool email_(const string& str) {
	if (str.size() > 64 or str.empty()) return false;
	const string::const_iterator at = find(str.cbegin(), str.cend(), '@'), dot = find(at, str.cend(), '.');
	if ((at == str.cend()) or (dot == str.cend())) return false;
	if (distance(str.cbegin(), at) < 1 or distance(at, str.cend()) < 5) return false;
	return true;
}
inline string OutputBanTime(int n) {
	string x;
	int day = n / (24 * 3600);
	if (day != 0) x.append(to_string(day) + " Days ");
	n = n % (24 * 3600);
	int hour = n / 3600;
	if (hour != 0) x.append(to_string(hour) + " Hours ");
	n %= 3600;
	int minutes = n / 60;
	if (minutes != 0) x.append(to_string(minutes) + " Minutes ");
	n %= 60;
	int seconds = n;
	if (seconds != 0) x.append(to_string(seconds) + " Seconds");
	return x;
}
inline int calcBanDuration(long long banDuration) {
	int duration = 0;
	duration = (int)(banDuration - GetCurrentTimeInternalSeconds());
	return duration;
}
int ch2n(char x) {
	switch (x) {
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'A':
		return 10;
	case 'B':
		return 11;
	case 'C':
		return 12;
	case 'D':
		return 13;
	case 'E':
		return 14;
	case 'F':
		return 15;
	default:
		break;
	}
	return -1;
}
inline int fix_ct(int number)
{
	if (number == -128)
	{
		return 128;
	}
	else if (number == -127)
	{
		return 129;
	}
	else if (number == -126)
	{
		return 130;
	}
	else if (number == -125)
	{
		return 131;
	}
	else if (number == -124)
	{
		return 132;
	}
	else if (number == -123)
	{
		return 133;
	}
	else if (number == -122)
	{
		return 134;
	}
	else if (number == -121)
	{
		return 135;
	}
	else if (number == -120)
	{
		return 136;
	}
	else if (number == -119)
	{
		return 137;
	}
	else if (number == -118)
	{
		return 138;
	}
	else if (number == -117)
	{
		return 139;
	}
	else if (number == -116)
	{
		return 140;
	}
	else if (number == -115)
	{
		return 141;
	}
	else if (number == -114)
	{
		return 142;
	}
	else if (number == -113)
	{
		return 143;
	}
	else if (number == -112)
	{
		return 144;
	}
	else if (number == -111)
	{
		return 145;
	}
	else if (number == -110)
	{
		return 146;
	}
	else if (number == -109)
	{
		return 147;
	}
	else if (number == -108)
	{
		return 148;
	}
	else if (number == -107)
	{
		return 149;
	}
	else if (number == -106)
	{
		return 150;
	}
	else if (number == -105)
	{
		return 151;
	}
	else if (number == -104)
	{
		return 152;
	}
	else if (number == -103)
	{
		return 153;
	}
	else if (number == -102)
	{
		return 154;
	}
	else if (number == -101)
	{
		return 155;
	}
	else if (number == -100)
	{
		return 156;
	}
	else if (number == -99)
	{
		return 157;
	}
	else if (number == -98)
	{
		return 158;
	}
	else if (number == -97)
	{
		return 159;
	}
	else if (number == -96)
	{
		return 160;
	}
	else if (number == -95)
	{
		return 161;
	}
	else if (number == -94)
	{
		return 162;
	}
	else if (number == -93)
	{
		return 163;
	}
	else if (number == -92)
	{
		return 164;
	}
	else if (number == -91)
	{
		return 165;
	}
	else if (number == -90)
	{
		return 166;
	}
	else if (number == -89)
	{
		return 167;
	}
	else if (number == -88)
	{
		return 168;
	}
	else if (number == -87)
	{
		return 169;
	}
	else if (number == -86)
	{
		return 170;
	}
	else if (number == -85)
	{
		return 171;
	}
	else if (number == -84)
	{
		return 172;
	}
	else if (number == -83)
	{
		return 173;
	}
	else if (number == -82) {
		return 174;
	}
	else if (number == -81) {
		return 175;
	}
	else if (number == -80) {
		return 176;
	}
	else if (number == -79) {
		return 177;
	}
	else if (number == -78) {
		return 178;
	}
	else if (number == -77) {
		return 179;
	}
	else if (number == -76) {
		return 180;
	}
	else if (number == -75) {
		return 181;
	}
	else if (number == -74) {
		return 182;
	}
	else if (number == -73) {
		return 183;
	}
	else if (number == -72) {
		return 184;
	}
	else if (number == -71) {
		return 185;
	}
	else if (number == -70) {
		return 186;
	}
	else if (number == -69) {
		return 187;
	}
	else if (number == -68) {
		return 188;
	}
	else if (number == -67) {
		return 189;
	}
	else if (number == -66) {
		return 190;
	}
	else if (number == -65) {
		return 191;
	}
	else if (number == -64) {
		return 192;
	}
	else if (number == -63) {
		return 193;
	}
	else if (number == -62) {
		return 194;
	}
	else if (number == -61) {
		return 195;
	}
	else if (number == -60) {
		return 196;
	}
	else if (number == -59) {
		return 197;
	}
	else if (number == -58) {
		return 198;
	}
	else if (number == -57) {
		return 199;
	}
	else if (number == -56) {
		return 200;
	}
	return 0;
}