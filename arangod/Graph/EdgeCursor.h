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

#include <cstdint>
#include <functional>
#include <string_view>

namespace arangodb {

namespace velocypack {
class Slice;
}

namespace graph {

struct EdgeDocumentToken;

/// @brief Abstract class used in the traversals
/// to abstract away access to indexes / DBServers.
/// Returns edges as VelocyPack.

class EdgeCursor {
 public:
  virtual ~EdgeCursor() = default;

  using Callback = std::function<void(EdgeDocumentToken&&,
                                      arangodb::velocypack::Slice, size_t)>;

  virtual bool next(
      std::function<void(EdgeDocumentToken&&, arangodb::velocypack::Slice,
                         size_t)> const& callback) = 0;

  virtual void readAll(
      std::function<void(EdgeDocumentToken&&, arangodb::velocypack::Slice,
                         size_t)> const& callback) = 0;

  virtual std::uint64_t httpRequests() const = 0;

  virtual void rearm(std::string_view vid, uint64_t depth) = 0;
};

}  // namespace graph
}  // namespace arangodb
