#include "RayTracingPass.h"

void RayTracingPass::init()
{
	
	std::vector<glm::vec3> square = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0) };
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * square.size(), &square[0], GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glBindVertexArray(0);
	mixTexture = genImageTexture2d(GL_RGBA16F, GL_FLOAT, GL_RGBA, width,height, 0);


	// hdr È«¾°Í¼
	HDRLoaderResult hdrRes;
	bool r = HDRLoader::load("./HDR/peppermint_powerplant_4k.hdr", hdrRes);
	float* cache = calculateHdrCache(hdrRes.cols, hdrRes.width, hdrRes.height);

	glGenTextures(1, &hdrmap);
	glBindTexture(GL_TEXTURE_2D, hdrmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrRes.width, hdrRes.height, 0, GL_RGB, GL_FLOAT, hdrRes.cols);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &hdrcache);
	glBindTexture(GL_TEXTURE_2D, hdrcache);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrRes.width, hdrRes.height, 0, GL_RGB, GL_FLOAT, cache);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);



	shader->use();
	shader->setInt("width", width);
	shader->setInt("height", height);
	shader->setInt("hdrmap", 0);

}

void RayTracingPass::update()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	
	shader->use();
	shader->setInt("frameCounter", frameCounter);
	shader->setVec3("eye", eye);
	shader->setMat4("cameraRotate", cameraRotate);
	glBindVertexArray(VAO);
	glViewport(0, 0, width, height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrmap);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glDisable(GL_BLEND);
	
}

