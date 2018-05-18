#pragma once
#include "base/Scene.h"
#include "out/SpriteBatch.h"
#include "out/SpriteRenderer.h"
#include "out/Camera.h"

class Viewer : public fk::Scene {
  public:
	fk::SpriteBatch* textBatchPtr{ nullptr };
	fk::Camera cam;
	fk::SpriteRenderer renderer;
	Viewer() = default;
	~Viewer() = default;
	// Inherited via fk::Scene
	virtual void create(fk::Tools& tools) override;
	virtual void destroy(fk::Tools& tools) override;
	virtual void open(fk::Tools& tools) override;
	virtual void close(fk::Tools& tools) override;
	virtual void update(fk::Tools& tools) override;
};