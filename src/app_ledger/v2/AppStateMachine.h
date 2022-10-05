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

#ifndef SRC_APP_LEDGER_V2_APPSTATEMACHINE_H_
#define SRC_APP_LEDGER_V2_APPSTATEMACHINE_H_

#include "../AppStateMachine.h"

namespace gringofts {
namespace ledger {
namespace v2 {

class AppStateMachine : public ledger::AppStateMachine {
 public:
  struct RocksDBConf {
    /// RocksDB key
    static constexpr const char *kLastAppliedIndexKey = "last_applied_index";
    static constexpr const char *kValueKey = "value";
  };

  /**
   * integration
   */
  void clearState() override { mValue = 0; }

  /// unit test
  bool hasSameState(const StateMachine &) const override { return true; }

 protected:
  /// command processors
  ProcessHint process(const CreateAccountCommand &command,
                      std::vector<std::shared_ptr<Event>> *events) const override;

  /// event appliers
  StateMachine &apply(const AccountCreatedEvent &event) override;

 protected:
  /// state owned by both Memory-backed SM and RocksDB-backed SM
  uint64_t mValue = 0;
};

}  /// namespace v2
}  /// namespace ledger
}  /// namespace gringofts

#endif  // SRC_APP_LEDGER_V2_APPSTATEMACHINE_H_
