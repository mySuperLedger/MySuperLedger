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

#include "RocksDBBackedAppStateMachine.h"

namespace gringofts {
namespace ledger {
namespace v2 {

uint64_t RocksDBBackedAppStateMachine::recoverSelf() {
  /// write batch should be empty.
  assert(mWriteBatch.Count() == 0);

  /// reload state from RocksDB
  clearState();
  loadFromRocksDB();

  SPDLOG_INFO("recovered from index {}.", mLastFlushedIndex);
  return mLastFlushedIndex;
}

void RocksDBBackedAppStateMachine::commit(uint64_t appliedIndex) {
  auto status = mWriteBatch.Put(mColumnFamilyHandles[RocksDBConf::DEFAULT],
                                RocksDBConf::kLastAppliedIndexKey, std::to_string(appliedIndex));
  if (!status.ok()) {
    SPDLOG_ERROR("Error writing RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }

  if (appliedIndex - mLastFlushedIndex < mMaxBatchSize) {
    return;
  }

  /// call flushToRocksDB() if needed
  flushToRocksDB();
  SPDLOG_INFO("flush range [{}, {}] to RocksDB", mLastFlushedIndex, appliedIndex);

  /// update lastFlushedIndex
  mLastFlushedIndex = appliedIndex;
}

void RocksDBBackedAppStateMachine::openRocksDB(const std::string &walDir,
                                               const std::string &dbDir,
                                               std::shared_ptr<rocksdb::DB> *dbPtr,
                                               std::vector<rocksdb::ColumnFamilyHandle *> *columnFamilyHandles) {
  /// options
  rocksdb::Options options;

  options.IncreaseParallelism();
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  options.wal_dir = walDir;

  /// column family options
  rocksdb::ColumnFamilyOptions columnFamilyOptions;

  /// default CompactionStyle for column family is kCompactionStyleLevel
  columnFamilyOptions.OptimizeLevelStyleCompaction();

  rocksdb::ColumnFamilyOptions smallColumnFamilyOptions;
  smallColumnFamilyOptions.OptimizeLevelStyleCompaction();
  smallColumnFamilyOptions.write_buffer_size = 32 << 20;
  smallColumnFamilyOptions.max_write_buffer_number = 2;

  rocksdb::ColumnFamilyOptions mediumColumnFamilyOptions;
  mediumColumnFamilyOptions.OptimizeLevelStyleCompaction();
  mediumColumnFamilyOptions.write_buffer_size = 64 << 20;
  mediumColumnFamilyOptions.max_write_buffer_number = 4;

  std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilyDescriptors;
  columnFamilyDescriptors.emplace_back(RocksDBConf::kDefault, smallColumnFamilyOptions);
  columnFamilyDescriptors.emplace_back(RocksDBConf::kChartOfAccounts, columnFamilyOptions);
  columnFamilyDescriptors.emplace_back(RocksDBConf::kAccountMetadata, columnFamilyOptions);
  columnFamilyDescriptors.emplace_back(RocksDBConf::kDoneMap, columnFamilyOptions);

  /// open DB
  rocksdb::DB *db;
  auto status = rocksdb::DB::Open(options, dbDir, columnFamilyDescriptors, columnFamilyHandles, &db);

  assert(status.ok());
  (*dbPtr).reset(db);

  SPDLOG_INFO("open RocksDB, wal.dir: {}, db.dir: {}", walDir, dbDir);
}

void RocksDBBackedAppStateMachine::closeRocksDB(std::shared_ptr<rocksdb::DB> *dbPtr) {
  while (!(*dbPtr).unique()) {
    usleep(1);
  }

  /// close DB
  /// dbPtr should be the last shared_ptr pointing to DB, we leverage it to delete DB.
  (*dbPtr).reset();

  SPDLOG_INFO("RocksDB closed");
}

void RocksDBBackedAppStateMachine::flushToRocksDB() {
  rocksdb::WriteOptions writeOptions;
  writeOptions.sync = true;

  auto status = mRocksDB->Write(writeOptions, &mWriteBatch);
  if (!status.ok()) {
    SPDLOG_ERROR("failed to write RocksDB, reason: {}", status.ToString());
    assert(0);
  }

  /// clear write batch since we will reuse it.
  mWriteBatch.Clear();
}

void RocksDBBackedAppStateMachine::loadFromRocksDB() {
  std::string value;

  /// load last applied index
  auto status = mRocksDB->Get(rocksdb::ReadOptions(),
                              mColumnFamilyHandles[RocksDBConf::DEFAULT],
                              RocksDBConf::kLastAppliedIndexKey, &value);
  if (status.ok()) {
    mLastFlushedIndex = std::stoull(value);
  } else if (status.IsNotFound()) {
    mLastFlushedIndex = 0;
  } else {
    SPDLOG_ERROR("Error in RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }

  /// load CoA
  rocksdb::Iterator *accountsIter = mRocksDB->NewIterator(rocksdb::ReadOptions(),
                                                          mColumnFamilyHandles[RocksDBConf::CHART_OF_ACCOUNTS]);
  for (accountsIter->SeekToFirst(); accountsIter->Valid(); accountsIter->Next()) {
    const auto &key = accountsIter->key();
    const auto &val = accountsIter->value();
    protos::Account accountProto;
    accountProto.ParseFromString(val.ToString());
    Account account;
    account.initWith(accountProto);
    auto nominalCode = account.nominalCode();
    assert(nominalCode == std::stoull(key.ToString()));
    assert(mCoA.find(nominalCode) == mCoA.end());
    mCoA[nominalCode] = account;
  }
  assert(accountsIter->status().ok());
  delete accountsIter;

  /// load AccountMetadata
  rocksdb::Iterator *accountMetadataIter = mRocksDB->NewIterator(rocksdb::ReadOptions(),
                                                                 mColumnFamilyHandles[RocksDBConf::ACCOUNT_METADATA]);
  for (accountMetadataIter->SeekToFirst(); accountMetadataIter->Valid(); accountMetadataIter->Next()) {
    const auto &key = accountMetadataIter->key();
    const auto &val = accountMetadataIter->value();
    protos::AccountMetadata accountMetadataProto;
    accountMetadataProto.ParseFromString(val.ToString());
    AccountMetadata accountMetadata;
    accountMetadata.initWith(accountMetadataProto);
    auto type = accountMetadata.accountType();
    assert(static_cast<int>(type) == std::stoi(key.ToString()));
    assert(mAccountMetadata.find(type) == mAccountMetadata.end());
    mAccountMetadata[type] = accountMetadata;
  }
  assert(accountMetadataIter->status().ok());
  delete accountMetadataIter;

  /// do not load doneMap as it may become too large and cost more time
}

void RocksDBBackedAppStateMachine::onAccountMetadataUpdated(const AccountMetadata &accountMetadata) {
  rocksdb::ReadOptions readOptions;

  auto type = static_cast<int>(accountMetadata.accountType());
  protos::AccountMetadata accountMetadataProto;
  accountMetadata.encodeTo(accountMetadataProto);
  auto status = mWriteBatch.Put(mColumnFamilyHandles[RocksDBConf::ACCOUNT_METADATA],
                                std::to_string(type),
                                rocksdb::Slice(accountMetadataProto.SerializeAsString()));
  if (!status.ok()) {
    SPDLOG_ERROR("Error writing RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }
}

void RocksDBBackedAppStateMachine::onAccountInserted(const Account &account) {
  rocksdb::ReadOptions readOptions;

  auto nominalCode = account.nominalCode();
  protos::Account accountProto;
  account.encodeTo(accountProto);
  auto status = mWriteBatch.Put(mColumnFamilyHandles[RocksDBConf::CHART_OF_ACCOUNTS],
                                std::to_string(nominalCode),
                                rocksdb::Slice(accountProto.SerializeAsString()));
  if (!status.ok()) {
    SPDLOG_ERROR("Error writing RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }
}

void RocksDBBackedAppStateMachine::onAccountUpdated(const gringofts::ledger::Account &account) {
  rocksdb::ReadOptions readOptions;

  auto nominalCode = account.nominalCode();
  /// account must exist either in write batch or on-disk rocksdb
  protos::Account accountProto;
  account.encodeTo(accountProto);
  auto status = mWriteBatch.Put(mColumnFamilyHandles[RocksDBConf::CHART_OF_ACCOUNTS],
                                std::to_string(nominalCode),
                                rocksdb::Slice(accountProto.SerializeAsString()));
  if (!status.ok()) {
    SPDLOG_ERROR("Error writing RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }
}

void RocksDBBackedAppStateMachine::onBookkeepingProcessed(std::string dedupId, uint64_t validTime) {
  rocksdb::ReadOptions readOptions;

  auto status = mWriteBatch.Put(mColumnFamilyHandles[RocksDBConf::DONE_MAP],
                                dedupId,
                                std::to_string(validTime));
  if (!status.ok()) {
    SPDLOG_ERROR("Error writing RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }
}

}  /// namespace v2
}  /// namespace ledger
}  /// namespace gringofts
