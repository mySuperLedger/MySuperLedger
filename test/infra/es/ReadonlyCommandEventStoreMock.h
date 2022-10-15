/************************************************************************
Copyright 2019-2020 eBay Inc.
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

#ifndef TEST_INFRA_ES_READONLYCOMMANDEVENTSTOREMOCK_H_
#define TEST_INFRA_ES_READONLYCOMMANDEVENTSTOREMOCK_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../../src/infra/es/ReadonlyCommandEventStore.h"

namespace gringofts {

class ReadonlyCommandEventStoreMock : public ReadonlyCommandEventStore {
 public:
  ReadonlyCommandEventStoreMock() {}

  MOCK_METHOD(void, init, ());
  MOCK_METHOD(std::unique_ptr<Event>, loadNextEvent, (
      const EventDecoder &eventDecoder));
  MOCK_METHOD(std::unique_ptr<Command>, loadNextCommand, (
      const CommandDecoder &commandDecoder));

  MOCK_METHOD(std::unique_ptr<Command>, loadCommandAfter, (
      Id prevCommandId,
      const CommandDecoder &commandDecoder));

  MOCK_METHOD(CommandEventsOpt, loadNextCommandEvents, (
      const CommandDecoder &commandDecoder,
      const EventDecoder &eventDecoder));

  MOCK_METHOD(uint64_t, loadCommandEventsList, (
      const CommandDecoder &commandDecoder,
      const EventDecoder &eventDecoder,
      Id commandId,
      uint64_t size,
      CommandEventsList * bundles));

  MOCK_METHOD(uint64_t, getCurrentOffset, (), (const));

  MOCK_METHOD(void, setCurrentOffset, (uint64_t));

  MOCK_METHOD(void, truncatePrefix, (uint64_t));
};

}  /// namespace gringofts

#endif  // TEST_INFRA_ES_READONLYCOMMANDEVENTSTOREMOCK_H_
