#include <raylib.h>
#include <cstdio>
#include "triangle.h"

Triangle::Triangle()
    : x(400), y(300), rectWidth(100), rectHeight(150), roll(0.0f)
{
    origin = { rectWidth / 2, rectHeight / 2 };  // Center of the rectangle
}

void Triangle::Update()
{
    if (IsKeyDown(KEY_W)) y -= 5;
    if (IsKeyDown(KEY_S)) y += 5;
    if (IsKeyDown(KEY_A)) x -= 5;
    if (IsKeyDown(KEY_D)) x += 5;
    if (IsKeyDown(KEY_Q)) roll -= 2.0f;
    if (IsKeyDown(KEY_E)) roll += 2.0f;
}

void Triangle::Draw() const
{
    Rectangle rec = { x, y, rectWidth, rectHeight };
    DrawRectanglePro(rec, origin, roll, WHITE);

    char positionText[64];
    sprintf(positionText, "x: %.0f, y: %.0f \n roll: %.0f", x, y,roll);  // Use this->x and this->y if needed

    int fontSize = 20;
    int textWidth = MeasureText(positionText, fontSize);
    int margin = 10;

    DrawText(positionText, GetScreenWidth() - textWidth - margin, margin, fontSize, WHITE);


}
