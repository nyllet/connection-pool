#include "ConnectionPool.h"

class DummyConnection : public Connection {

public:
	DummyConnection() {

	};

	~DummyConnection() {

	};

};

class DummyConnectionFactory : public ConnectionFactory {

public:
	DummyConnectionFactory() {

	};

	~DummyConnectionFactory() {

	};

	shared_ptr<Connection> create() {
        shared_ptr<DummyConnection>conn(new DummyConnection());
        return static_pointer_cast<Connection>(conn);
	};

};
