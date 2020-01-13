#ifndef THREADPOOL
#define THREADPOOL

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ThreadPoolClass {

class ThreadPool {
 public:
  ThreadPool(size_t);
  template <class F, class... Args>
  std::future<typename std::result_of<F(Args...)>::type> EnqueueTask(
      F &&f,
      Args &&... args) {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(*mutex);
      if (stop) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
      }
      tasks.emplace([task]() { (*task)(); });
      // std::cout << "task emplaced" << std::endl;
    }
    condition.notify_one();
    return res;
  }
  ~ThreadPool();

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()> > tasks;
  std::unique_ptr<std::mutex> mutex;
  std::condition_variable condition;
  bool stop;
};

}  // namespace ThreadPoolClass

#endif  // THREADPOOL
