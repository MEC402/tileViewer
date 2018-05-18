#include "base/Engine.h"
#include "scenes/Viewer.h"


int main(int argc, char** argv) {
	fk::Engine TileViewer;
	TileViewer.makeWindow("TileViewer", -100, -100, fk::Window::RESIZABLE);
	TileViewer.addWindowScene(0, "Viewer", new Viewer());
	TileViewer.setWindowScene(0, "Viewer");
	TileViewer.run();
	return 0;
}