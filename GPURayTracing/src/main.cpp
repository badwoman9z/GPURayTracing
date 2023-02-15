
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
//#include"RayTracingPass.h"
#include"CubeMapPass.h"
#include"TestPass.h"
#include"type.h"
#include"utils.h"
#include"camera.h"
#include"ShowPass.h"
#include"FilterPass.h"
// 相机参数
float upAngle = 0.0;
float rotatAngle = 0.0;
float r = 4.0;
// 绘制
clock_t t1, t2;
double dt, fps;
Camera *camera;
CubeMapPass* cubeMapPass;
TestPass* testPass;
ShowPass* showPass;
FilterPass* filterPass;
int frameCounter = 0;
// 显示回调函数 -- 咱啥也不做
void display()
{
    // 帧计时
    t2 = clock();
    dt = (double)(t2 - t1) / CLOCKS_PER_SEC;
    fps = 1.0 / dt;
    std::cout << "\r";
    std::cout << std::fixed << std::setprecision(2) << "FPS : " << fps << "    迭代次数: " << frameCounter;
    t1 = t2;

    // 相机参数
    glm::vec3 eye = glm::vec3(-sin(glm::radians(rotatAngle)) * cos(glm::radians(upAngle)), sin(glm::radians(upAngle)), cos(glm::radians(rotatAngle)) * cos(glm::radians(upAngle)));
    eye.x *= r; eye.y *= r; eye.z *= r;
    glm::mat4 cameraRotate = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));  // 相机注视着原点



    testPass->eye = eye;
    testPass->cameraRotate = glm::inverse(cameraRotate);
    testPass->frameCounter = frameCounter;
    filterPass->frameCounter = frameCounter;
    frameCounter++;
 
    
    testPass->update();
   
    filterPass->update();

    showPass->eye = eye;
    showPass->view = cameraRotate;
    showPass->update();
    glutSwapBuffers();
}

// 每一帧
void frameFunc() {
    glutPostRedisplay();
}
// 鼠标运动函数
double lastX = 0.0, lastY = 0.0;
void mouse(int x, int y) {
    frameCounter = 0;
    // 调整旋转
    rotatAngle += 150 * (x - lastX) / 512;
    upAngle += 150 * (y - lastY) / 512;
    upAngle = std::min(upAngle, 89.0f);
    upAngle = std::max(upAngle, -89.0f);
    lastX = x, lastY = y;
    glutPostRedisplay();    // 重绘

}

// 鼠标按下
void mouseDown(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        lastX = x, lastY = y;
    }
}

