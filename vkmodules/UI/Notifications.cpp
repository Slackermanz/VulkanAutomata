#include "Notifications.h"
#include "../Utils/Timer.h"

// Define the notification messages
const char* notification_list[20] = {
	"Welcome!",
	"Saved to Archive (PCD256)",
	"Target Updated\nScale & Zoom Discarded",
	"Target Updated\nScale & Zoom Confirmed",
	"Scale & Zoom Discarded",
	"Scale & Zoom Confirmed",
	"GUI disabled\nNote: GUI is always visible while paused",
	"GUI enabled",
	"Recording enabled",
	"Recording disabled",
	"Reset image filename index\nNote: This does not delete previously exported frames",
	"Scale randomized\nZoom reset to zero",
	"Planar Parameter Map",
	"Linear Parameter Map",
	"Circular Parameter Map",
	"Scale",
	"Frame-time Throttle enabled",
	"Frame-time Throttle disabled",
	"Zoom",
	"Export Frequency"
};

// Global for verbose loops control
extern uint32_t verbose_loops;

// Send a notification to the user
void send_notif(int idx, IMGUI_Config *gc, bool clear) {
	gc->notification_index 	= idx;
	gc->notification_timer	= start_timer(gc->notification_timer);
	gc->show_notification 	= true;
	gc->notification_age 	= 1.0f;
	if(!verbose_loops) { loglevel = 2; }
	nt(notification_list[idx]);
	if(!verbose_loops) { loglevel = -1; }
	if(clear) { gc->show_notification_float = false; }
}

// Send a notification with a float value
void send_notif_float(int idx, float f, IMGUI_Config *gc) {
	gc->notification_float_value = f;
	gc->show_notification_float	 = true;
	gc->notification_float_timer = start_timer(gc->notification_float_timer);
	send_notif(idx, gc, false);
	if(!verbose_loops) { loglevel = 2; }
	nt(std::to_string(f));
	if(!verbose_loops) { loglevel = -1; }
}