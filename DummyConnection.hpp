#include "ConnectionPool.hpp"

class DummyConnection {

public:
   DummyConnection() {

   };

   ~DummyConnection() {

   };

};

class DummyConnectionFactory : public ConnectionFactory<DummyConnection> {

public:
   DummyConnectionFactory() {

   };

   ~DummyConnectionFactory() {

   };

   std::unique_ptr<DummyConnection> create() {
      return std::make_unique<DummyConnection>();
   };

};
