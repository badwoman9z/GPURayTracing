#include "ShowPass.h"
#include<vector>
void ShowPass::init()
{
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    std::vector<glm::vec3> square = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0) };
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * square.size(), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * square.size(), &square[0]);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);   // layout (location = 0) 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);





    shader->setInt("texPass0", 0);
}

void ShowPass::update()
{

    shader->use();
    shader->setVec3("eye", eye);
    shader->setMat4("view", view);
    glBindVertexArray(VAO);
    glViewport(0, 0, 512, 512);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, filteredFrame);
    glDrawArrays(GL_TRIANGLES, 0, 6);
   

}
