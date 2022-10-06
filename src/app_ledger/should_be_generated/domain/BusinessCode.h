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
  static constexpr int ALREADY_EXISTS = 1000;
};

}  // namespace ledger
}  // namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_BUSINESSCODE_H_
