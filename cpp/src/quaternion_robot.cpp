#include "quaternion_robot.h"

#include <cmath>

namespace {
float clamp_unit(float value) {
    if (value > 1.0f) {
        return 1.0f;
    }
    if (value < -1.0f) {
        return -1.0f;
    }
    return value;
}
}

void quaternion_to_movement(KitronikPicoRobotics &robot, const QuaternionInput &q) {
    const float max_speed = 100.0f;
    float w = q.w;
    float x = q.x;
    float y = q.y;
    float z = q.z;

    float norm = std::sqrt(w * w + x * x + y * y + z * z);
    if (norm == 0.0f) {
        norm = 1.0f;
    }

    w /= norm;
    x /= norm;
    y /= norm;
    z /= norm;

    float move_x = x;
    float move_z = z;
    float spin = y;

    float motor_1 = clamp_unit(-move_x + move_z - spin) * max_speed;
    float motor_2 = clamp_unit(move_x + move_z - spin) * max_speed;
    float motor_3 = clamp_unit(move_x - move_z - spin) * max_speed;
    float motor_4 = clamp_unit(-move_x - move_z - spin) * max_speed;

    auto set_motor = [&robot](int motor, float speed) {
        if (speed > 0) {
            robot.motor_on(motor, 'f', static_cast<int>(std::fabs(speed)));
        } else if (speed < 0) {
            robot.motor_on(motor, 'r', static_cast<int>(std::fabs(speed)));
        } else {
            robot.motor_on(motor, 'f', 0);
        }
    };

    set_motor(1, motor_1);
    set_motor(2, motor_2);
    set_motor(3, motor_3);
    set_motor(4, motor_4);
}
