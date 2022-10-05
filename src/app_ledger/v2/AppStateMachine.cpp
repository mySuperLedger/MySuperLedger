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

namespace gringofts {
namespace ledger {
namespace v2 {

ProcessHint AppStateMachine::process(const CreateAccountCommand &command,
                                     std::vector<std::shared_ptr<Event>> *events) const {
  ProcessHint hint;
  return hint;
}

StateMachine &AppStateMachine::apply(const gringofts::ledger::AccountCreatedEvent &event) {
  return *this;
}

}  // namespace v2
}  // namespace ledger
}  // namespace gringofts
