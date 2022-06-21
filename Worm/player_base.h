#pragma once
#include "enet/enet.h"
#include "include/httplib.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <experimental/filesystem>
using namespace std;

struct InventoryItem {
	size_t itemID;
	size_t itemCount;
};
struct PlayerInventory {
	vector<InventoryItem> items;
};
struct PlayerInfo {
	//gw_id
	PlayerInventory inventory;
	vector<string> friendinfo{};
	vector<string> worldsowned{};

	string currentWorld = "", previousworld = "";
	//login_info
	int gems = 0;
	short currentInventorySize = 0;
	int peer_id = 0, xp = 0, userID = 0, skin = 0, level = 0, adminLevel = 0, effect = 0, netID = -1, x = -1, y = -1, update_req = 0, characterState = 0, tile_x = 0, tile_y = 0, tile_xy = 0, accwl_x = 0, accwl_y = 0;
	int cloth_back = 0, cloth_hand = 0, cloth_face = 0, cloth_shirt = 0, cloth_pants = 0, cloth_neck = 0, cloth_hair = 0, cloth_feet = 0, cloth_mask = 0, cloth_ances = 0, lastdrop_ct = 0, lastdrop_id = 0, lasttrash_ct = 0, lasttrash_id = 0;
	bool inGame = false, check_login = false, passed_data = false, lobby_guest_acc = false, haveGrowId = false, block_login = false, joinClothesUpdated = false, isRotatedLeft = false, RotatedLeft = false, Supporter = false, accwl_confirm = false, isSusspend = false, isOfLegend = false, isMuted = false, isDrTitle = false, isCursed = false, isBlueName = false, canDoubleJump = false, isInv = false;
	bool isRoledName = false, canWalkInBlocks = false;
	string note = "";
	string lastwrench_ = "", lastwrenchname_ = "", lastwrenchDisplay_ = "";
	string tankIDName = "", tankIDPass = "", requestedName = "", country = "", rawName = "", displayName = "", displayNameBackup = "", lastworld = "", guild = "", UUIDToken = "none";
	string ip_ply = "", mac_ply = "", game_Version = "", meta = "", platformID = "";

	//ingame
	vector<int> nowLock_x{}, nowLock_y{};
	//ct
	int warn = 0;
	long long int chat_ct = 0, speed_hack = 0, inv_bandwith = 0;
	long long int world_join_left = 0, lastBreak = 0;
	long long playtime = 0, account_created = 0, seconds = 0;
};
struct PlayerMoving {
	int packetType;
	int netID;
	int secondNetId;
	float x;
	float y;
	int characterState;
	int plantingTree;
	float XSpeed;
	float YSpeed;
	int punchX;
	int punchY;
};

