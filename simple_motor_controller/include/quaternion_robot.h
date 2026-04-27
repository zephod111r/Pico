#pragma once

#include "pico_robotics.h"

struct QuaternionInput {
    float w;
    float x;
    float y;
    float z;
};

void quaternion_to_movement(KitronikPicoRobotics &robot, const QuaternionInput &q);
