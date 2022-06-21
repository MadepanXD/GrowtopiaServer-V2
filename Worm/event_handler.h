#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <string>
#include <experimental/filesystem>

int musim_type = 0;
bool musim_semi = false, musim_panas = false, musim_gugur = false, musim_dingin = false;
long long event_time_ = 0;

inline void update_event(ENetPeer* peer) {
	long long time_s = time(nullptr);
	if (event_time_ - time_s <= 0) {
		musim_type += 1;
		if (musim_type > 4) musim_type = 0;
		switch (musim_type) {
			case 1: {
				musim_semi = true;
				event_time_ = time_s + 60;
				Send_::console_msg(peer, "MUSIM SEMI");
				break;
			}
			case 2: {
				musim_semi = false;
				musim_panas = true;
				event_time_ = time_s + 60;
				Send_::console_msg(peer, "MUSIM PANAS");
				break;
			}
			case 3: {
				musim_panas = false;
				musim_gugur = true;
				event_time_ = time_s + 60;
				Send_::console_msg(peer, "MUSIM GUGUR");
				break;
			}
			case 4: {
				musim_gugur = false;
				musim_dingin = true;
				event_time_ = time_s + 60;
				Send_::console_msg(peer, "MUSIM DINGIN");
				break;
			}
			default:
				break;
		}
	}
}