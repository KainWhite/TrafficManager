#include "ThreadPool.h"

namespace ThreadPoolClass {

ThreadPool::ThreadPool(size_t threads) : stop(false), mutex(new std::mutex) {
  for (size_t i = 0; i < threads; ++i) {
    workers.emplace_back([this] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(*(this->mutex));
          this->condition.wait(
              lock, [this] { return this->stop || !this->tasks.empty(); });
          if (this->stop) {
            return;
          }
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }
        task();
        // std::cout << "task runned" << std::endl;
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(*(this->mutex));
    stop = true;
  }
  condition.notify_all();
  for (std::thread &worker : workers) {
    worker.join();
  }
}
}  // namespace ThreadPoolClass