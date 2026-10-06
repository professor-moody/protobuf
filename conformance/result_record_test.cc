// Protocol Buffers - Google's data interchange format
// Copyright 2026 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "conformance/result_record.h"

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "conformance/testee.h"

namespace google {
namespace protobuf {
namespace conformance {
namespace internal {
namespace {

using ::testing::FieldsAre;
using ::testing::Optional;

TEST(ResultRecordTest, ToString) {
  EXPECT_EQ((ResultRecord{kP0, ResultRecord::Status::kPass, ""}).ToString(),
            "P0 PASS");
  EXPECT_EQ((ResultRecord{kP1, ResultRecord::Status::kFail, "boom"}).ToString(),
            "P1 FAIL: boom");
  EXPECT_EQ((ResultRecord{kP0, ResultRecord::Status::kCrash, "x"}).ToString(),
            "P0 CRASH: x");
  EXPECT_EQ((ResultRecord{kP0, ResultRecord::Status::kSkip, "why"}).ToString(),
            "P0 SKIP: why");
}

TEST(ResultRecordTest, ToStringStripsTrailingWhitespace) {
  EXPECT_EQ(
      (ResultRecord{kP0, ResultRecord::Status::kFail, "a: b \n\n"}).ToString(),
      "P0 FAIL: a: b");
  EXPECT_EQ((ResultRecord{kP0, ResultRecord::Status::kFail, " \n"}).ToString(),
            "P0 FAIL");
}

TEST(ResultRecordTest, ParseInvertsToString) {
  for (TestPriority priority : {kP0, kP1}) {
    for (ResultRecord::Status status :
         {ResultRecord::Status::kPass, ResultRecord::Status::kFail,
          ResultRecord::Status::kCrash, ResultRecord::Status::kSkip}) {
      for (absl::string_view message : {"", "msg", "a: b: c", "multi\nline"}) {
        ResultRecord record{priority, status, std::string(message)};
        EXPECT_THAT(ResultRecord::Parse(record.ToString()),
                    Optional(FieldsAre(priority, status, message)))
            << record.ToString();
      }
    }
  }
}

TEST(ResultRecordTest, ParseRejectsOtherValues) {
  EXPECT_EQ(ResultRecord::Parse(""), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("garbage"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0 "), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P2 PASS"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0 BOGUS"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0 pass"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0 PASS extra"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0 FAIL:"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse("P0 FAIL:msg"), absl::nullopt);
  EXPECT_EQ(ResultRecord::Parse(" P0 PASS"), absl::nullopt);
}

}  // namespace
}  // namespace internal
}  // namespace conformance
}  // namespace protobuf
}  // namespace google
