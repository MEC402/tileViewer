#include "Window.h"
#include "../out/Error.h"
namespace fk {


	Window::Window(
		const std::string& NAME,
		int width,
		int height,
		Flag flags
	) {
		// Account for window dimensions < 1.
		if (height < 1 || width < 1) {
			SDL_DisplayMode current;
			// Gets the resolution of the users monitor
			// https://wiki.libsdl.org/SDL_GetCurrentDisplayMode
			TRY_SDL(SDL_GetCurrentDisplayMode(0, &current));
			if (width < 1) { width = current.w + width; }
			if (height < 1) { height = current.h + height; }
		}
		// OR all the flags.
		Uint32 sdlFlags(SDL_WINDOW_OPENGL);
		if ((int)flags & FULLSCREEN) {
			sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			width = 0;
			height = 0;
		}
		if (flags & BORDERLESS) { sdlFlags |= SDL_WINDOW_BORDERLESS; }
		if (flags & HIGH_DPI) { sdlFlags |= SDL_WINDOW_ALLOW_HIGHDPI; }
		if (flags & MINIMIZED) { sdlFlags |= SDL_WINDOW_MINIMIZED; }
		if (flags & INVISIBLE) { sdlFlags |= SDL_WINDOW_HIDDEN; }
		if (flags & RESIZABLE) { sdlFlags |= SDL_WINDOW_RESIZABLE; }

		// Create a window with the specified name, position, dimensions, and flags
		// ^ https://wiki.libsdl.org/SDL_CreateWindow
		TRY_SDL(
			m_windowPtr = SDL_CreateWindow(
				NAME.c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				width,
				height,
				sdlFlags
			)
		);
		m_dimentions.x = width;
		m_dimentions.y = height;
		// Create an OpenGL context for use with an OpenGL window, and make it current
		// ^ https://wiki.libsdl.org/SDL_GL_CreateContext
		TRY_SDL(SDL_GL_CreateContext(m_windowPtr));
		// ^ http://glew.sourceforge.net/basic.html
		TRY_GLEW(glewInit());
		// Check OpenGL version
		std::printf("\nOpenGL Version: %s\n", glGetString(GL_VERSION));
		// Specify the clear value for the depth buffer
		//^ https://www.opengl.org/sdk/docs/man2/xhtml/glClearDepth.xml
		TRY_GL(glClearDepth(1.0f));
		// Set the background color
		TRY_GL(glClearColor(0.5f, 0.5f, 0.5f, 1.0f));
		// Vsync
		// ^ https://wiki.libsdl.org/SDL_GL_SetSwapInterval
		TRY_SDL(SDL_GL_SetSwapInterval(1));
		TRY_GL(glEnable(GL_BLEND));
		TRY_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		TRY_GL(glEnable(GL_CULL_FACE));
		TRY_GL(glCullFace(GL_FRONT));
		//TRY_GL(glEnable(GL_ALPHA_TEST));
		//TRY_GL(glAlphaFunc(GL_GREATER, 0));
		TRY_GL(glEnable(GL_SCISSOR_TEST));
		//TRY_GL(glEnable(GL_DEPTH_TEST));
		TRY_SDL(m_id = SDL_GetWindowID(m_windowPtr));
		m_windowIsInitialized = true;
	}
	glm::ivec2 Window::getDimensions() const { return m_dimentions; }
	// TODO: Make this actually adjust the window size.
	//void Window::setDimensions(const int& T_WINDOW_WIDTH, const int& T_WINDOW_HEIGHT) {
	//	m_width = T_WINDOW_WIDTH; m_height = T_WINDOW_HEIGHT;
	//}
	int Window::getID() const { return m_id; }
	void Window::minimize() { TRY_SDL(SDL_MinimizeWindow(m_windowPtr)); }
	void Window::restore() { TRY_SDL(SDL_RestoreWindow(m_windowPtr)); }
	void Window::swapGLBuffer() {
		BREAK_IF(!m_windowIsInitialized);
		// ^ https://wiki.libsdl.org/SDL_GL_SwapWindow
		TRY_SDL(SDL_GL_SwapWindow(m_windowPtr));
	}
	void Window::handleEvents(const SDL_Event& SDL_EVENTS) {
		if (SDL_EVENTS.type == SDL_WINDOWEVENT) {
			switch (SDL_EVENTS.window.event) {
			case SDL_WINDOWEVENT_SHOWN:
				///SDL_Log("Window %d shown", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_HIDDEN:
				///SDL_Log("Window %d hidden", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_EXPOSED:
				///SDL_Log("Window %d exposed", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_MOVED:
				///SDL_Log(
				///	"Window %d moved to %d,%d",
				///	sdlEvent.window.windowID, sdlEvent.window.data1, sdlEvent.window.data2
				///);
				break;
			case SDL_WINDOWEVENT_RESIZED:
				m_dimentions.x = SDL_EVENTS.window.data1;
				m_dimentions.y = SDL_EVENTS.window.data2;
				///SDL_Log(
				///	"Window %d resized to %dx%d",
				///	sdlEvent.window.windowID, sdlEvent.window.data1, sdlEvent.window.data2
				///);
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				m_dimentions.x = SDL_EVENTS.window.data1;
				m_dimentions.y = SDL_EVENTS.window.data2;
				///SDL_Log(
				///	"Window %d size changed to %dx%d",
				///	sdlEvent.window.windowID, sdlEvent.window.data1, sdlEvent.window.data2
				///);
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				///SDL_Log("Window %d minimized", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				///SDL_Log("Window %d maximized", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				///SDL_Log("Window %d restored", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_ENTER:
				// TODO: store mouse event
				///SDL_Log("Mouse entered window %d", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_LEAVE:
				// TODO: store mouse event
				///SDL_Log("Mouse left window %d", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				///SDL_Log("Window %d gained keyboard focus", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				///SDL_Log("Window %d lost keyboard focus", sdlEvent.window.windowID);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				///SDL_Log("Window %d closed", sdlEvent.window.windowID);
				break;
			default:
				break;
			}
		}
	}

}