#pragma once
#include"RenderPass.h"
#include<GL/glew.h>
#include<vector>
#include "hdrloader.h"
class TestPass :public RenderPass
{
public:

	int width = 512;
	int height = 512;
	int frameCounter = 0;
	int nTriangles;
	unsigned int hdrmap;
	unsigned int hdrcache;
	glm::vec3 eye;
	glm::mat4 cameraRotate;
	unsigned int unFilterFrame;
	TestPass(std::string name, const char* cspath) : RenderPass(name, cspath) {};
	virtual void init();
	virtual void update();

};

