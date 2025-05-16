#pragma once
#include <cmath>
namespace Holonomic {
    // Input : [avant,droite,rotation] ; Output : [motor1,motor2,motor3]
    void Convert(float input[3], float output[3],bool invert);
    
}
