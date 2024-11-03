#pragma once
#include "entities/general_components.hpp"
#include <array>

float normalizeAngle(float angle) {
    while (angle > M_PI) {
        angle -= 2.f * M_PI;
    }

    while (angle < -M_PI) {
        angle += 2.f * M_PI;
    }

    return angle;
}

float angularDifference(float a, float b) {
    float diff = normalizeAngle(a - b);
    return std::abs(diff);
}

float find_closest_angle(float dx, float dy) {
  float angle = atan2(dy, dx);

  constexpr std::array<float, 8> cardinalAngles = {
      0.f,                  // East
      -M_PI / 4,            // North-East
      -M_PI / 2,            // North
      -3 * M_PI / 4,        // North-West
      -M_PI,                // West
      3 * M_PI / 4,         // South-West
      M_PI / 2,             // South
      M_PI / 4              // South-East
  };

  float closestAngle = cardinalAngles[0];
  float smallestDifference = angularDifference(angle, closestAngle);

  for (const float& cardinalAngle : cardinalAngles) {
      float difference = angularDifference(angle, cardinalAngle);

      if (difference < smallestDifference) {
          smallestDifference = difference;
          closestAngle = cardinalAngle;
      }
  }

  return closestAngle;
}