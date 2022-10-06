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

#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_EVENTS_ACCOUNTCREATEDEVENT_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_EVENTS_ACCOUNTCREATEDEVENT_H_

#include "../../../../infra/es/Event.h"
#include "../../../generated/grpc/ledger.pb.h"
#include "../Account.h"
#include "../common_types.h"

namespace gringofts {
namespace ledger {

class AccountCreatedEvent : public Event {
 public:
  explicit AccountCreatedEvent(TimestampInNanos createdTimeInNanos, const Account &account);

  AccountCreatedEvent(TimestampInNanos createdTimeInNanos, std::string_view eventStr);

  std::string encodeToString() const override;

  void decodeFromString(std::string_view payload) override;

  const Account &account() const {
    return mAccount;
  }

 private:
  uint64_t mVersion = 1;
  Account mAccount;
};

}  /// namespace ledger
}  /// namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_EVENTS_ACCOUNTCREATEDEVENT_H_
