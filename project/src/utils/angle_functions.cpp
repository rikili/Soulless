#include "utils/angle_functions.hpp"

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

float find_closest_angle(float angle) {
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

Direction angleToDirection(float angle) {
    if (angle == 0.f) {
        return Direction::E;
    }
    else if (angle == -M_PI / 4) {
        return Direction::NE;
    }
    else if (angle == -M_PI / 2) {
        return Direction::N;
    }
    else if (angle == -3 * M_PI / 4) {
        return Direction::NW;
    }
    else if (angle == -M_PI) {
        return Direction::W;
    }
    else if (angle == 3 * M_PI / 4) {
        return Direction::SW;
    }
    else if (angle == M_PI / 2) {
        return Direction::S;
    }
    else if (angle == M_PI / 4) {
        return Direction::SE;
    }
    return Direction::E; // placeholder
}