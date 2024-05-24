#ifndef CONNECTIONPOOL_HPP_
#define CONNECTIONPOOL_HPP_

#include <condition_variable>
#include <queue>
#include <set>
#include <memory>
#include <string>
#include <mutex>
#include <utility>  // for std::move

template <class T>
class ConnectionFactory {
public:
   ConnectionFactory() = default;
   virtual ~ConnectionFactory() = default;
   virtual std::unique_ptr<T> create() = 0;
};

struct ConnectionPoolStats {
   size_t pool_size;
   size_t borrowed_size;
};

template <typename T>
class ConnectionPool {
public:

   ConnectionPool(size_t max_pool_size, std::shared_ptr<ConnectionFactory<T>> factory, int timeout_in_seconds = 5)
      : max_pool_size(max_pool_size), factory(factory), borrowed_size(0), timeout(timeout_in_seconds) {}

   ConnectionPoolStats getStats() {
      std::lock_guard<std::mutex> lock(cv_mutex);
      return { pool.size(), borrowed_size };
   }

   std::unique_ptr<T> borrow() {
      std::unique_lock<std::mutex> lock(cv_mutex);

// Wait until there is an available connection or space to create a new one
      if (!cv.wait_for(lock, std::chrono::seconds(timeout), [&] { return !pool.empty() || borrowed_size < max_pool_size; })) {
         throw std::runtime_error("Timeout while waiting for a connection.");
      }


      cv.wait(lock, [&] { return !pool.empty() || borrowed_size < max_pool_size; });
      if (!pool.empty()) {
         auto conn = std::move(pool.front());
         pool.pop();
         borrowed_size++;
         return conn;
      }

      if (borrowed_size < max_pool_size) {
         borrowed_size++;
         try {
            return factory->create();
         } catch (...) {
            borrowed_size--;  // Ensure borrowed_size is decremented if create() throws
            throw;
         }
      }
      throw std::runtime_error("Failed to borrow a connection."); // This should be unreachable due to the wait condition
   }

   void report_broken_connection() {
      std::lock_guard<std::mutex> lock(cv_mutex);
      borrowed_size--;
      cv.notify_one();
   }

   void return_connection(std::unique_ptr<T> conn) noexcept {
      std::lock_guard<std::mutex> lock(cv_mutex);
      pool.push(std::move(conn));
      borrowed_size--;
      cv.notify_one();
   }

   ~ConnectionPool() = default;

private:
   size_t max_pool_size;
   std::shared_ptr<ConnectionFactory<T> > factory;
   size_t borrowed_size;
   std::queue<std::unique_ptr<T> > pool;
   std::condition_variable cv;
   std::mutex cv_mutex;
   int timeout;
};

#endif  // CONNECTIONPOOL_HPP_
