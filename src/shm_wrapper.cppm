module;
#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <tuple>

#include "client.h" // DRAM(lightning)
#include "cxlmalloc.h" // CXL(CXM-SHM)

export module shm_wrapper;
import utils;

// Abstract class
export class ShmWrapper {
 public:
    virtual ~ShmWrapper() = default;

    virtual std::tuple<void*, size_t> get(uint64_t key) = 0;
    virtual void put(uint64_t object_id, const void* data, size_t size) = 0;
    virtual void release(void* ptr) = 0;
};

export class LightningShmWrapper : public ShmWrapper {
    LightningClient client;
    std::vector<uint64_t> allocatedObjects;

 public:
    LightningShmWrapper(const std::string& store_socket, const std::string& password)
            : client(store_socket, password) {}

    ~LightningShmWrapper() override {
        for (auto objId : allocatedObjects) {
            client.Delete(objId);
        }
        utils::WatermarkManager::getInstance().setHighWatermark(false);
    }

    LightningShmWrapper(const LightningShmWrapper&) = default;
    LightningShmWrapper& operator=(const LightningShmWrapper&) = default;
    LightningShmWrapper(LightningShmWrapper&&) = default;
    LightningShmWrapper& operator=(LightningShmWrapper&&) = default;

    std::tuple<void*, size_t> get(uint64_t key) override {
        uint8_t* ptr = nullptr;
        size_t size;
        if (client.Get(key, &ptr, &size) != 0) {
            throw std::runtime_error("LightningClient Get failed");
        }
        return std::make_tuple(ptr, size);
    }

    void put(uint64_t object_id, const void* data, size_t size) override {
        uint8_t *ptr;
        int status = client.Create(object_id, &ptr, size);
        if (status != 0) {
            throw std::runtime_error("LightningClient Create failed");
        }
        std::memcpy(ptr, data, size);
        status = client.Seal(object_id);
        if (status != 0) {
            throw std::runtime_error("LightningClient Seal failed");
        }
        allocatedObjects.push_back(object_id);
    }

    void release(void* ptr) override {
        uint64_t objectId = findObjectId(ptr);
        client.Delete(objectId);
    }

 private:
    uint64_t findObjectId(void* ptr) {
        for (auto objId : allocatedObjects) {
            uint8_t* objPtr = nullptr;
            size_t size;
            if (client.Get(objId, &objPtr, &size) == 0 && objPtr == ptr) {
                return objId;
            }
        }
        throw std::runtime_error("Object ID not found for given pointer");
    }
};

export class CxlShmWrapper : public ShmWrapper {
    std::unique_ptr<cxl_shm> shm;
    size_t length;
    int shm_id;

 public:
    CxlShmWrapper(size_t length, int shm_id)
            : shm(std::make_unique<cxl_shm>(length, shm_id)), length(length), shm_id(shm_id) {
        shm->thread_init();
    }

    std::tuple<void*, size_t> get(uint64_t key) override {
        CXLRef ref = shm->get_ref(key);
        return std::make_tuple(ref.get_addr(), ref.get_size());
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