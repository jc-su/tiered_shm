#include <iostream>
#include <cstring>

import tiered_shm;

int main() {
  TieredShm tieredShm;

  uint64_t object_id = 123;
  const char* data = "Hello, World!";
  size_t size = strlen(data) + 1;

  tieredShm.put(object_id, data, size);

  auto [ptr, out_size] = tieredShm.get(object_id);
  std::cout << "Retrieved data: " << static_cast<char*>(ptr) << " of size: " << out_size << std::endl;

  tieredShm.release(ptr);

  return 0;
}