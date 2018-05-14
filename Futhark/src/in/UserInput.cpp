#include "UserInput.h"


UserInput::UserInput() {
	m_keyStates.reserve(512);
	m_downKeys.reserve(16);
	m_unpressedKeys.reserve(16);
}
GameState UserInput::poll() {
	m_mouseHistory.second = m_mouseHistory.first;
	GameState gs = m_pollSDL();
	MouseInfo& mouseInfo = m_mouseHistory.first;
	// Get last Mod key.
	// Handle down keys/butts.
	for (auto&& key : m_downKeys) {
		KeyInfo& ki = m_keyStates[key];
		if (++ki.downFrames == 1) {
			ki.pressPos = m_mouseHistory.first.position;
		}
	}
	// Handle unpressed key/butts.
	for (auto&& key : m_unpressedKeys) {
		KeyInfo& ki = m_keyStates[key];
		ki.downFrames = 0;
		ki.unpressPos = m_mouseHistory.first.position;
	}
	// Clear unpressed lists.
	m_unpressedKeys.clear();
	return gs;
}
UserInput::MouseInfo UserInput::getMouseInfo(unsigned int framesAgo) const {
	if (framesAgo = 0) { return m_mouseHistory.first; }
	else { return m_mouseHistory.second; }
}
void UserInput::setShowCursor(bool show) {
	if (show) { SDL_ShowCursor(SDL_ENABLE); }
	else { SDL_ShowCursor(SDL_DISABLE); }
}
UserInput::KeyInfo UserInput::getKeyInfo(Key key) {
	return m_keyStates[key];
}
GameState UserInput::m_pollSDL() {
	SDL_Event sdlEvent;
	GameState gs = GameState::PLAY;
	MouseInfo& mouseInfo = m_mouseHistory.first;
	// Loops until there are no more events to process
	while (SDL_PollEvent(&sdlEvent)) {
		switch (sdlEvent.type) {
		  case SDL_QUIT:
			gs = GameState::EXIT;
		  break;
		  case SDL_KEYDOWN:
			m_keyEvent(true, sdlEvent.key.keysym.sym, true);
		  break;
		  case SDL_KEYUP:
			m_keyEvent(false, sdlEvent.key.keysym.sym, true);
		  break;
		  case SDL_MOUSEBUTTONDOWN:
			m_keyEvent(true, sdlEvent.button.button, false);
		  break;
		  case SDL_MOUSEBUTTONUP:
			m_keyEvent(false, sdlEvent.button.button, false);
		  break;
		  case SDL_MOUSEMOTION:
			mouseInfo.position.x = sdlEvent.motion.x;
			mouseInfo.position.y = sdlEvent.motion.y;
			mouseInfo.windowID = sdlEvent.motion.windowID;
		  break;
		  case SDL_MOUSEWHEEL:
			mouseInfo.wheel = sdlEvent.wheel.y;
		  break;
		  default:
			for (auto&& windowPtr : windowPtrs) { windowPtr->handleEvents(sdlEvent); }
		  break;
		}
	}
	return gs;
}
void UserInput::m_keyEvent(bool down, int keyID, bool notButt) {
	Key kiPtr = (Key)(notButt ? keyID : keyID);
	// Setup down/up info.
	if (down) {
		// Add key/butt to down list.
		m_downKeys.insert(kiPtr);
		// Note: m_unpressedKeyPtrs get cleared every frame so we don't have to remove from that.
	}
	else {
		// Find and remove key/butt from down list.
		m_downKeys.erase(kiPtr);
		// Add to the unpressed list. 
		m_unpressedKeys.insert(kiPtr);
	}
}

}
