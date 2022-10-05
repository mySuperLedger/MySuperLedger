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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNT_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNT_H_

#include "Balance.h"

namespace gringofts {
namespace ledger {

enum class AccountType {
  Unknown = 0,
  Asset = 1,
  Liability = 2,
  Capital = 3,
  Income = 4,
  CostOfGoodsSold = 5,
  Expense = 6,
};

class Account {
 public:
  Account() = default;
  explicit Account(const protos::Account &account) {
    initWith(account);
  }

  void initWith(const protos::Account &account) {
    mVersion = account.version();
    assert(mVersion > 0);
    switch (mVersion) {
      case 1: {
        mAccountType = typeOf(account.type());
        mBalance = Balance(account.balance());
        mNominalCode = account.nominal_code();
        mName = account.name();
        mDesc = account.desc();
        mCurrencyCode = account.currency_code();
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

 private:
  AccountType typeOf(protos::AccountType accountType) const {
    switch (accountType) {
      case protos::AccountType::Unknown:return AccountType::Unknown;
      case protos::AccountType::Asset:return AccountType::Asset;
      case protos::AccountType::Liability:return AccountType::Liability;
      case protos::AccountType::Capital:return AccountType::Capital;
      case protos::AccountType::Income:return AccountType::Income;
      case protos::AccountType::CostOfGoodsSold:return AccountType::CostOfGoodsSold;
      case protos::AccountType::Expense:return AccountType::Expense;
      default: {
        SPDLOG_ERROR("Cannot recognize this type {}, exiting now", accountType);
        exit(1);
      }
    }
  }

 private:
  uint64_t mVersion;  // keep every version for backward-compatibility
  AccountType mAccountType = AccountType::Unknown;
  uint64_t mNominalCode;  // unique id in CoA
  std::string mName;  // a short name without no space
  std::string mDesc;  // a long description to explain what the account is for
  uint64_t mCurrencyCode;  // ISO-4217 currency code
  Balance mBalance;  // how much left in this account
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNT_H_
