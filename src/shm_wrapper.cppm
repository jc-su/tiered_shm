#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// Include your client and cxlmalloc headers
#include "client.h" 
#include "cxlmalloc.h"

// Abstract class for Shared Memory handling
class ShmWrapper {
public:
    virtual ~ShmWrapper() = default;

    // Method to allocate/get shared memory
    virtual void* get(size_t size) = 0;

    // Method to deallocate/release shared memory
    virtual void release(void* ptr) = 0;
};

// Implementation using LightningClient
class LightningShmWrapper : public ShmWrapper {
    LightningClient client;
    // Additional necessary members and methods

public:
    LightningShmWrapper(const std::string& path, const std::string& password) : client(path, password) {
        // Additional initialization if necessary
    }

    void* get(size_t size) override {
        // Implementation using LightningClient
        uint8_t *ptr;
        int status = client.Create(0, &ptr, size);  // Example implementation
        return ptr;
    }

    void release(void* ptr) override {
        // Implementation for releasing memory
        // This will depend on how LightningClient handles memory release
    }
};

// Implementation using POSIX shared memory API
class PosixShmWrapper : public ShmWrapper {
    int shm_id;

public:
    PosixShmWrapper(int key, size_t size) {
        shm_id = shmget(key, size, IPC_CREAT | 0664);
        if (shm_id == -1) {
            // Handle error
            perror("shmget failed");
            exit(EXIT_FAILURE);
        }
    }

    void* get(size_t size) override {
        void* ptr = shmat(shm_id, NULL, 0);
        if (ptr == (void*)-1) {
            // Handle error
            perror("shmat failed");
            exit(EXIT_FAILURE);
        }
        return ptr;
    }

    void release(void* ptr) override {
        if (shmdt(ptr) == -1) {
            // Handle error
            perror("shmdt failed");
            exit(EXIT_FAILURE);
        }
    }

    ~PosixShmWrapper() {
        shmctl(shm_id, IPC_RMID, NULL); // Clean up shared memory
    }
};

// Implementation using cxl_shm
class CxlShmWrapper : public ShmWrapper {
    cxl_shm* shm;
    size_t length;
    int shm_id;

public:
    CxlShmWrapper(size_t length, int shm_id) : length(length), shm_id(shm_id) {
        shm = new cxl_shm(length, shm_id);
        // Additional initialization if required
    }

    ~CxlShmWrapper() {
        delete shm;
    }

    void* get(size_t size) override {
        // Implementation using cxl_shm
        return shm->cxl_malloc(size, 0); // Adjust parameters as necessary
    }

    void release(void* ptr) override {
        // Implementation for releasing memory
        shm->cxl_free(ptr); // Adjust this line according to the actual API
    }
};

// Main application for demonstration
int main() {
    ShmWrapper* shmWrapper;

    // Initialize the appropriate wrapper based on your needs
    // shmWrapper = new LightningShmWrapper("/tmp/lightning", "password");
    // OR
    // shmWrapper = new PosixShmWrapper(100, 1024);
    // OR
    // shmWrapper = new CxlShmWrapper(1024, 100);

    void* sharedMemory = shmWrapper->get(1024);
    // Use shared memory here

    shmWrapper->release(sharedMemory);

    delete shmWrapper;
    return 0;
}
