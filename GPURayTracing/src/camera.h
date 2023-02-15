#pragma once
#include<glm\glm.hpp>
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};
class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 WorldUp;
    glm::vec3 Up;
    glm::vec3 Right;
    float MovementSpeed=2.5;
    Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up) :Position(position), Front(front), WorldUp(up) { updateCameraVectors(); };
    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }
    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {

        Front = glm::normalize(Front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
