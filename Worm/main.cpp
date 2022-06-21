#define _CRT_SECURE_NO_DEPRECATE
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATIBON_WARNING
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include <iostream>
#include "enet/enet.h"
#include "json.hpp"
#include <string>
#include <vector>
#include <fstream>

#define pInfo(peer) ((PlayerInfo*)(peer->data))
#include "game_packet.h"
#include "back_end.h"
#include "world_base.h"
#include "server_base.h"
#include "player_base.h"
#include "dialog_handler.h"
#include "tile_update.h"
#include "command_handler.h"
#include "event_handler.h"

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) {
	save_allentity();
	return FALSE;
}

int main() {
	system("Color D");
	server_alert("Server made by: Madepan#2966");
	load_(), build_();

	if (atexit(save_allentity)) {
		server_alert("Saving Entity!");
	}
	enet_initialize();
	ENetAddress address;
	enet_address_set_host(&address, "0.0.0.0");
	address.port = atoi(SERVER_PORT.c_str());
	server = enet_host_create(&address, 1024, 1, 0, 0);
	if (server == NULL) {
		server_alert("An error occurred while trying to create an ENet server host.\n");
		while (1);
		exit(EXIT_FAILURE);
	}
	ifstream t("uids.txt");
	string str_uid((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	totaluserids = atoi(str_uid.c_str());
	server->checksum = enet_crc32;
	enet_host_compress_with_range_coder(server);

	SetConsoleCtrlHandler(HandlerRoutine, true);
	srand(unsigned int(time(nullptr)));

	ENetEvent event;
	while (true) {
		while (enet_host_service(server, &event, 1000) > 0) {
			try {
				ENetPeer* peer = event.peer;
				switch (event.type) {
					case ENET_EVENT_TYPE_CONNECT: {
						if (server_on_freeze) break;
						send_p(peer, 1, nullptr, 0);
						peer->data = new PlayerInfo;
						char clientConnection[16];
						enet_address_get_host_ip(&peer->address, clientConnection, 16);
						pInfo(peer)->ip_ply = clientConnection;
						pInfo(peer)->peer_id = peer->connectID;
						break;
					}
					case ENET_EVENT_TYPE_RECEIVE: {
						if (server_on_freeze) break;
						int m_type = message_ptr(event.packet);
						string cch = text_ptr(event.packet);
						switch (m_type) {
							case 2: {
								//cout << cch << endl;
								if (itemdathash == 0) {
									enet_peer_disconnect_later(peer, 0);
									break;
								}
								if (not pInfo(peer)->inGame) {
									if (server_on_freeze or pInfo(peer)->update_req >= 1) break;
									player_login(peer, cch);
									break;
								}
								if (cch.find("action|input") != string::npos) {
									try {
										if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
											if (not spam_guard(peer)) break;
											string msg = explode("\n", explode("|", cch).at(3)).at(0), a = "", check_ = "", msg2 = "";
											rm_space_(msg);
											if (msg.length() <= 0 or msg.length() > 120 or msg.empty() or all_of(msg.begin(), msg.end(), [](char c) {return isspace(c); })) continue;
											if (msg.at(0) == '`' and msg.size() <= 2) break;
											for (char c : msg) if (c < 0x20 or c > 0x7A) break;
											if (msg.at(0) == '/') { /*command*/
												command_process(peer, msg);
												break;
											}
											else {
												if (pInfo(peer)->isMuted) {
													for (int i = 0; i < (int)msg.length(); i++) msg2 += i % 2 == 0 ? "m" : "f";
													msg = msg2;
												}
												check_ = msg;
												transform(check_.begin(), check_.end(), check_.begin(), ::tolower);
												gamepacket_t p(180), p2(180);
												if (check_ != ":/" and check_ != ":p" and check_ != ":)" and check_ != "-_-" and check_ != ":*" and check_ != ";)" and check_ != ":d" and check_ != ":o" and check_ != ":'(" and check_ != ":(") {
													p.Insert("OnConsoleMessage");
													p.Insert("CP:_PL:0_OID:_CT:[W]_ `6<" + pInfo(peer)->displayName + "``>`` `$" + (pInfo(peer)->adminLevel == 10 ? "`5" : (pInfo(peer)->adminLevel == 8) ? "`^" : "`$") + msg + "`````");
												}
												p2.Insert("OnTalkBubble");
												p2.Insert(pInfo(peer)->netID);
												if (check_ != ":/" and check_ != ":p" and check_ != ":)" and check_ != "-_-" and check_ != ":*" and check_ != ";)" and check_ != ":d" and check_ != ":o" and check_ != ":'(" and check_ != ":(") {
													p2.Insert("CP:_PL:0_OID:_player_chat=" + a + (pInfo(peer)->adminLevel == 10 ? "`5" : (pInfo(peer)->adminLevel == 8) ? "`^" : "`0") + msg);
												}
												else {
													p2.Insert(msg);
												}
												for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
													if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
													if (isHere(peer, net_p)) {
														p.CreatePacket(net_p);
														p2.CreatePacket(net_p);
													}
												}
											}
										}
										break;
									}
									catch (out_of_range& e) {
										cout << "ERROR: " << e.what() << endl;
									}
								}
								else if (cch.substr(0, 17) == "action|enter_game" and pInfo(peer)->inGame and pInfo(peer)->currentWorld.empty()) {
									pInfo(peer)->update_req = 0;
									if (server_on_freeze) break;
									if (pInfo(peer)->block_login or pInfo(peer)->meta.empty() or pInfo(peer)->country.empty()) {
										enet_peer_disconnect_later(peer, 0);
										break;
									}
									if (pInfo(peer)->haveGrowId) {
										int o_ct = 0, totalcount = pInfo(peer)->friendinfo.size();
										string t_ = "No friends are online.", name = pInfo(peer)->displayName;
										if (not pInfo(peer)->passed_data) {
											enet_peer_disconnect_later(peer, 0);
											break;
										}
										settingname_(peer);
										gamepacket_t p;
										p.Insert("SetHasAccountSecured");
										p.Insert(1);
										p.CreatePacket(peer);
										Send_::gemupdate_(peer, pInfo(peer)->gems, 0);
										Send_::set_growID(peer, 1, pInfo(peer)->tankIDName, pInfo(peer)->tankIDPass);
										Send_::emoticon(peer, 151224576, u8"(wl)|─ü|0&(yes)|─é|0&(no)|─â|0&(love)|─ä|0&(oops)|─à|0&(shy)|─å|0&(wink)|─ç|0&(tongue)|─ê|1&(agree)|─ë|0&(sleep)|─è|0&(punch)|─ï|0&(music)|─î|0&(build)|─ì|0&(megaphone)|─Ä|0&(sigh)|─Å|1&(mad)|─É|1&(wow)|─æ|1&(dance)|─Æ|0&(see-no-evil)|─ô|0&(bheart)|─ö|0&(heart)|─ò|0&(grow)|─û|0&(gems)|─ù|0&(kiss)|─ÿ|1&(gtoken)|─Ö|0&(lol)|─Ü|0&(smile)|─Ç|1&(cool)|─£|0&(cry)|─¥|0&(vend)|─₧|0&(bunny)|─¢|0&(cactus)|─ƒ|0&(pine)|─ñ|0&(peace)|─ú|0&(terror)|─í|0&(troll)|─á|0&(evil)|─ó|0&(fireworks)|─ª|0&(football)|─Ñ|0&(alien)|─º|0&(party)|─¿|0&(pizza)|─⌐|0&(clap)|─¬|0&(song)|─½|0&(ghost)|─¼|0&(nuke)|─¡|0&(halo)|─«|0&(turkey)|─»|0&(gift)|─░|0&(cake)|─▒|0&(heartarrow)|─▓|0&(lucky)|─│|0&(shamrock)|─┤|0&(grin)|─╡|0&(ill)|─╢|0&(eyes)|─╖|0&(weary)|─╕|0&");
										for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
											if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
											if (find(pInfo(peer)->friendinfo.begin(), pInfo(peer)->friendinfo.end(), pInfo(net_p)->rawName) != pInfo(peer)->friendinfo.end()) {
												o_ct++;
											}
										}
										if (o_ct > 0) t_ = "`w" + to_string(o_ct) + "`o friend is online.";	
										Send_::console_msg(peer, "Welcome back, `w" + name + "`o. " + t_ + "");
										Send_::console_msg(peer, "`oWhere would you like to go? (`w" + to_string(serverplayer_c(peer)) + " `oonline)");
										if (pInfo(peer)->lastworld == "EXIT" or pInfo(peer)->lastworld.empty()) {
											pInfo(peer)->currentWorld = "EXIT";
											send_wo(peer);
										} else {
											join_world(peer, pInfo(peer)->lastworld, true);
										}
									}
									else {
										send_wo(peer);
										gamepacket_t p(500);
										p.Insert("OnDialogRequest");
										p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`wWelcome to `9GrowSenkai|left|5834|\nadd_spacer|small|\nadd_smalltext|`oGreetings, new `2PLAYERS!`o Welcome to `9GrowSenkai Private Server`o. Before you start, you need to create an account to Start Playing!.|\nadd_spacer|small|\nadd_label_with_icon|small|`oAfter you create an `2Account`o, you need to Disconnect first and Login back to play. You can visit our `5Discord `oto see Tips for Playing!|left|6102|\nadd_spacer|small|\nadd_url_button|comment|`5Discord``|noflags|https://discord.gg/9HqQh7NTsd|`wWanna check our `5Discord `wServer?|0|0|\nadd_button|growid|`wClick this to `2Get a GrowID|0|0|noflags|\n");
										p.CreatePacket(peer);
										pInfo(peer)->lobby_guest_acc = true;
									}
									break;
								}
								else if (cch == "action|refresh_item_data\n") {
									if (server_on_freeze) break;
									//pInfo(peer)->update_req++;
									/*if (pInfo(peer)->update_req >= 2 or pInfo(peer)->inGame) {
										ip_block(peer);
										break;
									}*/ //not make sure work!
									if (itemsDat != NULL) {
										ENetPacket* packet = enet_packet_create(itemsDat, itemsDatSize + 60, ENET_PACKET_FLAG_RELIABLE);
										enet_peer_send(peer, 0, packet);
									}
									break;
								}
								else if (cch == "action|respawn\n" or cch.find("action|respawn_spike\n") == 0) {
									if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
										if (server_on_freeze or pInfo(peer)->update_req >= 1 or not state_guard(peer)) break;
										if (cch.find("action|respawn_spike\n") == 0) {
											if (cch.find("tileX") == string::npos) {
												ip_block(peer);
												break;
											}
											player_respawn(peer, true);
											break;
										}
										player_respawn(peer, false);
										break;
									}
								}
								else if (cch.find("action|friends") == 0) {
									if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
										if (server_on_freeze or pInfo(peer)->update_req >= 1 or not state_guard(peer)) break;
										Send_::dialog_(peer, "set_default_color|`o\n\nadd_label_with_icon|big| `wSocial Portal`` |left|1366|\nadd_spacer|small|\nadd_button|show_friend|`wShow Friends``|noflags|0|0|\nadd_button|communityhub|`wCommunity Hub``|noflags|0|0|\nadd_button|show_apprentices|`wShow Apprentices``|noflags|0|0|\nadd_button|showguild|`wCreate Guild``|noflags|0|0|\nadd_button|trade_history|`wTrade History``|noflags|0|0|\nadd_quick_exit|\nend_dialog|socialportal||Back|\n");
										break;
									}
								}
								else if (cch.find("action|trash\n|itemID|") == 0) {
									try {
										if (server_on_freeze or pInfo(peer)->update_req >= 1 or not state_guard(peer)) break;
										if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
											stringstream ss(cch); string to;
											int id_ = 0, c_t = 0, found = 0, c_inv = 0;
											while (getline(ss, to, '\n')) {
												vector<string> a_ = explode("|", to);
												if (a_.size() == 3) {
													if (a_.at(1) == "itemID") id_ = atoi(a_.at(2).c_str());
													if (a_.at(1) == "count") c_t = atoi(a_.at(2).c_str()); //isn't work 2022
												}
											}
											for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
												if (pInfo(peer)->inventory.items.at(i).itemID == id_ and pInfo(peer)->inventory.items.at(i).itemCount >= 1) {
													found++, c_inv = pInfo(peer)->inventory.items.at(i).itemCount;
													break;
												}
											}
											if (found <= 0 or id_ > (int)itemDefs.size()) {
												ip_block(peer);
												//Auto_ban(peer, 24 * 3, "Exploit!");
												break;
											} else if (id_ == 18 or id_ == 32 or id_ == 6336 or id_ == 1424) {
												Send_::overlay_(peer, "You can't trash that!");
												break;
											} else {
												pInfo(peer)->lasttrash_id = id_, pInfo(peer)->lasttrash_ct = c_inv;
												Send_::dialog_(peer, "set_default_color|`o\n\nadd_label_with_icon|big|`4Trash `w" + itemDefs.at(id_).name + "``|left|" + to_string(id_) + "|\nadd_textbox|`oHow many to `4destroy`o? (you have " + to_string(c_inv) + ")|\nadd_text_input|count||0|3|||\nend_dialog|trash_dialog|Cancel|OK|\n");
											}
										}
									}
									catch (out_of_range& e) {
										server_alert(e.what());
									}
									break;
								}
								else if (cch.find("action|drop\n|itemID|") == 0) {
									try {
										if (server_on_freeze or pInfo(peer)->update_req >= 1 or not state_guard(peer)) break;
										if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
											stringstream ss(cch); string to;
											int id_ = 0, c_t = 0, found = 0, c_inv = 0;
											while (getline(ss, to, '\n')) {
												vector<string> a_ = explode("|", to);
												if (a_.size() == 3) {
													if (a_.at(1) == "itemID") id_ = atoi(a_.at(2).c_str());
													if (a_.at(1) == "count") c_t = atoi(a_.at(2).c_str()); //isn't work 2022
												}
											}
											for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
												if (pInfo(peer)->inventory.items.at(i).itemID == id_ and pInfo(peer)->inventory.items.at(i).itemCount >= 1) {
													found++, c_inv = pInfo(peer)->inventory.items.at(i).itemCount;
													break;
												}
											}
											if (found <= 0 or id_ > (int)itemDefs.size()) {
												ip_block(peer);
												//Auto_ban(peer, 24 * 3, "Exploit!");
												break;
											} else if (itemDefs.at(id_).properties & Property_Untradable or id_ == 6336 or id_ == 112 or id_ == 1424) {
												Send_::overlay_(peer, "You can't drop that!");
												break;
											} else {
												pInfo(peer)->lastdrop_id = id_, pInfo(peer)->lastdrop_ct = c_inv;
												Send_::dialog_(peer, "set_default_color|`o\n\nadd_label_with_icon|big|`wDrop " + itemDefs.at(id_).name + "``|left|" + std::to_string(id_) + "|\nadd_textbox|`oHow many to drop?|\nadd_text_input|count||" + to_string(c_inv) + "|3|||\nadd_textbox|`4Warning: `oIf someone is asking you to drop items, they are DEFINITELY trying to scam you. Do not drop items in other players' worlds|left|\nend_dialog|drop_dialog|Cancel|OK|\n");
											}
										}
									}
									catch (out_of_range& e) {
										server_alert(e.what());
									}
									break;
								}
								else if (cch.find("action|info") != string::npos) {
									try {
										if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
											bool valid = false;
											int id_ = atoi(explode("\n", explode("|", cch).at(3)).at(0).c_str());
											search_inv(peer, id_, 1, valid);
											if (not valid) break;
											gamepacket_t p;
											p.Insert("OnDialogRequest");
											string extra_ = "\nadd_textbox|";
											if (id_ == 18) {
												extra_ += "You've punched `w" + to_string((rand() % 1990) + 1) + "`` times.";
											} if (itemDefs.at(id_).blockType == BlockTypes::LOCK) {
												extra_ += "A lock makes it so only you (and designated friends) can edit an area.";
											}
											else {
											} if (itemDefs.at(id_).properties & Property_Dropless or itemDefs.at(id_).rarity == 999) {
												extra_ += "<CR>`1This item never drops any seeds.``";
											} if (itemDefs.at(id_).properties & Property_Untradable) {
												extra_ += "<CR>`1This item cannot be dropped or traded.``";
											} if (itemDefs.at(id_).properties & Property_AutoPickup) {
												extra_ += "<CR>`1This item can't be destroyed - smashing it will return it to your backpack if you have room!``";
											} if (itemDefs.at(id_).properties & Property_Wrenchable) {
												extra_ += "<CR>`1This item has special properties you can adjust with the Wrench.``";
											} if (itemDefs.at(id_).properties & Property_MultiFacing) {
												extra_ += "<CR>`1This item can be placed in two directions, depending on the direction you're facing.``";
											} if (itemDefs.at(id_).properties & Property_NoSelf) {
												extra_ += "<CR>`1This item has no use... by itself.``";
											}
											extra_ += "|left|";
											if (extra_ == "\nadd_textbox||left|") extra_ = "";
											else extra_ = replace_str(extra_, "add_textbox|<CR>", "add_textbox|");
											p.Insert("set_default_color|`o\nadd_label_with_ele_icon|big|`wAbout " + itemDefs.at(id_).name + "``|left|" + to_string(id_) + "|3|\nadd_spacer|small|\nadd_textbox|" + itemDefs.at(id_).description + "|left|\nadd_spacer|small|" + extra_ + "\nadd_spacer|small|\nembed_data|itemID|" + to_string(id_) + "\nend_dialog|continue||OK|\n");
											p.CreatePacket(peer);
										}
									}
									catch (out_of_range& e) {
										server_alert(e.what());
									}
								}
								else if (cch.find("action|wrench") == 0) {
									if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
										if (server_on_freeze or pInfo(peer)->update_req >= 1 or not state_guard(peer)) break;
										stringstream ss(cch);
										string to, w_ = pInfo(peer)->currentWorld;
										int id_ = -1;
										while (getline(ss, to, '\n')) {
											vector<string> infoDat = explode("|", to);
											if (infoDat.at(1).size() == 5 and infoDat.at(1) == "netid" and not infoDat.at(2).empty()) {
												id_ = atoi(infoDat.at(2).c_str());
												if (id_ <= 0) continue;
											}
										}
										vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
										if (p != worlds.end()) {
											World* world = &worlds.at(p - worlds.begin());
											WorldItem block_ = world->items[pInfo(peer)->accwl_x + (pInfo(peer)->accwl_y * world->width)]; //acc wl only!
											for (ENetPeer* neet_p = server->peers; neet_p < &server->peers[server->peerCount]; ++neet_p) {
												if (neet_p->state != ENET_PEER_STATE_CONNECTED or neet_p->data == NULL) continue;
												if (isHere(peer, neet_p)) {
													if (pInfo(neet_p)->netID == id_) {
														pInfo(peer)->lastwrench_ = pInfo(neet_p)->rawName;
														pInfo(peer)->lastwrenchname_ = pInfo(neet_p)->tankIDName;
														pInfo(peer)->lastwrenchDisplay_ = pInfo(neet_p)->displayName;
														if (pInfo(neet_p)->rawName != pInfo(peer)->rawName) {
															if (not pInfo(neet_p)->rawName.empty()) {
																string option_ = "";
																if (isDev(peer) or isMod(peer)) {
																	option_ += "\nadd_button|player_manage_asmod|`!Player-Manage|", option_ += "\nadd_button|kick_button|`4Kick|", option_ += "\nadd_button|pull_button|`5Pull|", option_ += "\nadd_button|worldBan_button|`4World Ban|";
																}
																else if (isWorldOwner(peer, world) or (isWorldAdmin(peer, world) and pInfo(neet_p)->rawName != world->owner_name)) {
																	option_ += "\nadd_button|kick_button|`4Kick|", option_ += "\nadd_button|pull_button|`5Pull|", option_ += "\nadd_button|worldBan_button|`4World Ban|";
																	if (isDev(neet_p) or isMod(neet_p)) option_ = "";
																}
																else if (isDev(neet_p) or isMod(neet_p)) {
																	option_ = "";
																}
																Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`w" + pInfo(neet_p)->displayName + " `w(`2" + to_string(pInfo(neet_p)->level) + "`w)``|left|18|\nembed_data|netID|" + to_string(pInfo(neet_p)->netID) + "\nadd_spacer|small|\nadd_button|trade|`wTrade``|noflags|0|0|\nadd_textbox|(No Battle Leash equipped)|left|\nadd_textbox|You need a valid license to battle!|left|" + option_ + "" + (find(pInfo(peer)->friendinfo.begin(), pInfo(peer)->friendinfo.end(), pInfo(neet_p)->rawName) != pInfo(peer)->friendinfo.end() ? "" : "\nadd_button|friend_add|`wAdd as friend``|noflags|0|0|") + "\nadd_button|ignore_player|`wIgnore Player``|noflags|0|0|\nadd_button|report_player|`wReport Player``|noflags|0|0|\nadd_spacer|small|\nend_dialog|popup||Continue|\nadd_quick_exit|");
															}
														}
														else {
															if (pInfo(neet_p)->rawName == pInfo(peer)->rawName) {
																if (not pInfo(neet_p)->rawName.empty()) {
																	time_t s_;
																	s_ = time(NULL);
																	int days_ = int(s_) / (60 * 60 * 24);
																	double hours_ = (double)((s_ - pInfo(peer)->playtime) + pInfo(peer)->seconds) / 3600;
																	string num_text = to_string(hours_), rounded = num_text.substr(0, num_text.find(".") + 3);
																	string activeeffect = get_ActEffect(peer), supporter = "\nadd_textbox|`oYou are not yet a `2Supporter``.``|left|", accessconfirm = "";
																	if (pInfo(peer)->Supporter) {
																		supporter = "\nadd_textbox|`oYou are `2Supporter `onow!`o.``|left|";
																	} if (pInfo(peer)->accwl_confirm) {
																		accessconfirm = "\nadd_button|accept_acc|`2Accept Access on " + itemDefs.at(block_.foreground).name + "|";
																	}
																	Send_::dialog_(peer, "embed_data|netID|" + to_string(id_) + "\nset_default_color|`o\nadd_player_info|`w" + pInfo(peer)->displayName + "``|" + to_string(pInfo(peer)->level) + "|" + to_string(pInfo(peer)->xp) + "|" + to_string(getmax_xp(pInfo(peer)->level)) + "|\nadd_spacer|small|\nadd_spacer|small|" + accessconfirm + "\nadd_button|change_password|`wChange GrowID Password|noflags|0|0|\nadd_spacer|small|\nset_custom_spacing|x:5;y:10|\nadd_custom_button|title_edit|image:interface/large/gui_wrench_title.rttex;image_size:400,260;width:0.24;|\nadd_custom_button|notebook_edit|image:interface/large/gui_wrench_notebook.rttex;image_size:400,260;width:0.24;|\nadd_custom_button|my_worlds|image:interface/large/gui_wrench_my_worlds.rttex;image_size:400,260;width:0.24;|\nadd_custom_button|emojis_info|image:interface/large/gui_wrench_growmojis.rttex;image_size:400,260;width:0.24;|\nadd_custom_break|\nadd_spacer|small|\nset_custom_spacing|x:0;y:0|\nadd_textbox|`wActive effects:``|left|" + activeeffect + "\nadd_spacer|small|\nadd_textbox|`oYou have `w" + to_string(pInfo(peer)->currentInventorySize) + "`` backpack slots.``|left|\nadd_textbox|`oCurrent world: `w" + pInfo(peer)->currentWorld + "`` (`w" + to_string(pInfo(peer)->x / 32) + "``, `w" + to_string(pInfo(peer)->y / 32) + "``) (`w" + to_string(worldplayer_c(pInfo(peer)->currentWorld, peer)) + "`` person)````|left|\nadd_smalltext|`oPrevious World: " + pInfo(peer)->previousworld + "``|left|" + supporter + "\nadd_spacer|small|\nadd_textbox|`oTotal time played is `w" + rounded + "`` hours.  This account was created `w" + to_string(days_ - pInfo(peer)->account_created) + "`` days ago.``|left|\nadd_spacer|small|\nend_dialog|popup||`wContinue|");
																}
															}
														}
													}
												}
											}
										}
										break;
									}
								}
								else if (cch.find("action|setSkin") != string::npos) {
									try {
										if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
											string id_ = explode("\n", explode("|", cch).at(2)).at(0);
											if (not isdigit(id_[0])) break;
											char* endptr = NULL;
											unsigned int skin_ = (unsigned int)strtoll(id_.c_str(), &endptr, 10);
											if (skin_ != 1348237567 and skin_ != 1685231359 and skin_ != 2022356223 and skin_ != 2190853119 and skin_ != 2527912447 and skin_ != 2864971775 and skin_ != 3033464831 and skin_ != 3370516479) break;
											pInfo(peer)->skin = skin_;
											gamepacket_t p(0, pInfo(peer)->netID);
											p.Insert("OnChangeSkin");
											p.Insert(pInfo(peer)->skin);
											for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
												if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
												if (isHere(peer, net_p)) {
													p.CreatePacket(net_p);
												}
											}
										}
									}
									catch (out_of_range& e) {
										server_alert(e.what());
									}
								}
								else if (cch.find("action|dialog_return") == 0) {
									if (server_on_freeze or pInfo(peer)->update_req >= 1 or not state_guard(peer)) break;
									if (pInfo(peer)->lobby_guest_acc and cch.find("buttonClicked|growid") == string::npos and cch.find("buttonClicked|creategrowid") == string::npos) break;
									dialog_handler(peer, cch);
									break;
								}
								break;
							}
							case 3: {
								if (server_on_freeze or pInfo(peer)->update_req >= 1 or not pInfo(peer)->inGame or pInfo(peer)->meta.empty()) break;
								stringstream ss(text_ptr(event.packet));
								string cch = text_ptr(event.packet), to = "", name_ = "", id = "", a_ = "", c_jr = "0";
								while (getline(ss, to, '\n')) {
									if (to.find('|') == -1) continue;
									id = to.substr(0, to.find("|")), a_ = to.substr(to.find("|") + 1, to.length() - to.find("|") - 1);
									if (id == "name" and c_jr == "1") {
										name_ = a_;
										join_setting(peer, name_);
									}
									if (id == "action") {
										if (a_ == "join_request") {
											c_jr = "1";
										}
										else if (a_ == "quit_to_exit") {
											if (pInfo(peer)->currentWorld == "EXIT" or not state_guard(peer)) break;
											left_world(peer), send_wo(peer), save_pinfo(peer);
											Send_::console_msg(peer, "Where would you like to go? (`w" + to_string(serverplayer_c(peer)) + " `oonline)");
										}
										else if (a_ == "quit") {
											if (pInfo(peer) != NULL) {
												save_pinfo(peer), left_world(peer);
											}
											enet_peer_disconnect_now(peer, 0);
											delete peer->data;
											peer->data = NULL;
										}
									}
								}
								break;
							}
							case 4: {
								if (server_on_freeze or pInfo(peer)->currentWorld.empty() or pInfo(peer)->update_req >= 1) break;
								BYTE* tank_up = struct_ptr(event.packet);
								if (tank_up) {
									PlayerMoving* pMov = unpackPlayerMoving(tank_up);
									if (pMov->x < 0 or pMov->y < 0) {
										try {
											int xblock = 3040, yblock = 736;
											string w_ = pInfo(peer)->currentWorld;
											vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
											if (p != worlds.end()) {
												World* world = &worlds.at(p - worlds.begin());
												for (auto i = 0; i < world->width * world->height; i++) {
													if (world->items[i].foreground == 6) {
														xblock = (i % world->width) * 32, yblock = (i / world->width) * 32;
													}
												}
												Send_::setpos_(peer, pInfo(peer)->netID, (float)xblock, (float)yblock);
											}
										}
										catch (out_of_range& e) {
											cout << e.what() << endl;
										}
										break;
									}
									switch (pMov->packetType) {
										case 0: {
											pInfo(peer)->x = (int)pMov->x + 5;
											pInfo(peer)->y = (int)pMov->y;
											pInfo(peer)->characterState = pMov->characterState;
											pInfo(peer)->isRotatedLeft = pMov->characterState & 0x10;
											sendPData(peer, pMov);
											if (not pInfo(peer)->joinClothesUpdated) {
												pInfo(peer)->joinClothesUpdated = true;
												update_player_join(peer);
												update_all_clothes(peer);
												send_state(peer);
											}
											if (pMov->XSpeed >= 260 or pMov->XSpeed <= -260) {
												if (not isDev(peer) and not isMod(peer)) {
													if (pMov->characterState != 32 and pMov->characterState != 48 and pMov->characterState != 128 and pMov->characterState != 0 and pMov->characterState != 144 and pMov->characterState != 16 or pMov->x / 32 == 0 or pMov->x / 32 == 99) break;
													if (not speed_guard(peer, pMov)) break;
												}
											}
											break;
										}
										case 11: {
											if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and pInfo(peer)->currentWorld != "EXIT" and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->x > 0 and pInfo(peer)->y > 0) {
												if (not state_guard(peer)) break;
												string w_ = pInfo(peer)->currentWorld;
												vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
												if (p != worlds.end()) {
													World* world = &worlds.at(p - worlds.begin());
													bool found = false;
													int id_ = pMov->plantingTree - 1;
													int atik = -1;
													try {
														for (int i = 0; i < (int)world->droppedItems.size(); i++) {
															if (world->droppedItems.at(i).uid == id_) {
																atik = i;
																break;
															}
														}
													}
													catch (...) { 
														break;
													}
													found = atik != -1;
													if (found) {
														if (check_fg(peer, world, (int)pMov->x, (int)pMov->y)) break;
														if ((pInfo(peer)->x / 32 == (int)pMov->x / 32) or ((pInfo(peer)->x / 32) + 1 == (int)pMov->x / 32) or ((pInfo(peer)->x / 32) - 1 == (int)pMov->x / 32) or ((pInfo(peer)->x / 32) + 2 == (int)pMov->x / 32) or ((pInfo(peer)->x / 32) - 2 == (int)pMov->x / 32)) {
															DroppedItem droppedItem = world->droppedItems.at(atik);
															if (droppedItem.id == 112) {
																try {
																	pInfo(peer)->gems += droppedItem.count;
																	Send_::gemupdate_(peer, pInfo(peer)->gems, 0);
																	world->droppedItems.erase(world->droppedItems.begin() + atik);
																	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
																		if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
																		if (isHere(peer, net_p)) {
																			PlayerMoving d_;
																			d_.packetType = 14;
																			d_.x = pMov->x, d_.y = pMov->y;
																			d_.netID = pInfo(peer)->netID;
																			d_.plantingTree = id_ + 1;
																			BYTE* raw = packPlayerMoving(&d_);
																			send_raw(4, raw, 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
																		}
																	}
																}
																catch (exception& e) {
																	cout << e.what() << endl;
																}
															}
															else {
																try {
																	world->droppedItems.erase(world->droppedItems.begin() + atik);
																	for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
																		if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
																		if (isHere(peer, net_p)) {
																			PlayerMoving d_;
																			d_.packetType = 14;
																			d_.x = pMov->x, d_.y = pMov->y;
																			d_.netID = pInfo(peer)->netID;
																			d_.plantingTree = id_ + 1;
																			BYTE* raw = packPlayerMoving(&d_);
																			send_raw(4, raw, 56, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
																		}
																	}
																	saveitem_drop(peer, world, droppedItem.id, droppedItem.count, droppedItem.x, droppedItem.y);
																}
																catch (exception& e) {
																	cout << e.what() << endl;
																}
															}
															if (itemDefs.at(droppedItem.id).rarity == 999) {
																if (droppedItem.id != 112) {
																	Send_::console_msg(peer, "`oCollected `w" + to_string(droppedItem.count) + " " + itemDefs.at(droppedItem.id).name + "`o.");
																}
															} else {
																Send_::console_msg(peer, "`oCollected `w" + to_string(droppedItem.count) + " " + itemDefs.at(droppedItem.id).name + "`o. Rarity: `w" + to_string(itemDefs.at(droppedItem.id).rarity) + "`o.");
															}
															break;
														}
													}
												}
											}
											break;
										}
										case 10: {
											try {
												int p_ = pMov->plantingTree;
												bool valid = false;
												search_inv(peer, p_, 1, valid);
												if (not inv_guard(peer)) break;
												if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and pInfo(peer)->currentWorld != "EXIT" and not pInfo(peer)->currentWorld.empty()) {
													if (pMov->plantingTree <= 0 or pMov->plantingTree > (int)itemDefs.size()) break;
													if (itemDefs.at(p_).blockType != BlockTypes::CLOTHING) {
														if (p_ == 242) {
															for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
																if (pInfo(peer)->inventory.items.at(i).itemID == 242) {
																	if (pInfo(peer)->inventory.items.at(i).itemCount >= 100) {
																		if (save_inv_check(1796, 1, peer, false)) {
																			remove_inv(peer, p_, 100);
																			Send_::talk_bubble(peer, pInfo(peer)->netID, "`wYou compressed 100 `2World Lock `winto a `2Diamond Lock`w!", 0, true);
																			Send_::console_msg(peer, "`oYou compressed 100 `2World Lock `ointo a `2Diamond Lock`o!");
																			update_set(peer);
																		}
																	}
																}
															}
															break;
														}
														else if (p_ == 1796) {
															for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
																if (pInfo(peer)->inventory.items.at(i).itemID == 1796) {
																	if (pInfo(peer)->inventory.items.at(i).itemCount >= 1) {
																		if (save_inv_check(242, 100, peer, false)) {
																			remove_inv(peer, p_, 1);
																			Send_::talk_bubble(peer, pInfo(peer)->netID, "`wYou shattered a `2Diamond Lock `winto 100 `2World Lock`w!", 0, true);
																			Send_::console_msg(peer, "`oYou shattered a `2Diamond Lock `ointo 100 `2World Lock`o!");
																			update_set(peer);
																		}
																	}
																}
															}
															break;
														}
														else if (p_ == 7188) {
															for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
																if (pInfo(peer)->inventory.items.at(i).itemID == 7188) {
																	if (pInfo(peer)->inventory.items.at(i).itemCount >= 1) {
																		if (save_inv_check(1796, 100, peer, false)) {
																			remove_inv(peer, p_, 1);
																			Send_::talk_bubble(peer, pInfo(peer)->netID, "`wYou shattered a `2Blue Gem Lock `winto 100 `2Diamond Lock`w!", 0, true);
																			Send_::console_msg(peer, "`oYou shattered a `2Blue Gem Lock `ointo 100 `2Diamond Lock`o!");
																			update_set(peer);
																		}
																	}
																}
															}
															break;
														}
														break;
													}
													if (valid) {
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
												}
											}
											catch (out_of_range& e) {
												server_alert(e.what());
											}
											break;
										}
										case 18: {
											sendPData(peer, pMov);
											break;
										}
										case 3: {
											if (pMov->punchX != -1 and pMov->punchY != -1) {
												if (pMov->packetType == 3) {
													if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT") {
														if (not state_guard(peer)) break;
														if (pInfo(peer)->isRotatedLeft = pMov->characterState & 0x10) pInfo(peer)->RotatedLeft = true;
														else pInfo(peer)->RotatedLeft = false;
														using namespace chrono;
														if (pInfo(peer)->lastBreak + 130 > (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() and pMov->plantingTree != 32) {
															break;
														}
														pInfo(peer)->lastBreak = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
														bool valid = false;
														search_inv(peer, pMov->plantingTree, 1, valid);
														if (valid) {
															if (pMov->plantingTree == 18) {
																tile_update(pMov->punchX, pMov->punchY, pMov->plantingTree, pInfo(peer)->netID, peer);
															}
															else {
																tile_update(pMov->punchX, pMov->punchY, pMov->plantingTree, pInfo(peer)->netID, peer);
															}
														}
													}
												}
											}
											break;
										}
										delete[] pMov;
										default:
											break;
									}
									if (not state_guard(peer)) break;
								}
								break;
							}
							default:
								break;
						}
						enet_packet_destroy(event.packet);
						break;
					}
					case ENET_EVENT_TYPE_DISCONNECT: {
						if (pInfo(peer) != NULL) {
							save_pinfo(peer);
							left_world(peer);
						}
						delete (ENetPeer*)(event.peer->data);
						event.peer->data = NULL;
						break;
					}
					default:
						break;
				}
			}
			catch (...) {
				server_alert("Server On Bad Range!");
				while (1);
				exit(EXIT_FAILURE);
			}
		}
	}
	delete[] server;
	server_alert("Program ended??? Huh?");
	while (1);
		return 0;
}