inline BYTE* packPlayerMoving(PlayerMoving* dataStruct, int size_ = 56) {
	BYTE* data = new BYTE[size_];
	memset(data, 0, size_);
	memcpy(data, &dataStruct->packetType, 4);
	memcpy(data + 4, &dataStruct->netID, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 20, &dataStruct->plantingTree, 4);
	memcpy(data + 24, &dataStruct->x, 4);
	memcpy(data + 28, &dataStruct->y, 4);
	memcpy(data + 32, &dataStruct->XSpeed, 4);
	memcpy(data + 36, &dataStruct->YSpeed, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	return data;
}
inline PlayerMoving* unpackPlayerMoving(BYTE* data) {
	PlayerMoving* dataStruct = new PlayerMoving;
	memcpy(&dataStruct->packetType, data, 4);
	memcpy(&dataStruct->netID, data + 4, 4);
	memcpy(&dataStruct->characterState, data + 12, 4);
	memcpy(&dataStruct->plantingTree, data + 20, 4);
	memcpy(&dataStruct->x, data + 24, 4);
	memcpy(&dataStruct->y, data + 28, 4);
	memcpy(&dataStruct->XSpeed, data + 32, 4);
	memcpy(&dataStruct->YSpeed, data + 36, 4);
	memcpy(&dataStruct->punchX, data + 44, 4);
	memcpy(&dataStruct->punchY, data + 48, 4);
	return dataStruct;
}

#include "player_broadcast.h"
inline void ip_block(ENetPeer* peer) {
	Send_::Serverfoward_(peer, pInfo(peer)->userID, (rand() % 1000000), "0.0.0.0", 1945, to_string((rand() % 10000)), 1);
	string cmd = "netsh advfirewall firewall add rule name=\"IP-BLOCK: " + pInfo(peer)->ip_ply + "\" protocol=UDP dir=in action=block remoteip=" + pInfo(peer)->ip_ply;
	system(cmd.c_str()), enet_peer_disconnect_later(peer, 0);;
	return;
}
inline void Auto_ban(ENetPeer* peer, long long time_, string r_) {
	ofstream a_;
	a_.open("playerbanned/account/" + pInfo(peer)->rawName + ".txt");
	a_ << to_string(GetCurrentTimeInternalSeconds() + (time_ * 3600));
	a_.close();
	ofstream i_;
	i_.open("playerbanned/ip/" + to_string(peer->address.host) + ".txt");
	i_ << to_string(GetCurrentTimeInternalSeconds() + (time_ * 3600));
	i_.close();

	server_alert("AUTOBAN SYSTEM: " + pInfo(peer)->rawName + " / " + OutputBanTime(calcBanDuration(GetCurrentTimeInternalSeconds() + (time_ * 3600))) + "(" + r_ + ")");
	send_(peer, "action|log\nmsg|`oWarning from `4System`o: You've been `4AUTO-BANNED `ofrom GrowSenkai for " + OutputBanTime(calcBanDuration(GetCurrentTimeInternalSeconds() + (time_ * 3600))) + "");
	send_(peer, "action|log\nmsg|`oReality flickers as you begin to wake up. (Ban mod added, " + OutputBanTime(calcBanDuration(GetCurrentTimeInternalSeconds() + (time_ * 3600))) + " left.)");
	send_(peer, "action|log\nmsg|`oReason Ban: Using third 3rd client (" + r_ + ")");
	Send_::notif_(peer, "`0Warning from `4System`0: You've been `4AUTO-BANNED `0from GrowSenkai for " + OutputBanTime(calcBanDuration(GetCurrentTimeInternalSeconds() + (time_ * 3600))) + "", "audio/hub_open.wav", "interface/atomic_button.rttex");
	Send_::Serverfoward_(peer, pInfo(peer)->userID, (rand() % 1000000), "20.173.85.2", 1945, to_string((rand() % 10000)), 1);
	enet_peer_disconnect_later(peer, 0); //banned!
	return;
}
inline bool isHere(ENetPeer* peer, ENetPeer* peer2) {
	return pInfo(peer)->currentWorld == pInfo(peer2)->currentWorld;
}
inline bool isDev(ENetPeer* peer) {
	if (pInfo(peer)->adminLevel == 10) return true;
	else return false;
}
inline bool isMod(ENetPeer* peer) {
	if (pInfo(peer)->adminLevel >= 8) return true;
	else return false;
}
inline bool isVip(ENetPeer* peer) {
	if (pInfo(peer)->adminLevel == 2) return true;
	else return false;
}
inline void player_update(ENetPeer* peer) {
	if (pInfo(peer)->haveGrowId and pInfo(peer)->rawName != "" and peer->data != NULL and !pInfo(peer)->passed_data) {
		try {
			ifstream r_p("players/_" + pInfo(peer)->rawName + ".json");
			if (not r_p.is_open()) {
				enet_peer_disconnect_later(peer, 0);
				return;
			}
			json j;
			r_p >> j;
			r_p.close();
			string username = j["username"], friends = j["friends"], worldsowned = j["worldsowned"];
			pInfo(peer)->rawName = toLowerText(username);
			pInfo(peer)->displayName = j["displayName"].get<string>();
			pInfo(peer)->displayNameBackup = j["displayNameBackup"].get<string>();
			pInfo(peer)->cloth_back = j["Back"].get<int>(), pInfo(peer)->cloth_hand = j["Hand"].get<int>(), pInfo(peer)->cloth_face = j["Face"].get<int>(), pInfo(peer)->cloth_shirt = j["Shirt"].get<int>(), pInfo(peer)->cloth_pants = j["Pants"].get<int>(), pInfo(peer)->cloth_neck = j["Neck"].get<int>(), pInfo(peer)->cloth_hair = j["Hair"].get<int>(), pInfo(peer)->cloth_feet = j["Feet"].get<int>(), pInfo(peer)->cloth_ances = j["Ances"].get<int>();
			pInfo(peer)->canDoubleJump = j["canDoubleJump"].get<bool>();
			pInfo(peer)->isSusspend = j["isSusspend"].get<bool>();
			pInfo(peer)->isCursed = j["isCursed"].get<bool>();
			pInfo(peer)->isMuted = j["isMuted"].get<bool>();
			pInfo(peer)->isDrTitle = j["isDrTitle"].get<bool>();
			pInfo(peer)->isOfLegend = j["isOfLegend"].get<bool>();
			pInfo(peer)->Supporter = j["Supporter"].get<bool>();
			pInfo(peer)->effect = j["effect"].get<int>();
			pInfo(peer)->adminLevel = j["adminLevel"].get<int>();
			pInfo(peer)->guild = j["guild"].get<string>();
			pInfo(peer)->account_created = j["date"].get<long long int>();
			if (pInfo(peer)->account_created == 0) pInfo(peer)->account_created = time(NULL) / 86400;
			pInfo(peer)->playtime = time(NULL), pInfo(peer)->seconds = j["playtime"].get<long long>();
			pInfo(peer)->level = j["level"].get<int>();
			pInfo(peer)->gems = j["gems"].get<int>();
			pInfo(peer)->xp = j["xp"].get<int>();
			pInfo(peer)->skin = j["skin"].get<int>();
			pInfo(peer)->currentInventorySize = j["inventorySize"].get<int>();
			pInfo(peer)->userID = j["userID"].get<int>();
			pInfo(peer)->note = j["note"].get<string>();
			pInfo(peer)->lastworld = j["lastworld"].get<string>();
			json inventory = j["inventory"];
			stringstream s_f(friends);
			while (s_f.good()) {
				string str;
				getline(s_f, str, ',');
				if (str.size() == 0) continue;
				pInfo(peer)->friendinfo.push_back(str);
			}
			stringstream s_w(worldsowned);
			while (s_w.good()) {
				string str;
				getline(s_w, str, ',');
				if (str.size() == 0) continue;
				pInfo(peer)->worldsowned.push_back(str);
			}
			PlayerInventory inventory_data; {
				InventoryItem item;
				for (int i = 0; i < pInfo(peer)->currentInventorySize; i++) {
					int ct_ = inventory["items"].at(i)["q_"];
					if (ct_ > 200) ct_ = 200;
					if (inventory["items"].at(i)["i_"] != 0 and ct_ != 0) {
						item.itemCount = ct_;
						item.itemID = inventory["items"].at(i)["i_"];
						inventory_data.items.push_back(item);
					}
				}
			}
			pInfo(peer)->inventory = inventory_data;
			pInfo(peer)->passed_data = true;
			j.clear(); inventory.clear();
		}
		catch (exception& e) {
			cout << e.what() << endl;
			enet_peer_disconnect_later(peer, 0);
		}
	}
}
inline void save_pinfo(ENetPeer* peer) {
	string w_owned = "", friends = "";
	if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and not pInfo(peer) == NULL) {
		try {
			ifstream r_w("players/_" + pInfo(peer)->rawName + ".json");
			if (not r_w.is_open()) {
				return;
			}
			json j;
			r_w >> j;
			r_w.close();
			for (int i = 0; i < (int)pInfo(peer)->worldsowned.size(); i++) {
				w_owned += pInfo(peer)->worldsowned.at(i) + ",";
			}
			for (int i = 0; i < (int)pInfo(peer)->friendinfo.size(); i++) {
				friends += pInfo(peer)->friendinfo.at(i) + ",";
			}
			bool update_inv = false;
			json inv_items; json jjall = json::array(); json jj;
			for (int i = 0; i < 200; i++) {
				if (i < (int)pInfo(peer)->inventory.items.size()) {
					jj["a_"] = i + 1;
					jj["i_"] = pInfo(peer)->inventory.items.at(i).itemID;
					jj["q_"] = pInfo(peer)->inventory.items.at(i).itemCount;
					jjall.push_back(jj);
				}
				else {
					jj["a_"] = i + 1, jj["i_"] = 0, jj["q_"] = 0;
					jjall.push_back(jj);
				}
			}
			inv_items["items"] = jjall;
			update_inv = true;
			j["Back"] = pInfo(peer)->cloth_back, j["Hand"] = pInfo(peer)->cloth_hand, j["Face"] = pInfo(peer)->cloth_face, j["Shirt"] = pInfo(peer)->cloth_shirt, j["Pants"] = pInfo(peer)->cloth_pants, j["Neck"] = pInfo(peer)->cloth_neck, j["Hair"] = pInfo(peer)->cloth_hair, j["Feet"] = pInfo(peer)->cloth_feet, j["Mask"] = pInfo(peer)->cloth_mask, j["Ances"] = pInfo(peer)->cloth_ances;
			j["canDoubleJump"] = pInfo(peer)->canDoubleJump;
			j["isSusspend"] = pInfo(peer)->isSusspend;
			j["isCursed"] = pInfo(peer)->isCursed;
			j["isMuted"] = pInfo(peer)->isMuted;
			j["isDrTitle"] = pInfo(peer)->isDrTitle;
			j["Supporter"] = pInfo(peer)->Supporter;
			j["isOfLegend"] = pInfo(peer)->isOfLegend;
			j["effect"] = pInfo(peer)->effect;
			j["adminLevel"] = pInfo(peer)->adminLevel;
			j["guild"] = pInfo(peer)->guild;
			j["date"] = pInfo(peer)->account_created;
			j["playtime"] = pInfo(peer)->seconds + (time(NULL) - pInfo(peer)->playtime);
			j["gems"] = pInfo(peer)->gems;
			j["level"] = pInfo(peer)->level;
			j["xp"] = pInfo(peer)->xp;
			j["skin"] = pInfo(peer)->skin;
			j["note"] = pInfo(peer)->note;
			j["inventorySize"] = pInfo(peer)->currentInventorySize;
			j["friends"] = friends;
			j["worldsowned"] = w_owned;
			j["lastworld"] = pInfo(peer)->lastworld;
			j["username"] = toLowerText(pInfo(peer)->tankIDName);
			j["password"] = toLowerText(pInfo(peer)->tankIDPass);
			j["userID"] = pInfo(peer)->userID;
			j["xp"] = pInfo(peer)->xp;
			if (update_inv) j["inventory"] = inv_items;
			ofstream w_p("players/_" + pInfo(peer)->rawName + ".json");
			w_p << j << std::endl;
			w_p.close(); j.clear(); inv_items.clear(); jj.clear(); jjall.clear();
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}
}
inline void player_login(ENetPeer* peer, string cch) {
	try {
		vector<string> a_pkt = explode("|", replace_str(cch, "\n", "|"));
		for (int i_ = 0; i_ < (int)a_pkt.size(); i_++) {
			if (a_pkt.at(i_) == "tankIDName") {
				pInfo(peer)->tankIDName = a_pkt.at(i_ + 1);
				ifstream ifs("players/_" + toLowerText(pInfo(peer)->tankIDName) + ".json");
				if (ifs.is_open()) {
					pInfo(peer)->haveGrowId = true;
					pInfo(peer)->rawName = toLowerText(pInfo(peer)->tankIDName);
				}
				else {
					send_(peer, "`4Unable to log on: `oThat `wGrowID`` doesn't seem valid, or the password is wrong. If you don't have one, press `wCancel``, un-check `w'I have a GrowID'``, then click `wConnect``.", "Retrieve lost password");
					send_(peer, "action|logon_fail");
					pInfo(peer)->block_login = true;
				}
				ifs.close();
			}
			else if (a_pkt.at(i_) == "tankIDPass") {
				pInfo(peer)->tankIDPass = a_pkt.at(i_ + 1);
			}
			else if (a_pkt.at(i_) == "requestedName") {
				pInfo(peer)->requestedName = a_pkt.at(i_ + 1);
			}
			else if (a_pkt.at(i_) == "country") {
				pInfo(peer)->country = a_pkt.at(i_ + 1);
				if (pInfo(peer)->country.length() > 4) {
					send_(peer, "action|log\nmsg|`4Unable to log on: `oSomething wrong about your account!");
					send_(peer, "action|logon_fail");
				}
			}
			else if (a_pkt.at(i_) == "mac") {
				pInfo(peer)->mac_ply = a_pkt.at(i_ + 1);
			}
			else if (a_pkt.at(i_) == "game_version") {
				pInfo(peer)->game_Version = a_pkt.at(i_ + 1);
			}
			else if (a_pkt.at(i_) == "meta") {
				pInfo(peer)->meta = a_pkt.at(i_ + 1);
			}
			else if (a_pkt.at(i_) == "platformID") {
				pInfo(peer)->platformID = a_pkt.at(i_ + 1);
				if (a_pkt.at(i_ + 1) == "2") {
					send_(peer, "action|log\nmsg|`4Linux Device is Unsupported This Server!");
					send_(peer, "action|logon_fail");
				}
			}
		}
	}
	catch (exception& e) {
		cout << e.what() << endl;
	}
	if (pInfo(peer)->haveGrowId) {
		try {
			ifstream ply("players/_" + toLowerText(pInfo(peer)->tankIDName) + ".json");
			json j;
			ply >> j;
			ply.close();
			string p_json = j["password"], p_req = toLowerText(pInfo(peer)->tankIDPass);
			bool is_spnd = j["isSusspend"], exist = experimental::filesystem::exists("playerbanned/ip/" + to_string(peer->address.host) + ".txt"), exist_2 = experimental::filesystem::exists("playerbanned/account/" + pInfo(peer)->rawName + ".txt");
			if (p_json != p_req) {
				send_(peer, "`4Unable to log on: `oThat `wGrowID`` doesn't seem valid, or the password is wrong. If you don't have one, press `wCancel``, un-check `w'I have a GrowID'``, then click `wConnect``.", "Retrieve lost password");
				send_(peer, "action|logon_fail");
				pInfo(peer)->block_login = true;
			}
			else if (exist) {
				string c_t = "0";
				if (experimental::filesystem::exists("playerbanned/ip/" + to_string(peer->address.host) + ".txt")) {
					ifstream i_f("playerbanned/ip/" + to_string(peer->address.host) + ".txt");
					string c_f((istreambuf_iterator<char>(i_f)), (istreambuf_iterator<char>()));
					c_t = c_f;
					i_f.close();
				}
				long long ban_d = atoi(c_t.c_str());
				int b_tleft = calcBanDuration(ban_d);
				if (b_tleft > 0) {
					send_(peer, "action|log\nmsg|`4Sorry, this account, device or location has been temporarily suspended.");
					send_(peer, "action|log\nmsg|`oif you didn't do anything wrong, it could be because you're playing from the same place or the same device as someone who did.");
					send_(peer, "action|log\nmsg|`oThis is a temporarily ban caused by `w" + pInfo(peer)->tankIDName + " `oand will be removed in `w" + OutputBanTime(calcBanDuration(ban_d)) + "`o. if that's not your name, try playing from another location or device to fix it!");
					send_(peer, "action|logon_fail");
					pInfo(peer)->block_login = true;
				}
			}
			else if (exist_2) {
				string c_t = "0";
				if (experimental::filesystem::exists("playerbanned/account/" + pInfo(peer)->rawName + ".txt")) {
					ifstream i_f("playerbanned/account/" + pInfo(peer)->rawName + ".txt");
					string c_f((istreambuf_iterator<char>(i_f)), (istreambuf_iterator<char>()));
					c_t = c_f;
					i_f.close();
				}
				long long ban_d = atoi(c_t.c_str());
				int b_tleft = calcBanDuration(ban_d);
				if (b_tleft > 0) {
					send_(peer, "action|log\nmsg|`4Sorry, this account is currently banned. You will have to wait `w" + OutputBanTime(calcBanDuration(ban_d)) + " `4for this ban to expire so you can use this account again.");
					send_(peer, "action|logon_fail");
					pInfo(peer)->block_login = true;
				}
			}
			else if (is_spnd) {
				send_(peer, "`4Sorry, this account [`5" + pInfo(peer)->tankIDName + "`4] has been suspended. Contact `#Panji#2249 `4in Discord if you have any questions.", "Contact Server");
				send_(peer, "action|logon_fail");
				pInfo(peer)->block_login = true;
			}
			else if ((pInfo(peer)->platformID == "0" or pInfo(peer)->platformID == "3") and pInfo(peer)->game_Version != SERVER_GAME_VERSION) {
				send_(peer, "`4UPDATE REQUIRED`o : The `$V" + SERVER_GAME_VERSION + " `oupdate is now avallable for your device. Go get it! You'll need that before you can login into private server.", "Download Latest Version", "https://www.growtopiagame.com/");
				send_(peer, "action|logon_fail");
				pInfo(peer)->block_login = true;
			}
			else if ((pInfo(peer)->platformID == "1" or pInfo(peer)->platformID == "4") and pInfo(peer)->game_Version != SERVER_ANDROID_GAME_VERSION) {
				send_(peer, "`4UPDATE REQUIRED`o : The `$V" + SERVER_ANDROID_GAME_VERSION + " `oupdate is now avallable for your device. Go get it! You'll need that before you can login into private server.", "Download Latest Version", "https://www.growtopiagame.com/");
				send_(peer, "action|logon_fail");
				pInfo(peer)->block_login = true;
			}
			else {
				pInfo(peer)->inGame = true;
				for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
					if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p == peer) continue;
					string upper_peer_name = pInfo(net_p)->rawName;
					if (upper_peer_name == toLowerText(pInfo(peer)->tankIDName) and pInfo(net_p) != pInfo(peer)) {
						Send_::console_msg(peer, "`4ALREADY ON?! `o: This account was already online, kicking it off so you can log on. (if you were just playing before, this is nothing to worry about)");
						save_pinfo(net_p);
						enet_peer_disconnect_later(net_p, 0);
						break;
					}
				}
				player_update(peer);
			}
			j.clear();
		}
		catch (exception& m) {
			cout << m.what() << endl;
			enet_peer_disconnect_later(peer, 0);
		}
	}
	else {
		bool exist = experimental::filesystem::exists("playerbanned/ip/" + to_string(peer->address.host) + ".txt");
		if (pInfo(peer)->game_Version != SERVER_GAME_VERSION) {
			send_(peer, "`4UPDATE REQUIRED`o : The `$V" + SERVER_GAME_VERSION + " `oupdate is now avallable for your device. Go get it! You'll need that before you can login into private server.", "Download Latest Version", "https://www.growtopiagame.com/");
			send_(peer, "action|logon_fail");
			pInfo(peer)->block_login = true;
		}
		else if (exist) {
			string c_t = "0";
			if (experimental::filesystem::exists("playerbanned/ip/" + to_string(peer->address.host) + ".txt")) {
				ifstream i_f("playerbanned/ip/" + to_string(peer->address.host) + ".txt");
				string c_f((istreambuf_iterator<char>(i_f)), (istreambuf_iterator<char>()));
				c_t = c_f;
				i_f.close();
			}
			long long ban_d = atoi(c_t.c_str());
			int b_tleft = calcBanDuration(ban_d);
			if (b_tleft > 0) {
				send_(peer, "action|log\nmsg|`4Sorry, this account, device or location has been temporarily suspended.");
				send_(peer, "action|log\nmsg|`oif you didn't do anything wrong, it could be because you're playing from the same place or the same device as someone who did.");
				send_(peer, "action|log\nmsg|`oThis is a temporarily ban caused by `wSomeone `oand will be removed in `w" + OutputBanTime(calcBanDuration(ban_d)) + "`o. if that's not your name, try playing from another location or device to fix it!");
				send_(peer, "action|logon_fail");
				pInfo(peer)->block_login = true;
			}
		}
		pInfo(peer)->inGame = true;
	}
	Send_::OSM(peer, itemdathash, "ubistatic-a.akamaihd.net", SERVER_CDN, "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", "proto=161|choosemusic=audio/mp3/tsirhc.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=33190254|clash_active=1|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|", 2357275721);
}
inline void player_reg(ENetPeer* peer, string user_, string pass, string pass_v, string email) {
	string err_ = "";
	bool err_b = false;
	if (user_.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != string::npos or pass.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#") != string::npos or pass_v.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#") != string::npos or email.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@.") != string::npos) {
		err_ += "`4Oops! `wYou can only use letters and numbers in your GrowID Data.", err_b = true;
	} else if (experimental::filesystem::exists("players/_" + toLowerText(user_) + ".json")) {
		err_ += "`4Oops!`` The name `w" + user_ + "`` is so cool someone else has already taken it.  Please choose a different name.", err_b = true;
	} else if (user_.size() < 3 or user_.size() > 18) {
		err_ += "`4Oops! `wYour `wGrowID`` must be between `$3`` and `$18`` characters long.", err_b = true;
	} else if (pass.size() < 8 or pass.size() > 18) {
		err_ += "`4Oops! `wYour password must be between `$8`` and `$18`` characters long.", err_b = true;
	} else if (toLowerText(pass) != toLowerText(pass_v) or pass_v.size() < 8 or pass_v.size() > 18) {
		err_ += "`4Oops! `wPasswords don't match. Try again.", err_b = true;
	} else if (not email_(email)) {
		err_ += "`4Oops! `wLook, if you'd like to be able try retrieve your password if you lose it, you'd better enter a real email.  We promise to keep your data 100% private and never spam you.", err_b = true;
	} if (err_b) {
		Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`wGet a GrowID``|left|206|\nadd_spacer|small|\nadd_smalltext|" + err_ + "|\nadd_textbox|By choosing a `wGrowID``, you can use a name and password to logon from any device.Your `wname`` will be shown to other players!|left|\nadd_spacer|small|\nadd_text_input|username|Name||18|\nadd_textbox|Your `wpassword`` must contain `w8 to 18 characters, 1 letter, 1 number`` and `w1 special character: @#!$^&*.,``|left|\nadd_text_input_password|password|Password||18|\nadd_text_input_password|passwordverify|Password Verify||18|\nadd_textbox|Your `wemail`` will only be used for account verification and support. If you enter a fake email, you can't verify your account, recover or change your password.|left|\nadd_text_input|email|Email||64|\nadd_textbox|We will never ask you for your password or email, never share it with anyone!|left|\nadd_button|creategrowid|`wGet My GrowID!|\n");
		return;
	} else {
		if (experimental::filesystem::exists("players/ipreg/_" + pInfo(peer)->ip_ply + ".txt")) {
			auto r_ipc = 0;
			ifstream g_p("players/ipreg/_" + pInfo(peer)->ip_ply + ".txt");
			g_p >> r_ipc; r_ipc += 1; g_p.close();
			if (r_ipc >= 3) {
				err_ += "`4Oops! `wYou have too much accounts created.", err_b = true;
				Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`wGet a GrowID``|left|206|\nadd_spacer|small|\nadd_smalltext|" + err_ + "|\nadd_textbox|By choosing a `wGrowID``, you can use a name and password to logon from any device.Your `wname`` will be shown to other players!|left|\nadd_spacer|small|\nadd_text_input|username|Name||18|\nadd_textbox|Your `wpassword`` must contain `w8 to 18 characters, 1 letter, 1 number`` and `w1 special character: @#!$^&*.,``|left|\nadd_text_input_password|password|Password||18|\nadd_text_input_password|passwordverify|Password Verify||18|\nadd_textbox|Your `wemail`` will only be used for account verification and support. If you enter a fake email, you can't verify your account, recover or change your password.|left|\nadd_text_input|email|Email||64|\nadd_textbox|We will never ask you for your password or email, never share it with anyone!|left|\nadd_button|creategrowid|`wGet My GrowID!|\n");
				return;
			}
			ofstream w_p("players/ipreg/_" + pInfo(peer)->ip_ply + ".txt");
			w_p << r_ipc; w_p.close();
		} else {
			ofstream w_p("players/ipreg/_" + pInfo(peer)->ip_ply + ".txt");
			w_p << 1; w_p.close();
		}
		totaluserids++;
		if (totaluserids == 1) totaluserids++;
		try {
			ifstream p_data("players/_" + toLowerText(user_) + ".json");
			if (p_data.is_open()) {
				p_data.close();
				enet_peer_disconnect_later(peer, 0);
				return;
			}
			json items, j_, j_l = json::array();
			j_["a_"] = 1, j_["i_"] = 18, j_["q_"] = 1;
			j_l.push_back(j_);
			j_["a_"] = 2, j_["i_"] = 32, j_["q_"] = 1;
			j_l.push_back(j_);
			j_["a_"] = 3, j_["i_"] = 98, j_["q_"] = 1;
			j_l.push_back(j_);
			j_["a_"] = 4, j_["i_"] = 6336, j_["q_"] = 1;
			j_l.push_back(j_);
			j_["a_"] = 5, j_["i_"] = 4584, j_["q_"] = 200;
			j_l.push_back(j_);
			for (int i = 5; i < 200; i++) {
				j_["a_"] = i + 1, j_["i_"] = 0, j_["q_"] = 0;
				j_l.push_back(j_);
			}
			items["items"] = j_l;
			ofstream w_ply("players/_" + toLowerText(user_) + ".json");
			if (not w_ply.is_open()) {
				enet_peer_disconnect_later(peer, 0);
				return;
			}
			time_t a_;
			a_ = time(NULL);
			int days_ = int(a_) / (60 * 60 * 24);
			json j;
			j["username"] = toLowerText(user_);
			j["password"] = toLowerText(pass);
			j["displayName"] = user_;
			j["displayNameBackup"] = user_;
			j["email"] = email;
			j["Back"] = 0;
			j["Hand"] = 0;
			j["Face"] = 0;
			j["Shirt"] = 0;
			j["Pants"] = 0;
			j["Neck"] = 0;
			j["Hair"] = 0;
			j["Feet"] = 0;
			j["Mask"] = 0;
			j["Ances"] = 0;
			j["canDoubleJump"] = false;
			j["isSusspend"] = false;
			j["isCursed"] = false;
			j["isMuted"] = false;
			j["isDrTitle"] = false;
			j["Supporter"] = false;
			j["isOfLegend"] = false;
			j["effect"] = 0x808000;
			j["adminLevel"] = 0;
			j["guild"] = "";
			j["date"] = days_;
			j["playtime"] = 0;
			j["level"] = 1;
			j["gems"] = 10000;
			j["xp"] = 0;
			j["note"] = "";
			j["skin"] = 0x8295C3FF;
			j["inventorySize"] = 16;
			j["friends"] = "";
			j["worldsowned"] = "";
			j["mac"] = pInfo(peer)->mac_ply;
			j["ip"] = pInfo(peer)->ip_ply;
			j["userID"] = totaluserids;
			j["lastworld"] = "";
			j["ipID"] = to_string(peer->address.host);
			j["inventory"] = items;
			w_ply << j << std::endl;
			w_ply.close(); 
			j.clear(); items.clear(); j_.clear(); j_l.clear();
		}
		catch (exception& e) {
			cout << e.what() << std::endl;
			enet_peer_disconnect_later(peer, 0);
		}
		ofstream f_uid;
		f_uid.open("uids.txt");
		f_uid << to_string(totaluserids);
		f_uid.close();

		pInfo(peer)->lobby_guest_acc = false;
		server_alert("NEW ACCOUNT: " + toLowerText(user_) + " / " + pInfo(peer)->ip_ply + "");
		gamepacket_t p;
		p.Insert("SetHasGrowID");
		p.Insert(1), p.Insert(user_), p.Insert(pass);
		p.CreatePacket(peer);
		Send_::console_msg(peer, "`cYou will be disconnected for a while after creating an account, come back again!");
		send_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0\n");
		enet_peer_disconnect_later(peer, 0);
		return;
	}
}
inline void settingname_(ENetPeer* peer) {
	if (pInfo(peer)->adminLevel >= 2) pInfo(peer)->isRoledName = true;
	switch (pInfo(peer)->adminLevel) {
		case 10:
			pInfo(peer)->displayName = "`6DEV `w" + pInfo(peer)->displayNameBackup;
			break;
		case 8:
			pInfo(peer)->displayName = "`#MOD `w" + pInfo(peer)->displayNameBackup;
			break;
		case 2:
			pInfo(peer)->displayName = "`cVIP `w" + pInfo(peer)->displayNameBackup;
			break;
		case 0:
			pInfo(peer)->displayName = "`w" + pInfo(peer)->displayNameBackup;
			break;
	}
	if (not pInfo(peer)->isRoledName) {
		if (pInfo(peer)->level >= 125) pInfo(peer)->isBlueName = true;
		if (pInfo(peer)->isOfLegend and not pInfo(peer)->isDrTitle) {
			pInfo(peer)->displayName = pInfo(peer)->displayNameBackup + " of Legend";
			return;
		}
		else if (pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
			pInfo(peer)->displayName = "`4Dr. " + pInfo(peer)->displayNameBackup;
			return;
		}
		else if (pInfo(peer)->isDrTitle and pInfo(peer)->isOfLegend) {
			pInfo(peer)->displayName = pInfo(peer)->displayNameBackup + " of Legend";
			return;
		}
	}
}
inline int getState(ENetPeer* peer) {
	int v_ = 0;
	v_ |= pInfo(peer)->canWalkInBlocks << 0;
	v_ |= pInfo(peer)->canDoubleJump << 1;
	v_ |= pInfo(peer)->isCursed << 12;
	v_ |= pInfo(peer)->isRoledName << 24;
	return v_;
}
inline void send_state(ENetPeer* peer) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			PlayerMoving d_;
			d_.packetType = 0x14;
			d_.characterState = 0;
			d_.x = 1000, d_.y = 400;
			d_.punchX = 0, d_.punchY = 0;
			d_.XSpeed = 250, d_.YSpeed = 1100;
			d_.netID = pInfo(peer)->netID;
			d_.plantingTree = getState(peer);
			BYTE* raw = packPlayerMoving(&d_);
			int p_ = pInfo(peer)->effect;
			float w_ = 125.0f;
			memcpy(raw + 1, &p_, 3);
			memcpy(raw + 16, &w_, 4);
			send_raw(4, raw, 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
		}
	}
}
inline void update_set(ENetPeer* peer, bool case_ = false, int p_tree = 0) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			gamepacket_t p(0, pInfo(peer)->netID);
			p.Insert("OnSetClothing");
			p.Insert((float)pInfo(peer)->cloth_hair, (float)pInfo(peer)->cloth_shirt, (float)pInfo(peer)->cloth_pants);
			p.Insert((float)pInfo(peer)->cloth_feet, (float)pInfo(peer)->cloth_face, (float)pInfo(peer)->cloth_hand);
			p.Insert((float)pInfo(peer)->cloth_back, (float)pInfo(peer)->cloth_mask, (float)pInfo(peer)->cloth_neck);
			p.Insert(pInfo(peer)->skin);
			p.CreatePacket(net_p);
		}
	}
}
inline void update_all_clothes(ENetPeer* peer) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			{
				gamepacket_t p(0, pInfo(peer)->netID);
				p.Insert("OnSetClothing");
				p.Insert((float)pInfo(peer)->cloth_hair, (float)pInfo(peer)->cloth_shirt, (float)pInfo(peer)->cloth_pants);
				p.Insert((float)pInfo(peer)->cloth_feet, (float)pInfo(peer)->cloth_face, (float)pInfo(peer)->cloth_hand);
				p.Insert((float)pInfo(peer)->cloth_back, (float)pInfo(peer)->cloth_mask, (float)pInfo(peer)->cloth_neck);
				p.Insert(pInfo(peer)->skin);
				p.CreatePacket(net_p);
			}
			{
				gamepacket_t p(0, pInfo(net_p)->netID);
				p.Insert("OnSetClothing");
				p.Insert((float)pInfo(net_p)->cloth_hair, (float)pInfo(net_p)->cloth_shirt, (float)pInfo(net_p)->cloth_pants);
				p.Insert((float)pInfo(net_p)->cloth_feet, (float)pInfo(net_p)->cloth_face, (float)pInfo(net_p)->cloth_hand);
				p.Insert((float)pInfo(net_p)->cloth_back, (float)pInfo(net_p)->cloth_mask, (float)pInfo(net_p)->cloth_neck);
				p.Insert(pInfo(net_p)->skin);
				p.CreatePacket(peer);
			}
		}
	}
}
inline void update_player_join(ENetPeer* peer) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			{
				gamepacket_t p(0, pInfo(peer)->netID);
				p.Insert("OnInvis");
				p.Insert(pInfo(peer)->isInv);
				p.CreatePacket(net_p);
			}
			{
				gamepacket_t p(0, pInfo(net_p)->netID);
				p.Insert("OnInvis");
				p.Insert(pInfo(net_p)->isInv);
				p.CreatePacket(peer);
			}
			if (pInfo(peer)->isBlueName) {
				gamepacket_t p(0, pInfo(peer)->netID);
				p.Insert("OnCountryState");
				p.Insert(pInfo(peer)->country);
				p.Insert("|showGuild|maxLevel");
				p.CreatePacket(net_p);
			}
			if (pInfo(net_p)->isBlueName) {
				gamepacket_t p(0, pInfo(net_p)->netID);
				p.Insert("OnCountryState");
				p.Insert(pInfo(net_p)->country);
				p.Insert("|showGuild|maxLevel");
				p.CreatePacket(peer);
			}
		}
	}
}
inline void peer_connect(ENetPeer* peer) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (peer != net_p) {
			if (isHere(peer, net_p)) {
				string invis_cp = "0", invis_ = "0", netid_ = to_string(pInfo(peer)->netID), netid_cp = to_string(pInfo(net_p)->netID), userid_cp = to_string(pInfo(net_p)->userID);
				if (pInfo(net_p)->isInv) invis_cp = "1";
				if (pInfo(peer)->isInv) invis_ = "1";
				Send_::OnSpawn(peer, "spawn|avatar\nnetID|" + netid_cp + "\nuserID|" + userid_cp + "\ncolrect|0|0|20|30\nposXY|" + to_string(pInfo(net_p)->x) + "|" + to_string(pInfo(net_p)->y) + "\nname|``" + pInfo(net_p)->displayName + "``\ncountry|" + pInfo(net_p)->country + "\ninvis|" + invis_cp + "\nmstate|0\nsmstate|0\n");
				Send_::OnSpawn(net_p, "spawn|avatar\nnetID|" + netid_ + "\nuserID|" + userid_cp + "\ncolrect|0|0|20|30\nposXY|" + to_string(pInfo(peer)->x) + "|" + to_string(pInfo(peer)->y) + "\nname|``" + pInfo(peer)->displayName + "``\ncountry|" + pInfo(peer)->country + "\ninvis|" + invis_ + "\nmstate|0\nsmstate|0\n");
			}
		}
	}
}
inline void send_inv(ENetPeer* peer, PlayerInventory inventory) {
	try {
		if (pInfo(peer)->currentWorld == "EXIT" or pInfo(peer)->currentWorld.empty()) return;
		if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer) != NULL) {
			const int inventoryLen = inventory.items.size();
			const int packetLen = 66 + (inventoryLen * 4) + 4;
			BYTE* data2 = new BYTE[packetLen];
			int MessageType = 0x4, PacketType = 0x9, NetID = -1, CharState = 0x8, val = 0;
			memset(data2, 0, packetLen), memcpy(data2, &MessageType, 4), memcpy(data2 + 4, &PacketType, 4);
			memcpy(data2 + 8, &NetID, 4), memcpy(data2 + 16, &CharState, 4);
			int endianInvVal = _byteswap_ulong(inventoryLen);
			memcpy(data2 + 66 - 4, &endianInvVal, 4);
			if (pInfo(peer)->haveGrowId) endianInvVal = _byteswap_ulong(pInfo(peer)->currentInventorySize - 1);
			else endianInvVal = _byteswap_ulong(2);
			memcpy(data2 + 66 - 8, &endianInvVal, 4);
			for (int i = 0; i < inventoryLen; i++) {
				val = 0, val |= inventory.items[i].itemID;
				val |= inventory.items[i].itemCount << 16;
				val &= 0x00FFFFFF, val |= 0x00 << 24;
				memcpy(data2 + (i * 4) + 66, &val, 4);
			}
			ENetPacket* packet3 = enet_packet_create(data2, packetLen, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet3);
			delete[] data2;
		}
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void search_inv(ENetPeer* peer, const int fItemid, const int fQuantity, bool& iscontains) {
	try {
		iscontains = false;
		for (auto i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
			if ((unsigned int)pInfo(peer)->inventory.items.at(i).itemID == (unsigned int)fItemid and (unsigned int)pInfo(peer)->inventory.items.at(i).itemCount >= (unsigned int)fQuantity) {
				iscontains = true;
				break;
			}
		}
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline int HasItemsInInv(ENetPeer* peer, int itemId, int count = -1) {
	try {
		PlayerInventory* inventory = &pInfo(peer)->inventory;
		int idx = 0;
		for (auto& a : inventory->items) {
			if (a.itemID == itemId) {
				if (a.itemCount >= (unsigned int)count or count == -1) return idx;
			}
			idx++;
		}
		return -1;
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void update_pset(ENetPeer* peer, const int id_, bool remove_inv = false) {
	bool valid = false;
	if (not remove_inv) {
		search_inv(peer, id_, 1, valid);
		if (not valid) return;
	}
	if (pInfo(peer)->cloth_ances == id_)
		pInfo(peer)->cloth_ances = 0, update_set(peer);
	else if (pInfo(peer)->cloth_back == id_)
		pInfo(peer)->cloth_back = 0, update_set(peer);
	else if (pInfo(peer)->cloth_face == id_)
		pInfo(peer)->cloth_face = 0, update_set(peer);
	else if (pInfo(peer)->cloth_feet == id_)
		pInfo(peer)->cloth_feet = 0, update_set(peer);
	else if (pInfo(peer)->cloth_hair == id_)
		pInfo(peer)->cloth_hair = 0, update_set(peer);
	else if (pInfo(peer)->cloth_hand == id_)
		pInfo(peer)->cloth_hand = 0, update_set(peer);
	else if (pInfo(peer)->cloth_mask == id_)
		pInfo(peer)->cloth_mask = 0, update_set(peer);
	else if (pInfo(peer)->cloth_neck == id_)
		pInfo(peer)->cloth_neck = 0, update_set(peer);
	else if (pInfo(peer)->cloth_pants == id_)
		pInfo(peer)->cloth_pants = 0, update_set(peer);
	else if (pInfo(peer)->cloth_shirt == id_)
		pInfo(peer)->cloth_shirt = 0, update_set(peer);
}
inline void remove_inv(ENetPeer* peer, int id_, int c_t) {
	try {
		bool valid = false;
		for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
			if (pInfo(peer)->inventory.items.at(i).itemID == id_) {
				if (unsigned int(pInfo(peer)->inventory.items.at(i).itemCount) > unsigned int(c_t) and unsigned int(pInfo(peer)->inventory.items.at(i).itemCount) != c_t) {
					pInfo(peer)->inventory.items.at(i).itemCount -= c_t;
				} else {
					pInfo(peer)->inventory.items.erase(pInfo(peer)->inventory.items.begin() + i);
				}
				send_inv(peer, pInfo(peer)->inventory);
				search_inv(peer, id_, 1, valid);
				if (not valid) {
					if (pInfo(peer)->cloth_ances == id_ or pInfo(peer)->cloth_back == id_
						or pInfo(peer)->cloth_feet == id_ or pInfo(peer)->cloth_face == id_
						or pInfo(peer)->cloth_hair == id_ or pInfo(peer)->cloth_hand == id_
						or pInfo(peer)->cloth_mask == id_ or pInfo(peer)->cloth_neck == id_
						or pInfo(peer)->cloth_pants == id_ or pInfo(peer)->cloth_shirt == id_) {
						Send_::OnPlayPositioned(peer, "audio/change_clothes.wav", pInfo(peer)->netID, false);
					}
					update_pset(peer, id_, true);
				}
				update_set(peer);
				break;
			}
		}
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void senddrop_(ENetPeer* peer, int netid, int x, int y, int item_, int c_t, BYTE bpkkaukntl) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			PlayerMoving d_;
			d_.packetType = 14;
			d_.x = (float)x, d_.y = (float)y;
			d_.netID = netid;
			d_.plantingTree = item_;
			float c_ = (float)c_t;
			BYTE val2 = bpkkaukntl;
			BYTE* raw = packPlayerMoving(&d_);
			memcpy(raw + 1, &val2, 1), memcpy(raw + 16, &c_, 4);
			send_raw(4, raw, 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
		}
	}
}
inline void remove_floatingItem(ENetPeer* peer, const int todo) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			const auto b = new BYTE[56];
			memset(b, 0, 56);
			*((int*)&b[0]) = 0xe;
			*((int*)&b[4]) = -2;
			*((int*)&b[8]) = -1;
			*((int*)&b[20]) = todo + 1;
			send_raw(4, b, 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
		}
	}
}
inline bool save_inv_check(int fItemid, int fQuantity, ENetPeer* peer, bool overlay_ = true) {
	size_t inv_size = pInfo(peer)->currentInventorySize;
	bool on_inv = false;
	for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
		if (pInfo(peer)->inventory.items.at(i).itemID == fItemid and pInfo(peer)->inventory.items.at(i).itemCount < 200) on_inv = true;
		if (pInfo(peer)->inventory.items.at(i).itemID == fItemid) {
			int m_item = pInfo(peer)->inventory.items.at(i).itemCount + fQuantity;
			if (m_item > 200) {
				if (overlay_) {
					Send_::talk_bubble(peer, pInfo(peer)->netID, "`4You already had max items!", 0, true);
				}
				return false;
			}
		}
	}
	if (pInfo(peer)->inventory.items.size() == inv_size - 1 and not on_inv) {
		if (overlay_) {
			Send_::talk_bubble(peer, pInfo(peer)->netID, "`4Your inventory is full!", 0, true);
		}
		return false;
	}
	if (not on_inv) {
		InventoryItem i_;
		i_.itemID = fItemid;
		i_.itemCount = fQuantity;
		pInfo(peer)->inventory.items.push_back(i_);
		send_inv(peer, pInfo(peer)->inventory), update_set(peer);
		return true;
	} else {
		for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
			if (pInfo(peer)->inventory.items.at(i).itemID == fItemid) {
				pInfo(peer)->inventory.items.at(i).itemCount += fQuantity;
				send_inv(peer, pInfo(peer)->inventory), update_set(peer);
				return true;
			}
		}
	}
	return false;
}
inline void SendTradeEffect(ENetPeer* peer, int id, int n1_, int n2_, int timeMs) {
	PlayerMoving p;
	p.packetType = 0x13;
	p.punchX = id;
	p.punchY = id;
	BYTE* raw = packPlayerMoving(&p);
	int three = 3;
	memcpy(raw + 3, &three, 1);
	memcpy(raw + 4, &n2_, 4);
	memcpy(raw + 8, &n1_, 4);
	memcpy(raw + 20, &timeMs, 4);
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, net_p)) {
			BYTE* raw2 = new BYTE[56];
			memcpy(raw2, raw, 56);
			send_raw(4, raw2, 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
		}
	}
	delete[] raw;
}
inline string get_ActEffect(ENetPeer* peer) {
	string effect_ = "";
	int hand_ = pInfo(peer)->cloth_hand, neck_ = pInfo(peer)->cloth_neck, back_ = pInfo(peer)->cloth_back, feet_ = pInfo(peer)->cloth_feet, mask_ = pInfo(peer)->cloth_mask;
	if (hand_ == 2754 or hand_ == 7830 or hand_ == 930 or hand_ == 1874 or hand_ == 6368 or hand_ == 2572 or hand_ == 1484 or hand_ == 734 or hand_ == 98 or hand_ == 1438 or hand_ == 4956 or hand_ == 1708) {
		effect_ += "|\nadd_label_with_icon|small|`wEnchanced Digging``|left|" + to_string(hand_) + "|";
	} if (pInfo(peer)->isMuted) {
		effect_ += "|\nadd_label_with_icon|small|`wDuct tape covered your mouth!``|left|408|";
	} if (hand_ == 9520 or hand_ == 9496 or neck_ == 6260) {
		int id_ = 0;
		if (neck_ == 6260) id_ = 6260;
		if (hand_ == 9520) id_ = 9520;
		if (hand_ == 9496) id_ = 9496;
		effect_ += "|\nadd_label_with_icon|small|`wTriple Break!``|left|" + to_string(id_) + "|";
	} if (hand_ == 6840) {
		effect_ += "|\nadd_label_with_icon|small|`wHarvester!``|left|" + to_string(hand_) + "|";
	} if (pInfo(peer)->canDoubleJump) {
		effect_ += "|\nadd_label_with_icon|small|`wDouble jump``|left|" + to_string(back_) + "|";
	} if (pInfo(peer)->canWalkInBlocks) {
		effect_ += "|\nadd_label_with_icon|small|`wI can walk in blocks. (Ghost)``|left|3106|";
	} if (pInfo(peer)->isInv) {
		effect_ += "|\nadd_label_with_icon|small|`wNinja, invisible to all!``|left|290|";
	} if (hand_ == 2952) {
		effect_ += "|\nadd_label_with_icon|small|`wDigger's Spade``|left|" + to_string(hand_) + "|";
	} if (hand_ == 5480) {
		effect_ += "|\nadd_label_with_icon|small|`wRayman's Fist``|left|" + to_string(hand_) + "|";
	} if (hand_ == 3066) {
		effect_ += "|\nadd_label_with_icon|small|`wFire Hose! Wuuufhh..``|left|" + to_string(hand_) + "|";
	} if (back_ == 1140 or back_ == 10718 or back_ == 160 or back_ == 2050 or back_ == 1762 or back_ == 4880) {
		effect_ += "|\nadd_label_with_icon|small|`wHigh Jump``|left|" + to_string(back_) + "|";
	} if (hand_ == 804 or hand_ == 802 or hand_ == 806 or hand_ == 808 or hand_ == 810 or hand_ == 812 or hand_ == 1330) {
		effect_ += "|\nadd_label_with_icon|small|`wFists O' Fury``|left|" + to_string(hand_) + "|";
	} if (hand_ == 4464 or hand_ == 6058 or hand_ == 768 or hand_ == 1016 or hand_ == 472) {
		effect_ += "|\nadd_label_with_icon|small|`wArmed And Dangerous``|left|" + to_string(hand_) + "|";
	} if (feet_ == 2220) {
		effect_ += "|\nadd_label_with_icon|small|`wTank Driver``|left|" + to_string(feet_) + "|";
	} if (mask_ == 1204) {
		effect_ += "|\nadd_label_with_icon|small|`wFocused Eyes``|left|" + to_string(mask_) + "|";
	}
	if (effect_.empty()) {
		effect_ += "|\nadd_label_with_icon|small|`wYou don't have any active effects!``|left|18|";
	}
	return effect_;
}
inline void sendPData(ENetPeer* peer, PlayerMoving* data) {
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		if (peer != net_p) {
			if (isHere(peer, net_p)) {
				data->netID = pInfo(peer)->netID;
				send_raw(4, packPlayerMoving(data), 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
			}
		}
	}
}
inline bool state_guard(ENetPeer* peer) {
	if (pInfo(peer)->x <= 0 and pInfo(peer)->y <= 0 and pInfo(peer)->characterState <= 0 and not pInfo(peer)->currentWorld.empty()) { //if p_type 0 was returned
		pInfo(peer)->warn++;
		if (pInfo(peer)->warn >= 2) {
			pInfo(peer)->warn, ip_block(peer);
			return false;
		}
	}
	return true;
}
inline bool spam_guard(ENetPeer* peer) {
	using namespace chrono;
	if (pInfo(peer)->chat_ct + 13000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
		pInfo(peer)->warn = 0, pInfo(peer)->chat_ct = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
	} else {
		pInfo(peer)->warn++;
		if (pInfo(peer)->warn >= 6) {
			pInfo(peer)->chat_ct += 2500;
			Send_::console_msg(peer, "`8>>`4Spam detected! `8Please wait a bit before typing anything else. Please note, any form of bot/macro/auto-paste will get all your accounts banned, so don't do it!");
			return false;
		}
	}
	return true;
}
inline bool speed_guard(ENetPeer* peer, PlayerMoving* pMov) {
	using namespace chrono;
	if (pInfo(peer)->speed_hack + 1000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
		pInfo(peer)->warn = 0, pInfo(peer)->speed_hack = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
	} else {
		pInfo(peer)->warn++;
		if (pInfo(peer)->warn >= 5) {
			server_alert("SPEED HACK DETECTED: " + pInfo(peer)->rawName + " / " + to_string(pMov->XSpeed));
			Auto_ban(peer, 24 * 3, "Speed Hack");
			return false;
		}
	}
	return true;
}
inline bool inv_guard(ENetPeer* peer) {
	using namespace chrono;
	if (pInfo(peer)->inv_bandwith + 4000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
		pInfo(peer)->warn = 0, pInfo(peer)->inv_bandwith = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
	} else {
		pInfo(peer)->warn++;
		if (pInfo(peer)->warn >= 4) return false;
	}
	return true;
}