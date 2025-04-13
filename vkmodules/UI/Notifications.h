#ifndef VKMODULES_NOTIFICATIONS_H
#define VKMODULES_NOTIFICATIONS_H

#include <string>
#include "../Types/Types.h"
#include "../Utils/Logger.h"

// Notification messages
extern const char* notification_list[20];

// Send a notification to the user
void send_notif(int idx, IMGUI_Config *gc, bool clear = true);

// Send a notification with a float value
void send_notif_float(int idx, float f, IMGUI_Config *gc);

#endif // VKMODULES_NOTIFICATIONS_H