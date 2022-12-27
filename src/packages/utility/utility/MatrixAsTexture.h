#pragma once

#include <SDL.h>

#include <opencv2/opencv.hpp>

namespace Utility {

void MatrixAsTexture(SDL_Renderer* renderer, cv::Mat* screenshot);

}