// 鼠标滚轮函数
void mouseWheel(int wheel, int direction, int x, int y) {
    frameCounter = 0;
    r += -direction * 0.5;
    glutPostRedisplay();    // 重绘
}
void Keyboard(int key, int x, int y)
{
    frameCounter = 0;
    switch (key) {
    case GLUT_KEY_UP:
    {
        camera->ProcessKeyboard(FORWARD, dt);
    }
    break;
    case GLUT_KEY_DOWN:
    {
        camera->ProcessKeyboard(BACKWARD, dt);
    }
    break;
    case GLUT_KEY_LEFT:
    {
        camera->ProcessKeyboard(LEFT, dt);
    }
    break;
    case GLUT_KEY_RIGHT:
    {
        camera->ProcessKeyboard(RIGHT, dt);
    }
    break;
    }
   glutPostRedisplay();
}
int main(int argc, char** argv)
{
    float id = 0;
    camera = new Camera(glm::vec3(0, 0, 4), glm::vec3(0, 0, -1.5), glm::vec3(0, 1, 0));
    std::vector<Triangle> triangles;

    Material m;
    m.baseColor = glm::vec3(1, 0.5, 0.5);
    m.roughness = 0.0;
    m.metallic = 0.3;
    m.clearcoat = 1.0;
    m.subsurface = 1.0;
    m.clearcoatGloss = 0.05;
    //readObj("./model/sphere2.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(-1, -0.85, 0), glm::vec3(0.75, 0.75, 0.75)),true,id);
    //readObj("models/Stanford Bunny.obj", triangles, m, getTransformMatrix(vec3(0, 0, 0), vec3(-2.6, -1.5, 0), vec3(2.5, 2.5, 2.5)), true);

    m.baseColor = glm::vec3(0.5, 1, 0.5);
    //m.baseColor = glm::vec3(0.75, 0.7, 0.15);
    m.roughness = 0.1;
    m.metallic = 0.0;
    m.clearcoat = 0.1;
    m.subsurface = 1.0;
    m.specular = 1.0;
    //m.emissive = glm::vec3(10, 20, 10);
   //readObj("./model/sphere2.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0, -0.85, 0), glm::vec3(0.75, 0.75, 0.75)), true,id);
    //readObj("./model/teapot.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0, -0.4, 0), glm::vec3(1.75, 1.75, 1.75)), true,id);

    //readObj("./model/Stanford Bunny.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.4, -2.0, 0), glm::vec3(2.5, 2.5, 2.5)), true,id);

    m.baseColor = glm::vec3(0.5, 0.5, 1);
    m.metallic = 0.9;
    m.roughness = 1;
    m.clearcoat = 1.0;
    //m.emissive = vec3(10, 10, 20);
    //readObj("./model/sphere2.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(1, -0.85, 0), glm::vec3(0.75, 0.75, 0.75)), true,id);
    readObj("./model/Stanford Bunny.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(3.4, -1.5, 0), glm::vec3(2.5, 2.5, 2.5)), true,id);

    m.emissive = glm::vec3(0, 0, 0);
    m.baseColor = glm::vec3(0.725, 0.71, 0.68);
    m.baseColor = glm::vec3(1, 1, 1);

    float len = 13.0;
    m.metallic = 0.0;
    m.roughness = 0.1;

    readObj("./model/quad.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0, -1.5, 0), glm::vec3(len, 0.01, len)), false,id);

    m.baseColor = glm::vec3(1, 1, 1);
    m.emissive = glm::vec3(20, 20, 20);

    //readObj("./model/quad.obj", triangles, m, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 2.0, -0.0), glm::vec3(1.7, 0.01, 1.7)), false,id);

    int nTriangles = triangles.size();
    std::cout << "模型读取完成: 共 " << nTriangles << " 个三角形" << std::endl;


    BVHNode virtualNode;
    virtualNode.left = 255;
    virtualNode.right = 128;
    virtualNode.n = 30;
    virtualNode.AA = glm::vec3(1, 1, 0);
    virtualNode.BB = glm::vec3(0, 1, 0);


    std::vector<BVHNode> nodes{ virtualNode };
    //buildBVHTree(triangles, nodes, 0, triangles.size() - 1, 8);
    buildBVHwithSAH(triangles, nodes, 0, triangles.size() - 1, 8);
    //buildBVH(triangles, nodes, 0, triangles.size() - 1, 8);
    int nNodes = nodes.size();
    std::cout << "BVH 建立完成: 共 " << nNodes << " 个节点" << std::endl;


  
    std::vector<SSBO_Triangle> triangles_encoded1;
    std::vector<SSBO_BVH> nodes_encoded1;


    EncodeSSBOTriangle(triangles, triangles_encoded1);
    EncodeSSBOBVH(nodes, nodes_encoded1);






    glutInit(&argc, argv);              // glut初始化
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(512, 512);// 窗口大小
    glutInitWindowPosition(0, 0);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutCreateWindow("Path Tracing GPU"); // 创建OpenGL上下文
    glewInit();

    std::cout << sizeof(triangles) << std::endl;
    std::cout << sizeof(float) << std::endl;

    


    
    //pass = new RayTracingPass("pass1", "./shader/pass1.vs", "./shader/pass1.fs");

    //pass->init();



    unsigned int ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Triangle) * triangles_encoded1.size(), &triangles_encoded1[0], GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



    unsigned int bvhssbo;
    glGenBuffers(1, &bvhssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_BVH)* nodes_encoded1.size(), &nodes_encoded1[0], GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bvhssbo);


    unsigned int frameInfossbo;
    glGenBuffers(1, &frameInfossbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, frameInfossbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FrameInfo)* 512*512, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, frameInfossbo);


    unsigned int cameraSSBO;
    glGenBuffers(1, &cameraSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cameraSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PreCameraParm), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cameraSSBO);

    unsigned int preFrameInfo;
    glGenBuffers(1, &preFrameInfo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, preFrameInfo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PreFrameInfo)*512*512, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, preFrameInfo);


    //glEnable(GL_DEPTH_TEST);  // 开启深度测试

    //
    //cubeMapPass = new CubeMapPass("pass2", "./shader/hdrToCubeMap.vs", "./shader/hdrToCubeMap.fs");
    //cubeMapPass->backgroundShader = std::make_shared<Shader>("./shader/background.vs", "./shader/background.fs");
    //cubeMapPass->init();

    testPass = new TestPass("pass3", "./shader/RayTracingPass.comp");
    filterPass = new FilterPass("pass4", "./shader/FilterPass.comp");
    showPass = new ShowPass("pass4", "./shader/ShowPass.vs", "./shader/ShowPass.fs");
    testPass->init();
    showPass->init();
    filterPass->init();
    filterPass->unfilterTexture = testPass->unFilterFrame;
    showPass->filteredFrame = filterPass->filterTexture;

    glutDisplayFunc(display);   // 设置显示回调函数
    glutIdleFunc(frameFunc);    // 闲置时刷新
    glutMotionFunc(mouse);      // 鼠标拖动
    glutMouseFunc(mouseDown);   // 鼠标左键按下
    glutMouseWheelFunc(mouseWheel); // 滚轮缩放
    glutSpecialFunc(Keyboard);
    glutMainLoop();


    return 0;
}
