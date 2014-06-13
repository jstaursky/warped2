#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <sstream>

#include "MatternGVTManager.hpp"
#include "serialization.hpp"

TEST_CASE("Mattern GVT messages can be serialized", "[serialization]") {
    std::stringstream ss;
    std::unique_ptr<warped::MatternGVTToken> msg
                        {new warped::MatternGVTToken{1, 5, 10}};
    std::unique_ptr<warped::MatternGVTToken> msg2;

    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(msg);
    }

    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(msg2);
    }

    auto m = dynamic_cast<warped::MatternGVTToken*>(msg2.get());
    REQUIRE(m != nullptr);
    REQUIRE(m->m_clock == 1);
    REQUIRE(m->m_send == 5);
    REQUIRE(m->count == 10);
}
