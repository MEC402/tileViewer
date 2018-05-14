#pragma once
#include <vector>
#include "Window.h"
#include <string>
#include "Tools.h"


/* A scene class for deriving your own scenes.
[T3chma] */
class Scene {
  public:
	friend class Engine;
	Scene() = default;
	virtual ~Scene() = default;
	/* This is called when a scene is added to the game.
	[T3chma : 2018/03/10] */
	virtual void create(Tools& tools) = 0;
	/* This is called when a scene is destroyed by the game.
	[T3chma : 2018/03/10] */
	virtual void destroy(Tools& tools) = 0;
	/* This is called when a scene is made the current active scene in a scene list.
	[T3chma : 2018/01/31] */
	virtual void open(Tools& tools) = 0;
	/* This is called when a DIFFERENT scene is made the current active scene in a scene list
	[T3chma : 2018/01/31] */
	virtual void close(Tools& tools) = 0;
	/* Updates scene elements before draw (called by App)
	[T3chma : 2018/01/31] */
	virtual void update(Tools& tools) = 0;
	virtual int getSceneIndex() const final;
protected:
	// Used to keep track of the scenes by the app
	int p_sceneIndex{ -1 };
};