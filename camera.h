#pragma once

#include <vector>

#include "vec3.h"
#include "mat4.h"

// Based on Learn OpenGL camera class

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const double YAW         = -90.0;
const double PITCH       =  0.0;
const double SPEED       =  2.5;
const double SENSITIVITY =  0.2;

class Camera
{
public:

    // camera Attributes
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;

    // euler Angles
    double Yaw;
    double Pitch;

    // camera options
    double MovementSpeed;
    double MouseSensitivity;

    // constructor
    Camera(vec3 position = vec3(0.0f, 1.0f, 3.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
        double pitch = PITCH) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }

    void reset()
    {
        Position = vec3(0.0f, 1.0f, 3.0f);
        Yaw = YAW;
        Pitch = PITCH;
        WorldUp = vec3(0.0f, 1.0f, 0.0f);

        updateCameraVectors();
    }

    void processMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

        updateCameraVectors();
    }

    void processKeyboard(Camera_Movement direction, double deltaTime)
    {
        double velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;

        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    inline mat4 GetViewMatrix()
    {
        return view_mat(Position, Position + Front, Up);
    }


private:
    void updateCameraVectors()
    {
        // calculate the new Front vector
        vec3 front;
        front[0] = cos(degrees_to_radians(Yaw)) * cos(degrees_to_radians(Pitch));
        front[1] = sin(degrees_to_radians(Pitch));
        front[2] = sin(degrees_to_radians(Yaw)) * cos(degrees_to_radians(Pitch));
        Front = unit_vector(front);
        // also re-calculate the Right and Up vector
        Right = unit_vector(cross(Front, WorldUp));
        Up = unit_vector(cross(Right, Front));
    }
};

