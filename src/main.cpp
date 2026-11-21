#include "Renderer.h"

int main() 
{
    Renderer renderer(1920, 1080, 3, 15, 1024, "scenes/scene_new_materials.xml");
    renderer.Run();
    
    return 0;
}
