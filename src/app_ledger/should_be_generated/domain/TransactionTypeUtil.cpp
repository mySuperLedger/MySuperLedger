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

#include "TransactionType.h"

namespace gringofts {
namespace ledger {

std::ostream &operator<<(std::ostream &os, TransactionType accountType) {
  switch (accountType) {
    case TransactionType::Unknown: return os << "Unknown";
    case TransactionType::Debit: return os << "Debit";
    case TransactionType::Credit:return os << "Credit";
    default: return os << "Unknown";
  }
}

TransactionType TransactionTypeUtil::typeOf(protos::TransactionType transactionType) {
  switch (transactionType) {
    case protos::TransactionType::UnknownTransactionType:return TransactionType::Unknown;
    case protos::TransactionType::Debit:return TransactionType::Debit;
    case protos::TransactionType::Credit:return TransactionType::Credit;
    default: return TransactionType::Unknown;
  }
}

protos::TransactionType TransactionTypeUtil::toType(TransactionType transactionType) {
  switch (transactionType) {
    case TransactionType::Unknown: return protos::TransactionType::UnknownTransactionType;
    case TransactionType::Debit: return protos::TransactionType::Debit;
    case TransactionType::Credit:return protos::TransactionType::Credit;
    default: return protos::TransactionType::UnknownTransactionType;
  }
}
}  ///  namespace ledger
}  ///  namespace gringofts
