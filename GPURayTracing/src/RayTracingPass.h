#pragma once
#include"RenderPass.h"
#include<GL/glew.h>
#include<vector>
#include"type.h"
#include "hdrloader.h"
class RayTracingPass:public RenderPass
{
public:

	unsigned int mixTexture;
	unsigned int tbo;
	unsigned int tssbo;
	unsigned int nbo;
	unsigned int hdrmap;
	unsigned int hdrcache;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int FBO;

	int width = 512;
	int height =512;
    int frameCounter = 0;
	int nTriangles;
	glm::vec3 eye;
	glm::mat4 cameraRotate;
	RayTracingPass(std::string name, const char* vspath, const char* fspath, const char* gspath = nullptr) : RenderPass(name, vspath, fspath, gspath) {
	
	};
	virtual void init();
	virtual void update();

	unsigned int genImageTexture2d(GLenum internalFormat, GLenum type, GLenum format, int w, int h, int location) {
		unsigned int  ID;
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindImageTexture(location, ID, 0, GL_FALSE, 0, GL_READ_WRITE, internalFormat);
		return ID;
	}


};

