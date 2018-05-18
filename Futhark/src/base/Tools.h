#pragma once
#include "../in/UserInput.h"
#include "../in/FileCache.h"
#include "Utility.h"
namespace fk {


	/* This is a convenient package of engine tools for passing into each scene. */
	struct Tools {
		/* Used to switch between app modes.
		^ Utility.h: GameState
		(PLAY) Normal operation mode.
		(EXIT) Shuts down the app. */
		GameState gameState{ GameState::PLAY };
		// For swicthing to the next sceen.
		std::string nextSceneName{ "" };
		// For logging player input.
		UserInput ui;
		// A handle to this screens window.
		Window* windowPtr{ nullptr };
		// A handle to the game's cached shaders.
		ShadersCache shaders;
		// A handle to the game's cached textures.
		TextureCache textures;
	};

}