#pragma once
#include <raylib.h>

class Triangle
{
public:
    Triangle();
    void Update();
    void Draw() const;

private:
    float x;
    float y;
    float rectWidth;
    float rectHeight;
    float roll;     // rotation in degrees
    Vector2 origin; // origin for rotation
};
