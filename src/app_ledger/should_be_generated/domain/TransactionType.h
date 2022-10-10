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
#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_TRANSACTIONTYPE_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_TRANSACTIONTYPE_H_

#include "../../generated/grpc/ledger.pb.h"

namespace gringofts {
namespace ledger {

enum class TransactionType {
  Unknown = 0,
  Debit = 1,
  Credit = 2,
};

class TransactionTypeUtil final {
 public:
  static TransactionType typeOf(protos::TransactionType transactionType) {
    switch (transactionType) {
      case protos::TransactionType::UnknownTransactionType:return TransactionType::Unknown;
      case protos::TransactionType::Debit:return TransactionType::Debit;
      case protos::TransactionType::Credit:return TransactionType::Credit;
      default: return TransactionType::Unknown;
    }
  }

  static protos::TransactionType toType(TransactionType transactionType) {
    switch (transactionType) {
      case TransactionType::Unknown: return protos::TransactionType::UnknownTransactionType;
      case TransactionType::Debit: return protos::TransactionType::Debit;
      case TransactionType::Credit:return protos::TransactionType::Credit;
      default: return protos::TransactionType::UnknownTransactionType;
    }
  }
};
}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_TRANSACTIONTYPE_H_
