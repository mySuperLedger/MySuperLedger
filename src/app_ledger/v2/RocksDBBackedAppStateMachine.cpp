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

void RocksDBBackedAppStateMachine::setValue(uint64_t value) {
  /// save in memory
  mValue = value;

  /// save in RocksDB
  auto status = mWriteBatch.Put(RocksDBConf::kValueKey, std::to_string(value));
  if (!status.ok()) {
    SPDLOG_ERROR("Error writing RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }
}

uint64_t RocksDBBackedAppStateMachine::getValue() const {
  return mValue;
}

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
  auto status = mWriteBatch.Put(RocksDBConf::kLastAppliedIndexKey, std::to_string(appliedIndex));
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

  SPDLOG_INFO("close RocksDB, timeCost: {}ms");
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

  /// load value
  status = mRocksDB->Get(rocksdb::ReadOptions(),
                         mColumnFamilyHandles[RocksDBConf::DEFAULT],
                         RocksDBConf::kValueKey, &value);
  if (status.ok()) {
    mValue = std::stoull(value);
  } else if (status.IsNotFound()) {
    mValue = 0;
  } else {
    SPDLOG_ERROR("Error in RocksDB: {}. Exiting...", status.ToString());
    assert(0);
  }
}

}  /// namespace v2
}  /// namespace ledger
}  /// namespace gringofts
