/************************************************************************
Copyright 2022 MySuperLedger
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_JOURNALENTRY_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_JOURNALENTRY_H_

#include "JournalLine.h"

namespace gringofts {
namespace ledger {

class JournalEntry {
 public:
  JournalEntry() = default;
  explicit JournalEntry(const protos::JournalEntry &journalEntry) {
    initWith(journalEntry);
  }

  void initWith(const protos::JournalEntry &journalEntry) {
    mVersion = journalEntry.version();
    assert(mVersion > 0);
    switch (mVersion) {
      case 1: {
        mId = journalEntry.id();
        mValidTime = journalEntry.valid_time();
        mRecordTime = journalEntry.record_time();
        mPurpose = journalEntry.purpose();
        for (const auto &journalLineProto : journalEntry.journal_lines()) {
          JournalLine journalLine;
          journalLine.initWith(journalLineProto);
          mJournalLines.push_back(std::move(journalLine));
        }
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  void encodeTo(protos::JournalEntry &journalEntry) const {  // NOLINT[runtime/references]
    journalEntry.set_version(mVersion);
    switch (mVersion) {
      case 1: {
        journalEntry.set_id(journalEntry.id());
        journalEntry.set_valid_time(mValidTime);
        journalEntry.set_record_time(mRecordTime);
        journalEntry.set_purpose(mPurpose);
        for (const auto &journalLine : mJournalLines) {
          journalLine.encodeTo(*(journalEntry.mutable_journal_lines()->Add()));
        }
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  bool isSame(const JournalEntry &another) const {
    if (mVersion != another.mVersion) {
      SPDLOG_WARN("version is not the same, {} vs {}", mVersion, another.mVersion);
      return false;
    }

    if (mVersion == 1) {
      if (mId != another.mId) {
        SPDLOG_WARN("id is not the same, {} vs {}", mId, another.mId);
        return false;
      }
      if (mValidTime != another.mValidTime) {
        SPDLOG_WARN("validTime type is not the same, {} vs {}", mValidTime, another.mValidTime);
        return false;
      }
      if (mRecordTime != another.mRecordTime) {
        SPDLOG_WARN("recordTime type is not the same, {} vs {}", mRecordTime, another.mRecordTime);
        return false;
      }
      if (mPurpose != another.mPurpose) {
        SPDLOG_WARN("purpose is not the same, {} vs {}", mPurpose, another.mPurpose);
        return false;
      }
      if (mJournalLines.size() != another.mJournalLines.size()) {
        SPDLOG_WARN("journal line size is not the same, {} vs {}", mJournalLines.size(), another.mJournalLines.size());
        return false;
      }
      for (auto i = 0; i < mJournalLines.size(); i++) {
        const auto &left = mJournalLines[i];
        const auto &right = another.mJournalLines[i];
        if (!left.isSame(right)) {
          SPDLOG_WARN("journal line is not the same");
          return false;
        }
      }
    } else {
      SPDLOG_ERROR("Cannot recognize this version {}", mVersion);
      return false;
    }

    return true;
  }

  const std::string &id() const {
    return mId;
  }

  uint64_t validTime() const {
    return mValidTime;
  }

  const std::vector<JournalLine> &journalLines() const {
    return mJournalLines;
  }

 private:
  uint64_t mVersion;  // keep every version for backward-compatibility
  std::string mId;  // uniquely identify this entry
  uint64_t mValidTime;  // the point this entry happened
  uint64_t mRecordTime;  // the point this entry is recorded in the journal
  std::vector<JournalLine> mJournalLines;
  std::string mPurpose;  // what this entry book, e.g., record a payment
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_JOURNALENTRY_H_
