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

class AccountTypeUtil final {
 public:
  static AccountType typeOf(protos::AccountType accountType) {
    switch (accountType) {
      case protos::AccountType::Unknown:return AccountType::Unknown;
      case protos::AccountType::Asset:return AccountType::Asset;
      case protos::AccountType::Liability:return AccountType::Liability;
      case protos::AccountType::Capital:return AccountType::Capital;
      case protos::AccountType::Income:return AccountType::Income;
      case protos::AccountType::CostOfGoodsSold:return AccountType::CostOfGoodsSold;
      case protos::AccountType::Expense:return AccountType::Expense;
      default: return AccountType::Unknown;
    }
  }

  static protos::AccountType toType(AccountType accountType) {
    switch (accountType) {
      case AccountType::Unknown: return protos::AccountType::Unknown;
      case AccountType::Asset: return protos::AccountType::Asset;
      case AccountType::Liability:return protos::AccountType::Liability;
      case AccountType::Capital:return protos::AccountType::Capital;
      case AccountType::Income:return protos::AccountType::Income;
      case AccountType::CostOfGoodsSold:return protos::AccountType::CostOfGoodsSold;
      case AccountType::Expense:return protos::AccountType::Expense;
      default: return protos::AccountType::Unknown;
    }
  }
};
}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_ACCOUNTTYPE_H_
