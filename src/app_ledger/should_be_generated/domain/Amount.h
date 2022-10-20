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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_AMOUNT_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_AMOUNT_H_

#include <stdint.h>

#include "../../generated/grpc/ledger.pb.h"

namespace gringofts {
namespace ledger {

class Amount {
 public:
  Amount() {
    mVersion = 1;
    mValue = 0;
  }

  explicit Amount(const protos::Amount &amount) {
    mVersion = amount.version();
    assert(mVersion > 0);
    switch (mVersion) {
      case 1: {
        mValue = amount.value();
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  void encodeTo(protos::Amount &amount) const {  // NOLINT[runtime/references]
    amount.set_version(mVersion);
    switch (mVersion) {
      case 1: {
        amount.set_value(mValue);
        break;
      }
      default: {
        SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
        exit(1);
      }
    }
  }

  bool isSame(const Amount &another) const {
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

  Amount &operator+=(const Amount &other) {
    /// later to support multiple versions
    assert(other.mVersion == mVersion);
    mValue += other.mValue;

    return *this;
  }

  bool operator==(const Amount &rhs) const {
    return mVersion == rhs.mVersion && mValue == rhs.mValue;
  }

  bool operator!=(const Amount &rhs) const {
    return !(*this == rhs);
  }

 private:
  uint64_t mVersion;  // keep every version for backward-compatibility
  uint64_t mValue;
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_AMOUNT_H_
