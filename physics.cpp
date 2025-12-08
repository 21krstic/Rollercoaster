#pragma once

float computeAcceleration(float slope) {
    if (slope < 0) return 0.5f;  // nizbrdo
    if (slope > 0) return -0.3f; // uzbrdo
    return 0.0f; // ravno
}
