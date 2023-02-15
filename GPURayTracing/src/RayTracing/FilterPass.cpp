#include"FilterPass.h"
#include"openglutils.h"
void FilterPass::init()
{
	filterTexture = genImageTexture2d(GL_RGBA16F, GL_FLOAT, GL_RGBA, 512, 512, 1);
}

void FilterPass::update()
{
	shader->use();
	shader->setInt("frameCounter", frameCounter);
	glDispatchCompute((unsigned int)32, (unsigned int)32, 1);

	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT| GL_SHADER_STORAGE_BARRIER_BIT);
	
}
