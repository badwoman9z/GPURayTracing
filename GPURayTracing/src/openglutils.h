#pragma once
#include <GL/glew.h>
#include <iostream>
unsigned int genImageTexture2d(GLenum internalFormat, GLenum type, GLenum format, int w, int h, int location);


unsigned int genTexture2d(GLenum internalFormat, GLenum type, GLenum format, int w, int h);