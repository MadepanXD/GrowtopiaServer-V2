#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <experimental/filesystem>

vector<int> disabled_item = {};

inline int getmax_xp(int l_) {
	auto xp = 3 * (300 * l_);
	if (l_ == 1) xp = 100;
	else if (l_ == 2) xp = 400;
	else if (l_ == 3) xp = 600;
	else if (l_ == 4) xp = 800;
	else if (l_ == 5) xp = 1000;
	else if (l_ == 6) xp = 1200;
	else if (l_ == 7) xp = 1500;
	else if (l_ == 8) xp = 1900;
	else if (l_ == 9) xp = 2300;
	else if (l_ == 10) xp = 3000;
	return xp;
}
inline void return_packet(ENetPeer* peer, int x, int y) {
	PlayerMoving d_;
	d_.netID = pInfo(peer)->netID;
	d_.packetType = 0x8, d_.plantingTree = 0, d_.netID = -1;
	d_.x = (float)x, d_.punchX = x;
	d_.y = (float)y, d_.punchY = y;
	BYTE* raw = packPlayerMoving(&d_);
	send_raw(4, raw, 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
}
inline int get_givingtree_prize(World* world, WorldItem block_, ENetPeer* peer) {
	int prize_ = 0;
	srand(GetTickCount());
	vector<int> list_ = { 1360, 3228, 5356, 3234, 10516, 3218, 5366, 3240, 3232, 11518, 7456, 11538, 11516, 11490, 11512, 10512, 10508, 10518, 440, 11520 };
	if ((rand() % 500) < 351) {
		if ((block_.int_data > 50 and block_.int_data < 99) or block_.int_data >= 100) {
			list_.insert(list_.end(), { 11460, 11458, 11498, 11496, 11486, 11540, 11484 });
			if ((rand() % 200) < 26) {
				list_.insert(list_.end(), { 11494, 11482, 11502, 11492, 11454 });
			}
		}
		if (block_.int_data >= 100) {
			int rand_ = (rand() % 200);
			list_.insert(list_.end(), { 242, 3236, 3230, 3252, 9192, 3216 });
			if (rand_ < 51) {
				for (int i = 0; i < 13; i++) {
					list_.erase(list_.begin() + 1);
				}
			}
			if (rand_ > 50 and rand_ < 60) list_.push_back(11456);
			else if (rand_ > 30 and rand_ < 40) list_.push_back(11488);
			else if (rand_ > 10 and rand_ < 20) list_.push_back(11466);
			else if (rand_ > 2 and rand_ < 10) list_.push_back(10454);
			else if (rand_ < 2) list_.push_back(11462);
		}
	}
	prize_ = list_[rand() % list_.size()];
	return prize_;
}
inline void tile_update(int x, int y, int tile_, int c_b, ENetPeer* peer) {
	PlayerMoving d_;
	d_.packetType = 0x3;
	d_.characterState = 0x0;
	d_.x = (float)x, d_.punchX = x, d_.XSpeed = 0;
	d_.y = (float)y, d_.punchY = y, d_.XSpeed = 0;
	d_.netID = c_b;
	d_.plantingTree = tile_;
	return_packet(peer, x, y);
	string w_ = pInfo(peer)->currentWorld;
	bool lock_update = false, sl_update = false, SL_onWorld = false, isSL_Block = false;
	bool giving_tree = false;
	vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
	if (p != worlds.end()) {
		World* world = &worlds.at(p - worlds.begin());
		WorldItem block_ = world->items[x + (y * world->width)];
		ItemDefinition df_ = itemDefs.at(world->items[x + (y * world->width)].foreground);
		for (int i = 0; i < world->width * world->height; i++) {
			if (world->items[i].foreground == 202 or world->items[i].foreground == 204 or world->items[i].foreground == 206) { //isn't builderlock!
				SL_onWorld = true;
				if (world->items[i].string_data == pInfo(peer)->displayNameBackup) isSL_Block = false;
				for (int j = 0; j < world->width * world->height; j++) {
					if (not world->items[j].string_data.empty() and world->items[j].string_data != pInfo(peer)->displayNameBackup) {
						isSL_Block = true;
					}
				}
			}
		}
		switch (tile_) {
			case 18: { //fist
				if ((block_.foreground == 6 or block_.foreground == 8) and not isDev(peer) and not isMod(peer)) {
					gamepacket_t p(0, pInfo(peer)->netID);
					p.Insert("OnPlayPositioned");
					p.Insert(itemDefs.at(block_.foreground).blockType == BlockTypes::MAIN_DOOR ? "audio/cant_place_tile_.wav" : "audio/punch_locked.wav");
					for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
						if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
						if (isHere(peer, net_p)) {
							p.CreatePacket(net_p);
						}
					}
					Send_::talk_bubble(peer, pInfo(peer)->netID, itemDefs.at(block_.foreground).blockType == BlockTypes::MAIN_DOOR ? "(stand over and punch to use)" : "It's too strong to break.", 0, true);
					return;
				}
				if (block_.foreground == 202 or block_.foreground == 204 or block_.foreground == 206) {
					if (block_.opened and not isDev(peer) and not isSLOwner(peer, world, x, y)) {
						Send_::talk_bubble(peer, pInfo(peer)->netID, block_.string_data + "'s `$" + itemDefs.at(block_.foreground).name + ". `w(`wOpen to public`w) (Last played ...?)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
						return;
					}
					else if (isSLAdmin(peer, world, x, y) and not isDev(peer) and not isSLOwner(peer, world, x, y)) {
						Send_::talk_bubble(peer, pInfo(peer)->netID, block_.string_data + "'s `$" + itemDefs.at(block_.foreground).name + ". `w(`9Access`w) (Last played ...?)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
						return;
					}
					else {
						if (not isDev(peer) and not isSLOwner(peer, world, x, y)) {
							Send_::talk_bubble(peer, pInfo(peer)->netID, block_.string_data + "'s `$" + itemDefs.at(block_.foreground).name + ". `w(`4No access`w) (Last played ...?)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
							return;
						}
					}
				}
				if (not world->owner_name.empty() and not isWorldOwner(peer, world) and not isDev(peer) and (block_.foreground != 202 and block_.foreground != 204 and block_.foreground != 206)) {
					string username = world->display_owner;
					if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
						if (isWorldAdmin(peer, world)) {
							if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
								Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + username + "'s `$" + itemDefs.at(block_.foreground).name + "`w. (`2Access Granted`w)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
								return;
							}
						}
						else if (world->isPublic) {
							if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
								Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + username + "'s `$" + itemDefs.at(block_.foreground).name + "`w. (`wOpen to Public`w)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
								return;
							}
						}
						else {
							if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
								Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + username + "'s `$" + itemDefs.at(block_.foreground).name + "`w. (`4No Access`w)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
								return;
							}
						}
					}
					if (not isWorldAdmin(peer, world) and not isWorldOwner(peer, world) and not world->isPublic) {
						if (not this_stricted_(world, x, y)) {
							if (block_.foreground != 0 and block_.background != 0 or block_.foreground != 0 and block_.background == 0 or block_.foreground == 0 and block_.background != 0) {
								Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
							}
							return;
						}
					}
				}
				if (SL_onWorld) {
					if (this_stricted_(world, x, y)) {
						if (not stricted_allow_check(peer, world, x, y)) {
							if (not public_stricted_(peer, world, x, y)) {
								if (block_.foreground != 0 and block_.background != 0 or block_.foreground != 0 and block_.background == 0 or block_.foreground == 0 and block_.background != 0) {
									Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, true);
								}
								return;
							}
						}
					}
				}
				if (itemDefs.at(block_.foreground).blockType == BlockTypes::GIVING_TREE) { /*harvest gtree*/
					if (block_.block_time - time(nullptr) <= 0) {
						int prize_ = get_givingtree_prize(world, world->items[x + (y * world->width)], peer);
						if (block_.int_data <= 5) prize_ = 1360;
						if (prize_ <= 0 or block_.int_data > 100) break;
						bool isValid = save_inv_check(prize_, 1, peer, true);
						if (isValid) {
							giving_tree = true;
							Send_::console_msg(peer, "I found a `2" + itemDefs.at(prize_).name + "`o under the tree!");
							Send_::talk_bubble(peer, pInfo(peer)->netID, "I found a `2" + itemDefs.at(prize_).name + "`w under the tree!", 0, false);
							world->items[x + (y * world->width)].int_data = 0, world->items[x + (y * world->width)].block_time = time(nullptr) + 1800;
						}
					}
				}
				break;
			}
			case 32: {
				if (block_.foreground == 202 or block_.foreground == 204 or block_.foreground == 206) {
					if ((isDev(peer) or isSLOwner(peer, world, x, y))) {
						string acc_list = "", name = "";
						for (vector<string>::const_iterator i = block_.randlist.begin(); i != block_.randlist.end(); ++i) name = *i, acc_list += "\nadd_checkbox|checkbox_" + name + "|" + name + "|1|\n";
						string allow_dialog = "\nadd_checkbox|checkbox_public|Allow anyone to Build and Break|" + to_string(block_.opened ? 1 : 0) + "", ignore_dialog = "\nadd_checkbox|checkbox_ignore|Ignore empty air|" + to_string(block_.empty_air ? 1 : 0) + "";
						string acc_ = world->items[(x + (y * world->width))].randlist.size() ? "\nadd_label|small|" + acc_list + "|left|" : "\nadd_label|small|Currently, you're the only one with access.``|left|";
						Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`wEdit " + itemDefs.at(block_.foreground).name + "|left| " + to_string(itemDefs.at(block_.foreground).id) + "|\nadd_label|small|`wAccess list:``|left\nadd_spacer|small|" + acc_ + "\nadd_spacer|small|\nadd_player_picker|playerNetID|`wAdd``|" + allow_dialog + ignore_dialog + "\nadd_button|re_apply|`wRe-apply lock``|noflags|0|0|\nembed_data|tilex|" + to_string(x) + "\nembed_data|tiley|" + to_string(y) + "\nend_dialog|sl_edit|Cancel|OK|");
						return;
					}
					else if (block_.opened and not isDev(peer) and not isSLOwner(peer, world, x, y)) {
						Send_::talk_bubble(peer, pInfo(peer)->netID, "I’m `4unable`` to pick the lock.", 0, true);
						return;
					}
					else if (isSLAdmin(peer, world, x, y) and not isDev(peer) and not isSLOwner(peer, world, x, y)) {
						Send_::talk_bubble(peer, pInfo(peer)->netID, "I’m `4unable`` to pick the lock.", 0, true);
						return;
					}
					else {
						if (not isDev(peer) and not isSLOwner(peer, world, x, y)) {
							Send_::talk_bubble(peer, pInfo(peer)->netID, "I’m `4unable`` to pick the lock.", 0, true);
							return;
						}
					}
				}
				if (not world->owner_name.empty() and not isWorldOwner(peer, world) and not isDev(peer) and (block_.foreground != 202 and block_.foreground != 204 and block_.foreground != 206)) {
					string username = world->display_owner;
					if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
						if (isWorldAdmin(peer, world)) {
							if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
								Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + username + "'s `$" + itemDefs.at(block_.foreground).name + "`w. (`2Access Granted`w)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
								return;
							}
						}
						else if (world->isPublic) {
							if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
								Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + username + "'s `$" + itemDefs.at(block_.foreground).name + "`w. (`wOpen to Public`w)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
								return;
							}
						}
						else {
							if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
								Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + username + "'s `$" + itemDefs.at(block_.foreground).name + "`w. (`4No Access`w)", 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
								return;
							}
						}
					}
				}
				if (block_.foreground == 242 or block_.foreground == 1796 or block_.foreground == 7188) {
					if (isWorldOwner(peer, world) or isDev(peer) or isMod(peer) or isWorldAdmin(peer, world)) {
						if (isWorldAdmin(peer, world)) {
							Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|small|Remove Your Access From World|left|242|\nadd_textbox|Are you sure you want to remove yourself from all locks in this world?|left|\nadd_textbox|Any locks you placed will still be owned by you, but you will be removed from any other locks that you have access on.|left|\nembed_data|tilex|" + to_string(x) + "\nembed_data|tiley|" + to_string(y) + "\nadd_spacer|small|\nend_dialog|unaccess|Cancel|`wYes|");
						}
						if (isWorldOwner(peer, world) or isDev(peer) or isMod(peer)) {
							string acc_list = "", world_key = "\nadd_button|getKey|Get World Key|noflags|0|0|"; bool in_inv = false;
							for (int i_ = 0; i_ < (int)world->accessed.size(); i_++) {
								acc_list += "\nadd_checkbox|checkbox_" + world->accessed.at(i_) + "|`w" + world->accessed.at(i_) + "``|1";
							}
							search_inv(peer, 1424, 1, in_inv);
							if (acc_list.empty()) acc_list = "\nadd_label|small|Currently, you're the only one with access.``|left";
							if (in_inv) world_key = "";
							Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`wEdit " + itemDefs.at(block_.foreground).name + "``|left|" + to_string(block_.foreground) + "|\nadd_label|small|`wAccess list:``|left|\nadd_spacer|small|" + acc_list + "\nadd_spacer|small|\nadd_player_picker|playerNetID|`wAdd``|\nadd_checkbox|checkbox_public_wl|Allow anyone to Build and Break|" + (world->isPublic ? "1" : "0") + "\nadd_checkbox|checkbox_allow_drop|Allow anyone to drop Items|" + (world->allow_drp ? "1" : "0") + "\nadd_checkbox|checkbox_disable_music|Disable Custom Music Blocks|" + (world->disable_music_blocks ? "1" : "0") + "\nadd_text_input|tempo|Music BPM|" + to_string(world->bpm) + "|3|\nadd_checkbox|checkbox_disable_music_render|Make Custom Music Blocks invisible|" + (world->make_music_blocks_invisible ? "1" : "0") + "\nadd_text_input|minimum_entry_level|World Level: |" + to_string(world->w_level) + "|3|\nadd_smalltext|Set minimum world entry level.|\nembed_data|tilex|" + to_string(x) + "\nembed_data|tiley|" + to_string(y) + "\nadd_smalltext|Your current world is: " + world->name + "|left|" + world_key + "\nend_dialog|lock_edit|Cancel|OK|");
						}
					}
				}
				break;
			}
			case 202: case 204: case 206: {
				if (block_.foreground != 0) return;
				if (this_stricted_(world, x, y)) {
					if (not stricted_allow_check(peer, world, x, y)) {
						Send_::talk_bubble(peer, pInfo(peer)->netID, "That area is owned by " + check_owner_sl(world, x, y), 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
						return;
					}
				}
				if (not world->owner_name.empty() and not isWorldOwner(peer, world)) {
					Send_::talk_bubble(peer, pInfo(peer)->netID, "That area is owned by " + world->display_owner, 0, true), Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
					return;
				}
				world->items[x + (y * world->width)].string_data = pInfo(peer)->displayNameBackup, sl_update = true, save_blockstate(world, x, y);
				Send_::talk_bubble(peer, pInfo(peer)->netID, "`wArea locked.", 0, false);
				for (ENetPeer* c_p = server->peers; c_p < &server->peers[server->peerCount]; ++c_p) {
					if (c_p->state != ENET_PEER_STATE_CONNECTED or c_p->data == NULL) continue;
					if (isHere(peer, c_p)) {
						Send_::audio_(c_p, "audio/use_lock.wav", 0);
					}
				}
				break;
			}
			case 242: case 1796: case 7188: {
				if (block_.foreground != 0) return;
				if (world->owner_name != "" or isSL_Block) {
					Send_::talk_bubble(peer, pInfo(peer)->netID, isSL_Block ? "`wYou can't lock this world if someone else's lock in this world!" : "`0Only one `$World Lock `0Can be placed in a world!", 1, false), Send_::OnPlayPositioned(peer, "audio/cant_place_tile.wav", pInfo(peer)->netID, false);
					return;
				}
				lock_update = true;
				world->owner_name = pInfo(peer)->rawName, world->display_owner = pInfo(peer)->displayNameBackup, world->ownerID = pInfo(peer)->userID, world->isPublic = false, pInfo(peer)->worldsowned.push_back(pInfo(peer)->currentWorld);
				Send_::talk_bubble(peer, pInfo(peer)->netID, "`5[`w" + world->name + " `whas been `$World Locked `wby " + pInfo(peer)->displayName + "`5]", 0, false);
				for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
					if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
					if (isHere(peer, net_p)) {
						Send_::console_msg(net_p, "`5[`w" + world->name + " `ohas been `$World Locked `wby " + world->display_owner + "`5]"), Send_::audio_(net_p, "audio/use_lock.wav", 0);
						if (not pInfo(peer)->isRoledName and not pInfo(peer)->isBlueName and not pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
							pInfo(peer)->displayName = "`2" + pInfo(peer)->displayNameBackup;
							Send_::change_name(net_p, pInfo(peer)->netID, pInfo(peer)->displayName);
						}
					}
				}
			}
			default:
				if (itemDefs.at(tile_).blockType == BlockTypes::CLOTHING) {
					if (c_b == pInfo(peer)->netID and x == pInfo(peer)->x / 32 and y == pInfo(peer)->y / 32) {
						int p_ = tile_;
						if (pInfo(peer)->cloth_ances == p_ or pInfo(peer)->cloth_back == p_
							or pInfo(peer)->cloth_feet == p_ or pInfo(peer)->cloth_face == p_
							or pInfo(peer)->cloth_hair == p_ or pInfo(peer)->cloth_hand == p_
							or pInfo(peer)->cloth_mask == p_ or pInfo(peer)->cloth_neck == p_
							or pInfo(peer)->cloth_pants == p_ or pInfo(peer)->cloth_shirt == p_) {
							Send_::OnPlayPositioned(peer, "audio/change_clothes.wav", pInfo(peer)->netID, false);
							update_pset(peer, p_);
						}
						else {
							if (itemDefs.at(p_).clothType == ClothTypes::ANCES)
								pInfo(peer)->cloth_ances = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::BACK)
								pInfo(peer)->cloth_back = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::FEET)
								pInfo(peer)->cloth_feet = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::FACE)
								pInfo(peer)->cloth_face = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::HAIR)
								pInfo(peer)->cloth_hair = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::HAND)
								pInfo(peer)->cloth_hand = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::MASK)
								pInfo(peer)->cloth_mask = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::NECKLACE)
								pInfo(peer)->cloth_neck = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::PANTS)
								pInfo(peer)->cloth_pants = p_;
							else if (itemDefs.at(p_).clothType == ClothTypes::SHIRT)
								pInfo(peer)->cloth_shirt = p_;
							Send_::OnPlayPositioned(peer, "audio/change_clothes.wav", pInfo(peer)->netID, false);
							update_set(peer);
						}
					}
					else {
						Send_::talk_bubble(peer, pInfo(peer)->netID, "`wTo wear clothing, use on yourself", 0, false);
					}
					return;
				}
				if (not world->owner_name.empty() and not isWorldOwner(peer, world) and not isDev(peer)) {
					string username = world->display_owner;
					if (not isWorldAdmin(peer, world) and not isWorldOwner(peer, world) and not world->isPublic) {
						if (not this_stricted_(world, x, y)) {
							if (tile_ != 18 and itemDefs.at(tile_).blockType != BlockTypes::BACKGROUND and block_.foreground != 0 or itemDefs.at(tile_).blockType == BlockTypes::CONSUMABLE) return;
							Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
							return;
						}
					}
				}
				if (SL_onWorld) {
					if (this_stricted_(world, x, y)) {
						if (not stricted_allow_check(peer, world, x, y)) {
							if (not public_stricted_(peer, world, x, y)) {
								if (tile_ != 18 and itemDefs.at(tile_).blockType != BlockTypes::BACKGROUND and block_.foreground != 0 or itemDefs.at(tile_).blockType == BlockTypes::CONSUMABLE) return;
								Send_::OnPlayPositioned(peer, "audio/punch_locked.wav", pInfo(peer)->netID, false);
								return;
							}
						}
					}
				}
				if (tile_ == 3200 or tile_ == 3204) {
					if (itemDefs.at(tile_).blockType == BlockTypes::GIVING_TREE) { /*placed tree*/
						giving_tree = true;
						world->items[x + (y * world->width)].block_time = time(nullptr) + 1800;
						world->items[x + (y * world->width)].int_data = 0;
					} 
					else if (tile_ == 3204) { /*decor*/
						if (block_.foreground == 3200) {
							if (block_.int_data >= 100) break;
							bool valid_10 = false;
							search_inv(peer, 3204, 10, valid_10);
							if (valid_10 and block_.int_data < 91) {
								remove_inv(peer, 3204, 10);
								world->items[x + (y * world->width)].int_data += 10;
							} else {
								remove_inv(peer, 3204, 1);
								world->items[x + (y * world->width)].int_data += 1;
							}
							if (world->items[x + (y * world->width)].int_data > 100) {
								bool isValid = save_inv_check(3204, world->items[x + (y * world->width)].int_data - 100, peer, false);
								if (isValid) {
									world->items[x + (y * world->width)].int_data = 100;
								}
							}
							for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
								if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
								if (isHere(peer, net_p)) {
									PlayerMoving data_{};
									data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
									BYTE* raw = packPlayerMoving(&data_, 112);
									BYTE* blc = raw + 56;
									form_visual(blc, world->items[(x + (y * world->width))], *world, net_p);
									send_raw(4, raw, 112, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
								}
							}
						} else {
							Send_::talk_bubble(peer, pInfo(peer)->netID, "`4Can't waste decorations on a non-Giving Tree!", 0, false);
							return;
						}
					}
				}
				break;
		}

		if (df_.blockType == BlockTypes::CLOTHING or itemDefs.at(tile_).blockType == BlockTypes::CLOTHING or itemDefs.at(tile_).blockType == BlockTypes::CONSUMABLE or tile_ == 32) return;
		if (tile_ != 18 and itemDefs.at(tile_).blockType != BlockTypes::BACKGROUND and block_.foreground != 0) return;
		if (tile_ == 18) {
			int t_ = block_.foreground;
			int b_ = block_.background;
			if (b_ == 0 and t_ == 0) return;
			if ((itemDefs.at(b_).name).find("Sheet Music") != string::npos and t_ == 0 and world->make_music_blocks_invisible) return;
			if ((itemDefs.at(tile_).name).find("null_item") != string::npos or (itemDefs.at(tile_).name).find("Kranken") != string::npos) return;
			d_.packetType = 0x8;
			d_.plantingTree = 5;
			using namespace chrono;
			if ((duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() - block_.breakTime >= 4000) {
				world->items[x + (y * world->width)].breakTime = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				world->items[x + (y * world->width)].breakLevel = 5;
			}
			else {
				if (y < world->height and block_.breakLevel + 4 >= df_.breakHits * 4) {
					d_.packetType = 0x3;
					d_.netID = -1;
					d_.plantingTree = tile_;
					d_.punchX = x, d_.punchY = y;
					world->items[x + (y * world->width)].breakLevel = 0;
					WorldItem block_ = world->items[x + (y * world->width)];
					if (block_.foreground != 0) {
						if (itemDefs.at(block_.foreground).properties & Property_AutoPickup) {
							bool valid = save_inv_check(block_.foreground, 1, peer, false);
							if (not valid) return;
						}
						if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
							if (block_.foreground == 202 or block_.foreground == 204 or block_.foreground == 206) {
								world->items[x + (y * world->width)].string_data.clear(), world->items[x + (y * world->width)].block_state.clear(), world->items[x + (y * world->width)].randlist.clear();
								world->items[x + (y * world->width)].opened = false, world->items[x + (y * world->width)].lock_x.clear(), world->items[x + (y * world->width)].lock_y.clear();
								world->items[x + (y * world->width)].empty_air = false, world->items[x + (y * world->width)].builder_allow = false;
							}
							else {
								if (not world->owner_name.empty() and not world->name.empty()) {
									for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
										if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
										if (isHere(peer, net_p)) {
											Send_::console_msg(net_p, "`5[`w" + world->name + " `ohas had its `$" + itemDefs.at(world->items[x + (y * world->width)].foreground).name + " `oremoved!`5]");
											Send_::audio_(net_p, "audio/explode.wav", 0);
											if (not world->owner_name.empty() and (isWorldOwner(peer, world) or isWorldAdmin(peer, world))) {
												if (not pInfo(peer)->isRoledName and not pInfo(peer)->isBlueName and not pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
													if (pInfo(peer)->displayName.find("`2") != string::npos) pInfo(peer)->displayName = "`w" + pInfo(peer)->displayNameBackup;
													else if (pInfo(peer)->displayName.find("`^") != string::npos) pInfo(peer)->displayName = "`w" + pInfo(peer)->displayNameBackup;
												}
												for (int i = 0; i < (int)pInfo(peer)->worldsowned.size(); i++) {
													if (pInfo(peer)->worldsowned.at(i) == world->name) {
														pInfo(peer)->worldsowned.erase(pInfo(peer)->worldsowned.begin() + i);
													}
												}
												for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
													if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
													if (isHere(peer, net_p)) {
														Send_::change_name(net_p, pInfo(peer)->netID, pInfo(peer)->displayName);
													}
												}
												world->owner_name.clear(), world->display_owner.clear(), world->accessed.clear(), world->accQueue.clear(), world->ownerID = 0, world->w_level = 0, world->bpm = 0;
												world->isPublic = false, world->allow_drp = true, world->disable_music_blocks = false, world->make_music_blocks_invisible = false;
											}
										}
									}
								}
							}
						}
						world->items[x + (y * world->width)].foreground = 0;
					} else {
						world->items[x + (y * world->width)].background = 0;
					}
				}
				else {
					if (y < world->height) {
						world->items[x + (y * world->width)].breakTime = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						world->items[x + (y * world->width)].breakLevel += 4;
					}
				}
			}
		} 
		else {
			for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
				if (pInfo(peer)->inventory.items.at(i).itemID == tile_) {
					if ((unsigned int)pInfo(peer)->inventory.items.at(i).itemCount > 1) {
						pInfo(peer)->inventory.items.at(i).itemCount--;
					} else {
						pInfo(peer)->inventory.items.erase(pInfo(peer)->inventory.items.begin() + i);
					}
				}
			}
			if (itemDefs.at(tile_).blockType == BlockTypes::BACKGROUND) {
				world->items[x + (y * world->width)].background = tile_;
			} else if (itemDefs.at(tile_).blockType == BlockTypes::SEED) {
				world->items[x + (y * world->width)].foreground = tile_;
			} else {
				world->items[x + (y * world->width)].foreground = tile_;
			}
			world->items[x + (y * world->width)].breakLevel = 0;
		}
		for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
			if (net_p->state != ENET_PEER_STATE_CONNECTED) continue;
			if (isHere(peer, net_p)) {
				send_raw(4, packPlayerMoving(&d_), 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
				if (lock_update or sl_update or giving_tree) {
					if (sl_update) {
						apply_lock(peer, x, y, world->items[x + (y * world->width)].foreground, world, false, false, false);
					}
					PlayerMoving data_{};
					data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
					BYTE* raw = packPlayerMoving(&data_, 112);
					BYTE* blc = raw + 56;
					form_visual(blc, world->items[(x + (y * world->width))], *world, net_p);
					send_raw(4, raw, 112, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
				}
			}
		}
	}
}