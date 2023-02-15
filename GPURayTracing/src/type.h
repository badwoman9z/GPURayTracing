#pragma once
#include<glm\glm.hpp>
typedef struct Material {
    float meshId = 0;
    glm::vec3 emissive = glm::vec3(0, 0, 0);  // 作为光源时的发光颜色
    glm::vec3 baseColor = glm::vec3(1, 1, 1);
    float subsurface = 0.0;
    float metallic = 0.0;
    float specular = 0.5;
    float specularTint = 0.0;
    float roughness = 0.5;
    float anisotropic = 0.0;
    float sheen = 0.0;
    float sheenTint = 0.5;
    float clearcoat = 0.0;
    float clearcoatGloss = 1.0;
    float IOR = 1.0;
    float transmission = 0.0;
};
typedef struct Triangle {
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;
    Material material;
};

typedef struct BVHNode {
    int left, right;
    int n, index;
    glm::vec3 AA, BB;
};


typedef struct SSBO_Triangle {
    
    
    
    glm::vec4 p1, p2, p3;    // 顶点坐标
    glm::vec4 n1, n2, n3;    // 顶点法线
    glm::vec3 baseColor;     // 颜色
    float meshId;
    glm::vec4 emissive;      // 自发光参数
    glm::vec4 param1;        // (subsurface, metallic, specular)
    glm::vec4 param2;        // (specularTint, roughness, anisotropic)
    glm::vec4 param3;        // (sheen, sheenTint, clearcoat)
};
typedef struct SSBO_BVH {
    int left, right, n, index;
    glm::vec4 AA, BB;
};
typedef struct BVHNode_encoded {
    glm::vec3 childs;        // (left, right, 保留)
    glm::vec3 leafInfo;      // (n, index, 保留)
    glm::vec3 AA, BB;
};
typedef struct FrameInfo {
    glm::vec4 position;
    glm::vec4 normal;
    
};
typedef struct PreCameraParm {
    glm::vec4 cameraPosition;
    glm::mat4 preViewMat;
};

typedef struct PreFrameInfo {

    glm::vec3 color;
    float meshId;
};

