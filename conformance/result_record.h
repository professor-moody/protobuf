// Protocol Buffers - Google's data interchange format
// Copyright 2026 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_CONFORMANCE_RESULT_RECORD_H__
#define GOOGLE_PROTOBUF_CONFORMANCE_RESULT_RECORD_H__

#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "conformance/testee.h"

namespace google {
namespace protobuf {
namespace conformance {
namespace internal {

// The outcome of one conformance test, as Yields() (matchers.h) reports it to
// the ResultLedger (see ResultLedger::Report() in result_ledger.h) when it
// checks the test's result, and as the test environment publishes it, as a
// gtest property named after the test whose value is ToString(), e.g.
// "P0 PASS" or "P1 FAIL: Failed to parse input.".
struct ResultRecord {
  enum class Status {
    kPass,   // The result matched the expectation.
    kFail,   // It didn't.
    kCrash,  // The testee hit a runtime error or timed out.
    kSkip,   // The testee declined to run the test.
  };

  TestPriority priority;
  Status status;
  // The failure message or skip reason, if any.
  std::string message;

  // "<priority> <status>[: <message>]": the priority as PriorityName() spells
  // it, the status as PASS, FAIL, CRASH or SKIP, and the message stripped of
  // trailing whitespace and left out when empty.
  std::string ToString() const;

  // The inverse of ToString().  Returns nullopt for a value in any other
  // form.
  static absl::optional<ResultRecord> Parse(absl::string_view value);
};

// A recorded outcome: the conformance test's name and its record, as the
// ResultLedger (result_ledger.h) keeps them.
struct RecordedResult {
  std::string test_name;
  ResultRecord record;
};

}  // namespace internal
}  // namespace conformance
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_CONFORMANCE_RESULT_RECORD_H__
