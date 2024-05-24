#include "gmock/gmock.h" 
#include "DummyConnection.hpp"

class Pool: public testing::Test {
public:
   std::shared_ptr<DummyConnectionFactory> connection_factory;
   std::shared_ptr<ConnectionPool<DummyConnection>> pool;
   ConnectionPoolStats stat;

   void SetUp() override {
      connection_factory = std::make_shared<DummyConnectionFactory>();
      pool = std::make_shared<ConnectionPool<DummyConnection>>(2, connection_factory);
   }
};

TEST_F(Pool, CheckPoolSize) {
   stat = pool->getStats();
   ASSERT_THAT(stat.pool_size, testing::Eq(0));  // we are using lazy population of the pool
}

TEST_F(Pool, BorrowAllConnections) {
   std::unique_ptr<DummyConnection> conn1 = pool->borrow();
   std::unique_ptr<DummyConnection> conn2 = pool->borrow();
   stat = pool->getStats();
   ASSERT_THAT(stat.borrowed_size, testing::Eq(2));
   ASSERT_THAT(stat.pool_size, testing::Eq(0));
}

TEST_F(Pool, BorrowAndReleaseAllConnections) {
   std::unique_ptr<DummyConnection> conn1 = pool->borrow();
   std::unique_ptr<DummyConnection> conn2 = pool->borrow();
   pool->return_connection(std::move(conn1));
   pool->return_connection(std::move(conn2));
   stat = pool->getStats();
   ASSERT_THAT(stat.borrowed_size, testing::Eq(0));
   ASSERT_THAT(stat.pool_size, testing::Eq(2));
}

TEST_F(Pool, BorrowAllReleaseOneandReBorrow) {
   std::unique_ptr<DummyConnection> conn1 = pool->borrow();
   std::unique_ptr<DummyConnection> conn2 = pool->borrow();
   pool->return_connection(std::move(conn1));
   std::unique_ptr<DummyConnection> conn3 = pool->borrow();
   stat = pool->getStats();
   ASSERT_THAT(stat.borrowed_size, testing::Eq(2));
   ASSERT_THAT(stat.pool_size, testing::Eq(0));
}

TEST_F(Pool, DirtyPoolCreation) {
   {
      std::unique_ptr<DummyConnection> conn1 = pool->borrow();
      std::unique_ptr<DummyConnection> conn2 = pool->borrow();
      pool->report_broken_connection();
      pool->report_broken_connection();
   }

   std::unique_ptr<DummyConnection> conn1 = pool->borrow();
   std::unique_ptr<DummyConnection> conn2 = pool->borrow();

   stat = pool->getStats();
   ASSERT_THAT(stat.borrowed_size, testing::Eq(2));
   ASSERT_THAT(stat.pool_size, testing::Eq(0));
}

TEST_F(Pool, BorrowOne) {
   std::unique_ptr<DummyConnection> conn1 = pool->borrow();
   stat = pool->getStats();
   ASSERT_THAT(stat.borrowed_size, testing::Eq(1));
   ASSERT_THAT(stat.pool_size, testing::Eq(0));
   pool->return_connection(std::move(conn1));
   stat = pool->getStats();
   ASSERT_THAT(stat.borrowed_size, testing::Eq(0));
   ASSERT_THAT(stat.pool_size, testing::Eq(1));
}
