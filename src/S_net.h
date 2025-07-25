#pragma once

#include <stdbool.h>
#include <stdint.h>

void net_init();
void net_update();
void net_teardown();

void net_host(uint16_t);
void net_connect(const char*, uint16_t);
void net_disconnect();

bool net_exists();
bool net_connected();
