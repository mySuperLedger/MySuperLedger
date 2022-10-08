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

#include "AccountType.h"
#include "Balance.h"

namespace gringofts {
namespace ledger {

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
        mAccountType = AccountTypeUtil::typeOf(account.type());
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

  void encodeTo(protos::Account &account) const {  // NOLINT[runtime/references]
    account.set_version(mVersion);
    switch (mVersion) {
      case 1: {
        account.set_type(AccountTypeUtil::toType(mAccountType));
        mBalance.encodeTo(*account.mutable_balance());
        account.set_nominal_code(mNominalCode);
        account.set_name(mName);
        account.set_desc(mDesc);
        account.set_currency_code(mCurrencyCode);
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  uint64_t nominalCode() const {
    return mNominalCode;
  }

  bool isSame(const Account &another) const {
    if (mVersion != another.mVersion) {
      SPDLOG_WARN("version is not the same, {} vs {}", mVersion, another.mVersion);
      return false;
    }

    if (mVersion == 1) {
      if (mAccountType != another.mAccountType) {
        SPDLOG_WARN("account type is not the same, {} vs {}", mAccountType, another.mAccountType);
        return false;
      }
      if (!mBalance.isSame(another.mBalance)) {
        SPDLOG_WARN("balance is not the same");
        return false;
      }
      if (mNominalCode != another.mNominalCode) {
        SPDLOG_WARN("nominal code is not the same, {} vs {}", mNominalCode, another.mNominalCode);
        return false;
      }
      if (mName != another.mName) {
        SPDLOG_WARN("name is not the same, {} vs {}", mName, another.mName);
        return false;
      }
      if (mDesc != another.mDesc) {
        SPDLOG_WARN("desc is not the same, {} vs {}", mDesc, another.mDesc);
        return false;
      }
      if (mCurrencyCode != another.mCurrencyCode) {
        SPDLOG_WARN("currency code is not the same, {} vs {}", mCurrencyCode, another.mCurrencyCode);
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
