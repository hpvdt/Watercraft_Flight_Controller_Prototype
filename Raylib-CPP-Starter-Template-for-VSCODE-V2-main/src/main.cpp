#include <raylib.h>
#include "triangle.h"
//#include "../../firmware/src/main/ajsr04m_read2.h"

int main() 
{
    const Color darkGreen = {20, 160, 133, 255};
    
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "WSV");
    SetTargetFPS(60);

    Triangle triangle;
    
    while (!WindowShouldClose())
    {
        triangle.Update();
        
        BeginDrawing();
            ClearBackground(darkGreen);
            triangle.Draw();
        EndDrawing();
    }
    
    CloseWindow();
}
