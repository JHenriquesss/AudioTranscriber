#include "app/JobStatus.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("JobStatus follows the pipeline sequence", "[app][job-status]") {
    REQUIRE(app::canTransition(app::JobStatus::Queued, app::JobStatus::ProbingMedia));
    REQUIRE(app::canTransition(app::JobStatus::ProbingMedia, app::JobStatus::ExtractingAudio));
    REQUIRE(app::canTransition(app::JobStatus::Exporting, app::JobStatus::Completed));
}

TEST_CASE("JobStatus rejects invalid transitions", "[app][job-status]") {
    REQUIRE_FALSE(app::canTransition(app::JobStatus::Queued, app::JobStatus::Completed));
    REQUIRE_FALSE(app::canTransition(app::JobStatus::Completed, app::JobStatus::Queued));
    REQUIRE_FALSE(app::canTransition(app::JobStatus::Failed, app::JobStatus::Transcribing));
}

TEST_CASE("JobStatus allows failure and cancellation from active states", "[app][job-status]") {
    REQUIRE(app::canTransition(app::JobStatus::Transcribing, app::JobStatus::Failed));
    REQUIRE(app::canTransition(app::JobStatus::Transcribing, app::JobStatus::Cancelled));
    REQUIRE(app::isTerminalStatus(app::JobStatus::Cancelled));
}
