#pragma once

#include <SDL.h>

#include <opencv2/opencv.hpp>

namespace Utility {

void matrixAsTexture(SDL_Renderer* renderer, cv::Mat* screenshot);

}
