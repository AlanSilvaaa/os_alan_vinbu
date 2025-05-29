#include <iostream>
#include <pthread.h>
#include <queue>
#include <chrono>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "./modules.h"

using namespace std;

struct img_data {
    int id;
    cv::Mat img;
};

struct Requirements {
    int imageWidth;
    int imageHeight;
    int frames;
    int num_threads;
    int duration_minutes;
};

struct Consumer_Args {
    int thread_id;
    Requirements* req;
};

// Shared state
static queue<img_data> q;
static pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;
static bool producerDone = false;
static bool timedOut = false;

// Random image generator
cv::Mat generateRandomImage(int w, int h) {
    cv::Mat img(h, w, CV_8UC3);
    cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));
    return img;
}

// Producer
void* producer(void* arg) {
    Requirements* req = static_cast<Requirements*>(arg);
    const double fps = req -> frames;
    const auto framePeriod = std::chrono::duration<double>(1.0 / fps);
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = startTime + std::chrono::minutes(req->duration_minutes);

    int frame_id = 0;

    while (std::chrono::high_resolution_clock::now() < endTime) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        pthread_mutex_lock(&queueMutex);
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        pthread_mutex_unlock(&queueMutex);

        // Generate image
        cv::Mat img = generateRandomImage(req->imageWidth, req->imageHeight);
        img_data data{ frame_id++, img };

        // Measure time taken
        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = frameEnd - frameStart;

        if (elapsed < framePeriod) {
            std::this_thread::sleep_for(framePeriod - elapsed);
        }

        // Push to queue
        pthread_mutex_lock(&queueMutex);
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        q.push(data);
        std::cout << "[Producer] queued image " << frame_id
                  << ", queue size = " << q.size() << "\n";
        pthread_cond_signal(&queueCond);
        pthread_mutex_unlock(&queueMutex);
    }

    pthread_mutex_lock(&queueMutex);
    producerDone = true;
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);
    return nullptr;
}

// Consumer
void* consumer(void* arg) {
    Consumer_Args* cargs = static_cast<Consumer_Args*>(arg);
    Requirements* req = cargs->req;
    int tid = cargs->thread_id;

    while (true) {
        pthread_mutex_lock(&queueMutex);
        while (q.empty() && !producerDone && !timedOut) {
            pthread_cond_wait(&queueCond, &queueMutex);
        }
        if (q.empty() && (producerDone || timedOut)) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        img_data item = q.front();
        q.pop();
        int remaining = q.size();
        pthread_mutex_unlock(&queueMutex);

        string filename = "../out/random_image_" + to_string(item.id) + ".jpg";
        cv::imwrite(filename, item.img);
        cout << "[Consumer " << tid << "] saved " << filename
             << ", queue size = " << remaining << "\n";
    }
    return nullptr;
}

// Main function
int main_generator(int frames, int minutes, int num_threads) {
    Requirements* req = new Requirements{1280, 720, frames, num_threads, minutes};
    Consumer_Args* args = new Consumer_Args[num_threads];

    // Reset state
    pthread_mutex_lock(&queueMutex);
    producerDone = false;
    timedOut = false;
    while (!q.empty()) q.pop();
    pthread_mutex_unlock(&queueMutex);

    // Create threads
    pthread_t threads[num_threads];
    pthread_create(&threads[0], nullptr, producer, req);

    for (int i = 1; i < num_threads; ++i) {
        args[i].thread_id = i;
        args[i].req = req;
        pthread_create(&threads[i], nullptr, consumer, (void*)&args[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    delete[] args;
    delete req;
    std::cout << "\nCaptura finalizada después de " << minutes << " minutos.\n";
    return 0;
}
