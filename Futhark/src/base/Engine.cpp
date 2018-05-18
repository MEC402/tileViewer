#pragma once
#include "Engine.h"
#include "../out/Error.h"
#include <SDL/TTF/SDL_ttf.h>
namespace fk {


	Engine::Engine() {
		// Initialize SDL library. Must be called before using any other SDL function.
		// ^ https://wiki.libsdl.org/SDL_Init
		TRY_SDL(SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_JOYSTICK));
		// Set an OpenGL window attribute before window creation.
		// ^ https://wiki.libsdl.org/SDL_GL_SetAttribute
		TRY_SDL(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1));
		TRY_SDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
	}
	Engine::~Engine() { stop(); }
	void Engine::run() {
		// Gameloop
		while (m_tools.gameState != GameState::EXIT) {
			m_tools.gameState = m_tools.ui.poll();
			for (int i = 0; i < m_currentScenePtrs.size(); ++i) {
				if (m_tools.nextSceneName != "") {
					std::string nextSceneName = m_tools.nextSceneName;
					m_tools.nextSceneName = "";
					setWindowScene(i, nextSceneName);
				}
				m_tools.windowPtr = &m_windows[i];
				m_currentScenePtrs[i]->update(m_tools);
				m_windows[i].swapGLBuffer();
			}
		}
		stop();
	}
	void Engine::stop() {
		clearScenes();
		// Use this function to clean up all initialized subsystems.
		// ^ https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();
	}
	int Engine::makeWindow(const std::string& NAME, int width, int height, Window::Flag flags) {
		m_windows.emplace_back(NAME, width, height, flags);
		m_currentScenePtrs.emplace_back(nullptr);
		m_tools.ui.windowPtrs.push_back(&m_windows.back());
		return m_windows.size() - 1;
	}
	void Engine::addWindowScene(int windowIndex, const std::string& name, Scene* const scenePtr) {
		m_tools.windowPtr = &m_windows[windowIndex];
		scenePtr->create(m_tools);
		m_scenePtrMap[name].second = scenePtr;
		m_scenePtrMap[name].first = &m_windows[windowIndex];
	}
	void Engine::setWindowScene(int windowIndex, const std::string& sceneName) {
		auto mapSelection(m_scenePtrMap.find(sceneName));
		if (mapSelection == m_scenePtrMap.end()) {
			LOG_LINE("Scene \"" + sceneName + "\" doesn't exist");
		}
		else {
			if (m_currentScenePtrs[windowIndex] != nullptr) { m_currentScenePtrs[windowIndex]->close(m_tools); }
			m_tools.windowPtr = &m_windows[windowIndex];
			mapSelection->second.first = m_tools.windowPtr;
			mapSelection->second.second->open(m_tools);
			m_currentScenePtrs[windowIndex] = mapSelection->second.second;
		}
	}
	Scene* Engine::getWindowScenePtr(int windowIndex, const std::string& sceneName) const {
		if (sceneName == "") {
			return m_currentScenePtrs[windowIndex];
		}
		else {
			auto mapSelection(m_scenePtrMap.find(sceneName));
			if (mapSelection == m_scenePtrMap.end()) { return nullptr; }
			else { return mapSelection->second.second; }
		}
	}
	void Engine::clearScenes() {
		for (const auto& e : m_scenePtrMap) {
			m_tools.windowPtr = e.second.first;
			e.second.second->destroy(m_tools);
			delete e.second.second;
		}
		m_scenePtrMap.clear();
	}

}