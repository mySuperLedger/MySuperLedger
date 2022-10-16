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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTTYPE_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTTYPE_H_

#include <spdlog/fmt/ostr.h>

#include "../../generated/grpc/ledger.pb.h"

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

std::ostream &operator<<(std::ostream &os, AccountType accountType);

class AccountTypeUtil final {
 public:
  static AccountType typeOf(protos::AccountType accountType);

  static protos::AccountType toType(AccountType accountType);
};
}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTTYPE_H_
