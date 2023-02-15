#include "TestPass.h"
#include"openglutils.h"


void TestPass::init()
{
	unFilterFrame = genImageTexture2d(GL_RGBA16F, GL_FLOAT, GL_RGBA, width, height, 0);
	HDRLoaderResult hdrRes;
	bool r = HDRLoader::load("./HDR/peppermint_powerplant_4k.hdr", hdrRes);

	float* cache = calculateHdrCache(hdrRes.cols, hdrRes.width, hdrRes.height);
	glGenTextures(1, &hdrmap);
	glBindTexture(GL_TEXTURE_2D, hdrmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrRes.width, hdrRes.height, 0, GL_RGB, GL_FLOAT, hdrRes.cols);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
	shader->setInt("hdrCache", 1);
	shader->setInt("hdrResolution", hdrRes.width);
}

void TestPass::update()
{
	
	shader->use();
	shader->setVec3("eye", eye);
	shader->setMat4("cameraRotate", cameraRotate);
	shader->setInt("frameCounter", frameCounter);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrmap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hdrcache);
	glDispatchCompute((unsigned int)32, (unsigned int)32, 1);

	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT|GL_SHADER_STORAGE_BARRIER_BIT);

}
void readObj(std::string filepath, std::vector<Triangle>& triangles, Material material, glm::mat4 trans, bool smoothNormal, float& id) {

    // ����λ�ã�����
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    // ���ļ���
    std::ifstream fin(filepath);
    std::string line;
    if (!fin.is_open()) {
        std::cout << "�ļ� " << filepath << " ��ʧ��" << std::endl;
        exit(-1);
    }

    // ���� AABB �У���һ��ģ�ʹ�С
    float maxx = -11451419.19;
    float maxy = -11451419.19;
    float maxz = -11451419.19;
    float minx = 11451419.19;
    float miny = 11451419.19;
    float minz = 11451419.19;

    // ���ж�ȡ
    while (std::getline(fin, line)) {
        std::istringstream sin(line);   // ��һ�е�������Ϊ string stream �������Ҷ�ȡ
        std::string type;
        GLfloat x, y, z;
        int v0, v1, v2;
        int vn0, vn1, vn2;
        int vt0, vt1, vt2;
        char slash;

        // ͳ��б����Ŀ���ò�ͬ��ʽ��ȡ
        int slashCnt = 0;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == '/') slashCnt++;
        }

        // ��ȡobj�ļ�
        sin >> type;
        if (type == "v") {
            sin >> x >> y >> z;
            vertices.push_back(glm::vec3(x, y, z));
            maxx = std::max(maxx, x); maxy = std::max(maxx, y); maxz = std::max(maxx, z);
            minx = std::min(minx, x); miny = std::min(minx, y); minz = std::min(minx, z);
        }
        if (type == "f") {
            if (slashCnt == 6) {
                sin >> v0 >> slash >> vt0 >> slash >> vn0;
                sin >> v1 >> slash >> vt1 >> slash >> vn1;
                sin >> v2 >> slash >> vt2 >> slash >> vn2;
            }
            else if (slashCnt == 3) {
                sin >> v0 >> slash >> vt0;
                sin >> v1 >> slash >> vt1;
                sin >> v2 >> slash >> vt2;
            }
            else {
                sin >> v0 >> v1 >> v2;
            }
            indices.push_back(v0 - 1);
            indices.push_back(v1 - 1);
            indices.push_back(v2 - 1);
        }
    }

    // ģ�ʹ�С��һ��
    float lenx = maxx - minx;
    float leny = maxy - miny;
    float lenz = maxz - minz;
    float maxaxis = std::max(lenx, std::max(leny, lenz));
    for (auto& v : vertices) {
        v.x /= maxaxis;
        v.y /= maxaxis;
        v.z /= maxaxis;
    }

    // ͨ�������������任
    for (auto& v : vertices) {
        glm::vec4 vv = glm::vec4(v.x, v.y, v.z, 1);
        vv = trans * vv;
        v = glm::vec3(vv.x, vv.y, vv.z);
    }

    // ���ɷ���
    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0, 0, 0));
    for (int i = 0; i < indices.size(); i += 3) {
        glm::vec3 p1 = vertices[indices[i]];
        glm::vec3 p2 = vertices[indices[i + 1]];
        glm::vec3 p3 = vertices[indices[i + 2]];
        glm::vec3 n = glm::normalize(cross(p2 - p1, p3 - p1));
        normals[indices[i]] += n;
        normals[indices[i + 1]] += n;
        normals[indices[i + 2]] += n;
    }

    // ���� Triangle ��������
    int offset = triangles.size();  // ��������
    triangles.resize(offset + indices.size() / 3);
    for (int i = 0; i < indices.size(); i += 3) {
        Triangle& t = triangles[offset + i / 3];
        // ����������
        t.p1 = vertices[indices[i]];
        t.p2 = vertices[indices[i + 1]];
        t.p3 = vertices[indices[i + 2]];
        if (!smoothNormal) {
            glm::vec3 n = normalize(cross(t.p2 - t.p1, t.p3 - t.p1));
            t.n1 = n; t.n2 = n; t.n3 = n;
        }
        else {
            t.n1 = normalize(normals[indices[i]]);
            t.n2 = normalize(normals[indices[i + 1]]);
            t.n3 = normalize(normals[indices[i + 2]]);
        }
        id++;
        material.meshId = id;
        // ������
        t.material = material;
    }
}
