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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BALANCE_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BALANCE_H_

#include <stdint.h>

#include "../../generated/grpc/ledger.pb.h"

namespace gringofts {
namespace ledger {

class Balance {
 public:
  Balance() {
    mVersion = 1;
    mValue = 0;
  }

  explicit Balance(const protos::Balance &balance) {
    mVersion = balance.version();
    assert(mVersion > 0);
    switch (mVersion) {
      case 1: {
        mValue = balance.value();
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  void encodeTo(protos::Balance &balance) const {  // NOLINT[runtime/references]
    balance.set_version(mVersion);
    switch (mVersion) {
      case 1: {
        balance.set_value(mValue);
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  bool isSame(const Balance &another) const {
    if (mVersion != another.mVersion) {
      SPDLOG_WARN("version is not the same, {} vs {}", mVersion, another.mVersion);
      return false;
    }

    if (mVersion == 1) {
      if (mValue != another.mValue) {
        SPDLOG_WARN("value is not the same, {} vs {}", mValue, another.mValue);
        return false;
      }
    } else {
      SPDLOG_ERROR("Cannot recognize this version {}", mVersion);
      return false;
    }

    return true;
  }

  Balance &operator+=(const Amount &amount) {
    /// later to support multiple versions
    mValue += amount.value();

    return *this;
  }

  Balance &operator-=(const Amount &amount) {
    /// later to support multiple versions
    mValue -= amount.value();

    return *this;
  }

 private:
  uint64_t mVersion;  // keep every version for backward-compatibility
  uint64_t mValue;
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BALANCE_H_
