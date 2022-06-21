#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <experimental/filesystem>

inline void command_process(ENetPeer* peer, string cch) {
	if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
		string w_ = pInfo(peer)->currentWorld;
		vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
		if (p != worlds.end()) {
			World* world = &worlds.at(p - worlds.begin());
			if (cch.substr(0, 6) == "/item ") {
				if (isDev(peer)) {
					int iteee = atoi(cch.substr(6).c_str());
					if (iteee <= 0) return;
					InventoryItem item;
					item.itemID = atoi(cch.substr(6).c_str());
					item.itemCount = 200;
					pInfo(peer)->inventory.items.push_back(item);
					send_inv(peer, pInfo(peer)->inventory);
					return;
				}
				Send_::console_msg(peer, "`4Unknown command. `oEnter `$/help `ofor a list of valid commands.");
			}
			else if (cch == "/find" or cch.substr(0, 6) == "/find ") {
				Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`wFind item``|left|6016|\nadd_spacer|small|\nadd_textbox|`wEnter a word below to find the item|\nadd_text_input|item|`wItem Name:||30|\nadd_spacer|small|\nadd_checkbox|checkbox_find_seed|`rFind with Seed|0\nend_dialog|findid|Cancel|Find the item!|\nadd_quick_exit|\n");
			}
			else if (cch == "/trashall") {
				for (int i = 0; i < int(pInfo(peer)->currentInventorySize); i++) {
					if (pInfo(peer)->inventory.items.at(i).itemID == 32 or pInfo(peer)->inventory.items.at(i).itemID == 18 or pInfo(peer)->inventory.items.at(i).itemID == 6336 or pInfo(peer)->inventory.items.at(i).itemID == 3204) continue;
					pInfo(peer)->inventory.items.erase(pInfo(peer)->inventory.items.begin() + i);
				}
				send_inv(peer, pInfo(peer)->inventory);
			}
		}
	}
}