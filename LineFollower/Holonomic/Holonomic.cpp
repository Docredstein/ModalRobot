#include "Holonomic.hpp"

namespace Holonomic
{
    constexpr float sqrt2 = 1.4142; // std::sqrtf(2);
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
    void normalize(float input[3])
    {
        float norme = 0;
        for (int i = 0; i < 3; i++)
        {
            norme += input[i] * input[i];
        }
        norme = std::sqrt(norme);
        if (norme >= 1 && norme > 0)
        {
            for (int i = 0; i < 3; i++)
            {
                input[i] = input[i] / norme;
            }
        }
    }
    void truncate(float input[3])
    {
        for (int i = 0; i < 3; i++)
        {
            if (input[i] > 1)
            {
                input[i] = 1;
            }
            if (input[i] < -1)
            {
                input[i] = -1;
            }
        }
    }
    // Input : [avant,droite,rotation] ; Output : [motor1,motor2,motor3]
    void scale(float input[3])
    {
        float maxi = 0;
        for (int i = 0; i < 3; i++)
        {
            if (std::abs(input[i]) > maxi)
            {
                maxi = std::abs(input[i]);
            }
        }
        if (maxi > 1 && maxi > 0)
        {
            for (int i = 0; i < 3; i++)
            {
                input[i] = input[i] / maxi;
            }
        }
    }
    void Convert(float input[3], float output[3],bool invert = false)
    {
        float avant[3] = {0, -1, 1};
        float droite[3] = {-1, 1 / sqrt2, 1 / sqrt2};
        float rotation[3] = {1, 1, 1};
        scale(input);
        float norme = 0;
        for (int i = 0; i < 3; i++)
        {
            output[i] = avant[i] * input[0] + droite[i] * input[1] + rotation[i] * input[2];
            if (invert) {
                output[i] = -output[i];
            }
            norme += output[i] * output[i];
        }
        scale(output);
        /*
        norme = std::sqrt(norme);
        if (norme>=1&&norme>0)  {
            for (int i =0;i<3;i++) {
                output[i] = output[i]/norme;
            }
        }*/
    }
}