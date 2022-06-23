#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <deque>
#include <experimental/filesystem>
using namespace std;

inline bool get_tile(int x, int y, World* world) {
	if (x > world->width or y > world->height) return 0;
	if (x < 0 or y < 0) return 0;
	return &world->items[(x + y * world->width)];
}
inline bool is_tile_locked_by_a_lock(World* world, int x, int y) {
	bool limau = false;
	for (int i = 0; i < world->width * world->height; i++) {
		if (world->items[i].lock_x.size() >= 1 and world->items[i].lock_y.size() >= 1) {
			for (int i2 = 0; i2 < (int)world->items[i].lock_x.size(); i2++) {
				if (world->items[i].lock_x[i2] == x and world->items[i].lock_y[i2] == y) limau = true;
			}
		}
	}
	return limau;
}
inline bool stricted_allow_check(ENetPeer* peer, World* world, const int x, const int y) {
	bool _owner = false, sl_onWorld = false;
	for (int i = 0; i < world->width * world->height; i++) {
		if (world->items[i].foreground == 202 or world->items[i].foreground == 204 or world->items[i].foreground == 206) {
			sl_onWorld = true;
			size_t loc_ = world->items[i].lock_x.size();
			for (int fac = 0; fac < (int)loc_; fac++) {
				if (x == world->items[i].lock_x[fac] and y == world->items[i].lock_y[fac]) {
					if (world->items[i].string_data == pInfo(peer)->displayNameBackup or find(world->items[i].randlist.begin(), world->items[i].randlist.end(), pInfo(peer)->rawName) != world->items[i].randlist.end() or isDev(peer) or isMod(peer)) _owner = true;
					else _owner = false;
				}
			}
		}
	}
	if (not _owner) return false;
	return true;
}
inline bool public_stricted_(ENetPeer* peer, World* world, const int x, const int y) {
	bool is_open = false, sl_onWorld = false;
	for (int i = 0; i < world->width * world->height; i++) {
		if (world->items[i].foreground == 202 or world->items[i].foreground == 204 or world->items[i].foreground == 206) {
			sl_onWorld = true;
			size_t loc_ = world->items[i].lock_x.size();
			for (int fac = 0; fac < (int)loc_; fac++) {
				if (x == world->items[i].lock_x[fac] and y == world->items[i].lock_y[fac]) {
					if (world->items[i].opened) is_open = true;
					else is_open = false;
				}
			}
		}
	}
	if (not is_open) return false;
	return true;
}
inline string check_owner_sl(World* world, const int x, const int y) {
	bool _owner = false, sl_onWorld = false;
	string name = "";
	for (int i = 0; i < world->width * world->height; i++) {
		if (world->items[i].foreground == 202 or world->items[i].foreground == 204 or world->items[i].foreground == 206) {
			sl_onWorld = true;
			size_t loc_ = world->items[i].lock_x.size();
			for (int fac = 0; fac < (int)loc_; fac++) {
				if (x == world->items[i].lock_x[fac] and y == world->items[i].lock_y[fac]) {
					if (not world->items[i].string_data.empty()) {
						_owner = true, name = world->items[i].string_data;
					}
				}
			}
		}
	}
	if (sl_onWorld and _owner and not name.empty()) {
		return name;
	}
	return name;
}
inline bool this_stricted_(World* world, const int x, const int y, bool all_check = false) {
	bool this_stricted = false, sl_onWorld = false;
	for (int i = 0; i < world->width * world->height; i++) {
		if (world->items[i].foreground == 202 or world->items[i].foreground == 204 or world->items[i].foreground == 206) {
			sl_onWorld = true;
			size_t loc_ = world->items[i].lock_x.size();
			for (int fac = 0; fac < (int)loc_; fac++) {
				if (x == world->items[i].lock_x[fac] and y == world->items[i].lock_y[fac]) {
					this_stricted = true;
				}
			}
		}
	}
	if (not sl_onWorld or not this_stricted) return false;
	return true;
}
inline void apply_lock(ENetPeer* peer, int x, int y, int tile_id, World* world, bool re_apply, bool lock_public, bool admin) {
	int s_ = 0, c_t = 0, locksz = 0;
	vector<vec2> total;
	vec2 start_pos = { x, y };
	deque<vec2> tisu_wajah{ start_pos };

	if (tile_id == 202) s_ = 10;
	else if (tile_id == 204) s_ = 48;
	else s_ = 200;

	world->items[x + (y * world->width)].lock_x.clear(), world->items[x + (y * world->width)].lock_y.clear();
	while (int(tisu_wajah.size()) < s_) {
		deque<vec2> she_;
		for (const auto& m_ : tisu_wajah) {
			vector<vec2> legion;
			if (m_.m_y + 1 < world->height) legion.push_back({ m_.m_x, m_.m_y + 1 });
			if (m_.m_x + 1 < world->width) legion.push_back({ m_.m_x + 1, m_.m_y });
			if (m_.m_y - 1 >= 0) legion.push_back({ m_.m_x, m_.m_y - 1 });
			if (m_.m_x - 1 >= 0) legion.push_back({ m_.m_x - 1, m_.m_y });
			if (re_apply) {
				if (m_.m_x + 1 < world->width) legion.push_back({ m_.m_x + 1, m_.m_y });
				if (m_.m_x - 1 >= 0) legion.push_back({ m_.m_x - 1, m_.m_y });
				if (m_.m_y + 1 < world->width) legion.push_back({ m_.m_x, m_.m_y + 1 });
				if (m_.m_y - 1 >= 0) legion.push_back({ m_.m_x, m_.m_y - 1 });
				if (m_.m_x + 1 < world->width and m_.m_y - 1 >= 0) {
					if (world->items[(m_.m_x + 1) + ((m_.m_y) * world->width)].foreground != 0 or world->items[(m_.m_x + 1) + ((m_.m_y) * world->width)].background != 0) legion.push_back({ m_.m_x + 1, m_.m_y - 1 });
					else if (world->items[(m_.m_x) + ((m_.m_y - 1) * world->width)].foreground != 0 or world->items[(m_.m_x) + ((m_.m_y - 1) * world->width)].background != 0) legion.push_back({ m_.m_x + 1, m_.m_y - 1 });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_x - 1 >= 0 and m_.m_y + 1 < world->height) {
					if (world->items[(m_.m_x - 1) + ((m_.m_y) * world->width)].foreground != 0 or world->items[(m_.m_x - 1) + ((m_.m_y) * world->width)].background != 0) legion.push_back({ m_.m_x - 1, m_.m_y + 1 });
					else if (world->items[(m_.m_x) + ((m_.m_y + 1) * world->width)].foreground != 0 or world->items[(m_.m_x) + ((m_.m_y + 1) * world->width)].background != 0) legion.push_back({ m_.m_x - 1, m_.m_y + 1 });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_x + 1 < world->width and m_.m_y + 1 < world->height) {
					if (world->items[(m_.m_x + 1) + ((m_.m_y) * world->width)].foreground != 0 or world->items[(m_.m_x + 1) + ((m_.m_y) * world->width)].background != 0) legion.push_back({ m_.m_x + 1, m_.m_y + 1 });
					else if (world->items[(m_.m_x) + ((m_.m_y + 1) * world->width)].foreground != 0 or world->items[(m_.m_x) + ((m_.m_y + 1) * world->width)].background != 0) legion.push_back({ m_.m_x + 1, m_.m_y + 1 });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_x - 1 >= 0 and m_.m_y - 1 >= 0) {
					if (world->items[(m_.m_x - 1) + ((m_.m_y) * world->width)].foreground != 0 or world->items[(m_.m_x - 1) + ((m_.m_y) * world->width)].background != 0) legion.push_back({ m_.m_x - 1, m_.m_y - 1 });
					else if (world->items[(m_.m_x) + ((m_.m_y - 1) * world->width)].foreground != 0 or world->items[(m_.m_x) + ((m_.m_y - 1) * world->width)].background != 0) legion.push_back({ m_.m_x - 1, m_.m_y - 1 });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_x + 2 < world->width) {
					if (world->items[(m_.m_x + 1) + (m_.m_y * world->width)].foreground != 0 or world->items[(m_.m_x + 1) + (m_.m_y * world->width)].background != 0) legion.push_back({ m_.m_x + 2, m_.m_y });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_x - 2 >= 0) {
					if (world->items[(m_.m_x - 1) + (m_.m_y * world->width)].foreground != 0 or world->items[(m_.m_x - 1) + (m_.m_y * world->width)].background != 0) legion.push_back({ m_.m_x - 2, m_.m_y });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_y + 2 < world->width) {
					if (world->items[(m_.m_x) + ((m_.m_y + 1) * world->width)].foreground != 0 or world->items[(m_.m_x) + ((m_.m_y + 1) * world->width)].background != 0) legion.push_back({ m_.m_x, m_.m_y + 2 });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
				if (m_.m_y - 2 >= 0) {
					if (world->items[(m_.m_x) + ((m_.m_y - 1) * world->width)].foreground != 0 or world->items[(m_.m_x) + ((m_.m_y - 1) * world->width)].background != 0) legion.push_back({ m_.m_x, m_.m_y - 2 });
					else legion.push_back({ m_.m_x, m_.m_y });
				}
			}
			if (s_ != 10 and not re_apply) {
				if (m_.m_x - 1 >= 0 and m_.m_y - 1 >= 0) legion.push_back({ m_.m_x - 1, m_.m_y - 1 });
				if (m_.m_x + 1 < world->width and m_.m_y - 1 >= 0) legion.push_back({ m_.m_x + 1, m_.m_y - 1 });
				if (m_.m_x + 1 < world->width and m_.m_y + 1 < world->height) legion.push_back({ m_.m_x + 1, m_.m_y + 1 });
				if (m_.m_x - 1 >= 0 and m_.m_y + 1 < world->height) legion.push_back({ m_.m_x - 1, m_.m_y + 1 });
			}
			for (const auto& kong : legion) {
				int f_ = world->items[(kong.m_x + (kong.m_y * world->width))].foreground;
				int b_ = world->items[(kong.m_x + (kong.m_y * world->width))].background;
				if (get_tile(x, y, world) == false) continue;
				if (find(total.begin(), total.end(), kong) != total.end()) continue;
				if (is_tile_locked_by_a_lock(world, kong.m_x, kong.m_y) or (re_apply and f_ == 0 and b_ == 0) or this_stricted_(world, x, y)) continue;
				if (itemDefs.at(f_).blockType == BlockTypes::LOCK or itemDefs.at(f_).blockType == BlockTypes::BEDROCK or itemDefs.at(f_).blockType == BlockTypes::MAIN_DOOR) continue;
				she_.emplace_back(kong), total.emplace_back(kong);
				if (int(total.size()) > s_) goto done;
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
	if (size > unsigned int(s_)) size = s_;
	PlayerMoving p_;
	p_.packetType = 15;
	p_.characterState = 0x8;
	p_.x = 0, p_.y = 0;
	p_.XSpeed = 0, p_.YSpeed = 0;
	p_.plantingTree = tile_id;
	p_.punchX = x, p_.punchY = y;
	p_.netID = pInfo(peer)->displayNameBackup == world->items[x + (y * world->width)].string_data ? pInfo(peer)->userID : -1;
	BYTE* pmovp = packPlayerMoving(&p_);
	BYTE* packet = new BYTE[64 + s_ * 2];
	memset(packet, 0, 64 + s_ * 2), packet[0] = 4;
	memcpy(packet + 4, pmovp, 56), packet[12] = s_, packet[16] = 8, locksz = s_ * 2, memcpy(packet + 56, &locksz, 4);
	uintmax_t offset = 0;
	for (const auto& pos : total) {
		if (c_t >= s_) break;
		bool tile = get_tile(x, y, world);
		if (tile) {
			if ((pos.m_x == x and pos.m_y == y) or this_stricted_(world, x, y)) continue;
			world->items[(x + (y * world->width))].lock_x.push_back(pos.m_x);
			world->items[(x + (y * world->width))].lock_y.push_back(pos.m_y);
			short lp = pos.m_x + pos.m_y * world->width;
			memcpy(packet + world->height + c_t * 2, &lp, 2);
			c_t += 1, offset += 2;
		}
	}
	if (not re_apply) {
		world->items[(x + (y * world->width))].lock_x.push_back(x), world->items[(x + (y * world->width))].lock_y.push_back(y);
	}
	ENetPacket* packetenet = enet_packet_create(packet, 64 + s_ * 2, ENET_PACKET_FLAG_RELIABLE);
	for (ENetPeer* cpeer = server->peers; cpeer < &server->peers[server->peerCount]; ++cpeer) {
		if (cpeer->state != ENET_PEER_STATE_CONNECTED) continue;
		if (isHere(peer, cpeer)) {
			enet_peer_send(peer, 0, packetenet);
			enet_peer_send(cpeer, 0, packetenet);
		}
	}
	delete[] packet; delete[] pmovp;
}
inline void lock_stricted_update(ENetPeer* peer, int x, int y, int tile_id, int background_id, World* world, bool re_apply, bool lock_public, bool admin) {
	int max, count = 0, locksz = 0;
	if (tile_id == 202) max = 10;
	else if (tile_id == 204) max = 48;
	else max = 200;
	PlayerMoving p_;
	p_.packetType = 0x0F;
	p_.characterState = 0;
	p_.x = 0, p_.y = 0;
	p_.XSpeed = 0, p_.YSpeed = 0;
	p_.plantingTree = tile_id;
	p_.punchX = x, p_.punchY = y;
	p_.netID = pInfo(peer)->displayNameBackup == world->items[(x + (y * world->width))].string_data ? pInfo(peer)->userID : -1;
	BYTE* pmovp = packPlayerMoving(&p_);
	BYTE* packet = new BYTE[64 + max * 2];
	memset(packet, 0, 64 + max * 2), packet[0] = 4, memcpy(packet + 4, pmovp, 56);
	packet[12] = max, packet[16] = 8, locksz = max * 2;
	memcpy(packet + 56, &locksz, 4);
	uintmax_t offset = 0;
	size_t sizex = world->items[(x + (y * world->width))].lock_x.size();
	for (int a = 0; a < (int)sizex; a++) {
		if (count >= max) break;
		bool tile = get_tile(x, y, world);
		if (tile) {
			if (world->items[(x + (y * world->width))].lock_x[a] == x and world->items[(x + (y * world->width))].lock_y[a] == y) continue;
			short lp = world->items[(x + (y * world->width))].lock_x[a] + world->items[(x + (y * world->width))].lock_y[a] * world->width;
			memcpy(packet + world->height + count * 2, &lp, 2);
			count += 1, offset += 2;
		}
	}
	ENetPacket* packetenet = enet_packet_create(packet, 64 + max * 2, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packetenet);
	delete[] packet; delete[] pmovp;
}
inline void update_lock(WorldItem block_2, World* world_, ENetPeer* peer) {
	for (int i = 0; i < world_->width; i++) {
		for (int j = 0; j < world_->height; j++) {
			int loc_xy = i + (j * 100), x = loc_xy % world_->width, y = loc_xy / world_->width;
			if (world_->items[(loc_xy)].foreground == 202 or world_->items[(loc_xy)].foreground == 204 or world_->items[(loc_xy)].foreground == 206) {
				lock_stricted_update(peer, x, y, world_->items[loc_xy].foreground, world_->items[loc_xy].background, world_, true, false, false);
				PlayerMoving data_{};
				data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
				BYTE* raw = packPlayerMoving(&data_, 112);
				BYTE* blc = raw + 56;
				form_visual(blc, world_->items[(loc_xy)], *world_, peer);
				send_raw(4, raw, 112, 0, peer, ENET_PACKET_FLAG_RELIABLE);
			}
		}
	}
}