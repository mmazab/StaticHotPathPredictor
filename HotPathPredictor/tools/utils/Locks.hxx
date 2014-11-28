#include <pthread.h>
#include <string.h>

namespace Locks {
    class Mutex {
    private:
        pthread_mutex_t m;

        Mutex(const Mutex &mutex) {}

        Mutex &operator=(const Mutex &mutex) {return *this;}

        void init() {
            pthread_mutexattr_t mutex_attr;
            pthread_mutexattr_init(&mutex_attr);
            pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);

            int result = pthread_mutex_init(&this->m, &mutex_attr);
            if (result != 0) {
                fprintf(stderr, "Unable to create lock\n");
                abort();
            }
        }

    public:
        Mutex() {
            init();
        }
        
        ~Mutex() {
            pthread_mutex_destroy(&this->m);
        }

        void reinit() {
            pthread_mutex_destroy(&this->m);
            init();
        }

        void lock() {
            pthread_mutex_lock(&this->m);
        }

        void unlock() {
            pthread_mutex_unlock(&this->m);
        }

     
    };

    class ReadWriteMutex {
    private:
        pthread_rwlock_t rw;

        ReadWriteMutex(const ReadWriteMutex &mutex) {}

        ReadWriteMutex &operator=(const ReadWriteMutex &mutex) {return *this;}

        void init() {
            pthread_rwlockattr_t rwlock_attr;
            pthread_rwlockattr_init(&rwlock_attr);

            int result = pthread_rwlock_init(&this->rw, &rwlock_attr);
            if (result != 0) {
                fprintf(stderr, "Unable to create rwlock\n");
                abort();
            }
        }
        
    public:
        ReadWriteMutex() {
            init();
        }
        
        ~ReadWriteMutex() {
            pthread_rwlock_destroy(&this->rw);
        }

        void reinit() {
            pthread_rwlock_destroy(&this->rw);
            init();
        }

        void readLock() {
            int result = pthread_rwlock_rdlock(&this->rw);
            if (result != 0) {
                fprintf(stderr, "Unable to acquire read lock: [%d] %s\n", result, strerror(result));
                abort();
            }
        }

        void writeLock() {
            int result = pthread_rwlock_wrlock(&this->rw);
            if (result != 0) {
                fprintf(stderr, "Unable to acquire write lock: [%d] %s\n", result, strerror(result));
                abort();
            }
        }
        
        void unlock() {
            int result = pthread_rwlock_unlock(&this->rw);
            if (result != 0) {
                fprintf(stderr, "Unable to release lock: [%d] %s\n", result, strerror(result));
                abort();
            }
        }
    };
}
