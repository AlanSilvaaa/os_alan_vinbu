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

// Counters & mutex for logging
static int imagesGenerated = 0;
static int imagesSaved = 0;
static pthread_mutex_t countMutex = PTHREAD_MUTEX_INITIALIZER;

// Random image generator
cv::Mat generateRandomImage(int w, int h) {
    cv::Mat img(h, w, CV_8UC3);
    cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));
    return img;
}

// Logger thread: prints counts every second
void* logger(void* arg) {
    using namespace std::chrono;
    while (true) {
        this_thread::sleep_for(seconds(1));

        // Check if done
        pthread_mutex_lock(&queueMutex);
        bool done = producerDone && q.empty();
        pthread_mutex_unlock(&queueMutex);

        // Print and reset counts
        pthread_mutex_lock(&countMutex);
        cout << "[Logger] Generated: " << imagesGenerated
             << ", Saved: " << imagesSaved << " images in last second\n";
        imagesGenerated = 0;
        imagesSaved = 0;
        pthread_mutex_unlock(&countMutex);

        if (done) break;
    }
    return nullptr;
}

// Producer thread
void* producer(void* arg) {
    Requirements* req = static_cast<Requirements*>(arg);
    const double fps = req->frames;
    const auto framePeriod = chrono::duration<double>(1.0 / fps);
    auto startTime = chrono::high_resolution_clock::now();
    auto endTime = startTime + chrono::minutes(req->duration_minutes);
    // auto endTime = startTime + chrono::seconds(10); // For testing, use a shorter duration. Comment the above line and uncomment this one.

    int frame_id = 0;

    while (chrono::high_resolution_clock::now() < endTime) {
        auto frameStart = chrono::high_resolution_clock::now();

        pthread_mutex_lock(&queueMutex);
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        pthread_mutex_unlock(&queueMutex);

        // Generate image
        cv::Mat img = generateRandomImage(req->imageWidth, req->imageHeight);
        img_data data{ frame_id, img };
        frame_id++;

        // Ensure constant frame rate
        auto frameEnd = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = frameEnd - frameStart;
        if (elapsed < framePeriod) {
            this_thread::sleep_for(framePeriod - elapsed);
        }

        // Push to queue and count
        pthread_mutex_lock(&queueMutex);
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        q.push(data);
        pthread_mutex_unlock(&queueMutex);

        pthread_mutex_lock(&countMutex);
        imagesGenerated++;
        pthread_mutex_unlock(&countMutex);

        // cout << "[Producer] queued image " << data.id
        //      << ", queue size = " << q.size() << "\n";
        pthread_cond_signal(&queueCond);
    }

    pthread_mutex_lock(&queueMutex);
    producerDone = true;
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);
    return nullptr;
}

// Consumer thread
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

        // Save image
        string filename = "../out/random_image_" + to_string(item.id) + ".jpg";
        cv::imwrite(filename, item.img);

        pthread_mutex_lock(&countMutex);
        imagesSaved++;
        pthread_mutex_unlock(&countMutex);

        // cout << "[Consumer " << tid << "] saved " << filename
        //      << ", queue size = " << remaining << "\n";
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
        pthread_create(&threads[i], nullptr, consumer, &args[i]);
    }

    // Start logger
    pthread_t loggerThread;
    pthread_create(&loggerThread, nullptr, logger, nullptr);

    // Wait for threads to finish
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // Wait for logger
    pthread_join(loggerThread, nullptr);

    delete[] args;
    delete req;
    cout << "\nCaptura finalizada después de " << minutes << " minutos.\n";
    return 0;
}

int main_generator(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <fps> <minutes> <num_threads>\n";
        return 1;
    }
    int fps = stoi(argv[1]);
    int minutes = stoi(argv[2]);
    int num_threads = stoi(argv[3]);
    return main_generator(fps, minutes, num_threads);
}

