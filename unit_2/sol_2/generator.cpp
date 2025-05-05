#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/**
 * @brief Generates a random color image of the specified dimensions.
 *
 * Creates an 8-bit, 3-channel (BGR format by default in OpenCV) image
 * and fills each pixel's channels with random values between 0 and 255.
 *
 * @param width The desired width of the image in pixels. Must be positive.
 * @param height The desired height of the image in pixels. Must be positive.
 * @return cv::Mat An OpenCV matrix representing the generated random image.
 * Returns an empty cv::Mat if width or height are not positive.
 */
cv::Mat generateRandomImage(int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Image dimensions must be positive." << std::endl;
        return cv::Mat(); // Return an empty Mat to indicate failure
    }

    // Create an 8-bit 3-channel BGR image (Height x Width)
    cv::Mat randomImage(height, width, CV_8UC3);

    // Fill the image with random values (0-255 for each channel)
    cv::randu(randomImage, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));

    return randomImage;
}

int main(int argc, char **argv) {
    int imageWidth = 1920;
    int imageHeight = 1280;

    std::cout << "Generating a " << imageWidth << "x" << imageHeight
        << " random image..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 50; ++i) {
        cv::Mat myRandomImage = generateRandomImage(imageWidth, imageHeight);

        if (!myRandomImage.empty()) {
            std::cout
                << "Random image generated successfully (stored in myRandomImage)."
                << std::endl;
            std::string filename = "../out/random_image_" + std::to_string(i + 1) + ".jpg";
            cv::imwrite(filename, myRandomImage); // Save the image to a file
                                                                   // Display the image and wait for enter
                                                                   // cv::imshow("Random Image", myRandomImage);
                                                                   // cv::waitKey(0); // Wait for a key press indefinitely
        } else {
            std::cerr << "Failed to generate the random image." << std::endl;
            return 1;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds." << std::endl;
    return 0;
}
