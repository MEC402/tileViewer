#pragma once
#include <vector>
#include "Window.h"
#include <string>
#include "Tools.h"
namespace fk {


	/* A scene class for deriving your own scenes. */
	class Scene {
	public:
		friend class Engine;
		Scene() = default;
		virtual ~Scene() = default;
		/* This is called when a scene is added to the game. */
		virtual void create(Tools& tools) = 0;
		/* This is called when a scene is destroyed by the game. */
		virtual void destroy(Tools& tools) = 0;
		/* This is called when a scene is made the current active scene in a scene list. */
		virtual void open(Tools& tools) = 0;
		/* This is called when a DIFFERENT scene is made the current active scene in a scene list */
		virtual void close(Tools& tools) = 0;
		/* Updates scene elements before draw (called by App) */
		virtual void update(Tools& tools) = 0;
		virtual int getSceneIndex() const final;
	protected:
		// Used to keep track of the scenes by the app
		int p_sceneIndex{ -1 };
	};

}