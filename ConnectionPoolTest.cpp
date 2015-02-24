#include "gmock/gmock.h" 
#include "DummyConnection.h"

using namespace testing;
using namespace std;

class Pool: public Test {
public:
	shared_ptr<DummyConnectionFactory> connection_factory;
	shared_ptr<ConnectionPool<DummyConnection>> pool;
	ConnectionPoolStats stat;

	void SetUp() override {
		connection_factory = shared_ptr<DummyConnectionFactory>(new DummyConnectionFactory());
	 	pool = shared_ptr<ConnectionPool<DummyConnection>>(new ConnectionPool<DummyConnection>(2, connection_factory));	
	}
};

TEST_F(Pool, CheckPoolSize) {
	stat = pool->getStats();
	ASSERT_THAT(stat.pool_size, Eq(2)); 
}

TEST_F(Pool, BorrowAllConnections) {
	shared_ptr<DummyConnection> conn1 = pool->borrow(); 
	shared_ptr<DummyConnection> conn2 = pool->borrow();
	stat = pool->getStats();
	ASSERT_THAT(stat.borrowed_size, Eq(2));	
    ASSERT_THAT(stat.pool_size, Eq(0));
}

TEST_F(Pool, BorrowAndReleaseAllConnections) {
	shared_ptr<DummyConnection> conn1 = pool->borrow();
	shared_ptr<DummyConnection> conn2 = pool->borrow();
	pool->release(conn1);
	pool->release(conn2);	
	stat = pool->getStats();
	ASSERT_THAT(stat.borrowed_size, Eq(0));	
    ASSERT_THAT(stat.pool_size, Eq(2)); 
}

TEST_F(Pool, BorrowAllReleaseOneandReBorrow) {
	shared_ptr<DummyConnection> conn1 = pool->borrow();
	shared_ptr<DummyConnection> conn2 = pool->borrow();
	pool->release(conn1);
	shared_ptr<DummyConnection> conn3 = pool->borrow();	
	stat = pool->getStats();
	ASSERT_THAT(stat.borrowed_size, Eq(2));
	ASSERT_THAT(stat.pool_size, Eq(0));	
}

TEST_F(Pool, DirtyPoolCreation) {
	{
	shared_ptr<DummyConnection> conn1 = pool->borrow();
	shared_ptr<DummyConnection> conn2 = pool->borrow();
	}

	shared_ptr<DummyConnection> conn1 = pool->borrow();
	shared_ptr<DummyConnection> conn2 = pool->borrow();

	stat = pool->getStats();
	ASSERT_THAT(stat.borrowed_size, Eq(2));
	ASSERT_THAT(stat.pool_size, Eq(0));
}

TEST_F(Pool,BorrowMoreThanMaximumConnections) {
	shared_ptr<DummyConnection> conn1 = pool->borrow();
	shared_ptr<DummyConnection> conn2 = pool->borrow();
	ASSERT_THROW(shared_ptr<DummyConnection> conn3 = pool->borrow(),ConnectionPoolException);
}
