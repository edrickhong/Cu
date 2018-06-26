
#pragma once

struct wl_registry;
struct wl_display;

_persist wl_registry* (*wl_display_get_registry_ftpr)(wl_display*) = 0;

#define wl_display_get_registry wl_display_get_registry_ftpr


