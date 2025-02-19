////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2024 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Business Source License 1.1 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     https://github.com/arangodb/arangodb/blob/devel/LICENSE
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Kaveh Vahedipour
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Metrics/Metric.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace arangodb::metrics {

/**
 * @brief Counter functionality
 */
class Counter final : public Metric {
 public:
  Counter(uint64_t n, std::string_view name, std::string_view help,
          std::string_view labels);
  ~Counter() final;

  [[nodiscard]] std::string_view type() const noexcept final;
  void toPrometheus(std::string& result, std::string_view globals,
                    bool ensureWhitespace) const final;

  [[nodiscard]] uint64_t load() const noexcept;
  void store(uint64_t n) noexcept;
  void count(uint64_t n) noexcept;
  void count() noexcept;

  Counter& operator=(uint64_t n) noexcept;
  Counter& operator+=(uint64_t n) noexcept;
  Counter& operator++() noexcept;

  std::ostream& print(std::ostream& output) const;

  void toVPack(velocypack::Builder& builder) const override {}

 private:
  mutable CounterType _c;
  mutable BufferType _b;
};

}  // namespace arangodb::metrics
