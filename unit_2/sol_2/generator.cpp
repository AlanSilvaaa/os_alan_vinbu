#include <iostream>
#include <pthread.h>
#include <queue>
#include <chrono>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "./modules.h"

#define NUM_THREADS   4
#define TOTAL_IMAGES  50
#define CYCLES        3   // how many one-second cycles

using namespace std;

struct img_data {
    int id;
    cv::Mat img;
};

struct Dimensions {
    int imageWidth;
    int imageHeight;
};

// Shared state
static queue<img_data>  q;
static pthread_mutex_t  queueMutex   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   queueCond    = PTHREAD_COND_INITIALIZER;
static bool             producerDone = false;
static bool             timedOut     = false;

// Random image generator
cv::Mat generateRandomImage(int w, int h) {
    cv::Mat img(h, w, CV_8UC3);
    cv::randu(img, cv::Scalar::all(0), cv::Scalar::all(255));
    return img;
}

// Producer: checks timedOut under lock
void* producer(void* arg) {
    Dimensions* dim = static_cast<Dimensions*>(arg);
    for (int i = 0; i < TOTAL_IMAGES; ++i) {
        pthread_mutex_lock(&queueMutex);
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        pthread_mutex_unlock(&queueMutex);

        cv::Mat img = generateRandomImage(dim->imageWidth, dim->imageHeight);
        img_data data{ i, img };

        pthread_mutex_lock(&queueMutex);
        // double-check in case timedOut flipped
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        q.push(data);
        cout << "[Producer] queued image " << (i+1)
             << ", queue size = " << q.size() << "\n";
        pthread_cond_signal(&queueCond);
        pthread_mutex_unlock(&queueMutex);
    }

    // signal consumers to exit
    pthread_mutex_lock(&queueMutex);
    producerDone = true;
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);
    return nullptr;
}

// Consumer: stops if queue empty and (producerDone || timedOut)
void* consumer(void* arg) {
    int tid = (int)(size_t)arg;
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

        string filename = "../out/random_image_" + to_string(item.id+1) + ".jpg";
        cv::imwrite(filename, item.img);
        cout << "[Consumer " << tid << "] saved " << filename
             << ", queue size = " << remaining << "\n";
    }
    return nullptr;
}

// clear queue under lock
static void clearQueue() {
    pthread_mutex_lock(&queueMutex);
    while (!q.empty()) q.pop();
    pthread_mutex_unlock(&queueMutex);
}

// run one 50-image burst with timeout
static void run_one_cycle(Dimensions* dim) {
    // reset state
    pthread_mutex_lock(&queueMutex);
    producerDone = false;
    timedOut     = false;
    while (!q.empty()) q.pop();
    pthread_mutex_unlock(&queueMutex);

    pthread_t threads[NUM_THREADS];
    pthread_create(&threads[0], nullptr, producer, dim);
    for (int i = 1; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], nullptr, consumer, (void*)(size_t)i);
    }

    // watch for 1 s timeout
    thread watchdog([&](){
        this_thread::sleep_for(chrono::seconds(1));
        pthread_mutex_lock(&queueMutex);
        timedOut     = true;
        producerDone = true;
        while (!q.empty()) q.pop();
        pthread_cond_broadcast(&queueCond);
        pthread_mutex_unlock(&queueMutex);
    });

    // join all worker threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // ensure that this cycle’s watchdog can’t bleed into the next one
    watchdog.join();
}


int main_generator() {
    Dimensions* dim = new Dimensions{1920, 1280};

    for (int cycle = 1; cycle <= CYCLES; ++cycle) {
        cout << "\n=== Cycle " << cycle << " start ===\n";
        run_one_cycle(dim);
        cout << "=== Cycle " << cycle << " end ===\n";
    }

    cout << "\nAll " << CYCLES << " cycles complete.\n";
    delete dim;
    return 0;
}

