#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <experimental/filesystem>
#include "player_base.h"
using namespace std;

enum ClothTypes {
	HAIR, SHIRT,
	PANTS, FEET, FACE,
	HAND, BACK, MASK,
	NECKLACE, ANCES, NONE
};
enum BlockTypes {
	FOREGROUND, BACKGROUND, CONSUMABLE, SEED, PAIN_BLOCK, BEDROCK,
	MAIN_DOOR, SIGN, DOOR, CLOTHING, FIST, WRENCH, CHECKPOINT, MANNEQUIN, LOCK, GATEWAY,
	TREASURE, WEATHER, TRAMPOLINE, TOGGLE_FOREGROUND, CHEMICAL_COMBINER, SWITCH_BLOCK, SFX_FOREGROUND,
	RANDOM_BLOCK, PORTAL, PLATFORM, MAILBOX, MAGIC_EGG, CRYSTAL, GEMS, DEADLY, CHEST, FACTION,
	BULLETIN_BOARD, BOUNCY, FORGE, GIVING_TREE, GEIGER_CHARGER, BATTLE_PET_CAGE, PAINTING_EASEL,
	ANIM_FOREGROUND, COMPONENT, SUCKER, FISH, STEAM, GROUND_BLOCK, DISPLAY, STORAGE,
	VENDING, DONATION, PHONE, SEWINGMACHINE, CRIME_VILLAIN, PROVIDER, UNKNOWN
};
enum ItemCollision {
	ITEMCOLLISION_NONE,
	ITEMCOLLISION_NORMAL,
	ITEMCOLLISION_JUMP_THROUGH,
	ITEMCOLLISION_GATEWAY,
	ITEMCOLLISION_IF_OFF,
	ITEMCOLLISION_ONE_WAY,
	ITEMCOLLISION_VIP,
	ITEMCOLLISION_WATERFALL,
	ITEMCOLLISION_ADVENTURE,
	ITEMCOLLISION_IF_ON,
	ITEMCOLLISION_TEAM_ENTRANCE,
	ITEMCOLLISION_GUILD,
	ITEMCOLLISION_CLOUD,
	ITEMCOLLISION_FRIEND_ENTRANCE,
};

struct ItemDefinition {
	int id;
	int rarity;
	int growTime;
	int breakHits;
	int properties;
	string name = "";
	string description = "";
	ClothTypes clothType;
	BlockTypes blockType;
};
struct ItemStateDefinition {
	int id = 0;
	int dropChance = 0;
	int collisionType = 0;
};
struct AccessWorldQ {
	int id;
	string name = "";
};
struct DroppedItem {
	int id, uid, count;
	float x, y;
};
struct WorldItem {
	__int16 foreground = 0;
	__int16 background = 0;
	int int_data = 0, growTime = 0, fruit_count = 0, breakLevel = 0;
	vector<int> lock_x, lock_y;
	vector<string> randlist{};
	string string_data = "", block_state = "", door_state = "";
	string destWorld = "", destId = "", currId = "", password = "";
	bool water_state = false, fire_state = false, flipped = false, activated = false, opened = false, silenced = false, empty_air = false, re_applied = false, builder_allow = false;
	long long block_time = 0, breakTime = 0;
};
struct World {
	string name, owner_name, display_owner;
	int width = 100, height = 60;
	int droppedCount = 0, weather = 0, ownerID = 0, w_level = 0, bpm = 100;
	bool allow_drp = true, isNuked = false, isPublic = false, disable_music_blocks = false, make_music_blocks_invisible = false;
	WorldItem* items;
	vector<AccessWorldQ> accQueue{};
	vector<DroppedItem> droppedItems{};
	vector<string> accessed{};
};
struct AWorld {
	World* ptr;
	World info;
	int id = -1;
};

class WorldDB {
public:
	World get_ptr(string name);
	World* playerworld_ptr(ENetPeer* peer);
	void flush(World info);
	void saveAll();
	AWorld initialize_(string name);
};
WorldDB worldDB;
vector<World> worlds;
vector<ItemDefinition> itemDefs;
vector<ItemStateDefinition> itemStates;

