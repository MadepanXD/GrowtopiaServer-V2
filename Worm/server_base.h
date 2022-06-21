#pragma once
#define Property_Zero 0
#define Property_NoSeed 1
#define Property_Dropless 2
#define Property_Beta 4
#define Property_Mod 8
#define Property_Chemical 12
#define Property_Untradable 16
#define Property_Wrenchable 32
#define Property_MultiFacing 64
#define Property_Permanent 128
#define Property_AutoPickup 256
#define Property_WorldLock 512
#define Property_NoSelf 1024
#define Property_RandomGrow 2048
#define Property_Public 4096
#define Property_Foreground 8192
#define Property_NoBlock 16384

#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <experimental/filesystem>
using namespace std;

inline void load_() {
	ifstream ifs("config/main.json");
	if (not ifs.is_open()) {
		server_alert("ERROR: Cannot load main.json for server_base!");
		while (1);
		exit(EXIT_FAILURE);
	}
	try {
		json j;
		ifs >> j;
		ifs.close();
		SERVER_PORT = j["port"].get<string>(), SERVER_IP = j["ip"].get<string>(), SERVER_META = j["meta"].get<string>(), SERVER_CDN = j["cdn"].get<string>(), SERVER_GAME_VERSION = j["version1"].get<string>(), SERVER_ANDROID_GAME_VERSION = j["version2"].get<string>();
		server_alert("Config Loaded!\n[INFO] Hosting on: " + SERVER_IP + ":" + SERVER_PORT + "\n[INFO] Server Game Version: PC[" + SERVER_GAME_VERSION + "] / ANDROID[" + SERVER_ANDROID_GAME_VERSION + "]");
		j.clear();
	}
	catch (...) {
		server_alert("ERROR: Invalid Config!");
		while (1);
		exit(EXIT_FAILURE);
	}
}
inline void desc_build() {
	try {
		ifstream infile("config/Descriptions.txt");
		for (string line; getline(infile, line);) {
			if (line.length() > 3 and line[0] != '/' && line[1] != '/') {
				auto ex = explode("|", line);
				if (atoi(ex.at(0).c_str()) + 1 < (int)itemDefs.size()) {
					itemDefs.at(atoi(ex[0].c_str())).description = ex[1];
					if (not (atoi(ex.at(0).c_str()) % 2)) {
						itemDefs.at(atoi(ex.at(0).c_str()) + 1).description = "This is a tree.";
					}
				}
			}
		}
	}
	catch (out_of_range& e) {
		server_alert(e.what());
	}
}
inline void build_() {
	int ct_ = -1;
	{
		ifstream file("items.dat", ios::binary | ios::ate);
		itemsDatSize = (int)file.tellg();
		itemsDat = new BYTE[60 + itemsDatSize];
		string asdf = "0400000010000000FFFFFFFF000000000800000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
		for (int i = 0; i < (int)asdf.length(); i += 2) {
			char x = char2(asdf[i]);
			x = x << 4, x += char2(asdf[i + 1]);
			memcpy(itemsDat + (i / 2), &x, 1);
			if (asdf.length() > 60 * 2) throw 0;
		}
		memcpy(itemsDat + 56, &itemsDatSize, 4);
		file.seekg(0, std::ios::beg);
		if (file.read((char*)(itemsDat + 60), itemsDatSize)) {
			uint8_t* data2;
			int size = 0;
			const char filename[] = "items.dat";
			size = (int)filesize(filename);
			data2 = getA((string)filename, &size, false, false);
			server_alert("Updating items data success! Hash: " + to_string(HashString((unsigned char*)data2, size)) + "");
			itemdathash = HashString((unsigned char*)data2, size);
			file.close();
		}
	}
	try {
		ifstream c_type("collisionType.txt");
		if (not c_type.is_open()) {
			server_alert("ERROR: Cannot load collisionType.txt for server_base!");
			while (1);
			exit(EXIT_FAILURE);
		}
		for (string line; getline(c_type, line);) {
			if (line.length() >= 3 and line[0] != '/' and line[1] != '/') {
				vector<string> ex = explode("|", line);
				ItemStateDefinition def;
				def.id = atoi(ex[0].c_str());
				def.dropChance = atoi(ex[2].c_str());
				def.collisionType = atoi(ex[1].c_str());
				itemStates.push_back(def);
			}
		}
		c_type.close();
	}
	catch (exception& e) {
		server_alert(e.what());
		while (1);
		exit(EXIT_FAILURE);
	}
	try {
		ifstream i_e("config/CoreData.txt");
		if (not i_e.is_open()) {
			server_alert("ERROR: Cannot load CoreData.txt for server_base!");
			while (1);
			exit(EXIT_FAILURE);
		}
		for (string line; getline(i_e, line);) {
			if (line.length() >= 9 and line[0] != '/' and line[1] != '/') {
				ItemDefinition def;
				vector<string> ex = explode("|", line), properties = explode(",", ex[3]);
				string b_type = ex[4], c_ = ex[9];
				def.id = atoi(ex[0].c_str());
				def.name = ex[1];
				def.rarity = atoi(ex[2].c_str());
				def.breakHits = atoi(ex[7].c_str());
				def.growTime = atoi(ex[8].c_str());
				def.properties = Property_Zero;
				for (auto& prop : properties) {
					if (prop == "NoSeed") {
						def.properties += Property_NoSeed;
					} else if (prop == "NoBlock") {
						def.properties += Property_NoBlock;
					} else if (prop == "Dropless") {
						def.properties += Property_Dropless;
					} else if (prop == "Beta") {
						def.properties += Property_Beta;
					} else if (prop == "Mod") {
						def.properties += Property_Mod;
					} else if (prop == "Untradable") {
						def.properties += Property_Untradable;
					} else if (prop == "Wrenchable") {
						def.properties += Property_Wrenchable;
					} else if (prop == "MultiFacing") {
						def.properties += Property_MultiFacing;
					} else if (prop == "Permanent") {
						def.properties += Property_Permanent;
					} else if (prop == "AutoPickup") {
						def.properties += Property_AutoPickup;
					} else if (prop == "WorldLock") {
						def.properties += Property_WorldLock;
					} else if (prop == "NoSelf") {
						def.properties += Property_NoSelf;
					} else if (prop == "RandomGrow") {
						def.properties += Property_RandomGrow;
					} else if (prop == "Public") {
						def.properties += Property_Public;
					} else if (prop == "Foreground") {
						def.properties += Property_Foreground;
					} else if (prop == "OnlyCombiner") {
						def.properties += Property_Chemical;
					}
				}
				if (b_type == "Foreground_Block") {
					def.blockType = BlockTypes::FOREGROUND;
				} else if (b_type == "Background_Block") {
					def.blockType = BlockTypes::BACKGROUND;
				} else if (b_type == "Seed") {
					def.blockType = BlockTypes::SEED;
				} else if (b_type == "Consummable") {
					def.blockType = BlockTypes::CONSUMABLE;
				} else if (b_type == "Pain_Block") {
					def.blockType = BlockTypes::PAIN_BLOCK;
				} else if (b_type == "Main_Door") {
					def.blockType = BlockTypes::MAIN_DOOR;
				} else if (b_type == "Mannequin") {
					def.blockType = BlockTypes::MANNEQUIN;
				} else if (b_type == "Bedrock") {
					def.blockType = BlockTypes::BEDROCK;
				} else if (b_type == "Door") {
					def.blockType = BlockTypes::DOOR;
				} else if (b_type == "Fist") {
					def.blockType = BlockTypes::FIST;
				} else if (b_type == "Sign") {
					def.blockType = BlockTypes::SIGN;
				} else if (b_type == "Back") {
					def.blockType = BlockTypes::BACKGROUND;
				} else if (b_type == "Background") {
					def.blockType = BlockTypes::BACKGROUND;
				} else if (b_type == "Wrench") {
					def.blockType = BlockTypes::WRENCH;
				} else if (b_type == "Checkpoint") {
					def.blockType = BlockTypes::CHECKPOINT;
				} else if (b_type == "Lock") {
					def.blockType = BlockTypes::LOCK;
				} else if (b_type == "Gateway") {
					def.blockType = BlockTypes::GATEWAY;
				} else if (b_type == "Clothing") {
					def.blockType = BlockTypes::CLOTHING;
				} else if (b_type == "Platform") {
					def.blockType = BlockTypes::PLATFORM;
				} else if (b_type == "SFX_Foreground") {
					def.blockType = BlockTypes::SFX_FOREGROUND;
				} else if (b_type == "Battle_Pet_Cage") {
					def.blockType = BlockTypes::BATTLE_PET_CAGE;
				} else if (b_type == "Painting_Easel") {
					def.blockType = BlockTypes::PAINTING_EASEL;
				} else if (b_type == "Gems") {
					def.blockType = BlockTypes::GEMS;
				} else if (b_type == "Toggleable_Foreground") {
					def.blockType = BlockTypes::TOGGLE_FOREGROUND;
				} else if (b_type == "Chemical_Combiner") {
					def.blockType = BlockTypes::CHEMICAL_COMBINER;
				} else if (b_type == "Treasure") {
					def.blockType = BlockTypes::TREASURE;
				} else if (b_type == "Deadly_Block") {
					def.blockType = BlockTypes::DEADLY;
				} else if (b_type == "Trampoline_Block") {
					def.blockType = BlockTypes::TRAMPOLINE;
				} else if (b_type == "Forge") {
					def.blockType = BlockTypes::FORGE;
				} else if (b_type == "Giving_Tree") {
					def.blockType = BlockTypes::GIVING_TREE;
				} else if (b_type == "Geiger_Charger") {
					def.blockType = BlockTypes::GEIGER_CHARGER;
				} else if (b_type == "Animated_Foreground_Block") {
					def.blockType = BlockTypes::ANIM_FOREGROUND;
				} else if (b_type == "Portal") {
					def.blockType = BlockTypes::PORTAL;
				} else if (b_type == "Random_Block") {
					def.blockType = BlockTypes::RANDOM_BLOCK;
				} else if (b_type == "Bouncy") {
					def.blockType = BlockTypes::BOUNCY;
				} else if (b_type == "Chest") {
					def.blockType = BlockTypes::CHEST;
				} else if (b_type == "Switch_Block") {
					def.blockType = BlockTypes::SWITCH_BLOCK;
				} else if (b_type == "Magic_Egg") {
					def.blockType = BlockTypes::MAGIC_EGG;
				} else if (b_type == "Crystal") {
					def.blockType = BlockTypes::CRYSTAL;
				} else if (b_type == "Mailbox") {
					def.blockType = BlockTypes::MAILBOX;
				} else if (b_type == "Bulletin_Board") {
					def.blockType = BlockTypes::BULLETIN_BOARD;
				} else if (b_type == "Faction") {
					def.blockType = BlockTypes::FACTION;
				} else if (b_type == "Component") {
					def.blockType = BlockTypes::COMPONENT;
				} else if (b_type == "Weather_Machine" or b_type == "Weather_Machine_S1" or b_type == "Weather_Machine_S2") {
					def.blockType = BlockTypes::WEATHER;
				} else if (b_type == "ItemSucker") {
					def.blockType = BlockTypes::SUCKER;
				} else if (b_type == "Fish_Tank_Port") {
					def.blockType = BlockTypes::FISH;
				} else if (b_type == "Steam_Block") {
					def.blockType = BlockTypes::STEAM;
				} else if (b_type == "ground_Block") {
					def.blockType = BlockTypes::GROUND_BLOCK;
				} else if (b_type == "Display") {
					def.blockType = BlockTypes::DISPLAY;
				} else if (b_type == "Untrade_A_Box" or b_type == "Safe_Vault") {
					def.blockType = BlockTypes::STORAGE;
				} else if (b_type == "Vending") {
					def.blockType = BlockTypes::VENDING;
				} else if (b_type == "Donation_Box") {
					def.blockType = BlockTypes::DONATION;
				} else if (b_type == "Phone_Booth") {
					def.blockType = BlockTypes::PHONE;
				} else if (b_type == "Sewing_Machine") {
					def.blockType = BlockTypes::SEWINGMACHINE;
				} else if (b_type == "Crime_Villain") {
					def.blockType = BlockTypes::CRIME_VILLAIN;
				} else if (b_type == "Provider") {
					def.blockType = BlockTypes::PROVIDER;
				} else {
					def.blockType = BlockTypes::UNKNOWN;
				}
				if (c_ == "None") {
					def.clothType = ClothTypes::NONE;
				} else if (c_ == "Hat" or c_ == "Hair") {
					def.clothType = ClothTypes::HAIR;
				} else if (c_ == "Shirt") {
					def.clothType = ClothTypes::SHIRT;
				} else if (c_ == "Pants") {
					def.clothType = ClothTypes::PANTS;
				} else if (c_ == "Feet") {
					def.clothType = ClothTypes::FEET;
				} else if (c_ == "Face") {
					def.clothType = ClothTypes::FACE;
				} else if (c_ == "Hand") {
					def.clothType = ClothTypes::HAND;
				} else if (c_ == "Back") {
					def.clothType = ClothTypes::BACK;
				} else if (c_ == "Mask") {
					def.clothType = ClothTypes::MASK;
				} else if (c_ == "Chest" or c_ == "Necklace") {
					def.clothType = ClothTypes::NECKLACE;
				} else if (c_ == "Ances") {
					def.clothType = ClothTypes::ANCES;
				} else {
					def.clothType = ClothTypes::NONE;
				}
				if (++ct_ != def.id) {
					server_alert("Critical error! Unordered database at item " + to_string(ct_) + "/" + to_string(def.id) + "!");
				}
				itemDefs.push_back(def);
			}
			maxItems++;
		}
		i_e.close();
		desc_build();
		server_alert("Built " + to_string(maxItems) + " Items Database!\n");
	}
	catch (exception& e) {
		server_alert(e.what());
		while (1);
		exit(EXIT_FAILURE);
	}
}
inline void save_allentity() {
	int cw_ = 0, cp_ = 0;
	server_on_freeze = true;
	server_alert("Saving worlds & players...");
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
		save_pinfo(net_p);
		cp_++;
	}
	for (int i = 0; i < (int)worlds.size(); i++) cw_++;
	worldDB.saveAll();
	server_alert("Success Saving " + to_string(cw_) + " Worlds & " + to_string(cp_) + " Players!");
}