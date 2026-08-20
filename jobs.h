#ifndef CARP_JOBS_H
#define CARP_JOBS_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// --- Job Definition ---
typedef void (*job_work_fn)(void *data);

typedef struct job_handle {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int32_t count;
} job_handle_t;

typedef struct {
    job_work_fn work;
    void *data;
    job_handle_t *handle;
} job_t;

// --- Thread-Safe Queue ---
typedef struct {
    job_t *jobs;
    int capacity;
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} job_queue_t;

static inline job_queue_t* job_queue_create(int capacity) {
    job_queue_t *q = (job_queue_t*)malloc(sizeof(job_queue_t));
    q->jobs = (job_t*)malloc(capacity * sizeof(job_t));
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
}

static inline void job_queue_destroy(job_queue_t *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
    free(q->jobs);
    free(q);
}

static inline void job_queue_push(job_queue_t *q, job_t job) {
    pthread_mutex_lock(&q->mutex);
    while (q->count >= q->capacity) {
        int new_capacity = q->capacity * 2;
        job_t *new_jobs = (job_t*)malloc(new_capacity * sizeof(job_t));
        for (int i = 0; i < q->count; i++) {
            new_jobs[i] = q->jobs[(q->head + i) % q->capacity];
        }
        free(q->jobs);
        q->jobs = new_jobs;
        q->head = 0;
        q->tail = q->count;
        q->capacity = new_capacity;
    }
    q->jobs[q->tail] = job;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

static inline job_t job_queue_pop(job_queue_t *q) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    job_t job = q->jobs[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    pthread_mutex_unlock(&q->mutex);
    return job;
}

// --- Thread Pool ---
typedef struct {
    pthread_t *threads;
    int num_threads;
    job_queue_t *queue;
} thread_pool_t;

static void* worker_thread_fn(void *arg) {
    thread_pool_t *pool = (thread_pool_t*)arg;
    while (1) {
        job_t job = job_queue_pop(pool->queue);
        
        // Tombstone check: work == NULL signals thread termination
        if (job.work == NULL) {
            break;
        }
        
        job.work(job.data);
        
        if (job.handle) {
            pthread_mutex_lock(&job.handle->mutex);
            job.handle->count--;
            if (job.handle->count <= 0) {
                pthread_cond_broadcast(&job.handle->cond);
            }
            pthread_mutex_unlock(&job.handle->mutex);
        }
    }
    return NULL;
}

static inline thread_pool_t* thread_pool_create(int num_threads) {
    thread_pool_t *pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    pool->num_threads = num_threads;
    pool->queue = job_queue_create(128);
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread_fn, pool);
    }
    return pool;
}

static inline void thread_pool_destroy(thread_pool_t *pool) {
    // Push exactly one tombstone per thread
    for (int i = 0; i < pool->num_threads; i++) {
        job_t tombstone = { .work = NULL, .data = NULL, .handle = NULL };
        job_queue_push(pool->queue, tombstone);
    }
    
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    job_queue_destroy(pool->queue);
    free(pool->threads);
    free(pool);
}

// --- Job Handle / Counter ---
static inline job_handle_t* job_handle_create(int count) {
    job_handle_t *h = (job_handle_t*)malloc(sizeof(job_handle_t));
    pthread_mutex_init(&h->mutex, NULL);
    pthread_cond_init(&h->cond, NULL);
    h->count = count;
    return h;
}

static inline bool job_handle_complete(job_handle_t *h) {
    if (h == NULL) return true;
    pthread_mutex_lock(&h->mutex);
    bool done = (h->count <= 0);
    pthread_mutex_unlock(&h->mutex);
    return done;
}

static inline void job_handle_wait(job_handle_t *h) {
    if (h == NULL) return;
    pthread_mutex_lock(&h->mutex);
    while (h->count > 0) {
        pthread_cond_wait(&h->cond, &h->mutex);
    }
    pthread_mutex_unlock(&h->mutex);
}

static inline void job_handle_destroy(job_handle_t *h) {
    if (h == NULL) return;
    pthread_mutex_destroy(&h->mutex);
    pthread_cond_destroy(&h->cond);
    free(h);
}

#endif
