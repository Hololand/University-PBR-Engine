#include "Engine.h"

int main(void)
{
    try 
    {
        // Create engine with name, resolution and boolean value for window resize mode
        Engine engine("3D Graphics Engine", 1280, 720, true);
        // Main render loop
        while (!engine.getWindowShouldClose())
        {
            engine.update();
            engine.render();
        }
        return 0;
    }
    catch (...)
    {
        std::cout << "Its all broken" << std::endl;
    }
}
    