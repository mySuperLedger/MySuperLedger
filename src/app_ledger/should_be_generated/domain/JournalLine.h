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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_JOURNALLINE_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_JOURNALLINE_H_

#include "Amount.h"
#include "TransactionType.h"

namespace gringofts {
namespace ledger {

class JournalLine {
 public:
  JournalLine() = default;
  explicit JournalLine(const protos::JournalLine &journalLine) {
    initWith(journalLine);
  }

  void initWith(const protos::JournalLine &journalLine) {
    mVersion = journalLine.version();
    assert(mVersion > 0);
    switch (mVersion) {
      case 1: {
        mNominalCode = journalLine.nominal_code();
        mTransactionType = TransactionTypeUtil::typeOf(journalLine.transaction_type());
        mAmount = Amount(journalLine.amount());
        mCurrencyCode = journalLine.currency_code();
        mRefData = journalLine.ref_data();
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  void encodeTo(protos::JournalLine &journalLine) const {  // NOLINT[runtime/references]
    journalLine.set_version(mVersion);
    switch (mVersion) {
      case 1: {
        journalLine.set_transaction_type(TransactionTypeUtil::toType(mTransactionType));
        journalLine.set_nominal_code(mNominalCode);
        mAmount.encodeTo(*journalLine.mutable_amount());
        journalLine.set_currency_code(mCurrencyCode);
        journalLine.set_ref_data(mRefData);

        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  TransactionType type() const {
    return mTransactionType;
  }

  uint64_t nominalCode() const {
    return mNominalCode;
  }

  bool isSame(const JournalLine &another) const {
    if (mVersion != another.mVersion) {
      SPDLOG_WARN("version is not the same, {} vs {}", mVersion, another.mVersion);
      return false;
    }

    if (mVersion == 1) {
      if (mNominalCode != another.mNominalCode) {
        SPDLOG_WARN("nominal code is not the same, {} vs {}", mNominalCode, another.mNominalCode);
        return false;
      }
      if (mTransactionType != another.mTransactionType) {
        SPDLOG_WARN("journalLine type is not the same, {} vs {}", mTransactionType, another.mTransactionType);
        return false;
      }
      if (!mAmount.isSame(another.mAmount)) {
        SPDLOG_WARN("amount is not the same");
        return false;
      }
      if (mCurrencyCode != another.mCurrencyCode) {
        SPDLOG_WARN("currency code is not the same, {} vs {}", mCurrencyCode, another.mCurrencyCode);
        return false;
      }
      if (mRefData != another.mRefData) {
        SPDLOG_WARN("refdata is not the same, {} vs {}", mRefData, another.mRefData);
        return false;
      }
    } else {
      SPDLOG_ERROR("Cannot recognize this version {}", mVersion);
      return false;
    }

    return true;
  }

 private:
  uint64_t mVersion;  // keep every version for backward-compatibility
  uint64_t mNominalCode;  // unique account id in CoA
  TransactionType mTransactionType = TransactionType::Unknown;  // debit or credit
  Amount mAmount;
  uint64_t mCurrencyCode;  // ISO-4217 currency code
  std::string mRefData;  // ref data related to this journal line, e.g., receipt number, etc
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_JOURNALLINE_H_
