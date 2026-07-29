#include <GLFW/glfw3.h>

#include "InputHandler.h"
#include "../Utils/Logger.h"
#include "../../lib/imgui.h"
#include "../../lib/imgui_impl_glfw.h"

// External globals - will be initialized in the main file
GLFW_key glfw_key;
GLFW_mouse glfw_mouse;
KeyCapture kc;

void clear_glfw_key(GLFW_key *e) {
	e->key 		= 0;
	e->scancode = 0;
	e->action 	= 0;
	e->mods 	= 0;
}

void clear_glfw_mouse(GLFW_mouse *e) {
	e->mods 	= 0;
	e->xoffset 	= 0;
	e->yoffset 	= 0;
	// Do not clear scroll accumulators here; high-resolution scroll deltas
	// may arrive as fractional values that need to survive across frames.
}

void glfw_keyboard_event(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(kc.has_keyboard) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}
	else {
		glfw_key.window 	= window;
		glfw_key.key		= key;
		glfw_key.scancode 	= scancode;
		glfw_key.action 	= action;
		glfw_key.mods 		= mods;
/*		loglevel = MAXLOG;
		hd("INPUT:", "KEY");
		ov( "key scancode", scancode);
		ov( "key action", 	action 	);
		ov( "key mods", 	mods 	);
		loglevel = -1;*/
	}
}

void glfw_mousemove_event(GLFWwindow* window, double xpos, double ypos) {
	glfw_mouse.window 	= window;
	glfw_mouse.xpos		= xpos;
	glfw_mouse.ypos 	= ypos;
}

void glfw_mouseclick_event(GLFWwindow* window, int button, int action, int mods) {
	if(kc.has_mouse) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}
	else {
		glfw_mouse.window 	= window;
		glfw_mouse.button	= button;
		glfw_mouse.action 	= action;
		glfw_mouse.mods 	= mods;
	}
/*	loglevel = MAXLOG;
	hd("INPUT:", "MOUSE");
	ov( "mouse button", button 	);
	ov( "mouse action", action 	);
	ov( "mouse mods", 	mods 	);
	loglevel = -1;*/
}

void glfw_mousescroll_event(GLFWwindow* window, double xoffset, double yoffset) {
	if(kc.has_mouse) {
		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	}
	else {
		glfw_mouse.window 			= window;
		glfw_mouse.xoffset			+= xoffset;
		glfw_mouse.yoffset 			+= yoffset;
		glfw_mouse.scroll_x_accum	+= xoffset;
		glfw_mouse.scroll_y_accum	+= yoffset;
	}
/*	loglevel = MAXLOG;
	hd("INPUT:", "MOUSE");
	ov( "mouse xoffset", xoffset );
	ov( "mouse yoffset", yoffset );
	loglevel = -1;*/
}

void init_input_handlers(GLFWwindow* window) {
	// Initialize the input handlers
	glfwSetKeyCallback(window, glfw_keyboard_event);
	glfwSetCursorPosCallback(window, glfw_mousemove_event);
	glfwSetMouseButtonCallback(window, glfw_mouseclick_event);
	glfwSetScrollCallback(window, glfw_mousescroll_event);
    
	// Initialize the input state
	clear_glfw_key(&glfw_key);
	clear_glfw_mouse(&glfw_mouse);
	kc.has_keyboard = false;
	kc.has_mouse = false;
}