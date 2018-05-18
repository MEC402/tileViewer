#include "Viewer.h"
#include "base/Utility.h"
#include <iostream>


void Viewer::create(fk::Tools& tools) {
	std::vector<fk::Shader> shaders{
		tools.shaders.get("Sprite.vert"),
		tools.shaders.get("Sprite.frag"),
		tools.shaders.get("Sprite.geom")
	};
	textBatchPtr = new fk::SpriteBatch(true);
	renderer.setShaders(shaders);
	cam.setDimensions(tools.windowPtr->getDimensions());
	cam.setZoom(70);

	std::vector<fk::Texture> textures;
	textures.push_back(tools.textures.get("Selector.png", 1));
}
void Viewer::destroy(fk::Tools& tools) {

}
void Viewer::open(fk::Tools& tools) {

}
void Viewer::close(fk::Tools& tools) {

}
void Viewer::update(fk::Tools& tools) {
	cam.update();
	cam.setPosition(glm::vec2(0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (tools.ui.getKeyInfo(fk::Key::CTRL_L).downFrames > 1) {
		int downFrames = tools.ui.getKeyInfo(fk::Key::MOUSE_RIGHT).downFrames;
	}
}