#pragma once
#include"type.h"
#include<vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define INF 114514.0

void EncodeSSBOTriangle(std::vector<Triangle>& triangles, std::vector<SSBO_Triangle>& triangles_encoded);
void EncodeSSBOBVH(std::vector<BVHNode>& nodes, std::vector<SSBO_BVH>& nodes_encoded);

// 模型变换矩阵
glm::mat4 getTransformMatrix(glm::vec3 rotateCtrl, glm::vec3 translateCtrl, glm::vec3 scaleCtrl);


// 读取 obj
void readObj(std::string filepath, std::vector<Triangle>& triangles, Material material, glm::mat4 trans, bool smoothNormal, float& id);


bool cmpx(const Triangle& t1, const Triangle& t2);
bool cmpy(const Triangle& t1, const Triangle& t2);
bool cmpz(const Triangle& t1, const Triangle& t2);

int buildBVHTree(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int n);

// SAH 优化构建 BVH
int buildBVHwithSAH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int n);
 

float* calculateHdrCache(float* HDR, int width, int height);