#include "Holonomic.hpp"


namespace Holonomic {
    constexpr float sqrt2 = 1.4142;//std::sqrtf(2);
    /* Motor are set to turn positively in anti trigonometric direction
            1
           * *
          *   *
         *     *
        2 * * * 3 
            |
            |
            V
    */
    void Convert(float input[3], float  output[3]) {
        float avant[3] = {0,-1,1};
        float droite[3] = {1,1/sqrt2,1/sqrt2};
        float rotation[3] = {1,1,1};
        float norme = 0;
        for (int i = 0;i<3;i++) {
            output[i]=avant[i]*input[0]+droite[i]*input[1]+rotation[i]*input[2];
            norme += output[i]*output[i];
        }
        
        norme = std::sqrtf(norme);
        if (norme>=1&&norme>0)  {
            for (int i =0;i<3;i++) {
                output[i] = output[i]/norme;
            }
        }
    }
}