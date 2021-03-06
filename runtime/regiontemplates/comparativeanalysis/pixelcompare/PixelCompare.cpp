//
// Created by taveira on 11/25/15.
//

#include "PixelCompare.h"

PixelCompare::PixelCompare(DenseDataRegion2D *dr1, DenseDataRegion2D *dr2, float *diffPixels) {
    this->dr1 = dr1;
    this->dr2 = dr2;
    this->diff = diffPixels;
}

bool PixelCompare::run(int procType, int tid) {
    uint64_t t1 = Util::ClockGetTimeProfile();
    float compId = diff[0];
    cv::Mat image1 = this->dr1->getData();
    cv::Mat image2 = this->dr2->getData();
    /*std::cout << "dr1->id: "<< dr1->getId() << " version: " << dr1->getVersion() << std::endl;
    std::cout << "dr2->id: "<< dr2->getId() << " version: " << dr2->getVersion() << std::endl;*/

    int diffPixels = 0;
    if (image1.rows == image2.rows && image1.cols == image2.cols && image1.rows > 0 && image1.cols > 0) {
        diffPixels = countNonZero(image1 != image2);
    } else {
        diffPixels = max(image1.rows * image1.cols, image2.rows * image2.cols);
    }
    int nonZeroRef = countNonZero(image2);

    std::cout << "diffPixels: " << diffPixels << " nonZeroRef: " << nonZeroRef << std::endl;
    this->diff[0] = (float) diffPixels;
    this->diff[1] = (float) nonZeroRef;
    std::cout << "CompId: " << compId << " diffPixels: " << this->diff[0] << " reference: " << this->diff[1] <<
    std::endl;

    uint64_t t2 = Util::ClockGetTimeProfile();

    std::cout << "Task Diff Computation time elapsed: " << t2 - t1 << std::endl;
}
