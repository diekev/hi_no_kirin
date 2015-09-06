#pragma once

const int WIDTH = 1280;
const int HEIGHT = 960;

class Camera;
class VolumeShader;

class Viewer {
	int m_state;
	glm::vec4 m_bg;

	Camera *m_camera;
	VolumeShader *m_volume_shader;

public:
	Viewer();
	~Viewer();

	void init(const char *filename);
	void shutDown();
	void resize(int w, int h);
	void mouseDownEvent(int button, int s, int x, int y);
	void mouseMoveEvent(int x, int y);
	void keyboardEvent(unsigned char key, int x, int y);
	void render();
	void setViewDir();
};
