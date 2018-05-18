#pragma once
#include <vector>
#include <SDL/SDL.h>
#include "../base/Window.h"
#include "../base/Utility.h"
#include <unordered_map>
#include <queue>
#include <GLM/vec2.hpp>
#include <unordered_set>
namespace fk {


	// Types of action triggers for keys
	enum class Trigger { PRESS, UNPRESS, HOLD };


	// Key enums
	enum class Key {
		// Numbers
		NUM0 = SDLK_0, NUM1 = SDLK_1, NUM2 = SDLK_2, NUM3 = SDLK_3, NUM4 = SDLK_4,
		NUM5 = SDLK_5, NUM6 = SDLK_6, NUM7 = SDLK_7, NUM8 = SDLK_8, NUM9 = SDLK_9,
		// Numpad
		PAD0 = SDLK_0, PAD1 = SDLK_1, PAD2 = SDLK_2, PAD3 = SDLK_3, PAD4 = SDLK_4,
		PAD5 = SDLK_5, PAD6 = SDLK_6, PAD7 = SDLK_7, PAD8 = SDLK_8, PAD9 = SDLK_9,
		// Function Keys
		F1 = SDLK_F1, F2 = SDLK_F2, F3 = SDLK_F3, F4 = SDLK_F4, F5 = SDLK_F5, F6 = SDLK_F6,
		F7 = SDLK_F7, F8 = SDLK_F8, F9 = SDLK_F9, F10 = SDLK_F10, F11 = SDLK_F11, F12 = SDLK_F12,
		// Letters
		A = SDLK_a, B = SDLK_b, C = SDLK_c, D = SDLK_d, E = SDLK_e, F = SDLK_f, G = SDLK_g, H = SDLK_h,
		I = SDLK_i, J = SDLK_j, K = SDLK_k, L = SDLK_l, M = SDLK_m, N = SDLK_n, O = SDLK_o, P = SDLK_p,
		Q = SDLK_q, R = SDLK_r, S = SDLK_s, T = SDLK_t, U = SDLK_u, V = SDLK_v, W = SDLK_w, X = SDLK_x,
		Y = SDLK_y, Z = SDLK_z,
		// Misc Keys
		SPACE = SDLK_SPACE, APOSTROPHE = SDLK_BACKQUOTE, COMMA = SDLK_COMMA, PERIOD = SDLK_PERIOD,
		SEMICOLON = SDLK_SEMICOLON, QUOTE = SDLK_QUOTE, BRACKET_L = SDLK_LEFTBRACKET, BRACKET_R = SDLK_RIGHTBRACKET,
		SLASH_F = SDLK_SLASH, SLASH_B = SDLK_BACKSLASH, MINUS = SDLK_MINUS, EQUAL = SDLK_EQUALS, ENTER = SDLK_RETURN,
		RETURN = SDLK_RETURN, TAB = SDLK_TAB, BACK_SPC = SDLK_BACKSPACE,
		// Mod Keys
		SHIFT_L = SDLK_LSHIFT, SHIFT_R = SDLK_RSHIFT,
		ALT_L = SDLK_LALT, ALT_R = SDLK_RALT,
		CTRL_L = SDLK_LCTRL, CTRL_R = SDLK_RCTRL,
		// Mouse Buttons
		MOUSE_LEFT = SDL_BUTTON_LEFT, MOUSE_MIDDLE = SDL_BUTTON_MIDDLE, MOUSE_RIGHT = SDL_BUTTON_RIGHT,
		MOUSE_X1 = SDL_BUTTON_X1, MOUSE_X2 = SDL_BUTTON_X2
	};


	/* Used to handle user input through SDL.
	[t3chma] */
	class UserInput {
	public:
		/* Keybinding info.
		[t3chma] */
		struct KeyInfo {
			// How many frames this has been down for.
			long downFrames{ 0 };
			// Mouse position when this was last pressed.
			glm::ivec2 pressPos{ 0 };
			// Mouse position when this was last unpressed.
			glm::ivec2 unpressPos{ 0 };
		};
		/* Information about the mouse.
		[t3chma] */
		struct MouseInfo {
			// ID of the window the mouse is in.
			int windowID{ 0 };
			// Window position of the mouse.
			glm::ivec2 position{ 0 };
			// Direction of mouse wheel scroll. Pos for up and neg for down.
			int wheel;
		};
		// Window handles.
		std::vector<Window*> windowPtrs;
		/* Constructor
		(history) How many frames of history to keep about mouse information.
		[t3chma] */
		UserInput();
		/* Polls SDL for input and queues actions based on it.
		(actions) Which actions list to queue to.
		[t3chma] */
		GameState poll();
		/* Get mouse info from frame ago.
		(frameAgo) How many frames ago you want the info for.
		[t3chma] */
		MouseInfo getMouseInfo(unsigned int framesAgo) const;
		/* Set showing the mouse cursor or not.
		(show) If the cursor should be show.
		[t3chma] */
		void setShowCursor(bool show);
		/* Get the key info for a key.
		(key) The key you want info for.
		[t3chma] */
		KeyInfo getKeyInfo(Key key);
	private:
		// History of mouse placement.
		std::pair<MouseInfo, MouseInfo> m_mouseHistory;
		// Key info for keys.
		std::unordered_map<Key, KeyInfo> m_keyStates;
		// The currently down keys.
		std::unordered_set<Key> m_downKeys;
		// The just unpressed keys.
		std::unordered_set<Key> m_unpressedKeys;
		// Polls SDL for key/button info.
		GameState m_pollSDL();
		/* Handles key event.
		(down) If this is a down key event.
		(keyID) The ID of the key this event is for.
		(notButt) If this is not a button.
		[t3chma] */
		void m_keyEvent(bool down, int keyID, bool notButt);
	};

}