#pragma once
#include "Scene.h"
#include <map>


/* This class is used to launch and drive the entire aplication. It functions like a state machine where each Scene is a given state. */
class Engine {
  public:
	Engine();
	virtual ~Engine();
	/* Starts the app.
	Make sure to add your screens before you call this. */
	void run();
	/* Stops the app.
	Use this when you want to restart or something without stopping the program. */
	void stop();
	/* Creates the window for the app.
	This is just a wrapper and can be called on p_window instead.
	(name) The name of the window.
	(flags) These ORable flags specify the type of window.
	(width) The width of the window in pixels, 0 = scene width, width < 0 = scene width + width.
	(height) The height of the window in pixels, 0 = scene height, height < 0 = scene width + height.
	< The index of the window which can be use for SDL things. */
	int makeWindow(
		const std::string& name = "Default Window Name",
		int width = 1000,
		int height = 500,
		Window::Flag flags = Window::RESIZABLE
	);
	/* Adds a scene to the internally managed scene list.
	(name) The name of the scene.
	(scenePtr) A pointer to the scene. */
	void addWindowScene(int windowIndex, const std::string& name, Scene* const scenePtr);
	/* Sets the current active scene, closing the old one.
	(sceneName) The name of the scene to switch to. */
	void setWindowScene(int windowIndex, const std::string& sceneName);
	/* Gets the pointer of the scene name given.
	The current scene pointer is returned by default.
	(sceneName) The name of the scene whos pointer will be returned.
	< The pointer to scene with the given name, or nullptr otherwise. */
	Scene* getWindowScenePtr(int windowIndex, const std::string& sceneName = "") const;
	/* Deletes all the scenes from the scene list. */
	void clearScenes();
  private:
	// Window handle.
	std::vector<Window> m_windows;
	// Hash table for looking up scene pointers by name.
	std::map<std::string, std::pair<Window*,Scene*>> m_scenePtrMap;
	// A handle to the current scene.
	std::vector<Scene*> m_currentScenePtrs;
	// Tools for using in your scene.
	Tools m_tools;
};