#include "VideoBaseInput.h"

void VideoBaseInput::GetCurrentRectangle(unsigned int* x, unsigned int* y, unsigned int* width, unsigned int* height)
{
    if (x) {
        x = 0;
    }

    if (y) {
        y = 0;
    }

    if (width) {
        width = 0;
    }

    if (height) {
        height = 0;
    }
}

void VideoBaseInput::GetCurrentSize(unsigned int* width, unsigned int* height)
{
    if (width) {
        width = 0;
    }

    if (height) {
        height = 0;
    }
}
