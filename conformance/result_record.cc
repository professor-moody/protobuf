// Protocol Buffers - Google's data interchange format
// Copyright 2026 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "conformance/result_record.h"

#include <iterator>
#include <string>

#include "absl/algorithm/container.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/types/optional.h"
#include "conformance/testee.h"

namespace google {
namespace protobuf {
namespace conformance {
namespace internal {
namespace {

// The status words of ResultRecord::ToString(), indexed by Status.
constexpr absl::string_view kStatusNames[] = {"PASS", "FAIL", "CRASH", "SKIP"};

}  // namespace

std::string ResultRecord::ToString() const {
  std::string value = absl::StrCat(PriorityName(priority), " ",
                                   kStatusNames[static_cast<int>(status)]);
  absl::string_view stripped = absl::StripTrailingAsciiWhitespace(message);
  if (!stripped.empty()) absl::StrAppend(&value, ": ", stripped);
  return value;
}

absl::optional<ResultRecord> ResultRecord::Parse(absl::string_view value) {
  ResultRecord record;
  bool found = false;
  for (TestPriority priority : {kP0, kP1}) {
    if (absl::ConsumePrefix(&value,
                            absl::StrCat(PriorityName(priority), " "))) {
      record.priority = priority;
      found = true;
      break;
    }
  }
  if (!found) return absl::nullopt;
  absl::string_view status = value.substr(0, value.find(':'));
  const absl::string_view* name = absl::c_find(kStatusNames, status);
  if (name == std::end(kStatusNames)) return absl::nullopt;
  record.status = static_cast<Status>(name - std::begin(kStatusNames));
  value.remove_prefix(status.size());
  if (!value.empty() && !absl::ConsumePrefix(&value, ": ")) {
    return absl::nullopt;
  }
  record.message = std::string(value);
  return record;
}

}  // namespace internal
}  // namespace conformance
}  // namespace protobuf
}  // namespace google
