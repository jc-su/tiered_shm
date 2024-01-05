module;
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/sysinfo.h>
#include <bpf/libbpf.h>
#include <linux/types.h>
#include <iostream>
#include <cstdint>
#include "mmap_monitor.skel.h"

export module utils;

export enum class WtermarkType { LOW, HIGH };

export class WatermarkManager {
 public:
  WatermarkManager() : watermark(WtermarkType::LOW) {}

  static WatermarkManager& getInstance() {
    static WatermarkManager instance;
    return instance;
  }

  void setHighWatermark() {
    std::lock_guard<std::mutex> lock(mutex);
    watermark = WtermarkType::HIGH;
  }

  void setLowWatermark() {
    std::lock_guard<std::mutex> lock(mutex);
    watermark = WtermarkType::LOW;
  }

  auto getWatermark() {
    std::lock_guard<std::mutex> lock(mutex);
    return watermark;
  }

 private:
  static WatermarkManager* instance;
  std::mutex mutex;
  WtermarkType watermark;

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
  std::thread ebpfEventThread;
  std::atomic<bool> ebpfEventDone;

  double lowWatermarkThreshold = 60.0;
  double highWatermarkThreshold = 80.0;

  struct mmap_monitor_bpf* skel = mmap_monitor_bpf__open_and_load();
  if (!skel) {
    std::cerr << "Failed to open and load BPF program\n";
    return;
  }

  if (mmap_monitor_bpf__attach(skel)) {
    std::cerr << "Failed to attach BPF program\n";
    return;
  }

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
      // 80% of DRAM usage is considered as high watermark
      if (usage > highWatermarkThreshold) { watermarkManager.setHighWatermark(); }

      // TODO Perform memory demotion immediately

      // 60% of DRAM usage is considered as low watermark
      if (usage < lowWatermarkThreshold) { watermarkManager.setLowWatermark(); }

      this->cv.wait_for(lock, std::chrono::seconds(interval), [this] { return this->done.load(); });
    }
  }

  // Event-driven approach to set the watermark
  void handle_ebpf_event(void* ctx, int cpu, void* data, uint32_t size) {
    auto& watermarkManager = WatermarkManager::getInstance();
    double usage = checkDRAMUsage();

    // TODO
    if (usage > highWatermarkThreshold) {
      watermarkManager.setHighWatermark();
    } else if (usage < lowWatermarkThreshold) {
      watermarkManager.setLowWatermark();
    }
  }

  void listenForEbpfEvents() {
    struct perf_buffer* pb = nullptr;
    struct perf_buffer_opts pb_opts{};
    pb_opts.sample_cb = handle_ebpf_event;

    pb = perf_buffer__new(bpf_map__fd(skel->maps.events), 64, &pb_opts);
    if (!pb) {
      std::cerr << "Failed to set up perf buffer\n";
      mmap_monitor_bpf__destroy(skel);
      return;
    }

    while (!ebpfEventDone) { perf_buffer__poll(pb, 100); }

    perf_buffer__free(pb);
    mmap_monitor_bpf__destroy(skel);
  }

 public:
  MemoryMonitor() : done(false) {}

  MemoryMonitor(const MemoryMonitor&) = delete;
  MemoryMonitor& operator=(const MemoryMonitor&) = delete;
  MemoryMonitor(MemoryMonitor&&) = delete;
  MemoryMonitor& operator=(MemoryMonitor&&) = delete;

  void start(int interval) {
    worker = std::thread(&MemoryMonitor::periodicCheck, this, interval);
    // ebpfEventThread = std::thread(&MemoryMonitor::listenForEbpfEvents, this);
    // ebpfEventDone = false;
  }

  void stop() {
    done = true;
    ebpfEventDone = true;
    cv.notify_one();
    if (worker.joinable()) worker.join();
    if (ebpfEventThread.joinable()) ebpfEventThread.join();
  }

  ~MemoryMonitor() { stop(); }
};
