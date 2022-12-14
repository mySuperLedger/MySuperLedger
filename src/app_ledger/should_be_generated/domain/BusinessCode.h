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

#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BUSINESSCODE_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BUSINESSCODE_H_

namespace gringofts {
namespace ledger {

/// starting with 1000 to avoid conflict with HttpCode
class BusinessCode {
 public:
  static constexpr int ACCOUNT_ALREADY_EXISTS = 1000;
  static constexpr int ACCOUNT_METADATA_RANGE_OVERLAP = 1001;
  static constexpr int ACCOUNT_METADATA_RANGE_NOT_COVER_EXISTING_ACCOUNT = 1002;
  static constexpr int JOURNAL_ENTRY_ALREADY_PROCESSED = 1003;
  static constexpr int ACCOUNT_IN_JOURNAL_ENTRY_NOT_EXIST = 1004;
  static constexpr int NO_DEBIT_IN_JOURNAL_ENTRY = 1005;
  static constexpr int NO_CREDIT_IN_JOURNAL_ENTRY = 1006;
  static constexpr int CREDIT_NOT_EQUAL_TO_DEBIT = 1007;
  static constexpr int INVALID_TRANSACTION_TYPE = 1008;
  static constexpr int JOURNAL_LINE_ISO_CURRENCY_CODE_DOES_NOT_MATCH_ACCOUNT = 1009;
};

}  // namespace ledger
}  // namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BUSINESSCODE_H_
