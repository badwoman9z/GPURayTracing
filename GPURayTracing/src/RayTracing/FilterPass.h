#pragma once
#include"RenderPass.h"
class FilterPass:public RenderPass
{
public:
	FilterPass(std::string name, const char* cspath) : RenderPass(name, cspath) {};
	unsigned int filterTexture;
	unsigned int unfilterTexture;
	int frameCounter;
	virtual void init();
	virtual void update();


};

