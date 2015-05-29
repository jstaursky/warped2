#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include <memory>
#include <utility>

#include "LadderQueue.hpp"
#include "Event.hpp"
#include "mocks.hpp"

TEST_CASE("Ladder Queue operations") {

    warped::LadderQueue q;
    std::shared_ptr<warped::Event> e;
    REQUIRE(q.begin() == nullptr);

    SECTION("Insert, read and erase event on a Laddder Queue") {

        // Basic check for insert(), begin() and erase()
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"1", 10}));
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "1");
        CHECK(e->timestamp() == 10);
        q.erase(e);
        e = q.begin();
        REQUIRE(e == nullptr);

        // Check for algorithmic accuracy
        // Add events in decreasing order
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"1", 10}));
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"2", 5}));
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "2");
        CHECK(e->timestamp() == 5);
        q.erase(e);
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "1");
        CHECK(e->timestamp() == 10);

        // Add event with a larger timestamp
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"3", 15}));
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "1");
        CHECK(e->timestamp() == 10);

        // Add event with a smaller timestamp
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"3", 8}));
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "3");
        CHECK(e->timestamp() == 8);

        // Delete the 3 events
        q.erase(e);
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "1");
        CHECK(e->timestamp() == 10);
        q.erase(e);
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "3");
        CHECK(e->timestamp() == 15);
        q.erase(e);
        e = q.begin();
        REQUIRE(e == nullptr);
    }

    SECTION("Test events with same timestamp but different insertion order") {

        q.insert(std::shared_ptr<warped::Event>(new test_Event {"3", 10}));
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"1", 10}));
        q.insert(std::shared_ptr<warped::Event>(new test_Event {"2", 10}));
        e = q.begin();
        REQUIRE(e != nullptr);
        CHECK(e->receiverName() == "1");
        CHECK(e->timestamp() == 10);
        //q.erase(e);
        //e = q.begin();
        //REQUIRE(e != nullptr);
        //CHECK(e->receiverName() == "2");
        //CHECK(e->timestamp() == 10);
        //q.erase(e);
        //e = q.begin();
        //REQUIRE(e != nullptr);
        //CHECK(e->receiverName() == "3");
        //CHECK(e->timestamp() == 10);
        //q.erase(e);
        //e = q.begin();
        //REQUIRE(e == nullptr);
    }
}
