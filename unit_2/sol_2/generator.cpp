#include <iostream>
#include <pthread.h>
#include <queue>
#include <chrono>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "./modules.h"

#define CYCLES        3   // how many one-second cycles

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
    int cycle;
};

struct Consumer_Args
{
    int thread_id;
    Requirements* req;
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
    Requirements* req = static_cast<Requirements*>(arg);

    // target frame interval = 1/50 sec = 0.02 sec
    const auto framePeriod = std::chrono::duration<double>(1.0 / 50.0);

    for (int i = 0; i < req->frames; ++i) {
        // mark start of this frame
        auto start = std::chrono::high_resolution_clock::now();

        // quick timedOut check
        pthread_mutex_lock(&queueMutex);
        if (timedOut) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        pthread_mutex_unlock(&queueMutex);

        // generate the image
        cv::Mat img = generateRandomImage(req->imageWidth, req->imageHeight);
        img_data data{ i, img };

        // figure out how long generation + queueing took
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // sleep for the remainder of the 20 ms slot
        if (elapsed < framePeriod) {
            cout << "time elapsed for frame " << (i+1) << ": "
                 << elapsed.count() << " seconds\n";
            cout << "sleeping for " << (framePeriod - elapsed).count() << " seconds\n";
            std::this_thread::sleep_for(framePeriod - elapsed);
        }

        // push into the queue
        pthread_mutex_lock(&queueMutex);
        if (timedOut) {  // double-check before pushing
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        q.push(data);
        std::cout << "[Producer] queued image " << (i+1)
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

        string filename = "../out/random_image_cycle" + to_string(req->cycle) + "_" + to_string(item.id+1) + ".jpg";
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
static void run_one_cycle(Requirements* req, int cycle) {
    req->cycle = cycle;
    Consumer_Args* args = new Consumer_Args[req->num_threads];
    // reset state
    pthread_mutex_lock(&queueMutex);
    producerDone = false;
    timedOut     = false;
    while (!q.empty()) q.pop();
    pthread_mutex_unlock(&queueMutex);

    pthread_t threads[req->num_threads];
    pthread_create(&threads[0], nullptr, producer, req);
    for (int i = 1; i < req->num_threads; ++i) {
        args[i].thread_id = i;
        args[i].req = req;
        pthread_create(&threads[i], nullptr, consumer, (void*)&args[i]);
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
    for (int i = 0; i < req->num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // ensure that this cycle’s watchdog can’t bleed into the next one
    watchdog.join();
}


int main_generator(int frames, int minutes, int num_threads) {
    Requirements* req = new Requirements{1280, 720, frames, num_threads};

    for (int cycle = 1; cycle <= CYCLES; ++cycle) {
        cout << "\n=== Cycle " << cycle << " start ===\n";
        run_one_cycle(req, cycle);
        cout << "=== Cycle " << cycle << " end ===\n";
    }

    cout << "\nAll " << CYCLES << " cycles complete.\n";
    delete req;
    return 0;
}

