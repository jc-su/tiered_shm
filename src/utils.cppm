module;
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/sysinfo.h>

export module utils;

export class WatermarkManager {
 public:
  WatermarkManager() : highWatermark(false) {}

  static WatermarkManager& getInstance() {
    static WatermarkManager instance;
    return instance;
  }

  void setHighWatermark(bool value) {
    std::lock_guard<std::mutex> lock(mutex);
    highWatermark = value;
  }

  bool isHighWatermark() const { return highWatermark; }

 private:
  static WatermarkManager* instance;
  std::mutex mutex;
  bool highWatermark;
  WatermarkManager(const WatermarkManager&) = delete;
  WatermarkManager& operator=(const WatermarkManager&) = delete;
  WatermarkManager(WatermarkManager&&) = delete;
  WatermarkManager& operator=(WatermarkManager&&) = delete;

  ~WatermarkManager() {}
};

export class MemoryMonitor {
 private:
  std::mutex mtx;
  std::condition_variable cv;
  std::atomic<bool> done;
  std::thread worker;

  static double checkDRAMUsage() {
    struct sysinfo memInfo {};
    sysinfo(&memInfo);

    long long totalPhysicalMemory = memInfo.totalram;
    totalPhysicalMemory *= memInfo.mem_unit;

    long long freePhysicalMemory = memInfo.freeram;
    freePhysicalMemory *= memInfo.mem_unit;

    long long usedPhysicalMemory = totalPhysicalMemory - freePhysicalMemory;

    return static_cast<double>((usedPhysicalMemory * 100) / totalPhysicalMemory);
  }

  void periodicCheck(int interval) {
    std::unique_lock<std::mutex> lock(this->mtx);
    auto& watermarkManager = WatermarkManager::getInstance();
    double usage = 0.0;
    while (!this->done) {
      usage = checkDRAMUsage();
      if (usage > 80.0) { watermarkManager.setHighWatermark(true); }

      this->cv.wait_for(lock, std::chrono::seconds(interval), [this] { return this->done.load(); });
    }
  }

 public:
  MemoryMonitor() : done(false) {}

  MemoryMonitor(const MemoryMonitor&) = delete;
  MemoryMonitor& operator=(const MemoryMonitor&) = delete;
  MemoryMonitor(MemoryMonitor&&) = delete;
  MemoryMonitor& operator=(MemoryMonitor&&) = delete;

  void start(int interval) { worker = std::thread(&MemoryMonitor::periodicCheck, this, interval); }

  void stop() {
    done = true;
    cv.notify_one();
    if (worker.joinable()) { worker.join(); }
  }

  ~MemoryMonitor() { stop(); }
};
