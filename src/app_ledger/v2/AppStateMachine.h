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

#include <rocksdb/db.h>
#include <rocksdb/options.h>

#include "../AppStateMachine.h"

namespace gringofts {
namespace ledger {
namespace v2 {

class AppStateMachine : public ledger::AppStateMachine {
 public:
  struct RocksDBConf {
    /**
     * You should obey following order when preparing columnFamilyDescriptors,
     * so that you can manipulate mColumnFamilyHandles correctly.
     */
    enum ColumnFamilyIndices {
      DEFAULT = 0,
      CHART_OF_ACCOUNTS = 1,
      ACCOUNT_METADATA = 2,
      DONE_MAP = 3,
    };

    /// RocksDB key
    static constexpr const char *kLastAppliedIndexKey = "last_applied_index";

    /**
     * ColumnFamily Names
     */
    static constexpr const char *kDefault = "default";
    static constexpr const char *kChartOfAccounts = "chart_of_accounts";
    static constexpr const char *kAccountMetadata = "account_metadata";
    static constexpr const char *kDoneMap = "done_ids";
  };

  /**
   * integration
   */
  void clearState() override {
    mDoneMap.clear();
    mCoA.clear();
    mAccountMetadata.clear();
  }

  /// unit test
  bool hasSameState(const StateMachine &anotherStateMachine) const override {
    const auto &another = dynamic_cast<const v2::AppStateMachine &>(anotherStateMachine);
    /// do not compare doneMap as it will be lazy-loaded
    /// compare CoA
    if (mCoA.size() != another.mCoA.size()) {
      SPDLOG_WARN("CoA has different size. {} vs {}", mCoA.size(), another.mCoA.size());
      return false;
    }
    for (const auto &[k, v] : another.mCoA) {
      const auto &iter = mCoA.find(k);
      if (iter == mCoA.cend()) {
        SPDLOG_WARN("entry with key {} not found in map", k);
        return false;
      }
      if (!v.isSame(iter->second)) {
        SPDLOG_WARN("key {} have different values.", k);
        return false;
      }
    }
    for (const auto &[k, v] : another.mAccountMetadata) {
      const auto &iter = mAccountMetadata.find(k);
      if (iter == mAccountMetadata.cend()) {
        SPDLOG_WARN("entry with key {} not found in map", k);
        return false;
      }
      if (!v.isSame(iter->second)) {
        SPDLOG_WARN("key {} have different values.", k);
        return false;
      }
    }

    return true;
  }

 protected:
  /// command processors
  ProcessHint process(const ConfigureAccountMetadataCommand &command,
                      std::vector<std::shared_ptr<Event>> *events) const override;
  ProcessHint process(const CreateAccountCommand &command,
                      std::vector<std::shared_ptr<Event>> *events) const override;
  ProcessHint process(const RecordJournalEntryCommand &command,
                      std::vector<std::shared_ptr<Event>> *events) const override;

  /// event appliers
  StateMachine &apply(const AccountMetadataConfiguredEvent &event) override;
  StateMachine &apply(const AccountCreatedEvent &event) override;
  StateMachine &apply(const JournalEntryRecordedEvent &event) override;

  /// callbacks
  virtual void onAccountInserted(const Account &account) {}
  virtual void onAccountMetadataUpdated(const AccountMetadata &accountMetadata) {}
  virtual void onBookkeepingProcessed(std::string dedupId, uint64_t validTime) {}
  virtual void onAccountUpdated(const Account &account) {}

 protected:
  /// read-only rocksDB
  std::shared_ptr<rocksdb::DB> mRocksDB;
  /// column family handles
  std::vector<rocksdb::ColumnFamilyHandle *> mColumnFamilyHandles;
  /// ReadOptions hold a snapshot
  rocksdb::ReadOptions mReadOptions;
  /// state owned by both Memory-backed SM and RocksDB-backed SM
  /// key: dedupId, value: validTime
  /// TODO(ISSUE-20): only keep dedupIds no older than 6 months to make the rocksdb size consistent
  std::map<std::string, uint64_t> mDoneMap;
  /// key: account's nominalCode, value: account
  std::map<uint64_t, Account> mCoA;  /// Chart of Accounts
  /// key: accountType, value: metaData
  std::map<AccountType, AccountMetadata> mAccountMetadata;
};

}  /// namespace v2
}  /// namespace ledger
}  /// namespace gringofts

#endif  // SRC_APP_LEDGER_V2_APPSTATEMACHINE_H_
