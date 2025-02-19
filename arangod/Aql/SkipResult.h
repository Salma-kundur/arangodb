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
/// @author Michael Hackstein
////////////////////////////////////////////////////////////////////////////////

#pragma once

// for size_t
#include <cstddef>
#include <ostream>
#include <vector>

namespace arangodb {
template<typename T>
class ResultT;
}
namespace arangodb::velocypack {
class Builder;
class Slice;
}  // namespace arangodb::velocypack

namespace arangodb::aql {

class SkipResult {
 public:
  static auto fromVelocyPack(velocypack::Slice)
      -> arangodb::ResultT<SkipResult>;

  SkipResult() = default;

  SkipResult(SkipResult const& other) = default;
  SkipResult& operator=(const SkipResult&) = default;
  SkipResult(SkipResult&& other) = default;
  SkipResult& operator=(SkipResult&&) = default;

  auto getSkipCount() const noexcept -> size_t;

  auto didSkip(size_t skipped) -> void;

  // depthOffset is added to depth, except it won't underflow.
  template<int depthOffset = 0>
  requires(depthOffset == 0 || depthOffset == -1)  //
      auto didSkipSubquery(size_t skipped, size_t depth) -> void;

  auto getSkipOnSubqueryLevel(size_t depth) const -> size_t;

  auto nothingSkipped() const noexcept -> bool;

  auto toVelocyPack(arangodb::velocypack::Builder& builder) const noexcept
      -> void;

  auto incrementSubquery() -> void;

  auto decrementSubquery() -> void;

  auto subqueryDepth() const noexcept -> size_t;

  auto reset() -> void;

  auto merge(SkipResult const& other, bool excludeTopLevel) noexcept -> void;
  auto mergeOnlyTopLevel(SkipResult const& other) noexcept -> void;

  auto operator+=(SkipResult const& b) noexcept -> SkipResult&;

  auto operator==(SkipResult const& b) const noexcept -> bool;
  auto operator!=(SkipResult const& b) const noexcept -> bool;

 private:
  std::vector<size_t> _skipped{0};
};

std::ostream& operator<<(std::ostream&, arangodb::aql::SkipResult const&);

}  // namespace arangodb::aql
