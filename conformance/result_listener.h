// Protocol Buffers - Google's data interchange format
// Copyright 2026 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_CONFORMANCE_RESULT_LISTENER_H__
#define GOOGLE_PROTOBUF_CONFORMANCE_RESULT_LISTENER_H__

#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "absl/base/nullability.h"
#include "conformance/result_ledger.h"
#include "conformance/result_record.h"

namespace google {
namespace protobuf {
namespace conformance {
namespace internal {

// A snapshot of the ResultLedger's counters.  The ResultListener reports the
// per-test and per-suite deltas, and the environment the run's total, as test
// properties.
struct Statistics {
  int skipped_tests = 0;
  int listed_skips = 0;
  int tolerated_failures = 0;
  int expected_failures = 0;
  int unexpected_failures = 0;
  int expected_successes = 0;
  int unexpected_successes = 0;

  static Statistics From(const ResultLedger& ledger);
  Statistics operator-(const Statistics& other) const;
  Statistics& operator+=(const Statistics& other);

  // Records each statistic as a gtest property of the current test (or suite,
  // or run) under the names skipped_tests, listed_skips,
  // tolerated_failures, expected_failures, unexpected_failures,
  // expected_successes and unexpected_successes.
  void RecordProperties() const;
};

// Publishes what every conformance test did.  Yields() (matchers.h) reports
// each outcome to the ResultLedger as it checks the result (see
// ReportOutcome() in global_test_environment.h); the ledger tallies it and
// keeps the record.  At the end of every gtest test and test suite
// this listener records the outcomes checked there as properties named after
// the conformance tests (see ReportRecordedResults()) and the statistics the
// test or suite added as its properties.  gtest still attributes properties
// to the test at that point (it forgets the current test only after the
// listeners ran), and likewise for the suite.
//
// ConformanceEnvironment (test_environment.h) registers one with gtest.  The
// listener doesn't depend on the environment, so unit tests drive it directly.
class ResultListener : public testing::EmptyTestEventListener {
 public:
  explicit ResultListener(ResultLedger* absl_nonnull ledger)
      : ledger_(ledger) {}

  void OnTestSuiteStart(const testing::TestSuite& suite) override;
  void OnTestStart(const testing::TestInfo& test_info) override;
  void OnTestEnd(const testing::TestInfo& test_info) override;
  void OnTestSuiteEnd(const testing::TestSuite& suite) override;

  // Records the outcome of every conformance test checked since the last call
  // as a property of the current gtest test (or suite, or run) named after the
  // conformance test, with ResultRecord::ToString() as the value, so that the
  // reports can tell what each test did.  Runs when a suite ends, and from
  // ConformanceEnvironment::TearDown() for anything checked outside a test
  // suite.  A test's own outcomes are recorded when the test ends.
  void ReportRecordedResults();

 private:
  // Records the ledger's records since the last call (or OnTestStart()) as
  // properties of the current test, suite or run.
  void ReportNewResults();

  ResultLedger* const absl_nonnull ledger_;

  // The counters when the current suite and test started.
  Statistics suite_start_;
  Statistics test_start_;

  // How many of ledger_->records() have been recorded as properties or held
  // back in suite_results_.
  size_t published_ = 0;

  // The results checked in SetUpTestSuite(), held back while the tests run.
  std::vector<RecordedResult> suite_results_;
};

}  // namespace internal
}  // namespace conformance
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_CONFORMANCE_RESULT_LISTENER_H__
