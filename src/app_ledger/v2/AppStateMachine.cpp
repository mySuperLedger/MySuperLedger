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

#include "AppStateMachine.h"
#include "../../infra/util/HttpCode.h"
#include "../should_be_generated/domain/BusinessCode.h"

namespace gringofts {
namespace ledger {
namespace v2 {

ProcessHint AppStateMachine::process(const CreateAccountCommand &command,
                                     std::vector<std::shared_ptr<Event>> *events) const {
  ProcessHint hint;
  hint.mCode = HttpCode::OK;
  hint.mMessage = "Success";

  auto accountOpt = command.accountOpt();
  if (accountOpt) {
    const auto &account = *accountOpt;
    if (mCoA.find(account.nominalCode()) != mCoA.end()) {
      hint.mCode = BusinessCode::ALREADY_EXISTS;
      hint.mMessage = "Account already exists";
    } else {
      auto now = TimeUtil::currentTimeInNanos();
      auto event = std::make_shared<AccountCreatedEvent>(now, account);
      events->push_back(std::move(event));
    }
  } else {
    hint.mCode = HttpCode::BAD_REQUEST;
    hint.mMessage = "Invalid command as no account passed";
  }

  return hint;
}

StateMachine &AppStateMachine::apply(const gringofts::ledger::AccountCreatedEvent &event) {
  const auto &account = event.account();
  auto nominalCode = account.nominalCode();
  assert(mCoA.find(nominalCode) == mCoA.end());
  mCoA[nominalCode] = account;
  onAccountInserted(account);

  return *this;
}

}  // namespace v2
}  // namespace ledger
}  // namespace gringofts
