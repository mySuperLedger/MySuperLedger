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

#include "AppStateMachine.h"

#include <absl/strings/str_format.h>

#include "../../infra/util/HttpCode.h"
#include "../should_be_generated/domain/BusinessCode.h"

namespace gringofts {
namespace ledger {
namespace v2 {

ProcessHint AppStateMachine::process(const ConfigureAccountMetadataCommand &command,
                                     std::vector<std::shared_ptr<Event>> *events) const {
  ProcessHint hint;
  hint.mCode = HttpCode::OK;
  hint.mMessage = "Success";

  const auto &accountMetadataOpt = command.accountMetadataOpt();
  if (accountMetadataOpt) {
    const auto &accountMetadata = *accountMetadataOpt;
    auto accountType = accountMetadata.accountType();
    auto lowInclusive = accountMetadata.lowInclusive();
    auto highInclusive = accountMetadata.highInclusive();
    /// rule 1: metadatas' ranges should never overlap with each other
    for (const auto &[type, metadata] : mAccountMetadata) {
      if (accountType == type) continue;
      if (!(lowInclusive > metadata.highInclusive() || highInclusive < metadata.lowInclusive())) {
        hint.mCode = BusinessCode::ACCOUNT_METADATA_RANGE_OVERLAP;
        hint.mMessage = "range overlaps with other account metadata";
        return hint;
      }
    }
    /// rule 2: existing accounts with the same type should be within [lowInclusive, highInclusive]
    for (const auto &[nominalCode, account] : mCoA) {
      if (accountType != account.type()) continue;
      if (nominalCode < lowInclusive || nominalCode > highInclusive) {
        hint.mCode = BusinessCode::ACCOUNT_METADATA_RANGE_NOT_COVER_EXISTING_ACCOUNT;
        hint.mMessage = "new range cannot cover existing account's nominal code";
        return hint;
      }
    }

    auto now = TimeUtil::currentTimeInNanos();
    auto event = std::make_shared<AccountMetadataConfiguredEvent>(now, accountMetadata);
    events->push_back(std::move(event));
  }

  return hint;
}

StateMachine &AppStateMachine::apply(const AccountMetadataConfiguredEvent &event) {
  const auto &accountMetadata = event.accountMetadata();
  mAccountMetadata[accountMetadata.accountType()] = accountMetadata;
  onAccountMetadataUpdated(accountMetadata);

  return *this;
}

ProcessHint AppStateMachine::process(const CreateAccountCommand &command,
                                     std::vector<std::shared_ptr<Event>> *events) const {
  ProcessHint hint;
  hint.mCode = HttpCode::OK;
  hint.mMessage = "Success";

  const auto &accountOpt = command.accountOpt();
  if (accountOpt) {
    const auto &account = *accountOpt;
    if (mCoA.find(account.nominalCode()) != mCoA.end()) {
      hint.mCode = BusinessCode::ACCOUNT_ALREADY_EXISTS;
      hint.mMessage = "Account already exists";
    } else {
      auto now = TimeUtil::currentTimeInNanos();
      auto event = std::make_shared<AccountCreatedEvent>(now, account);
      events->push_back(std::move(event));
    }
  } else {
    hint.mCode = HttpCode::BAD_REQUEST;
    hint.mMessage = "Invalid command as no account passed";
  }

  return hint;
}

StateMachine &AppStateMachine::apply(const AccountCreatedEvent &event) {
  const auto &account = event.account();
  auto nominalCode = account.nominalCode();
  assert(mCoA.find(nominalCode) == mCoA.end());
  mCoA[nominalCode] = account;
  onAccountInserted(account);

  return *this;
}

ProcessHint AppStateMachine::process(const RecordJournalEntryCommand &command,
                                     std::vector<std::shared_ptr<Event>> *events) const {
  ProcessHint hint;
  hint.mCode = HttpCode::OK;
  hint.mMessage = "Success";

  /// validation steps
  /// 1. dedup
  const auto &journalEntryOpt = command.journalEntryOpt();
  assert(journalEntryOpt);  // request needs to pass the validation before it can be processed here
  const auto &journalEntry = *journalEntryOpt;
  if (mDoneMap.find(journalEntry.id()) != mDoneMap.end()) {
    hint.mCode = BusinessCode::JOURNAL_ENTRY_ALREADY_PROCESSED;
    hint.mMessage = "journal entry has already processed";
    return hint;
  } else {
    /// TODO(ISSUE-21): do not load all dedup ids when becoming new leader,
    /// and read dedup id from rocksdb snapshot instead in CPL
  }
  /// 2. every account must exist
  /// 3. at least one credit and one debit journal lines
  /// 4. sum of credits' amount == sum of debits' amount
  const auto &journalLines = journalEntry.journalLines();
  uint32_t debitCnt = 0;
  uint32_t creditCnt = 0;
  Amount debitSum, creditSum;
  for (const auto &journalLine : journalLines) {
    auto nominalCode = journalLine.nominalCode();
    if (mCoA.find(nominalCode) == mCoA.end()) {
      hint.mCode = BusinessCode::ACCOUNT_IN_JOURNAL_ENTRY_NOT_EXIST;
      hint.mMessage = absl::StrFormat("account %u does not exist", nominalCode);
      return hint;
    }
    if (journalLine.type() == TransactionType::Debit) {
      debitCnt++;
      debitSum += journalLine.amount();
    }
    if (journalLine.type() == TransactionType::Credit) {
      creditCnt++;
      creditSum += journalLine.amount();
    }
  }
  if (debitCnt == 0) {
    hint.mCode = BusinessCode::NO_DEBIT_IN_JOURNAL_ENTRY;
    hint.mMessage = absl::StrFormat("no debit in journal entry");
    return hint;
  }
  if (creditCnt == 0) {
    hint.mCode = BusinessCode::NO_CREDIT_IN_JOURNAL_ENTRY;
    hint.mMessage = absl::StrFormat("no credit in journal entry");
    return hint;
  }
  if (debitSum != creditSum) {
    hint.mCode = BusinessCode::CREDIT_NOT_EQUAL_TO_DEBIT;
    hint.mMessage = absl::StrFormat("credit amount not equal to debit amount");
    return hint;
  }

  return hint;
}

StateMachine &AppStateMachine::apply(const JournalEntryRecordedEvent &event) {
  /// 1. update doneMap
  /// 2. update account's balance

  return *this;
}

}  // namespace v2
}  // namespace ledger
}  // namespace gringofts
