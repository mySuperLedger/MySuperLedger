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

#include "JournalEntryRecordedEvent.h"

#include <spdlog/spdlog.h>

#include "../common_types.h"

namespace gringofts {
namespace ledger {

JournalEntryRecordedEvent::JournalEntryRecordedEvent(TimestampInNanos createdTimeInNanos,
                                                     const JournalEntry &journalEntry)
    : Event(JOURNAL_ENTRY_RECORDED_EVENT, createdTimeInNanos) {
  mJournalEntry = journalEntry;
}

JournalEntryRecordedEvent::JournalEntryRecordedEvent(TimestampInNanos createdTimeInNanos, std::string_view eventStr)
    : Event(JOURNAL_ENTRY_RECORDED_EVENT, createdTimeInNanos) {
  decodeFromString(eventStr);
}

std::string JournalEntryRecordedEvent::encodeToString() const {
  protos::RecordJournalEntry::RecordedEvent eventProto;
  eventProto.set_version(mVersion);
  switch (mVersion) {
    case 1: {
      mJournalEntry.encodeTo(*eventProto.mutable_journal_entry());
      break;
    }
    default: {
      SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
      exit(1);
    }
  }

  return eventProto.SerializeAsString();
}

void JournalEntryRecordedEvent::decodeFromString(std::string_view payload) {
  protos::RecordJournalEntry::RecordedEvent eventProto;
  eventProto.ParseFromString(std::string(payload));
  mVersion = eventProto.version();
  assert(mVersion > 0);
  switch (mVersion) {
    case 1: {
      mJournalEntry.initWith(eventProto.journal_entry());
      break;
    }
    default: {
      SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
      exit(1);
    }
  }
}

}  /// namespace ledger
}  /// namespace gringofts
