#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <experimental/filesystem>

inline int matching_state(ENetPeer* peer, World* worldInfo, int loc_) {
	int type = 0x00000000;
	WorldItem info_ = worldInfo->items[loc_];
	if (info_.activated) type |= 0x00400000;
	if (info_.flipped) type |= 0x00200000;
	if (info_.silenced) type |= 0x02000000;
	if (info_.water_state) type |= 0x04000000;
	if (info_.fire_state) type |= 0x10000000;
	if (itemDefs.at(info_.foreground).blockType != BlockTypes::DOOR and itemDefs.at(info_.foreground).blockType != BlockTypes::LOCK) {
		if (info_.opened) type |= 0x00800000;
	}
	return type;
}
inline int matching_state2(ENetPeer* peer, WorldItem block_) {
	int blockStateFlags = 0x00000000;
	if (block_.activated) blockStateFlags |= 0x00400000;
	if (block_.flipped) blockStateFlags |= 0x00200000;
	if (block_.silenced) blockStateFlags |= 0x02000000;
	if (block_.water_state) blockStateFlags |= 0x04000000;
	if (block_.fire_state) blockStateFlags |= 0x10000000;
	if (itemDefs.at(block_.foreground).blockType != BlockTypes::DOOR and itemDefs.at(block_.foreground).blockType != BlockTypes::LOCK) {
		if (block_.opened) blockStateFlags |= 0x00800000;
	}
	return blockStateFlags;
}
inline int form_visual(BYTE*& blc, WorldItem block_, World world_, ENetPeer* peer, bool case_ = true) {
	uint32_t xSize = world_.width, ySize = world_.height, square = xSize * ySize, blockStateFlags = matching_state2(peer, block_);
	if (case_) {
		memcpy(blc, &block_.foreground, 2);
		memcpy(blc + 2, &block_.background, 2);
		memcpy(blc + 4, &blockStateFlags, 4);
	}
	switch (itemDefs.at(block_.foreground).blockType) {
		case BlockTypes::WEATHER: {
			switch (itemDefs.at(block_.foreground).id) {
				case 3694: {
					break;
				}
				case 5000: {
					break;
				}
				case 3832: {
					break;
				}
				default: {
					BYTE btype = 5;
					memcpy(blc + 6, &btype, 1);
					if (case_) {
						blc += 1;
						return 1;
					}
					break;
				}
			}
		}
		case BlockTypes::CRYSTAL: {
			BYTE btype = 20;
			memcpy(blc + 8, &btype, 1);
			string text = "";
			int l_ = (int)text.size();
			memcpy(blc + 9, &l_, 2);
			memcpy(blc + 11, text.c_str(), l_);
			if (case_) {
				blc += 3 + l_;
				return 3 + l_;
			}
			break;
		}
		case BlockTypes::GIVING_TREE: {
			BYTE btype = 0x1C, count = block_.int_data, zero = 0;
			uint16_t times = uint16_t(block_.block_time - time(nullptr) <= 0 ? 14400 : 14400 - (block_.block_time - time(nullptr)));
			memset(blc + 8, 0, 7);
			memcpy(blc, &block_.foreground, 2);
			memcpy(blc + 4, &blockStateFlags, 4);
			memcpy(blc + 8, &btype, 1);
			memcpy(blc + 8 + 1, &zero, 1);
			memcpy(blc + 8 + 2, &times, 4);
			memcpy(blc + 8 + 6, &count, 1);
			if (case_) {
				blc += 7;
				return 7;
			}
		}
		case BlockTypes::DISPLAY: {
			BYTE btype = 23;
			memcpy(blc + 8, &btype, 1);
			uint32_t id_ = block_.int_data;
			memcpy(blc + 9, &id_, 4);
			if (case_) {
				blc += 5;
				return 5;
			}
			break;
		}
		case BlockTypes::GEIGER_CHARGER: {
			BYTE btype = 57;
			int test_ = block_.int_data != 0 ? 0x00400000 : blockStateFlags;
			uint16_t sk = uint16_t(block_.block_time - time(nullptr) <= 0 ? 3600 : 3600 - (block_.block_time - time(nullptr)));
			memcpy(blc + 4, &test_, 4);
			memcpy(blc + 8, &btype, 1);
			memcpy(blc + 9, &sk, 2);
			if (case_) {
				blc += 5;
				return 5;
			}
			break;
		}
		case BlockTypes::PROVIDER: {
			BYTE btype = 9;
			uint32_t times = uint32_t((time(nullptr) - block_.block_time <= itemDefs.at(block_.foreground).growTime ? time(nullptr) - block_.block_time : itemDefs.at(block_.foreground).growTime));
			memcpy(blc + 8, &btype, 1);
			memcpy(blc + 9, &times, 4);
			if (case_) {
				blc += 5;
				return 5;
			}
			break;
		}
		case BlockTypes::LOCK: {
			if (block_.foreground == 202 or block_.foreground == 204 or block_.foreground == 206) {
				if (block_.opened) {
					int new_ = 0x00800000;
					memcpy(blc + 4, &new_, 4);
				}
				BYTE btype = 3;
				memcpy(blc + 8, &btype, 1);
				int world_owner_id = -1, count_of_admins = 1;
				for (ENetPeer* c_ = server->peers; c_ < &server->peers[server->peerCount]; ++c_) {
					if (c_->state != ENET_PEER_STATE_CONNECTED or c_->data == NULL) continue;
					if (pInfo(c_)->displayNameBackup == block_.string_data) {
						world_owner_id = pInfo(c_)->userID;
					}
				}
				memcpy(blc + 10, &world_owner_id, 4);
				{
					if (find(block_.randlist.begin(), block_.randlist.end(), pInfo(peer)->rawName) != block_.randlist.end()) {
						memcpy(blc + 22, &pInfo(peer)->userID, 4), count_of_admins++;
					} for (ENetPeer* c_ = server->peers; c_ < &server->peers[server->peerCount]; ++c_) {
						if (c_->state != ENET_PEER_STATE_CONNECTED or c_->data == NULL) continue;
						if (isHere(peer, c_)) {
							if (find(block_.randlist.begin(), block_.randlist.end(), pInfo(c_)->rawName) != block_.randlist.end()) {
								memcpy(blc + 22 + (count_of_admins * 4), &pInfo(c_)->userID, 4), count_of_admins++;
								if (count_of_admins > 7) break;
							}
						}
					}
					memcpy(blc + 14, &count_of_admins, 1);
				}
				if (case_) {
					blc += 10 + (count_of_admins * 4);
					return 10 + (count_of_admins * 4);
				}
			}
			else {
				BYTE btype = 3;
				memcpy(blc + 8, &btype, 1);
				int world_settings = world_.disable_music_blocks ? (world_.make_music_blocks_invisible ? 12345 : 1234) : (world_.make_music_blocks_invisible ? 100 : 0), world_owner_id = -1, admin_id = 0, bpm_ = world_.bpm * -1, count_of_admins = 1;
				for (ENetPeer* c_ = server->peers; c_ < &server->peers[server->peerCount]; ++c_) {
					if (c_->state != ENET_PEER_STATE_CONNECTED or c_->data == NULL) continue;
					if (pInfo(c_)->rawName == world_.owner_name) {
						world_owner_id = pInfo(c_)->userID;
					}
				}
				memcpy(blc + 9, &world_settings, 1);
				memcpy(blc + 10, &world_owner_id, 4);
				memcpy(blc + 18, &bpm_, 4);
				{
					if (find(world_.accessed.begin(), world_.accessed.end(), pInfo(peer)->rawName) != world_.accessed.end() or world_.isPublic) {
						memcpy(blc + 22, &pInfo(peer)->userID, 4), count_of_admins++;
					} for (ENetPeer* c_ = server->peers; c_ < &server->peers[server->peerCount]; ++c_) {
						if (c_->state != ENET_PEER_STATE_CONNECTED or c_->data == NULL) continue;
						if (isHere(peer, c_)) {
							if (find(world_.accessed.begin(), world_.accessed.end(), pInfo(c_)->rawName) != world_.accessed.end() or world_.isPublic) {
								memcpy(blc + 22 + (count_of_admins * 4), &pInfo(c_)->userID, 4), count_of_admins++;
								if (count_of_admins > 7) break;
							}
						}
					}
					memcpy(blc + 14, &count_of_admins, 1);
				}
				if (case_) {
					blc += 10 + (count_of_admins * 4);
					return 10 + (count_of_admins * 4);
				}
			}
			break;
		}
		case BlockTypes::RANDOM_BLOCK: {
			BYTE btype = 8;
			memcpy(blc + 8, &btype, 1);
			memcpy(blc + 9, &block_.int_data, 1);
			if (case_) {
				blc += 2;
				return 2;
			}
			break;
		}
		case BlockTypes::SEED: {
			int visual = blockStateFlags | 0x100000;
			memcpy(blc + 4, &visual, 4);
			BYTE data_type = 4;
			memcpy(blc + 8, &data_type, 1);
			uint32_t times = uint32_t((time(nullptr) - block_.block_time <= itemDefs.at(block_.foreground).growTime ? time(nullptr) - block_.block_time : itemDefs.at(block_.foreground).growTime)); //maybesoon
			uint8_t count = uint8_t(block_.fruit_count); 
			memcpy(blc + 9, &times, 4);
			memcpy(blc + 13, &count, 1);
			if (case_) {
				blc += 6;
				return 6;
			}
			break;
		}
		case BlockTypes::DOOR: case BlockTypes::PORTAL: {
			BYTE data_type = 1;
			memcpy(blc + 8, &data_type, 1);
			string header = (block_.string_data == "" ? (block_.destId == "" ? block_.destWorld : block_.destWorld + "...") : block_.string_data);
			uint32_t inthead = uint32_t(header.size());
			memcpy(blc + 9, &inthead, 2);
			memcpy(blc + 11, header.c_str(), inthead);
			uint8_t locked_ = (block_.opened ? 0 : 0x08);
			memcpy(blc + 11 + inthead, &locked_, 1);
		}
		case BlockTypes::SIGN: {
			BYTE data_type = 2;
			memcpy(blc + 8, &data_type, 1);
			string header = block_.string_data;
			uint32_t inthead = uint32_t(header.size());
			memcpy(blc + 9, &inthead, 2);
			memcpy(blc + 11, header.c_str(), inthead);
			uint32_t zeros = 0;
			memcpy(blc + 11 + inthead, &zeros, 1);
			if (case_) {
				blc += 7 + inthead;
				return 7 + inthead;
			}
			break;
		}
		case BlockTypes::MAIN_DOOR: {
			BYTE btype = 1;
			memcpy(blc + 8, &btype, 1);
			string text = "EXIT";
			int l_ = (int)text.size();
			memcpy(blc + 9, &l_, 2);
			memcpy(blc + 11, text.c_str(), l_);
			if (case_) {
				blc += 4 + l_;
				return 4 + l_;
			}
			break;
		}
	}
	return blockStateFlags;
}
#include "lock_handler.h"
inline void sendWorld(ENetPeer* peer, World* worldInfo) {
	if (pInfo(peer)->lobby_guest_acc or not pInfo(peer)->passed_data) {
		enet_peer_disconnect_later(peer, 0);
		return;
	}
	pInfo(peer)->joinClothesUpdated = false;
	int xSize = worldInfo->width, ySize = worldInfo->height, square = xSize * ySize;
	size_t namelen = worldInfo->name.length();
	int alloc = (8 * square) + (worldInfo->droppedItems.size() * 20), s1 = 4, s3 = 8, zero = 0;
	int total = 78 + namelen + square + 24 + alloc;
	BYTE* data = (byte*)malloc(999999);
	memcpy(data, &s1, 1);
	memcpy(data + 4, &s1, 1);
	memcpy(data + 16, &s3, 1);
	memcpy(data + 66, &namelen, 1);
	memcpy(data + 68, worldInfo->name.c_str(), namelen);
	memcpy(data + 68 + namelen, &xSize, 1);
	memcpy(data + 72 + namelen, &ySize, 1);
	memcpy(data + 76 + namelen, &square, 2);
	BYTE* blc = data + 80 + namelen;
	for (int i_ = 0; i_ < square; i_++) {
		int type = matching_state(peer, worldInfo, i_);
		int zero = 0;
		switch (itemDefs.at(worldInfo->items[i_].foreground).blockType) {
			case BlockTypes::CRYSTAL: case BlockTypes::WEATHER: case BlockTypes::VENDING: case BlockTypes::GEIGER_CHARGER: case BlockTypes::MAIN_DOOR: case BlockTypes::PROVIDER: case BlockTypes::GIVING_TREE: case BlockTypes::DISPLAY: case BlockTypes::RANDOM_BLOCK: case BlockTypes::LOCK: case BlockTypes::SEED: case BlockTypes::DOOR: case BlockTypes::PORTAL: case BlockTypes::SIGN: {
				total += form_visual(blc, worldInfo->items[i_], *worldInfo, peer);
				break;
			}
			case BlockTypes::BATTLE_PET_CAGE: case BlockTypes::PAINTING_EASEL: case BlockTypes::MANNEQUIN: case BlockTypes::CHEMICAL_COMBINER: case BlockTypes::FORGE: case BlockTypes::MAGIC_EGG: case BlockTypes::MAILBOX: case BlockTypes::BULLETIN_BOARD: case BlockTypes::DONATION: case BlockTypes::STEAM: case BlockTypes::FISH: case BlockTypes::SUCKER: case BlockTypes::SWITCH_BLOCK: case BlockTypes::CHEST: case BlockTypes::TOGGLE_FOREGROUND: {
				worldInfo->items[i_].foreground = 0; //not ready yet.
				memcpy(blc, &zero, 2);
				break;
			}
			default:
				memcpy(blc, &worldInfo->items[i_].foreground, 2);
				memcpy(blc + 2, &worldInfo->items[i_].background, 2);
				memcpy(blc + 4, &type, 4);
				break;
		}
		blc += 8;
	}
	int count = int(worldInfo->droppedItems.size()), last_uid = worldInfo->droppedCount;
	memcpy(blc, &count, 4);
	memcpy(blc + 4, &last_uid, 4);
	blc += 8;
	for (int i_ = 0; i_ < (int)worldInfo->droppedItems.size(); i_++) {
		int item = worldInfo->droppedItems.at(i_).id, count = worldInfo->droppedItems.at(i_).count, uid = worldInfo->droppedItems.at(i_).uid + 1;
		float x = worldInfo->droppedItems.at(i_).x, y = worldInfo->droppedItems.at(i_).y;
		memcpy(blc, &item, 2);
		memcpy(blc + 2, &x, 4);
		memcpy(blc + 6, &y, 4);
		memcpy(blc + 10, &count, 2);
		memcpy(blc + 12, &uid, 4);
		blc += 16;
	}
	if (worldInfo->weather >= 0) {
		Send_::set_weather(peer, worldInfo->weather);
	}
	pInfo(peer)->currentWorld = worldInfo->name;
	pInfo(peer)->lastworld = worldInfo->name;
	ENetPacket* p_ = enet_packet_create(data, total, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, p_);
	for (int i_ = 0; i_ < square; i_++) {
		switch (itemDefs.at(worldInfo->items[i_].foreground).blockType) {
			case BlockTypes::LOCK: {
				if (worldInfo->items[i_].foreground == 202 or worldInfo->items[i_].foreground == 204 or worldInfo->items[i_].foreground == 206) {
					lock_stricted_update(peer, i_ % worldInfo->width, i_ / worldInfo->width, worldInfo->items[i_].foreground, worldInfo->items[i_].background, worldInfo, true, true, true);
				}
			}
		}
	}
	delete[] data;
}