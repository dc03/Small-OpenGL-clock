#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma once

#ifndef CAMERA_CLASS_HPP
#  define CAMERA_CLASS_HPP

enum class CameraMovement
{
    forward,
    backward,
    left,
    right
};

constexpr double YAW = -90.0f;
constexpr double PITCH = 0.0f;
constexpr double SPEED = 2.5f;
constexpr double SENSITIVITY = 0.1f;
constexpr double ZOOM = 95.0f;

class Camera
{
    void update_camera_vectors() noexcept;

public:
    glm::vec3 m_position{};
    glm::vec3 m_front{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 m_world_up{};

    double m_yaw{};
    double m_pitch{};

    double m_movement_speed{};
    double m_mouse_sensitivity{};
    double m_zoom{};

    Camera(glm::vec3 = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 = glm::vec3(0.0f, 1.0f, 0.0f), double = YAW, double = PITCH)
        noexcept;
    Camera(double, double, double, double, double, double, double, double)
        noexcept;

    glm::mat4 get_view_matrix() noexcept;
    void process_keyboard(CameraMovement, double) noexcept;
    void process_mouse_movement(double, double, bool) noexcept;
    void process_mouse_scroll(double) noexcept;
    void set_speed(double movement_speed) noexcept
    {
        this->m_movement_speed = movement_speed;
    }
};

Camera::Camera(glm::vec3 position, glm::vec3 up, double yaw, double pitch)
noexcept : m_front{ 0.0f, 0.0f, -1.0f }, m_movement_speed{ SPEED },
m_mouse_sensitivity{ SENSITIVITY }, m_zoom{ ZOOM }
{
    this->m_position = position;
    this->m_world_up = up;
    this->m_yaw = yaw;
    this->m_pitch = pitch;
    this->update_camera_vectors();
}

Camera::Camera(double pos_x, double pos_y, double pos_z, double up_x,
    double up_y, double up_z, double yaw, double pitch) noexcept :
    m_front{ 0.0f, 0.0f, -1.0f }, m_movement_speed{ SPEED },
    m_mouse_sensitivity{ SENSITIVITY }, m_zoom{ ZOOM }
{
    this->m_position = glm::vec3(pos_x, pos_y, pos_z);
    this->m_up = glm::vec3(up_x, up_y, up_z);
    this->m_yaw = yaw;
    this->m_pitch = pitch;
    this->update_camera_vectors();
}

glm::mat4 Camera::get_view_matrix() noexcept
{
    return glm::lookAt(this->m_position, (this->m_position + this->m_front),
        this->m_up);
}

void Camera::process_keyboard(CameraMovement movement_type, double dt) noexcept
{
    float velocity{ static_cast<float>(this->m_movement_speed * dt) };
    if (movement_type == CameraMovement::forward)
        this->m_position += (this->m_front * velocity);
    else if (movement_type == CameraMovement::backward)
        this->m_position -= (this->m_front * velocity);
    else if (movement_type == CameraMovement::left)
        this->m_position -= (this->m_right * velocity);
    else if (movement_type == CameraMovement::right)
        this->m_position += (this->m_right * velocity);
}

void Camera::process_mouse_movement(double x_off, double y_off, bool constrain)
noexcept
{
    x_off *= this->m_mouse_sensitivity;
    y_off *= this->m_mouse_sensitivity;

    this->m_yaw += x_off;
    this->m_pitch += y_off;

    if (constrain)
    {
        if (this->m_pitch > 89.0f)
            this->m_pitch = 89.0f;
        if (this->m_pitch < -89.0f)
            this->m_pitch = -89.0f;
    }

    this->update_camera_vectors();
}

void Camera::process_mouse_scroll(double y_off) noexcept
{
    if (this->m_zoom >= 1.0f && this->m_zoom <= 95.0f)
        this->m_zoom -= y_off;
    if (this->m_zoom <= 1.0f)
        this->m_zoom = 1.0f;
    if (this->m_zoom >= 95.0f)
        this->m_zoom = 95.0f;
}

void Camera::update_camera_vectors() noexcept
{
    glm::vec<3, double> front;
    front.x = cos(glm::radians(this->m_yaw)) * cos(glm::radians(this->m_pitch));
    front.y = sin(glm::radians(this->m_pitch));
    front.z = sin(glm::radians(this->m_yaw)) * cos(glm::radians(this->m_pitch));

    this->m_front = glm::normalize(front);
    this->m_right = glm::normalize(glm::cross(this->m_front, this->m_world_up));
    this->m_up = glm::normalize(glm::cross(this->m_right, this->m_front));
}

#endif