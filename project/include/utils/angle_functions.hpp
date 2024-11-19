#pragma once
#include "entities/general_components.hpp"
#include <array>

float normalizeAngle(float angle);
float angularDifference(float a, float b);
float find_closest_angle(float angle);
Direction angleToDirection(float angle);