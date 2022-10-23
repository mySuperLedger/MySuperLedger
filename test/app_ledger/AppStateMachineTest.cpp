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

#include <gtest/gtest.h>
#include <INIReader.h>

#include "../../src/app_ledger/should_be_generated/domain/BusinessCode.h"
#include "../../src/app_ledger/v2/MemoryBackedAppStateMachine.h"
#include "../../src/app_util/AppInfo.h"
#include "../../src/infra/util/HttpCode.h"
#include "../../src/infra/util/PerfConfig.h"

namespace gringofts {
namespace ledger {

class LedgerAppStateMachineTest : public ::testing::Test {
 public:
  void SetUp() override {
    gringofts::Util::executeCmd("rm -rf ../test/app_ledger/data && mkdir ../test/app_ledger/data");
    mRocksDBBackedStateMachine = std::make_unique<v2::RocksDBBackedAppStateMachine>("../test/app_ledger/data/rocksdb",
                                                                                    "../test/app_ledger/data/rocksdb");
    auto configPath = "../test/app_ledger/config/app.ini";
    INIReader reader(configPath);
    if (reader.ParseError() < 0) {
      SPDLOG_WARN("Cannot load config file {}, exiting", configPath);
      assert(1);
    }
    app::AppInfo::init(reader);
    initMemoryPool();

    mInMemoryStateMachine = std::make_unique<v2::MemoryBackedAppStateMachine>(mFactory);

    mInMemoryStateMachine->swapState(mRocksDBBackedStateMachine.get());
    mRocksDBBackedStateMachine->recoverSelf();
  }

  void TearDown() override {
  }

 private:
  void initMemoryPool() {
    if (gringofts::PerfConfig::getInstance().getMemoryPoolType() == "monotonic") {
      mFactory = std::make_shared<gringofts::PMRContainerFactory>(
          "PMRFactory",
          std::make_unique<gringofts::MonotonicPMRMemoryPool>(
              "monotonicPool",
              gringofts::PerfConfig::getInstance().getMaxMemoryPoolSizeInMB()));
    } else {
      mFactory = std::make_shared<gringofts::PMRContainerFactory>(
          "PMRFactory",
          std::make_unique<gringofts::NewDeleteMemoryPool>(
              "newDeletePool"));
    }
  }

 protected:
  std::shared_ptr<CreateAccountCommand> createSampleCreateAccountCommand(protos::AccountType type,
                                                                         uint64_t nominalCode,
                                                                         uint64_t iso4217CurrencyCode) {
    protos::CreateAccount::Request request;
    request.mutable_account()->set_version(1);
    request.mutable_account()->set_type(type);
    request.mutable_account()->set_nominal_code(nominalCode);
    request.mutable_account()->set_name("creditCard");
    request.mutable_account()->set_desc("Shanghai credit card");
    request.mutable_account()->set_iso4217_currency_code(iso4217CurrencyCode);
    request.mutable_account()->mutable_balance()->set_version(1);
    request.mutable_account()->mutable_balance()->set_value(100);
    auto createdTimeInNanos = TimeUtil::currentTimeInNanos();
    auto command = std::make_shared<CreateAccountCommand>(createdTimeInNanos, request);
    command->setRequestHandle(nullptr);
    command->setCreatorId(app::AppInfo::subsystemId());
    command->setGroupId(app::AppInfo::groupId());
    command->setGroupVersion(app::AppInfo::groupVersion());

    return command;
  }

  std::shared_ptr<ConfigureAccountMetadataCommand> createSampleConfigureAccountMetadataCommand(protos::AccountType type,
                                                                                               uint64_t lowInclusive,
                                                                                               uint64_t highInclusive) {
    protos::ConfigureAccountMetadata::Request request;
    request.mutable_metadata()->set_version(1);
    request.mutable_metadata()->set_type(type);
    request.mutable_metadata()->set_low_inclusive(lowInclusive);
    request.mutable_metadata()->set_high_inclusive(highInclusive);
    auto createdTimeInNanos = TimeUtil::currentTimeInNanos();
    auto command = std::make_shared<ConfigureAccountMetadataCommand>(createdTimeInNanos, request);
    command->setRequestHandle(nullptr);
    command->setCreatorId(app::AppInfo::subsystemId());
    command->setGroupId(app::AppInfo::groupId());
    command->setGroupVersion(app::AppInfo::groupVersion());

    return command;
  }

  JournalLine createSampleV1JournalLine(uint64_t nominalCode,
                                        TransactionType transactionType,
                                        Amount amount,
                                        uint64_t iso4217CurrencyCode,
                                        std::string refData) {
    protos::JournalLine journalLineProto;
    journalLineProto.set_version(1);
    journalLineProto.set_nominal_code(nominalCode);
    journalLineProto.set_transaction_type(TransactionTypeUtil::toType(transactionType));
    amount.encodeTo(*journalLineProto.mutable_amount());
    journalLineProto.set_iso4217_currency_code(iso4217CurrencyCode);
    journalLineProto.set_ref_data(refData);

    return JournalLine(journalLineProto);
  }

