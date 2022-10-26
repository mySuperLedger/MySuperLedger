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

#ifndef SRC_APP_LEDGER_V2_MEMORYBACKEDAPPSTATEMACHINE_H_
#define SRC_APP_LEDGER_V2_MEMORYBACKEDAPPSTATEMACHINE_H_

#include <optional>

#include "../../infra/util/PMRContainerFactory.h"
#include "AppStateMachine.h"
#include "RocksDBBackedAppStateMachine.h"

namespace gringofts {
namespace ledger {
namespace v2 {

class MemoryBackedAppStateMachine : public v2::AppStateMachine {
 public:
  explicit MemoryBackedAppStateMachine(
      std::shared_ptr<gringofts::PMRContainerFactory> containerFactory) {}

  /**
   * integration
   */
  void swapState(StateMachine *anotherStateMachine) override {
    auto &another = dynamic_cast<RocksDBBackedAppStateMachine &>(*anotherStateMachine);
    std::swap(mDoneMap, another.mDoneMap);
    std::swap(mCoA, another.mCoA);
    std::swap(mAccountMetadata, another.mAccountMetadata);

    /// refresh ptr of RocksDB if needed.
    mRocksDB = another.mRocksDB;

    /// refresh column family handles if needed
    mColumnFamilyHandles = another.mColumnFamilyHandles;
    /// explicitly trigger a flush in RocksDB
    another.flushToRocksDB();
    /// take a snapshot
    mReadOptions.snapshot = mRocksDB->GetSnapshot();
  }
};

}  /// namespace v2
}  /// namespace ledger
}  /// namespace gringofts

#endif  // SRC_APP_LEDGER_V2_MEMORYBACKEDAPPSTATEMACHINE_H_
