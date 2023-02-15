#pragma once
#include"RenderPass.h"
class ShowPass:public RenderPass
{
public:
	ShowPass(std::string name, const char* vspath, const char* fspath, const char* gspath = nullptr) : RenderPass(name, vspath, fspath, gspath) {
	};
	
	unsigned int VAO;
	unsigned int VBO;
	unsigned int filteredFrame;
	
	glm::vec3 eye;
	glm::mat4 view;
	virtual void init();
	virtual void update();


};


