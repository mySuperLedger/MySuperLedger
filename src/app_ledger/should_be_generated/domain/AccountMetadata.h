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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTMETADATA_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTMETADATA_H_

#include "AccountType.h"

namespace gringofts {
namespace ledger {

class AccountMetadata {
 public:
  AccountMetadata() = default;
  explicit AccountMetadata(const protos::AccountMetadata &accountMetadata) {
    initWith(accountMetadata);
  }

  void initWith(const protos::AccountMetadata &accountMetadata) {
    mVersion = accountMetadata.version();
    assert(mVersion > 0);
    switch (mVersion) {
      case 1: {
        mAccountType = AccountTypeUtil::typeOf(accountMetadata.type());
        mLowInclusive = accountMetadata.low_inclusive();
        mHighInclusive = accountMetadata.high_inclusive();
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  void encodeTo(protos::AccountMetadata &accountMetadata) const {  // NOLINT[runtime/references]
    accountMetadata.set_version(mVersion);
    switch (mVersion) {
      case 1: {
        accountMetadata.set_type(AccountTypeUtil::toType(mAccountType));
        accountMetadata.set_low_inclusive(mLowInclusive);
        accountMetadata.set_high_inclusive(mHighInclusive);
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  bool isSame(const AccountMetadata &another) const {
    if (mVersion != another.mVersion) {
      SPDLOG_WARN("version is not the same, {} vs {}", mVersion, another.mVersion);
      return false;
    }

    if (mVersion == 1) {
      if (mAccountType != another.mAccountType) {
        SPDLOG_WARN("account type is not the same, {} vs {}", mAccountType, another.mAccountType);
        return false;
      }
      if (mLowInclusive != another.mLowInclusive) {
        SPDLOG_WARN("low inclusive is not the same, {} vs {}", mLowInclusive, another.mLowInclusive);
        return false;
      }
      if (mHighInclusive != another.mHighInclusive) {
        SPDLOG_WARN("high inclusive is not the same, {} vs {}", mHighInclusive, another.mHighInclusive);
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
  uint64_t mLowInclusive;
  uint64_t mHighInclusive;
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTMETADATA_H_
