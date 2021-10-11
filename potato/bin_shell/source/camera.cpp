// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "camera.h"

#include <glm/gtx/rotate_vector.hpp>

void up::Camera::lookAt(glm::vec3 pos, glm::vec3 at, glm::vec3 up) {
    _position = pos;
    _view = normalize(at - pos);
    _right = normalize(cross(_view, up));
    _up = cross(_right, _view);
    _matrix = lookAtRH(_position, at, _up);
}

void up::Camera::position(glm::vec3 pos) {
    _position = pos;
    _matrix = lookAtRH(_position, _position + _view, _up);
}

void up::Camera::rotateYaw(float radians) {
    _view = glm::rotate(_view, radians, _up);
    _right = glm::rotate(_right, radians, _up);
    _matrix = lookAtRH(_position, _position + _view, _up);
}

void up::Camera::rotatePitch(float radians) {
    _view = glm::rotate(_view, radians, _right);
    _up = glm::rotate(_up, radians, _right);
    _matrix = lookAtRH(_position, _position + _view, _up);
}
