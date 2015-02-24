#include "gmock/gmock.h" 
#include "DummyConnection.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

using namespace testing;
using namespace std;
using std::chrono::milliseconds;

class PoolDispatchedWithMultipleThreads: public Test {
public:

    vector<shared_ptr<thread>> threads;
	shared_ptr<DummyConnectionFactory> connection_factory;
	shared_ptr<ConnectionPool<DummyConnection>> pool;
	ConnectionPoolStats stat;
    condition_variable wasExecuted;
    mutex m;
    unsigned int count{0};

    void incrementCountAndNotify() {
        unique_lock<mutex> lock(m);
        ++count;
        wasExecuted.notify_all();
    }

    void waitForCountAndFailOnTimeout(unsigned int expectedCount, 
                                const milliseconds& time=milliseconds(500)) {
        unique_lock<mutex> lock(m);
        ASSERT_THAT(wasExecuted.wait_for(lock, time, [&] { return expectedCount == count; }), Eq(true));  
    } 
    
	void SetUp() override {
		connection_factory = shared_ptr<DummyConnectionFactory>(new DummyConnectionFactory());
	 	pool = shared_ptr<ConnectionPool<DummyConnection>>(new ConnectionPool<DummyConnection>(200, connection_factory));	
	}

    void TearDown() override {
        for (auto& t: threads) t->join();
    }

};

TEST_F(PoolDispatchedWithMultipleThreads, HoldsUpUnderClientStress) {
    unsigned int NumberOfThreads{300};

    for (unsigned int i{0}; i < NumberOfThreads; i++)
          threads.push_back(
            make_shared<thread>([&] {
               auto conn = pool->borrow();
               pool->release(conn);
               incrementCountAndNotify();
            })
          );

   waitForCountAndFailOnTimeout(NumberOfThreads);
}
