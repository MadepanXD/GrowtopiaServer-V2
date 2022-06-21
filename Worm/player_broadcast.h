#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <experimental/filesystem>
using namespace std;

class Send_ {
	public:
		static void console_msg(ENetPeer* peer, string t_);
		static void OSM(ENetPeer* peer, int item_dash, string url_path, string cdn_path, string web_path, string proto_info, unsigned int tribute);
		static void dialog_(ENetPeer* peer, string text);
		static void set_growID(ENetPeer* peer, int c_t, string g_id, string p_id);
		static void emoticon(ENetPeer* peer, int c_t, string text);
		static void gemupdate_(ENetPeer* peer, int c_t, int acc_state);
		static void notif_(ENetPeer* peer, string txt, string audio_, string _image);
		static void talk_bubble(ENetPeer* peer, int netID, string txt, int chat_Color, bool overlay);
		static void OnSpawn(ENetPeer* peer, string txt);
		static void setpos_(ENetPeer* peer, int netID, float x, float y);
		static void OnRemove(ENetPeer* peer, int netID);
		static void audio_(ENetPeer* peer, string file_, int delay);
		static void OnPlayPositioned(ENetPeer* peer, string file_, int netID, bool todo_all);
		static void change_name(ENetPeer* peer, int netID, string name);
		static void set_weather(ENetPeer* peer, int weather);
		static void overlay_(ENetPeer* peer, string txt);
		static void Serverfoward_(ENetPeer* peer, int userID, int token, string ip, int port, string doorId, int lmode);
};

inline bool isHere2(ENetPeer* peer, ENetPeer* peer2) {
	return pInfo(peer)->currentWorld == pInfo(peer2)->currentWorld;
}
inline void Send_::console_msg(ENetPeer* peer, string t_) {
	gamepacket_t p(95);
	p.Insert("OnConsoleMessage");
	p.Insert(t_);
	p.CreatePacket(peer);
}
inline void Send_::OSM(ENetPeer* peer, int item_dash, string url_path, string cdn_path, string web_path, string proto_info, unsigned int tribute) {
	gamepacket_t p;
	p.Insert("OnSuperMainStartAcceptLogonHrdxs47254722215a");
	p.Insert(item_dash), p.Insert(url_path);
	p.Insert(cdn_path), p.Insert(web_path);
	p.Insert(proto_info), p.Insert(tribute);
	p.CreatePacket(peer);
}
inline void Send_::dialog_(ENetPeer* peer, string text) {
	gamepacket_t p(400);
	p.Insert("OnDialogRequest");
	p.Insert(text);
	p.CreatePacket(peer);
}
inline void Send_::set_growID(ENetPeer* peer, int c_t, string g_id, string p_id) {
	gamepacket_t p;
	p.Insert("SetHasGrowID");
	p.Insert(c_t);
	p.Insert(g_id);
	p.Insert(p_id);
	p.CreatePacket(peer);
}
inline void Send_::emoticon(ENetPeer* peer, int c_t, string text) {
	gamepacket_t p;
	p.Insert("OnEmoticonDataChanged");
	p.Insert(c_t);
	p.Insert(text);
	p.CreatePacket(peer);
}
inline void Send_::gemupdate_(ENetPeer* peer, int c_t, int acc_state) {
	gamepacket_t p;
	p.Insert("OnSetBux");
	p.Insert(c_t);
	p.Insert(acc_state);
	p.CreatePacket(peer);
}
inline void Send_::notif_(ENetPeer* peer, string txt, string audio_, string _image) {
	gamepacket_t p;
	p.Insert("OnAddNotification");
	p.Insert(_image);
	p.Insert(txt);
	p.Insert(audio_);
	p.CreatePacket(peer);
}
inline void Send_::talk_bubble(ENetPeer* peer, int netID, string txt, int chat_Color, bool overlay) {
	gamepacket_t p(180);
	p.Insert("OnTalkBubble");
	p.Insert(netID);
	p.Insert(txt);
	p.Insert(chat_Color);
	p.Insert(overlay ? 1 : 0);
	p.CreatePacket(peer);
}
inline void Send_::OnSpawn(ENetPeer* peer, string txt) {
	gamepacket_t p;
	p.Insert("OnSpawn");
	p.Insert(txt);
	p.CreatePacket(peer);
}
inline void Send_::set_weather(ENetPeer* peer, int weather) {
	gamepacket_t p(0);
	p.Insert("OnSetCurrentWeather");
	p.Insert(weather);
	p.CreatePacket(peer);
}
inline void Send_::setpos_(ENetPeer* peer, int netID, float x, float y) {
	gamepacket_t p(0, netID);
	p.Insert("OnSetPos");
	p.Insert(x);
	p.Insert(y);
	p.CreatePacket(peer);
}
inline void Send_::OnRemove(ENetPeer* peer, int netID) {
	gamepacket_t p;
	p.Insert("OnRemove");
	p.Insert("netID|" + to_string(netID) + "");
	p.CreatePacket(peer);
}
inline void Send_::audio_(ENetPeer* peer, string file_, int delay) {
	string w_ = "action|play_sfx\nfile|" + file_ + "\ndelayMS|" + to_string(delay) + "\n";
	int y_ = 3;
	BYTE z_ = 0;
	BYTE* const d_ = new BYTE[5 + w_.length()];
	memcpy(d_, &y_, 4);
	memcpy(d_ + 4, w_.c_str(), w_.length());
	memcpy(d_ + 4 + w_.length(), &z_, 1);
	ENetPacket* const p = enet_packet_create(d_, 5 + w_.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, p);
	delete[] d_;
}
inline void Send_::OnPlayPositioned(ENetPeer* peer, string file_, int netID, bool todo_all) {
	gamepacket_t p(0, netID);
	p.Insert("OnPlayPositioned");
	p.Insert(file_);
	if (not todo_all) {
		p.CreatePacket(peer);
		return;
	}
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
		if (isHere2(peer, net_p)) {
			p.CreatePacket(net_p);
		}
	}
}
inline void Send_::change_name(ENetPeer* peer, int netID, string name) {
	gamepacket_t p(0, netID);
	p.Insert("OnNameChanged");
	p.Insert(name);
	p.CreatePacket(peer);
}
inline void Send_::overlay_(ENetPeer* peer, string txt) {
	gamepacket_t p(150);
	p.Insert("OnTextOverlay");
	p.Insert(txt);
	p.CreatePacket(peer);
}
inline void Send_::Serverfoward_(ENetPeer* peer, int userID, int token, string ip, int port, string doorId, int lmode) {
	gamepacket_t p;
	p.Insert("OnSendToServer"), p.Insert(port), p.Insert(token), p.Insert(userID), p.Insert(ip + "|" + doorId), p.Insert(lmode);
	p.CreatePacket(peer);
}