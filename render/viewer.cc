#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>

#define DWREAL_IS_DOUBLE 0
#include <openvdb/openvdb.h>

#include <iostream>

#include "objects/grid.h"
#include "objects/volume.h"

#include "camera.h"
#include "viewer.h"

Viewer::Viewer()
    : m_mouse_button(0)
    , m_bg(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f))
    , m_camera(new Camera())
    , m_grid(new Grid(20, 20))
    , m_volume(nullptr)
{}

Viewer::~Viewer()
{
	delete m_camera;
	delete m_grid;
	delete m_volume;
}

void Viewer::init()
{
	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);
	m_camera->updateViewDir();
}

void Viewer::resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	m_camera->resize(w, h);
}

void Viewer::mouseDownEvent(int button, int s, int x, int y)
{
	bool need_redisplay = false;

	m_modifier = glutGetModifiers();

	if (button == GLUT_MIDDLE_BUTTON) {
		m_mouse_button = 0;
	}
	else if ((button == 3 || button == 4) && (s != GLUT_UP)) {
		m_mouse_button = button;
		need_redisplay = true;
	}
	else {
		m_mouse_button = 1;
	}

	m_camera->mouseDownEvent(m_mouse_button, s, x, y);

	if (need_redisplay) {
		glutPostRedisplay();
	}
}

void Viewer::mouseMoveEvent(int x, int y)
{
	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
	glutPostRedisplay();
}

void Viewer::keyboardEvent(unsigned char key, int /*x*/, int /*y*/)
{
	bool need_slicing = false;

	switch (key) {
		case '-':
			m_volume->changeNumSlicesBy(-1);
			need_slicing = true;
			break;
		case '+':
			m_volume->changeNumSlicesBy(1);
			need_slicing = true;
			break;
		case 'l':
			m_volume->toggleUseLUT();
			break;
		case 'b':
			m_volume->toggleBBoxDrawing();
			break;
		case 't':
			m_volume->toggleTopologyDrawing();
			break;
	}

	if (need_slicing) {
		m_volume->slice(m_camera->viewDir());
	}

	glutPostRedisplay();
}

void Viewer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_camera->updateViewDir();
	auto view_dir = m_camera->viewDir();
	auto MVP = m_camera->MVP();

	m_grid->render(MVP);

	if (m_volume != nullptr) {
		m_volume->render(view_dir, MVP);
	}

	glutSwapBuffers();
}

void Viewer::setVolume(Volume *volume)
{
	m_volume = volume;
}
