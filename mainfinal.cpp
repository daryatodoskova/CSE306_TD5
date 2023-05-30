#include <iostream>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.h"

std::vector<unsigned char> calculateEnergy(const std::vector<unsigned char>& intensity, int width, int height) {
    int energy;
    int temp;
    int size = width * height;
    std::vector<unsigned char> energyMap(size, 0);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            temp = 0;
            energy = 0;
            if (j + 1 < width) {
                temp += intensity[i * width + j + 1];
            }
            if (j - 1 >= 0) {
                temp -= intensity[i * width + j - 1];
            }
            energy += std::abs(temp);

            temp = 0;
            if (i + 1 < height) {
                temp += intensity[(i + 1) * width + j];
            }
            if (i - 1 >= 0) {
                temp -= intensity[(i - 1) * width + j];
            }
            energy += std::abs(temp);

            energyMap[i * width + j] = energy;
        }
    }

    return energyMap;
}

std::vector<unsigned char> calculateCumulative(const std::vector<unsigned char>& energyMap, int width, int height) {
    int size = width * height;
    std::vector<unsigned char> cumulativeMap(size, 0);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cumulativeMap[i * width + j] = energyMap[i * width + j];
            if (j + 1 < width && i - 1 >= 0 && j - 1 >= 0) {
                cumulativeMap[i * width + j] += std::min(
                    std::min(energyMap[(i - 1) * width + j + 1], energyMap[(i - 1) * width + j]),
                    energyMap[(i - 1) * width + j - 1]
                );
            }
        }
    }

    return cumulativeMap;
}

std::vector<unsigned char> calculateIntensity(const unsigned char* image, int width, int height) {
    int size = width * height;
    std::vector<unsigned char> intensity(size);

    for (int i = 0; i < size; i++) {
        intensity[i] = image[i * 3] + image[i * 3 + 1] + image[i * 3 + 2];
    }

    return intensity;
}

void translatePixels(unsigned char* image, int width, int col, int row) {
    for (int i = col; i < width - 1; i++) {
        image[(row * width + i) * 3] = image[(row * width + i + 1) * 3];
        image[(row * width + i) * 3 + 1] = image[(row * width + i + 1) * 3 + 1];
        image[(row * width + i) * 3 + 2] = image[(row * width + i + 1) * 3 + 2];
    }
}


std::vector<unsigned char> cropImage(const unsigned char* image, const std::vector<int>& seam,
                                     int width, int height) {
    int newWidth = width - 1;
    int size = newWidth * height * 3;
    std::vector<unsigned char> croppedImage(size, 0);

    for (int i = 0; i < height; i++) {
        int seamCol = seam[i];
        for (int j = 0; j < newWidth; j++) {
            if (j >= seamCol) {
                croppedImage[(i * newWidth + j) * 3] = image[(i * width + j + 1) * 3];
                croppedImage[(i * newWidth + j) * 3 + 1] = image[(i * width + j + 1) * 3 + 1];
                croppedImage[(i * newWidth + j) * 3 + 2] = image[(i * width + j + 1) * 3 + 2];
            }
            else {
                croppedImage[(i * newWidth + j) * 3] = image[(i * width + j) * 3];
                croppedImage[(i * newWidth + j) * 3 + 1] = image[(i * width + j) * 3 + 1];
                croppedImage[(i * newWidth + j) * 3 + 2] = image[(i * width + j) * 3 + 2];
            }
        }
    }

    return croppedImage;
}

int main() {
    int width, height, channels;
    unsigned char* image = stbi_load("original.jpg", &width, &height, &channels, 0);

    std::vector<unsigned char> intensity = calculateIntensity(image, width, height);
    std::vector<unsigned char> energyMap = calculateEnergy(intensity, width, height);

    int newWidth = width - 40;

    for (int k = 0; k < 40; k++) {
        std::vector<unsigned char> cumulativeMap = calculateCumulative(energyMap, width, height);

        double minEnergy = std::numeric_limits<double>::max();
        int minEnergyIndex = -1;

        for (int i = 0; i < width; i++) {
            if (cumulativeMap[(height - 1) * width + i] < minEnergy) {
                minEnergy = cumulativeMap[(height - 1) * width + i];
                minEnergyIndex = i;
            }
        }
        

        std::vector<int> seam(height, 0);
        seam[height - 1] = minEnergyIndex;

        int col = minEnergyIndex;
        for (int i = height - 2; i >= 0; i--) {
            int minVal = std::numeric_limits<int>::max();
            int minCol = -1;

            if (col - 1 >= 0 && cumulativeMap[i * width + col - 1] < minVal) {
                minVal = cumulativeMap[i * width + col - 1];
                minCol = col - 1;
            }

            if (cumulativeMap[i * width + col] < minVal) {
                minVal = cumulativeMap[i * width + col];
                minCol = col;
            }

            if (col + 1 < width && cumulativeMap[i * width + col + 1] < minVal) {
                minVal = cumulativeMap[i * width + col + 1];
                minCol = col + 1;
            }

            col = minCol;
            seam[i] = col;
            translatePixels(image, width, col, i);
        }

        std::vector<unsigned char> croppedImage = cropImage(image, seam, width, height);
        image = croppedImage.data();
        width = newWidth;
    }

    stbi_write_png("result.png", width, height, channels, image, 0);

    return 0;
}
