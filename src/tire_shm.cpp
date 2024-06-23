module;
#include <tuple>
#include <memory>

export module tiered_shm;

import shm_wrapper;
import utils;

export class TieredShm {
 public:
  TieredShm()
      : lightningWrapper(std::make_unique<LightningShmWrapper>("/mnt/tmpfs", "password")),
        cxlWrapper(std::make_unique<CxlShmWrapper>(1024 * 1024 * 1024, 12345)) {}

  std::tuple<void*, size_t> get(uint64_t key) {
    if (WatermarkManager::getInstance().getWatermark() == WatermarkManager::WatermarkType::LOW) {
      return lightningWrapper->get(key);
    } else {
      return cxlWrapper->get(key);
    }
  }

  void put(uint64_t object_id, const void* data, size_t size) {
    if (WatermarkManager::getInstance().getWatermark() == WatermarkManager::WatermarkType::LOW) {
      lightningWrapper->put(object_id, data, size);
    } else {
      cxlWrapper->put(object_id, data, size);
    }
  }

  void release(void* ptr) {
    if (WatermarkManager::getInstance().getWatermark() == WatermarkManager::WatermarkType::LOW) {
      lightningWrapper->release(ptr);
    } else {
      cxlWrapper->release(ptr);
    }
  }

 private:
  std::unique_ptr<ShmWrapper> lightningWrapper;
  std::unique_ptr<ShmWrapper> cxlWrapper;
};