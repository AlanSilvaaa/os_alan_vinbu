#include <iostream>
#include <pthread.h>
#include <queue>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "./modules.h"

#define NUM_THREADS 4
#define TOTAL_IMAGES 50

using namespace std;

struct img_data {
    int id;
    cv::Mat img;
};

struct Dimensions{
    int imageWidth;
    int imageHeight;
};

// Shared queue & sync primitives
queue<img_data> q;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueCond  = PTHREAD_COND_INITIALIZER;
bool producerDone = false;

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
    cv::Mat img(height, width, CV_8UC3);
    cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));
    return img;
}

// Producer: runs in thread 0
void* producer(void* arg) {
    Dimensions* dimensions = static_cast<Dimensions*>(arg);
    int w = dimensions->imageWidth;
    int h = dimensions->imageHeight;
    for (int i = 0; i < TOTAL_IMAGES; ++i) {
        cv::Mat img = generateRandomImage(w, h);
        img_data data;
        data.id = i;
        data.img = img;

        // push into queue
        pthread_mutex_lock(&queueMutex);
        q.push(data);
        cout << "[Producer] queued image " << i+1 
            << ", queue size = " << q.size() << "\n";
        pthread_cond_signal(&queueCond);
        pthread_mutex_unlock(&queueMutex);
    }

    // signal consumers to exit when queue is drained
    pthread_mutex_lock(&queueMutex);
    producerDone = true;
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);

    return nullptr;
}

// Consumer: threads 1..NUM_THREADS-1
void* consumer(void* arg) {
    int threadId = (int)(size_t)arg;
    while (true) {
        pthread_mutex_lock(&queueMutex);
        // wait until there's work or producer is done
        while (q.empty() && !producerDone) {
            pthread_cond_wait(&queueCond, &queueMutex);
        }

        // if no work & producer done => exit
        if (q.empty() && producerDone) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }

        // pop one image
        cv::Mat img = q.front().img;
        int id = q.front().id + 1; // 1-based index
        q.pop();
        int remaining = q.size();
        pthread_mutex_unlock(&queueMutex);

        string filename = "../out/random_image_" + to_string(id) + ".jpg";
        cv::imwrite(filename, img);
        cout << "[Consumer " << threadId << "] saved " 
            << filename << ", queue size = " << remaining << "\n";
    }

    return nullptr;
}

int main_generator() {
    pthread_t threads[NUM_THREADS];
    Dimensions* dimensions = new Dimensions{1920, 1280};

    auto start = std::chrono::high_resolution_clock::now();
    // start producer
    pthread_create(&threads[0], nullptr, producer, dimensions);
    // start consumers, pass them IDs 1..NUM_THREADS-1
    for (int i = 1; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], nullptr, consumer, (void*)(size_t)i);
    }

    // join all
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds." << std::endl << std::flush;
    delete dimensions;

    cout << "All done.\n";
    return 0;
}