  std::shared_ptr<RecordJournalEntryCommand> createSampleV1RecordJournalEntryCommand(
      const std::string &id,
      std::vector<JournalLine> &journalLines) {
    auto now = TimeUtil::currentTimeInNanos();
    protos::JournalEntry journalEntryProto;
    journalEntryProto.set_version(1);
    journalEntryProto.set_id(id);
    journalEntryProto.set_valid_time(now);
    journalEntryProto.set_record_time(now);
    for (const auto &journalLine : journalLines) {
      journalLine.encodeTo(*journalEntryProto.add_journal_lines());
    }
    journalEntryProto.set_purpose("test");

    protos::RecordJournalEntry::Request request;
    *request.mutable_journal_entry() = journalEntryProto;

    auto command = std::make_shared<RecordJournalEntryCommand>(now, request);
    command->setRequestHandle(nullptr);
    command->setCreatorId(app::AppInfo::subsystemId());
    command->setGroupId(app::AppInfo::groupId());
    command->setGroupVersion(app::AppInfo::groupVersion());

    return command;
  }

  std::shared_ptr<gringofts::PMRContainerFactory> mFactory;
  std::unique_ptr<v2::RocksDBBackedAppStateMachine> mRocksDBBackedStateMachine;
  std::unique_ptr<v2::MemoryBackedAppStateMachine> mInMemoryStateMachine;
};

TEST_F(LedgerAppStateMachineTest, CreateAccount) {
  /// 1. arrange
  auto command = createSampleCreateAccountCommand(protos::AccountType::Asset, 1000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events;

  /// 2. act
  auto result = mInMemoryStateMachine->processCommandAndApply(*command, &events);
  for (const auto &event : events) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, HttpCode::OK);
  EXPECT_EQ(events.size(), 1);
  const auto &event = *(events[0]);
  ASSERT_EQ(&typeid(AccountCreatedEvent), &typeid(event));
  const auto &accountCreatedEvent = dynamic_cast<const AccountCreatedEvent &>(event);
  const auto &accountFromCommandOpt = command->accountOpt();
  EXPECT_TRUE(accountFromCommandOpt);
  const auto &accountFromCommand = *accountFromCommandOpt;
  const auto &accountFromEvent = accountCreatedEvent.account();
  EXPECT_TRUE(accountFromCommand.isSame(accountFromEvent));

  /// flush to disk and re-init the state from persisted
  mRocksDBBackedStateMachine->flushToRocksDB();
  mRocksDBBackedStateMachine->recoverSelf();
  EXPECT_TRUE(mInMemoryStateMachine->hasSameState(*mRocksDBBackedStateMachine));
}

TEST_F(LedgerAppStateMachineTest, CreateExistingAccount) {
  /// 1. arrange
  auto command = createSampleCreateAccountCommand(protos::AccountType::Asset, 1000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events;
  /// create an account
  mInMemoryStateMachine->processCommandAndApply(*command, &events);
  for (const auto &event : events) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// act
  /// try to create an account that already exists
  auto command1 = createSampleCreateAccountCommand(protos::AccountType::Liability, 1000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events1;
  /// create an account
  auto result = mInMemoryStateMachine->processCommandAndApply(*command1, &events1);
  for (const auto &event : events1) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, BusinessCode::ACCOUNT_ALREADY_EXISTS);
  EXPECT_TRUE(events1.empty());
}

TEST_F(LedgerAppStateMachineTest, ConfigureAccountMetadata) {
  /// 1. arrange
  /// create an account
  auto command1 = createSampleCreateAccountCommand(protos::AccountType::Asset, 1000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events1;
  mInMemoryStateMachine->processCommandAndApply(*command1, &events1);
  for (const auto &event : events1) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// act
  /// create an account metadata
  auto command2 = createSampleConfigureAccountMetadataCommand(protos::AccountType::Asset, 10, 2000);
  std::vector<std::shared_ptr<gringofts::Event>> events2;
  auto result = mInMemoryStateMachine->processCommandAndApply(*command2, &events2);
  for (const auto &event : events2) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, HttpCode::OK);
  EXPECT_EQ(events2.size(), 1);
  const auto &event = *(events2[0]);
  ASSERT_EQ(&typeid(AccountMetadataConfiguredEvent), &typeid(event));
  const auto &accountMetadataConfiguredEvent = dynamic_cast<const AccountMetadataConfiguredEvent &>(event);
  const auto &accountMetadataFromCommandOpt = command2->accountMetadataOpt();
  EXPECT_TRUE(accountMetadataFromCommandOpt);
  const auto &accountMetadataFromCommand = *accountMetadataFromCommandOpt;
  const auto &accountMetadataFromEvent = accountMetadataConfiguredEvent.accountMetadata();
  EXPECT_TRUE(accountMetadataFromCommand.isSame(accountMetadataFromEvent));

  /// flush to disk and re-init the state from persisted
  mRocksDBBackedStateMachine->flushToRocksDB();
  mRocksDBBackedStateMachine->recoverSelf();
  EXPECT_TRUE(mInMemoryStateMachine->hasSameState(*mRocksDBBackedStateMachine));
}

TEST_F(LedgerAppStateMachineTest, AccountMetadataNotCoverExistingAccountNominalCode) {
  /// 1. arrange
  /// create an account
  auto command1 = createSampleCreateAccountCommand(protos::AccountType::Asset, 1000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events1;
  mInMemoryStateMachine->processCommandAndApply(*command1, &events1);
  for (const auto &event : events1) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// act
  /// create an account metadata
  auto command2 = createSampleConfigureAccountMetadataCommand(protos::AccountType::Asset, 10, 900);
  std::vector<std::shared_ptr<gringofts::Event>> events2;
  auto result = mInMemoryStateMachine->processCommandAndApply(*command2, &events2);
  for (const auto &event : events2) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, BusinessCode::ACCOUNT_METADATA_RANGE_NOT_COVER_EXISTING_ACCOUNT);
  EXPECT_TRUE(events2.empty());
}

TEST_F(LedgerAppStateMachineTest, AccountMetadataRangeOverlap) {
  /// 1. arrange
  /// create an account metadata
  auto command1 = createSampleConfigureAccountMetadataCommand(protos::AccountType::Expense, 10, 1000);
  std::vector<std::shared_ptr<gringofts::Event>> events1;
  mInMemoryStateMachine->processCommandAndApply(*command1, &events1);
  for (const auto &event : events1) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// act
  /// create another account metadata whose range overlaps
  auto command2 = createSampleConfigureAccountMetadataCommand(protos::AccountType::Asset, 50, 900);
  std::vector<std::shared_ptr<gringofts::Event>> events2;
  auto result = mInMemoryStateMachine->processCommandAndApply(*command2, &events2);
  for (const auto &event : events2) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, BusinessCode::ACCOUNT_METADATA_RANGE_OVERLAP);
  EXPECT_TRUE(events2.empty());
}

TEST_F(LedgerAppStateMachineTest, BookkeepingOneJournalEntry) {
  /// 1. arrange
  /// create two accounts
  auto command1 = createSampleCreateAccountCommand(protos::AccountType::Asset, 1000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events1;
  mInMemoryStateMachine->processCommandAndApply(*command1, &events1);
  for (const auto &event : events1) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }
  auto command2 = createSampleCreateAccountCommand(protos::AccountType::Liability, 2000, 156);
  std::vector<std::shared_ptr<gringofts::Event>> events2;
  mInMemoryStateMachine->processCommandAndApply(*command2, &events2);
  for (const auto &event : events2) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// act
  /// create two journal lines
  protos::Amount amountProto;
  amountProto.set_version(1);
  amountProto.set_value(500);
  auto journalLine1 = createSampleV1JournalLine(1000, TransactionType::Debit, Amount(amountProto), 156, "refdata1");
  amountProto.set_value(500);
  auto journalLine2 = createSampleV1JournalLine(2000, TransactionType::Credit, Amount(amountProto), 156, "refdata2");
  std::vector<JournalLine> journalLines;
  journalLines.push_back(journalLine1);
  journalLines.push_back(journalLine2);
  auto command3 = createSampleV1RecordJournalEntryCommand("dedup1", journalLines);
  std::vector<std::shared_ptr<gringofts::Event>> events3;
  auto result = mInMemoryStateMachine->processCommandAndApply(*command3, &events3);
  for (const auto &event : events3) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, HttpCode::OK);
  EXPECT_EQ(events3.size(), 1);
  const auto &event = *(events3[0]);
  ASSERT_EQ(&typeid(JournalEntryRecordedEvent), &typeid(event));
  const auto &journalEntryRecordedEvent = dynamic_cast<const JournalEntryRecordedEvent &>(event);
  const auto &journalEntryFromCommandOpt = command3->journalEntryOpt();
  EXPECT_TRUE(journalEntryFromCommandOpt);
  const auto &journalEntryFromCommand = *journalEntryFromCommandOpt;
  const auto &journalEntryFromEvent = journalEntryRecordedEvent.journalEntry();
  EXPECT_TRUE(journalEntryFromCommand.isSame(journalEntryFromEvent));

  /// flush to disk and re-init the state from persisted
  mRocksDBBackedStateMachine->flushToRocksDB();
  mRocksDBBackedStateMachine->recoverSelf();
  EXPECT_TRUE(mInMemoryStateMachine->hasSameState(*mRocksDBBackedStateMachine));
}

}  // namespace ledger
}  // namespace gringofts
