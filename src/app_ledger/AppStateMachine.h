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

#ifndef SRC_APP_LEDGER_APPSTATEMACHINE_H_
#define SRC_APP_LEDGER_APPSTATEMACHINE_H_

#include "should_be_generated/domain/commands/ConfigureAccountMetadataCommand.h"
#include "should_be_generated/domain/commands/CreateAccountCommand.h"
#include "should_be_generated/domain/commands/RecordJournalEntryCommand.h"
#include "should_be_generated/domain/events/AccountCreatedEvent.h"
#include "should_be_generated/domain/events/AccountMetadataConfiguredEvent.h"
#include "should_be_generated/domain/events/JournalEntryRecordedEvent.h"
#include "../app_util/AppStateMachine.h"

namespace gringofts {
namespace ledger {

class AppStateMachine : public gringofts::app::AppStateMachine {
 public:
  AppStateMachine();
  ~AppStateMachine() override = default;

  ProcessHint processCommandAndApply(const Command &command, std::vector<std::shared_ptr<Event>> *events) override;

 protected:
  virtual ProcessHint process(const CreateAccountCommand &command,
                              std::vector<std::shared_ptr<Event>> *events) const = 0;

  virtual StateMachine &apply(const AccountCreatedEvent &event) = 0;

  virtual ProcessHint process(const ConfigureAccountMetadataCommand &command,
                              std::vector<std::shared_ptr<Event>> *events) const = 0;

  virtual StateMachine &apply(const AccountMetadataConfiguredEvent &event) = 0;

  virtual ProcessHint process(const RecordJournalEntryCommand &command,
                              std::vector<std::shared_ptr<Event>> *events) const = 0;

  virtual StateMachine &apply(const JournalEntryRecordedEvent &event) = 0;
};

}  /// namespace ledger
}  /// namespace gringofts

#endif  // SRC_APP_LEDGER_APPSTATEMACHINE_H_
