#include "gmock/gmock.h" 
#include "DummyConnection.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class PoolDispatchedWithMultipleThreads: public testing::Test {
public:
   std::vector<std::shared_ptr<std::thread>> threads;
   std::shared_ptr<DummyConnectionFactory> connection_factory;
   std::shared_ptr<ConnectionPool<DummyConnection>> pool;
   ConnectionPoolStats stat;
   std::condition_variable wasExecuted;
   std::mutex m;
   unsigned int count{0};

   void incrementCountAndNotify() {
      std::unique_lock<std::mutex> lock(m);
      ++count;
      wasExecuted.notify_all();
   }

   void waitForCountAndFailOnTimeout(unsigned int expectedCount,
                                     const std::chrono::milliseconds& time=std::chrono::milliseconds(500)) {
      std::unique_lock<std::mutex> lock(m);
      ASSERT_THAT(wasExecuted.wait_for(lock, time, [&] { return expectedCount == count; }), testing::Eq(true));
   }

   void SetUp() override {
      connection_factory = std::make_shared<DummyConnectionFactory>();
      pool = std::make_shared<ConnectionPool<DummyConnection>>(200, connection_factory);
   }

   void TearDown() override {
      for (auto& t: threads) t->join();
   }

};

TEST_F(PoolDispatchedWithMultipleThreads, HoldsUpUnderClientStress) {
   unsigned int NumberOfThreads{300};

   for (unsigned int i{0}; i < NumberOfThreads; i++)
      threads.push_back(
         std::make_shared<std::thread>([&] {
            auto conn = pool->borrow();
            pool->return_connection(std::move(conn));
            incrementCountAndNotify();
         })
         );

   waitForCountAndFailOnTimeout(NumberOfThreads);
}
