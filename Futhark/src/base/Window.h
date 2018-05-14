#pragma once
#include <string>
#include <GLM/vec2.hpp>
struct SDL_Window;
union SDL_Event;


/* Window class using SDL
[t3chma] */
class Window {
  public:
	using Flag = unsigned int;
	static const Flag
		BORDERED = 1 << 0,
		FULLSCREEN = 1 << 1,
		BORDERLESS = 1 << 2,
		HIGH_DPI = 1 << 3,
		MINIMIZED = 1 << 4,
		INVISIBLE = 1 << 5,
		RESIZABLE = 1 << 6;
	glm::ivec2 getDimensions() const;
	//TODO: void setDimensions(const int& WINDOW_WIDTH, const int& WINDOW_HEIGHT);
	//TODO: void setMouseWindowCoordinates(const float& X, const float& Y);
	int getID() const;
	void minimize();
	void restore();
	/* Launch the SDL window.
	(NAME) The name of the window.
	(width) The width of the window.
	(height) The height of the window.
	(flags) ORable flags: INVISIBLE, FULLSCREEN, BORDERLESS. */
	Window(
		const std::string& NAME = "Default Window Name",
		int width = 1000,
		int height = 500,
		Flag flags = RESIZABLE
	);
	/* Swap the second OpenGL screen buffer to the screen.
	[T3chma : 2018/01/31] */
	void swapGLBuffer();
	/* Handle any SDL window events not including mouse events.
	(sdlEvents) The events to handle. (only window events will be handled) */
	void handleEvents(const SDL_Event& sdlEvent);
  private:
	// Current dimensions of the window.
	glm::ivec2 m_dimentions{ 720, 480 };
	// A handle to the actual SDL window this class wraps.
	SDL_Window* m_windowPtr{ nullptr };
	// A debug flag for checking if a window is initialized or not.
	bool m_windowIsInitialized{ false };
	// The SDL ID for this window.
	int m_id{ 0 };
};