#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include "client.h"
#include "cxlmalloc.h"

// Abstract class
class ShmWrapper {
 public:
    virtual ~ShmWrapper() = default;

    virtual void* get(size_t size) = 0;
    virtual void put(uint64_t object_id, const void* data, size_t size) = 0;
    virtual void release(void* ptr) = 0;
};

class LightningShmWrapper : public ShmWrapper {
    LightningClient client;
    std::vector<uint64_t> allocatedObjects;

 public:
    LightningShmWrapper(const std::string& store_socket, const std::string& password)
            : client(store_socket, password) {}

    ~LightningShmWrapper() override {
        // for (auto objId : allocatedObjects) { client.Release(objId); }
    }

    LightningShmWrapper(const LightningShmWrapper&) = default;
    LightningShmWrapper& operator=(const LightningShmWrapper&) = default;
    LightningShmWrapper(LightningShmWrapper&&) = default;
    LightningShmWrapper& operator=(LightningShmWrapper&&) = default;

    void* get(size_t size) override {
        uint8_t* ptr = nullptr;
        uint64_t objectId = generateUniqueId();
        if (client.Create(objectId, &ptr, size) != 0) {
            throw std::runtime_error("LightningClient Create failed");
        }
        allocatedObjects.push_back(objectId);
        return ptr;
    }

    void put(uint64_t object_id, const void* data, size_t size) override {
        client.MultiPut(object_id, {/* fields */}, {static_cast<int64_t>(size)},
                                        {static_cast<uint8_t*>(const_cast<void*>(data))});
    }

    //   void release(void* ptr) override {
    //     uint64_t objectId = findObjectId(ptr);
    //     client.Release(objectId);
    //   }

 private:
    uint64_t generateUniqueId() {
        static uint64_t id = 0;
        return ++id;
    }
};

class CxlShmWrapper : public ShmWrapper {
    std::unique_ptr<cxl_shm> shm;
    size_t length;
    int shm_id;

 public:
    CxlShmWrapper(size_t length, int shm_id)
            : shm(std::make_unique<cxl_shm>(length, shm_id)), length(length), shm_id(shm_id) {
        shm->thread_init();
    }

    void* get(size_t size) override {
        CXLRef ref = shm->cxl_malloc(size, 0);
        return ref.get_addr();
    }

    void put(uint64_t object_id, const void* data, size_t size) override {
        CXLRef ref = shm->get_ref(object_id);
        std::memcpy(ref.get_addr(), data, size);
    }

    void release(void* ptr) override {
        cxl_block* block = static_cast<cxl_block*>(ptr);
        shm->cxl_free(false, block);
    }
};

class PosixShmWrapper : public ShmWrapper {
    int shm_id;

 public:
    PosixShmWrapper(int key, size_t size) : shm_id(shmget(key, size, IPC_CREAT | 0664)) {
        if (shm_id == -1) { throw std::runtime_error("shmget failed"); }
    }

    ~PosixShmWrapper() override { shmctl(shm_id, IPC_RMID, nullptr); }

    PosixShmWrapper(const PosixShmWrapper&) = default;
    PosixShmWrapper& operator=(const PosixShmWrapper&) = default;
    PosixShmWrapper(PosixShmWrapper&&) = default;
    PosixShmWrapper& operator=(PosixShmWrapper&&) = default;

    void* get(size_t size) override {
        void* ptr = shmat(shm_id, nullptr, 0);
        if (ptr == reinterpret_cast<void*>(-1)) { throw std::runtime_error("shmat failed"); }
        return ptr;
    }

    void put(uint64_t, const void* data, size_t size) override {
        void* ptr = shmat(shm_id, nullptr, 0);
        if (ptr == reinterpret_cast<void*>(-1)) { throw std::runtime_error("shmat failed"); }
        std::memcpy(ptr, data, size);
        if (shmdt(ptr) == -1) { throw std::runtime_error("shmdt failed"); }
    }

    void release(void* ptr) override {
        if (shmdt(ptr) == -1) { throw std::runtime_error("shmdt failed"); }
    }
};


