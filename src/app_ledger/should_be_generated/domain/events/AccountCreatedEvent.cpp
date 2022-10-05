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

#include "AccountCreatedEvent.h"

#include <spdlog/spdlog.h>

namespace gringofts {
namespace ledger {

AccountCreatedEvent::AccountCreatedEvent(TimestampInNanos createdTimeInNanos, const Account &account)
    : Event(ACCOUNT_CREATED_EVENT, createdTimeInNanos) {
  mAccount = account;
}

AccountCreatedEvent::AccountCreatedEvent(TimestampInNanos createdTimeInNanos, std::string_view eventStr)
    : Event(ACCOUNT_CREATED_EVENT, createdTimeInNanos) {
  /// TODO: init protos::Account from eventStr
  protos::Account account;
  mAccount.initWith(account);
  decodeFromString(eventStr);
}

std::string AccountCreatedEvent::encodeToString() const {
  return "";
}

void AccountCreatedEvent::decodeFromString(std::string_view payload) {
}

}  /// namespace ledger
}  /// namespace gringofts