#include "world_entity.h"
World createWorld(string name, int width, int height) {
	World world;
	world.name = name;
	world.width = width;
	world.height = height;
	world.items = new WorldItem[world.width * world.height];
	int randMB = (rand() % 100);
	for (int i = 0; i < world.width * world.height; i++) {
		if (i >= 3700) world.items[i].background = 14;
		if (i >= 3700) world.items[i].foreground = 2;
		if (i == 3600 + randMB) world.items[i].foreground = 6;
		else if (i >= 3600 and i < 3700) world.items[i].foreground = 0;
		if (i == 3700 + randMB) world.items[i].foreground = 8;
		if (i >= 3800 and i < 5400 and not (rand() % 48)) { world.items[i].foreground = 10; }
		if (i >= 5000 and i < 5400 and not (rand() % 6)) { world.items[i].foreground = 4; }
		else if (i >= 5400) { world.items[i].foreground = 8; }
	}
	return world;
}
AWorld WorldDB::initialize_(string name) {
	AWorld ret;
	try {
		name = toUpperText(name);
		if (name.length() < 1) throw 1;
		for (char c : name) {
			if ((c < 'A' || c>'Z') and (c < '0' || c>'9')) throw 2;
		}
		if (name == "EXIT") throw 3;
		const auto p = std::find_if(worlds.begin(), worlds.end(), [name](const World& a) { return a.name == name; });
		if (p != worlds.end()) {
			ret.id = p - worlds.begin();
			ret.info = worlds.at(p - worlds.begin());
			ret.ptr = &worlds.at(p - worlds.begin());
			return ret;
		}
		try {
			ifstream r_w("worlds/_" + name + ".json");
			if (not r_w.is_open()) {
				World info = createWorld(name, 100, 60);
				worlds.push_back(info);
				ret.id = worlds.size() - 1;
				ret.info = info;
				ret.ptr = &worlds.at(worlds.size() - 1);
				return ret;
			}
			json j;
			r_w >> j;
			r_w.close();
			World info;
			info.name = j["name"].get<string>();
			info.owner_name = j["owner"].get<string>();
			info.display_owner = j["displayOwner"].get<string>();
			info.ownerID = j["ownerID"].get<int>();
			info.width = j["width"].get<int>();
			info.height = j["height"].get<int>();
			info.weather = j["weather"].get<int>();
			info.w_level = j["worldlevel"].get<int>();
			info.bpm = j["bpm"].get<int>();
			info.droppedCount = j["dc"].get<int>();
			info.disable_music_blocks = j["dmb"].get<bool>();
			info.make_music_blocks_invisible = j["mmbi"].get<bool>();
			info.allow_drp = j["allowDrop"].get<bool>();
			info.isNuked = j["isNuked"].get<bool>();
			info.isPublic = j["isPublic"].get<bool>();
			string admin_at = j["admins"];
			if (admin_at != "") {
				vector<string> infoDats = explode("|", admin_at);
				for (auto i = 0; i < (int)infoDats.size(); i++) {
					if (infoDats.at(i) == "") continue;
					info.accessed.push_back(infoDats.at(i));
				}
			}
			json dropped_ = j["dropped"];
			for (auto i = 0; i < info.droppedCount; i++) {
				DroppedItem d_;
				d_.count = dropped_[i]["c"].get<BYTE>();
				d_.id = dropped_[i]["id"].get<short>();
				d_.x = (float)dropped_[i]["x"].get<int>();
				d_.y = (float)dropped_[i]["y"].get<int>();
				d_.uid = dropped_[i]["uid"].get<int>();
				info.droppedItems.push_back(d_);
			}
			json tiles = j["tiles"];
			info.items = new WorldItem[6000];
			for (auto i = 0; i < 6000; i++) {
				info.items[i].foreground = tiles[i]["f"].get<int>();
				info.items[i].background = tiles[i]["b"].get<int>();
				info.items[i].int_data = tiles[i]["id"].get<int>();
				info.items[i].growTime = tiles[i]["gt"].get<int>();
				info.items[i].block_time = tiles[i]["bt"].get<long long>();
				info.items[i].fruit_count = tiles[i]["fc"].get<int>();
				info.items[i].lock_x = tiles[i]["lx"].get<vector<int>>();
				info.items[i].lock_y = tiles[i]["ly"].get<vector<int>>();
				info.items[i].string_data = tiles[i]["sd"].get<string>();
				info.items[i].block_state = tiles[i]["bs"].get<string>();
				info.items[i].door_state = tiles[i]["ds"].get<string>();
				string rl_ = j["rl"].get<string>();
				if (not rl_.empty()) {
					vector<string> id_ = explode("|", rl_);
					for (auto i = 0; i < (int)id_.size(); i++) {
						if (id_.at(i) == "") continue;
						info.items[i].randlist.push_back(id_.at(i));
					}
				}
				if (not info.items[i].door_state.empty()) {
					if (info.items[i].door_state.find("|")) {
						vector<string> s_ = explode("|", info.items[i].door_state);
						info.items[i].destWorld = s_.at(0);
						info.items[i].destId = s_.at(1);
						info.items[i].currId = s_.at(2);
						info.items[i].password = s_.at(3);
						s_.clear();
					}
				}
				if (not info.items[i].block_state.empty()) {
					if (info.items[i].block_state.find(",")) {
						vector<string> s_ = explode(",", info.items[i].block_state);
						if (s_.at(0) == "1") info.items[i].water_state = true;
						if (s_.at(1) == "1") info.items[i].fire_state = true;
						if (s_.at(2) == "1") info.items[i].flipped = true;
						if (s_.at(3) == "1") info.items[i].activated = true;
						if (s_.at(4) == "1") info.items[i].opened = true;
						if (s_.at(5) == "1") info.items[i].silenced = true;
						if (s_.at(6) == "1") info.items[i].empty_air = true;
						if (s_.at(7) == "1") info.items[i].re_applied = true;
						s_.clear();
					}
				}
			}
			worlds.push_back(info);
			j.clear(); tiles.clear(); dropped_.clear();
			ret.id = worlds.size() - 1;
			ret.info = info;
			ret.ptr = &worlds.at(worlds.size() - 1);
			return ret;
		}
		catch (exception& e) {
			server_alert(e.what());
			World info = createWorld(name, 100, 60);
			worlds.push_back(info);
			ret.id = worlds.size() - 1;
			ret.info = info;
			ret.ptr = &worlds.at(worlds.size() - 1);
			return ret;
		}
	}
	catch (const out_of_range& e) {
		server_alert(e.what());
		AWorld ret;
		ret.id = -1;
		return ret;
	}
	throw 1;
}
void WorldDB::flush(World info) {
	try {
		ofstream o_w("worlds/_" + info.name + ".json");
		if (not o_w.is_open()) {
			cout << GetLastError() << endl;
		}
		json j;
		j["name"] = info.name;
		j["ownerID"] = info.ownerID;
		j["displayOwner"] = info.display_owner;
		j["width"] = info.width;
		j["height"] = info.height;
		j["owner"] = info.owner_name;
		j["dc"] = info.droppedItems.size();
		j["weather"] = info.weather;
		j["worldlevel"] = info.w_level;
		j["bpm"] = info.bpm;
		j["dmb"] = info.disable_music_blocks;
		j["mmbi"] = info.make_music_blocks_invisible;
		j["allowDrop"] = info.allow_drp;
		j["isNuked"] = info.isNuked;
		j["isPublic"] = info.isPublic;
		string w_admin = "";
		for (int i = 0; i < (int)info.accessed.size(); i++) {
			w_admin += info.accessed.at(i) + "|";
		}
		j["admins"] = w_admin;
		json tiles = json::array();
		json droppedarr = json::array();
		int square = info.width * info.height;
		for (auto i = 0; i < (int)info.droppedItems.size(); i++)
		{
			json droppedJ;
			droppedJ["c"] = (BYTE)info.droppedItems[i].count;
			droppedJ["id"] = (short)info.droppedItems[i].id;
			droppedJ["x"] = (float)info.droppedItems[i].x;
			droppedJ["y"] = (float)info.droppedItems[i].y;
			droppedJ["uid"] = info.droppedItems[i].uid;
			droppedarr.push_back(droppedJ);
			droppedJ.clear();
		}
		j["dropped"] = droppedarr;
		for (int i = 0; i < square; i++)
		{
			json tile;
			tile["f"] = info.items[i].foreground;
			tile["b"] = info.items[i].background;
			tile["id"] = info.items[i].int_data;
			tile["gt"] = info.items[i].growTime;
			tile["bt"] = info.items[i].block_time;
			tile["fc"] = info.items[i].fruit_count;
			tile["lx"] = info.items[i].lock_x;
			tile["ly"] = info.items[i].lock_y;
			tile["sd"] = info.items[i].string_data;
			tile["bs"] = info.items[i].block_state;
			tile["ds"] = info.items[i].door_state;
			string w_rl = "";
			for (int i = 0; i < (int)info.items[i].randlist.size(); i++) {
				w_rl += info.items[i].randlist.at(i) + "|";
			}
			j["rl"] = w_rl;
			tiles.push_back(tile);
			tile.clear();
		}
		j["tiles"] = tiles;
		o_w << j << endl;
		o_w.close(); j.clear(); tiles.clear(); droppedarr.clear();
		server_on_freeze = false;
	}
	catch (exception& e) {
		cout << e.what() << endl;
	}
}
void WorldDB::saveAll() {
	for (int i = 0; i < (int)worlds.size(); i++) {
		flush(worlds.at(i));
	}
	server_on_freeze = false;
}
World WorldDB::get_ptr(string name) {
	return this->initialize_(name).info;
}
World* WorldDB::playerworld_ptr(ENetPeer* peer) {
	return this->initialize_(pInfo(peer)->currentWorld).ptr;
}
inline bool isWorldOwner(ENetPeer* peer, World* world) {
	if (pInfo(peer)->rawName == world->owner_name) return true;
	else return false;
}
inline bool isWorldAdmin(ENetPeer* peer, World* world) {
	for (int i = 0; i < (int)world->accessed.size(); i++) {
		if (world->accessed.at(i) == "") continue;
		if (world->accessed.at(i) == pInfo(peer)->rawName) return true;
	}
	return false;
}
inline bool isSLOwner(ENetPeer* peer, World* world, int x, int y) {
	if (world->items[(x + (y * world->width))].string_data == pInfo(peer)->displayNameBackup) return true;
	return false;
}
inline bool isSLAdmin(ENetPeer* peer, World* world, int x, int y) {
	if (find(world->items[(x + (y * world->width))].randlist.begin(), world->items[(x + (y * world->width))].randlist.end(), pInfo(peer)->rawName) != world->items[(x + (y * world->width))].randlist.end()) return true;
	return false;
}
inline string world_def(World* world) {
	string t_ = "";
	for (auto jss = 0; jss < world->width * world->height; jss++) {
		if (world->items[jss].foreground == 226 and world->items[jss].activated) {
			if (t_.find("4JAMMED") != string::npos) continue;
			if (t_ == " `0[") t_ += "`4JAMMED";
			else t_ += "``, `4JAMMED";
		}
		if (world->items[jss].foreground == 1276 and world->items[jss].activated) {
			if (t_.find("2NOPUNCH") != string::npos) continue;
			if (t_ == " `0[") t_ += "`2NOPUNCH";
			else t_ += "``, `2NOPUNCH";
		}
		if (world->items[jss].foreground == 1278 and world->items[jss].activated) {
			if (t_.find("2IMMUNE") != string::npos) continue;
			if (t_ == " `0[") t_ += "`2IMMUNE";
			else t_ += "``, `2IMMUNE";
		}
		if (world->items[jss].foreground == 4992 and world->items[jss].activated) {
			if (t_.find("2ANTIGRAVITY") != string::npos) continue;
			if (t_ == " `0[") t_ += "`2ANTIGRAVITY";
			else t_ += "``, `2ANTIGRAVITY";
		}
	}
	return t_;
}
inline void save_blockstate(World* world, int x, int y) {
	int fire_state = 0, water_state = 0, flipped = 0, activated = 0, opened = 0, silenced = 0, empty_air = 0, re_applied = 0, builder_allow = 0;
	WorldItem ingfo_ = world->items[(x + (y * world->width))];
	if (ingfo_.fire_state == true) fire_state = 1; if (ingfo_.water_state == true) water_state = 1; if (ingfo_.flipped == true) flipped = 1; if (ingfo_.activated == true) activated = 1;
	if (ingfo_.opened == true) opened = 1; if (ingfo_.silenced == true) silenced = 1; if (ingfo_.empty_air == true) empty_air = 1; if (ingfo_.re_applied == true) re_applied = 1; if (ingfo_.builder_allow == true) builder_allow = 1;
	world->items[(x + (y * world->width))].block_state = to_string(fire_state) + "," + to_string(water_state) + "," + to_string(flipped) + "," + to_string(activated) + "," + to_string(opened) + "," + to_string(silenced) + "," + to_string(empty_air) + "," + to_string(re_applied) + "," + to_string(builder_allow) + ",";
}
inline bool check_fg(ENetPeer* peer, World* world, int x, int y) {
	int loc_xy = (x / 32) + ((y / 32) * world->width), t_ = world->items[loc_xy].foreground;
	if (itemStates.at(t_).collisionType == 1 or itemStates.at(t_).collisionType == 3 or itemStates.at(t_).collisionType == 6) {
		return true;
	} else if (itemStates.at(t_).collisionType == 4 and not world->items[loc_xy].activated) {
		return true;
	}
	return false;
}
inline int serverplayer_c(ENetPeer* peer) {
	int c = 0;
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED or not pInfo(net_p)->inGame) continue;
		c++;
	}
	return c;
}
inline int worldplayer_c(const string name, ENetPeer* peer) {
	int c_ = 0;
	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
		if (net_p->state != ENET_PEER_STATE_CONNECTED or pInfo(net_p)->isInv) continue;
		if (pInfo(net_p)->currentWorld == name) {
			c_++;
		}
	}
	return c_;
}
inline void send_wo(ENetPeer* peer) {
	if (pInfo(peer)->inGame and pInfo(peer) != NULL) {
		gamepacket_t p;
		p.Insert("OnRequestWorldSelectMenu");
		if (not pInfo(peer)->haveGrowId) {
			p.Insert("");
		}
		else {
			p.Insert("add_filter|\nadd_heading|Random Worlds<ROW2>|\nadd_floater|START|1|0.5|3529161471\n");
		}
		p.CreatePacket(peer);
	}
}
inline void join_world(ENetPeer* peer, string act_, bool fixed_gateway = false, bool door_gateway = false, int x_spwn = -1, int y_spwn = -1) {
	try {
		if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer) != NULL) {
			World w_ = worldDB.get_ptr(act_);
			int p_world = worldplayer_c(act_, peer), isvalid = 1, fixed_ = 0;
			string def = " `0[", status = "`5]", is_invis = "";
			if (p_world >= 60) {
				send_(peer, "action|log\nmsg|`oWorld is at max capacity. Try again later...");
				isvalid = 0;
			} else if (w_.isNuked and not isDev(peer) and not isMod(peer)) {
				send_(peer, "action|log\nmsg|`oThat world is inaccessible.");
				isvalid = 0;
			} else if (w_.w_level > pInfo(peer)->level and w_.owner_name != pInfo(peer)->rawName and not isDev(peer) and not isMod(peer)) {
				send_(peer, "action|log\nmsg|`oPlayers lower than level " + to_string(w_.w_level) + " can't enter " + w_.name + ".");
				isvalid = 0;
			} else if (w_.height * w_.width > 6000) {
				send_(peer, "action|log\nmsg|`4To reduce confusion, that is not a valid world.``  Try another?");
				isvalid = 0;
			}  if (isvalid <= 0) {
				if (fixed_gateway) {
					send_wo(peer);
					return;
				}
				gamepacket_t p;
				p.Insert("OnFailedToEnterWorld");
				p.CreatePacket(peer);
				return;
			}
			sendWorld(peer, &w_);
			send_inv(peer, pInfo(peer)->inventory);
			int x = 3040, y = 736;
			for (auto j = 0; j < w_.width * w_.height; j++) {
				if (w_.items[j].foreground == 6) {
					x = (j % w_.width) * 32 + 5, y = (j / w_.width) * 32;
				}
			}
			pInfo(peer)->x = x, pInfo(peer)->y = y;
			if (door_gateway and x_spwn != -1 and y_spwn != -1) {
				x = x_spwn * 32 + 5, y = y_spwn * 32;
				pInfo(peer)->x = x_spwn * 32 + 5, pInfo(peer)->y = y_spwn * 32;
			}
			def += world_def(&w_), def += "`0]";
			if (def == " `0[`0]") def = "";
			Send_::console_msg(peer, "World `w" + w_.name + "``" + def + " `oentered. There are `w" + to_string(worldplayer_c(w_.name, peer) - 1) + "`` other people here, `w" + to_string(serverplayer_c(peer)) + "`` online.");
			if (not w_.owner_name.empty()) {
				try {
					ifstream r_p("players/_" + w_.owner_name + ".json");
					if (not r_p.is_open()) return;
					json j; r_p >> j; r_p.close();
					if (w_.owner_name == pInfo(peer)->rawName or isWorldAdmin(peer, &w_)) status = " ``(`2ACCESS GRANTED``)`5]";
					else if (w_.isPublic) status = " ``(`9PUBLIC WORLD``)`5]";
					Send_::console_msg(peer, "`5[`w" + w_.name + "`$ World Locked `oby " + j["displayNameBackup"].get<string>() + "" + status + "");
					j.clear();
				}
				catch (exception& e) {
					cout << e.what() << endl;
				}
			}
			for (int i = 0; i < (int)w_.accQueue.size(); i++) {
				if (w_.accQueue.at(i).id == pInfo(peer)->netID) {
					w_.accQueue.erase(w_.accQueue.begin() + i);
					pInfo(peer)->accwl_confirm = false, pInfo(peer)->accwl_x = 0, pInfo(peer)->accwl_y = 0;
				}
			}
			if (w_.owner_name == pInfo(peer)->rawName) {
				if (not pInfo(peer)->isRoledName and not pInfo(peer)->isBlueName and not pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
					pInfo(peer)->displayName = "`2" + pInfo(peer)->displayNameBackup;
				}
			}
			if (isWorldAdmin(peer, &w_)) {
				if (not pInfo(peer)->isRoledName and not pInfo(peer)->isBlueName and not pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
					pInfo(peer)->displayName = "`^" + pInfo(peer)->displayNameBackup;
				}
			}
			remove_inv(peer, 1424, 1);
			if (pInfo(peer)->isInv) is_invis = "1";
			gamepacket_t p;
			p.Insert("OnSpawn");
			p.Insert("spawn|avatar\nnetID|" + to_string(cId) + "\nuserID|" + to_string(pInfo(peer)->userID) + "\ncolrect|0|0|20|30\nposXY|" + to_string(x) + "|" + to_string(y) + "\nname|``" + pInfo(peer)->displayName + "``\ncountry|" + pInfo(peer)->country + "\ninvis|" + is_invis + "\nmstate|0\nsmstate|0\nonlineID|\ntype|local\n");
			p.CreatePacket(peer);
			for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
				if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
				if (peer != net_p) {
					if (isHere(peer, net_p)) {
						if (not pInfo(peer)->isInv) {
							Send_::talk_bubble(net_p, pInfo(peer)->netID, "`5<`w" + pInfo(peer)->displayName + "`` `5entered, `w" + to_string(worldplayer_c(pInfo(peer)->currentWorld, peer) - 1) + "`` `5others here>```w", 0, true);
							Send_::console_msg(net_p, "`5<`w" + pInfo(peer)->displayName + "`` `5entered, `w" + to_string(worldplayer_c(pInfo(peer)->currentWorld, peer) - 1) + "`` `5others here>```w");
							Send_::audio_(net_p, "audio/door_open.wav", 0);
						}
					}
				}
			}
			Send_::audio_(peer, "audio/door_open.wav", 0);
			pInfo(peer)->netID = cId;
			peer_connect(peer);
			for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
				if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
				if (peer != net_p) {
					if (isHere(peer, net_p)) {
						if (not pInfo(net_p)->isInv) {
							Send_::talk_bubble(peer, pInfo(net_p)->netID, pInfo(net_p)->displayName, 1, false);
						}
					}
				}
			}
			cId++, pInfo(peer)->x = 0, pInfo(peer)->y = 0;
		}
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void left_world(ENetPeer* peer, bool door_gateway = false) {
	try {
		if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty()) {
			if (pInfo(peer)->lobby_guest_acc) enet_peer_disconnect_later(peer, 0);
			if (not door_gateway) {
				using namespace chrono;
				if (pInfo(peer)->world_join_left + 1500 > (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
					gamepacket_t p;
					p.Insert("OnFailedToEnterWorld");
					p.CreatePacket(peer);
					return;
				}
				pInfo(peer)->world_join_left = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			}
			for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
				if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
				if (peer != net_p) {
					if (isHere(peer, net_p)) {
						if (not pInfo(peer)->isInv) {
							Send_::OnRemove(net_p, pInfo(peer)->netID);
							Send_::console_msg(net_p, "`5<`w" + pInfo(peer)->displayName + "`` `5left, `w" + to_string(worldplayer_c(pInfo(peer)->currentWorld, peer) - 1) + "`` `5others here>```w");
							Send_::talk_bubble(net_p, pInfo(peer)->netID, "`5<`w" + pInfo(peer)->displayName + "`` `5left, `w" + to_string(worldplayer_c(pInfo(peer)->currentWorld, peer) - 1) + "`` `5others here>```w", 0, true);
							Send_::audio_(net_p, "audio/door_shut.wav", 0);
						}
					}
				}
			}
			if (not pInfo(peer)->isRoledName and not pInfo(peer)->isBlueName and not pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
				if (pInfo(peer)->displayName.find("`2") != string::npos) {
					pInfo(peer)->displayName = "`w" + pInfo(peer)->displayNameBackup;
				}
				if (pInfo(peer)->displayName.find("`^") != string::npos) {
					pInfo(peer)->displayName = "`w" + pInfo(peer)->displayNameBackup;
				}
			}
			remove_inv(peer, 1424, 1);
			Send_::audio_(peer, "audio/door_shut.wav", 0);
			pInfo(peer)->previousworld = pInfo(peer)->currentWorld;
			pInfo(peer)->currentWorld = "EXIT", pInfo(peer)->lastworld = "EXIT";
		}
	}
	catch (const out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void join_setting(ENetPeer* peer, string act_, bool warp_to = false, bool door_gateway = false, int x_to = -1, int y_to = -1) {
	try {
		bool is_valid = true;
		act_ = toUpperText(act_);
		vector<string> bad_name = { "MEMEK", "KONTOL", "FUCK", "SHIT", "DICK", "BOBO", "TANGA", "ANJING", "FVCK", "PUSSY", "AUTISM", "ANAL", "ANUS", "ERROR", "ASS", "BALLS", "BASTARD", "BITCH", "SYSTEM", "ADMIN", "VAGINA", "TITS", "SPUNKS", "SEX" };
		if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty()) {
			if (not door_gateway and not warp_to) {
				using namespace chrono;
				if (pInfo(peer)->world_join_left + 1500 > (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
					gamepacket_t p;
					p.Insert("OnFailedToEnterWorld");
					p.CreatePacket(peer);
					return;
				}
				pInfo(peer)->world_join_left = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			}
			for (int i = 0; i < (int)bad_name.size(); i++) {
				if (act_.find(bad_name[i]) != string::npos) act_ = "DISNEYLAND";
			} 
			if (act_.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != string::npos) {
				send_(peer, "action|log\nmsg|`oSorry, spaces and special characters are not allowed in world or door names.  Try again.");
				is_valid = false;
			} else if (act_ == "EXIT") {
				send_(peer, "action|log\nmsg|`oExit from what? Press back if you're done playing.");
				is_valid = false;
			} else if (act_.size() < 1 or act_.size() >= 24) {
				send_(peer, "action|log\nmsg|`4To reduce confusion, that is not a valid world name.``  Try another?");
				is_valid = false;
			} else if (act_ == "") {
				act_ = "START";
			} if (not is_valid) {
				gamepacket_t p;
				p.Insert("OnFailedToEnterWorld");
				p.CreatePacket(peer);
				return;
			} 
			if (door_gateway) {
				join_world(peer, act_, false, true, x_to, x_to);
				return;
			}
			join_world(peer, act_);
		} else {
			gamepacket_t p;
			p.Insert("OnFailedToEnterWorld");
			p.CreatePacket(peer);
			return;
		}
	}
	catch (const out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void take_floatingItem(World* world, ENetPeer* peer, const int item) {
	try {
		bool found = false;
		int atik = -1;
		for (int i = 0; i < (int)world->droppedItems.size(); i++) {
			if (world->droppedItems.at(i).uid == item) {
				atik = i;
				break;
			}
		}
		if ((found = atik != -1) == true) {
			world->droppedItems.erase(world->droppedItems.begin() + atik);
		}
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void dropitem(ENetPeer* peer, World* world, int netID, float x, float y, int item_, int c_t, BYTE bapakkaupuki, bool from_player = false) {
	try {
		if (world == nullptr or item_ <= 0 or item_ > (int)itemDefs.size() or x <= 0 or y <= 0 or pInfo(peer)->currentWorld == "EXIT" or pInfo(peer)->currentWorld.empty()) return;
		if ((x / 32) > world->width or (y / 32) > world->height or (x / 32) < 0 or (y / 32) < 0) return;
		if (item_ != 112 and from_player) {
			vector<int> id_remove;
			for (int i = 0; i < (int)world->droppedItems.size(); i++) {
				if (int(x / 32) == int(world->droppedItems.at(i).x / 32) and int(y / 32) == int(world->droppedItems.at(i).y / 32)) {
					if (world->droppedItems.at(i).count < 200 and world->droppedItems.at(i).id == item_) {
						if (world->droppedItems.at(i).count + c_t > 200) {
							dropitem(peer, world, -1, x, y, item_, (world->droppedItems.at(i).count + c_t) - 200, 0);
							c_t = 200, x = world->droppedItems.at(i).x, y = world->droppedItems.at(i).y;
							remove_floatingItem(peer, world->droppedItems.at(i).uid);
							id_remove.push_back(world->droppedItems.at(i).uid);
							break;
						}
						c_t = world->droppedItems.at(i).count + c_t, x = world->droppedItems.at(i).x, y = world->droppedItems.at(i).y;
						remove_floatingItem(peer, world->droppedItems.at(i).uid);
						id_remove.push_back(world->droppedItems.at(i).uid);
						break;
					}
				}
			}
			for (int i = 0; i < (int)id_remove.size(); i++) take_floatingItem(world, peer, id_remove.at(i));
		}
		DroppedItem drp_;
		drp_.id = item_;
		drp_.count = c_t;
		drp_.x = x, drp_.y = y;
		drp_.uid = world->droppedCount++;
		world->droppedItems.push_back(drp_);
		senddrop_(peer, netID, (int)x, (int)y, item_, c_t, bapakkaupuki);
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void saveitem_drop(ENetPeer* peer, World* world, int itemId, int Quantity, float x, float y) {
	try {
		if (Quantity > 200) {
			Send_::console_msg(peer, "[ERROR VAL:11] While Collecting Items!");
			server_alert("NGEBUG DROP ITEM LEBIH DARI > 200 : " + pInfo(peer)->currentWorld);
			return;
		} else {
			bool valid = false;
			search_inv(peer, itemId, 1, valid);
			if (not valid) {
				InventoryItem i_;
				i_.itemCount = Quantity;
				i_.itemID = itemId;
				pInfo(peer)->inventory.items.push_back(i_);
			} else {
				short c_inv = 0, idx = -1;
				for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
					if (pInfo(peer)->inventory.items.at(i).itemID == itemId) {
						c_inv = short(pInfo(peer)->inventory.items.at(i).itemCount), idx = i;
						break;
					}
				}
				int plusItem = c_inv + Quantity;
				if (idx != -1) {
					if (plusItem > 200) {
						pInfo(peer)->inventory.items.at(idx).itemCount = 200;
						dropitem(peer, world, -1, x, y, itemId, plusItem - 200, 0);
					} else {
						pInfo(peer)->inventory.items.at(idx).itemCount = c_inv + Quantity;
					}
				}
			}
			send_inv(peer, pInfo(peer)->inventory);
		}
	}
	catch (out_of_range& e) {
		cout << e.what() << endl;
	}
}
inline void drop_prize(ENetPeer* peer, World* world, int x, int y, int tile_id) {
	int s_ = 3, ct_ = 0;
	vector<vec2> total;
	vec2 start_pos = { x, y };
	deque<vec2> tisu_wajah{ start_pos };
	while ((int)tisu_wajah.size() < s_) {
		deque<vec2> she_;
		for (const auto& m_ : tisu_wajah) {
			vector<vec2> legion;
			if (m_.m_y + 1 < world->height) legion.push_back({ m_.m_x, m_.m_y + 1 });
			if (m_.m_x - 1 >= 0) legion.push_back({ m_.m_x - 1, m_.m_y });
			if (m_.m_x + 1 < world->width) legion.push_back({ m_.m_x + 1, m_.m_y });
			if (m_.m_y - 1 >= 0) legion.push_back({ m_.m_x, m_.m_y - 1 });
			for (const auto& kong : legion) {
				if (find(total.begin(), total.end(), kong) != total.end()) continue;
				else if (check_fg(peer, world, kong.m_x * 32, kong.m_y * 32)) continue;
				she_.emplace_back(kong), total.emplace_back(kong);
				if ((int)total.size() > s_) goto done;
			}
		}
		if (tisu_wajah.empty()) goto done;
		tisu_wajah.pop_front();
		if (not she_.empty()) {
			deque<vec2>::iterator begin = she_.begin();
			for (deque<vec2>::iterator p = she_.end() - 1; p != begin; --p) {
				tisu_wajah.emplace_back(*p);
			}
		}
	}
done:;
	size_t size = total.size();
	if (size > (unsigned int)s_) size = s_;
	for (const auto& pos : total) {
		if (ct_ >= s_) break;
		if (not check_fg(peer, world, pos.m_x * 32, pos.m_y * 32)) {
			dropitem(peer, world, -1, float(pos.m_x * 32), float(pos.m_y * 32), 242, (rand() % 10) + 1, 0);
			ct_ += 1;
		}
	}
}
inline void player_respawn(ENetPeer* peer, bool bytile_) {
	string w_ = pInfo(peer)->currentWorld;
	vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
	if (p != worlds.end()) {
		World* world = &worlds.at(p - worlds.begin());
		if (pInfo(peer)->currentWorld.empty() or pInfo(peer)->currentWorld == "EXIT" or world == NULL) return;
		if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty()) {
			if (not bytile_) {
				gamepacket_t p(0, pInfo(peer)->netID);
				p.Insert("OnKilled");
				p.CreatePacket(peer);
			}
			{
				gamepacket_t p(2000, pInfo(peer)->netID);
				p.Insert("OnSetFreezeState");
				p.Insert(0);
				p.CreatePacket(peer);
			}
			{
				gamepacket_t p(0, pInfo(peer)->netID);
				p.Insert("OnSetFreezeState");
				p.Insert(2);
				p.CreatePacket(peer);
			}
			int x = 3040, y = 736;
			for (auto i = 0; i < world->width * world->height; i++) {
				if (world->items[i].foreground == 6) {
					x = (i % world->width) * 32 + 5, y = (i / world->width) * 32;
					break;
				}
			}
			{
				gamepacket_t p(2000, pInfo(peer)->netID);
				p.Insert("OnSetPos");
				p.Insert((float)x, (float)y);
				p.CreatePacket(peer);
			}
			{
				gamepacket_t p(2000, pInfo(peer)->netID);
				p.Insert("OnPlayPositioned");
				p.Insert("audio/teleport.wav");
				p.CreatePacket(peer);
			}
		}
	}
}