// Protocol Buffers - Google's data interchange format
// Copyright 2026 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Declares the process-wide state the Yields() matcher works against (see
// matchers.h).  The definition is deliberately not provided here.  It is the
// link-time seam between the matchers and the process that hosts them.  The
// conformance test binary provides it from its global test environment,
// populated from command-line flags and the failure list on disk.  Unit tests
// of the matchers provide a lightweight in-memory stand-in.

#ifndef GOOGLE_PROTOBUF_CONFORMANCE_GLOBAL_TEST_ENVIRONMENT_H__
#define GOOGLE_PROTOBUF_CONFORMANCE_GLOBAL_TEST_ENVIRONMENT_H__

#include "absl/base/nullability.h"
#include "absl/strings/string_view.h"
#include "conformance/failure_list.h"
#include "conformance/result_record.h"

namespace google {
namespace protobuf {
namespace conformance {
namespace internal {

// Returns the process-wide expected-failure list.  Yields() asks it for the
// verdict on every test, which applies the expected failures and the
// enforcement level (see FailureList in failure_list.h).  The list never
// changes once the tests run; the test environment owns it as part of its
// ResultLedger (see ConformanceEnvironment in test_environment.h).
const FailureList& GetGlobalFailureList();

// Reports `record`, the outcome Yields() decided for the conformance test
// `test_name`, to the process-wide ResultLedger (see ResultLedger::Report()).
// This is the one way the matchers change anything.  Returns null if the
// record was kept, else the record kept for an earlier check of the same
// test, which this one doesn't replace: gtest evaluates a failing matcher
// again to explain the failure, and Yields() fails a check that reaches a
// different outcome than the recorded one.
//
// A process-wide ledger is safe because gtest runs the tests of a binary one
// after the other; a sharded run is several processes, each with its own
// environment and ledger, and the environment knows it only saw a subset (see
// ConformanceEnvironment::TearDown()).  Checking results from several threads
// at once is not supported, and neither is running them: a testee connection
// is a single pipe (see Testee in testee.h).
const ResultRecord* absl_nullable ReportOutcome(absl::string_view test_name,
                                                const ResultRecord& record);

}  // namespace internal
}  // namespace conformance
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_CONFORMANCE_GLOBAL_TEST_ENVIRONMENT_H__
