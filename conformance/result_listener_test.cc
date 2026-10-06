// Protocol Buffers - Google's data interchange format
// Copyright 2026 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "conformance/result_listener.h"

#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/strings/string_view.h"
#include "conformance/result_ledger.h"
#include "conformance/result_record.h"
#include "conformance/testee.h"

namespace google {
namespace protobuf {
namespace conformance {
namespace internal {
namespace {

using ::testing::AllOf;
using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Key;
using ::testing::Not;
using ::testing::Pair;

constexpr absl::string_view kFooTestName =
    "Required.Proto3.ProtobufInput.Foo.ProtobufOutput";
constexpr absl::string_view kSetUpTestName =
    "Required.Proto3.ProtobufInput.SetUpTestSuite.ProtobufOutput";
constexpr absl::string_view kTearDownTestName =
    "Required.Proto3.ProtobufInput.TearDownTestSuite.ProtobufOutput";

ResultRecord Pass() { return {kP0, ResultRecord::Status::kPass, ""}; }
ResultRecord Fail() { return {kP0, ResultRecord::Status::kFail, "boom"}; }

// Drives a listener by hand, with the gtest test and suite it hears about
// being this one.  Whatever it records lands on the running test, including
// what it records when told that the suite ended.
class ResultListenerTest : public testing::Test {
 protected:
  ~ResultListenerTest() override {
    // ResultLedger insists on being finalized before destruction.
    ledger_.Finalize().IgnoreError();
  }

  // What Yields() does when it checks a result: reports its outcome to the
  // ledger.
  void Report(absl::string_view test_name, const ResultRecord& record) {
    ledger_.Report(test_name, record);
  }

  static const testing::TestSuite& CurrentSuite() {
    return *testing::UnitTest::GetInstance()->current_test_suite();
  }
  static const testing::TestInfo& CurrentTest() {
    return *testing::UnitTest::GetInstance()->current_test_info();
  }

  // The properties of the running test so far, as (key, value) pairs.
  static std::vector<std::pair<std::string, std::string>> Properties() {
    const testing::TestResult& result = *CurrentTest().result();
    std::vector<std::pair<std::string, std::string>> properties;
    for (int i = 0; i < result.test_property_count(); ++i) {
      const testing::TestProperty& property = result.GetTestProperty(i);
      properties.emplace_back(property.key(), property.value());
    }
    return properties;
  }

  // The outcomes among Properties(): those whose value is a ResultRecord.
  static std::vector<std::pair<std::string, std::string>> Outcomes() {
    std::vector<std::pair<std::string, std::string>> outcomes;
    for (const auto& property : Properties()) {
      if (ResultRecord::Parse(property.second).has_value()) {
        outcomes.push_back(property);
      }
    }
    return outcomes;
  }

  ResultLedger ledger_;
  ResultListener listener_{&ledger_};
};

TEST_F(ResultListenerTest, ReportsWhenTheTestEnds) {
  listener_.OnTestSuiteStart(CurrentSuite());
  listener_.OnTestStart(CurrentTest());

  Report(kFooTestName, Pass());
  EXPECT_THAT(Properties(), IsEmpty());

  listener_.OnTestEnd(CurrentTest());
  EXPECT_THAT(Properties(), AllOf(Contains(Pair(kFooTestName, "P0 PASS")),
                                  Contains(Pair("expected_successes", "1")),
                                  Contains(Pair("unexpected_failures", "0"))));
}

TEST_F(ResultListenerTest, ReportsEachOutcomeOnce) {
  listener_.OnTestStart(CurrentTest());
  Report(kFooTestName, Fail());
  listener_.OnTestEnd(CurrentTest());
  EXPECT_THAT(Outcomes(), ElementsAre(Pair(kFooTestName, "P0 FAIL: boom")));

  listener_.OnTestStart(CurrentTest());
  listener_.OnTestEnd(CurrentTest());
  EXPECT_THAT(Outcomes(), ElementsAre(Pair(kFooTestName, "P0 FAIL: boom")));
}

TEST_F(ResultListenerTest, ReportsRecordedResultsOnRequest) {
  // What ConformanceEnvironment::TearDown() does for the results checked
  // outside any test suite.
  Report(kFooTestName, Fail());
  EXPECT_THAT(Properties(), Not(Contains(Key(kFooTestName))));
  listener_.ReportRecordedResults();
  EXPECT_THAT(Properties(), Contains(Pair(kFooTestName, "P0 FAIL: boom")));
}

TEST_F(ResultListenerTest, HoldsSetUpTestSuiteResultsBackUntilTheSuiteEnds) {
  listener_.OnTestSuiteStart(CurrentSuite());
  Report(kSetUpTestName, Pass());

  listener_.OnTestStart(CurrentTest());
  Report(kFooTestName, Fail());
  listener_.OnTestEnd(CurrentTest());
  // The test reports its own outcome and statistics only.
  EXPECT_THAT(Outcomes(), ElementsAre(Pair(kFooTestName, "P0 FAIL: boom")));
  EXPECT_THAT(Properties(), AllOf(Contains(Pair("expected_successes", "0")),
                                  Contains(Pair("unexpected_failures", "1"))));

  Report(kTearDownTestName, Pass());
  listener_.OnTestSuiteEnd(CurrentSuite());
  // The suite reports what SetUpTestSuite() checked, then what
  // TearDownTestSuite() did, and the statistics of the whole suite.
  EXPECT_THAT(Outcomes(), ElementsAre(Pair(kFooTestName, "P0 FAIL: boom"),
                                      Pair(kSetUpTestName, "P0 PASS"),
                                      Pair(kTearDownTestName, "P0 PASS")));
  EXPECT_THAT(Properties(), AllOf(Contains(Pair("expected_successes", "2")),
                                  Contains(Pair("unexpected_failures", "1"))));
}

}  // namespace
}  // namespace internal
}  // namespace conformance
}  // namespace protobuf
}  // namespace google
