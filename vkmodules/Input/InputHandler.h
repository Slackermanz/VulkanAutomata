#ifndef VKMODULES_INPUT_HANDLER_H
#define VKMODULES_INPUT_HANDLER_H

#include "../Types/Types.h"

// External globals used by input handlers
extern GLFW_key glfw_key;
extern GLFW_mouse glfw_mouse;
extern KeyCapture kc;

// Clear input event structures
void clear_glfw_key(GLFW_key *e);
void clear_glfw_mouse(GLFW_mouse *e);

// GLFW input callback functions
void glfw_keyboard_event(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_mousemove_event(GLFWwindow* window, double xpos, double ypos);
void glfw_mouseclick_event(GLFWwindow* window, int button, int action, int mods);
void glfw_mousescroll_event(GLFWwindow* window, double xoffset, double yoffset);

// Initialize input handlers for a GLFW window
void init_input_handlers(GLFWwindow* window);

#endif // VKMODULES_INPUT_HANDLER_H
