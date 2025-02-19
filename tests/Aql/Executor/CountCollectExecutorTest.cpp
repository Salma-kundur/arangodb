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
/// @author Heiko Kernbach
////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "AqlExecutorTestCase.h"
#include "TestLambdaExecutor.h"

#include "Aql/Executor/CountCollectExecutor.h"
#include "Aql/Executor/SubqueryEndExecutor.h"
#include "Aql/Executor/SubqueryStartExecutor.h"
#include "Aql/SingleRowFetcher.h"
#include "Aql/Stats.h"

using namespace arangodb;
using namespace arangodb::aql;

using LambdaExe = TestLambdaSkipExecutor;

namespace arangodb {
namespace tests {
namespace aql {

using CountCollectParamType = std::tuple<SplitType>;

class CountCollectExecutorTest
    : public AqlExecutorTestCaseWithParam<CountCollectParamType, false> {
 protected:
  auto MakeCountCollectRegisterInfos(RegisterCount outReg) -> RegisterInfos {
    return RegisterInfos({}, RegIdSet{static_cast<RegisterId::value_t>(outReg)},
                         outReg, outReg + 1, RegIdFlatSet{},
                         RegIdFlatSetStack{{}, {}});
  }

  auto MakeCountCollectExecutorInfos(RegisterId outReg)
      -> CountCollectExecutorInfos {
    return CountCollectExecutorInfos{outReg};
  }
  auto GetSplit() -> SplitType {
    auto const& [split] = GetParam();
    return split;
  }

  auto MakeSubqueryStartRegisterInfos() -> RegisterInfos {
    auto inputRegisterSet = RegIdSet{0};
    auto outputRegisterSet = RegIdSet{};

    auto toKeepRegisterSet = RegIdSetStack{RegIdSet{0}, RegIdSet{0}};

    auto nrInputRegisters = static_cast<RegisterCount>(inputRegisterSet.size());
    auto nrOutputRegisters = static_cast<RegisterCount>(
        inputRegisterSet.size() + outputRegisterSet.size());
    return RegisterInfos(inputRegisterSet, outputRegisterSet, nrInputRegisters,
                         nrOutputRegisters, {}, toKeepRegisterSet);
  }

  auto MakeSubqueryStartExecutorInfos() -> SubqueryStartExecutor::Infos {
    auto inputRegisterSet = RegIdSet{0};
    auto outputRegisterSet = RegIdSet{};

    auto toKeepRegisterSet = RegIdSetStack{RegIdSet{0}};

    auto nrInputRegisters = static_cast<RegisterCount>(inputRegisterSet.size());
    auto nrOutputRegisters = static_cast<RegisterCount>(
        inputRegisterSet.size() + outputRegisterSet.size());
    return SubqueryStartExecutor::Infos(inputRegisterSet, outputRegisterSet,
                                        nrInputRegisters, nrOutputRegisters, {},
                                        toKeepRegisterSet);
  }

  auto MakeSubqueryEndRegisterInfos(RegisterId inputRegister) -> RegisterInfos {
    auto const outputRegister =
        RegisterId{static_cast<RegisterId::value_t>(inputRegister.value() + 1)};
    auto inputRegisterSet = RegIdSet{};
    for (RegisterId::value_t r = 0; r <= inputRegister.value(); ++r) {
      inputRegisterSet.emplace(r);
    }
    auto outputRegisterSet = RegIdSet{outputRegister};
    auto toKeepRegisterSet = RegIdSetStack{{}, {}};

    auto nrInputRegisters = static_cast<RegisterCount>(inputRegisterSet.size());
    auto nrOutputRegisters = static_cast<RegisterCount>(
        inputRegisterSet.size() + outputRegisterSet.size());
    return RegisterInfos(inputRegisterSet, outputRegisterSet, nrInputRegisters,
                         nrOutputRegisters, {}, toKeepRegisterSet);
  }

  auto MakeSubqueryEndExecutorInfos(RegisterId inputRegister)
      -> SubqueryEndExecutor::Infos {
    auto const outputRegister =
        RegisterId{static_cast<RegisterId::value_t>(inputRegister.value() + 1)};

    return SubqueryEndExecutor::Infos(nullptr, monitor, inputRegister,
                                      outputRegister);
  }

  auto MakeRemoveAllLinesRegisterInfos() -> RegisterInfos {
    auto numRegs = RegisterCount{1};

    RegIdSetStack toKeep{{}, {}};
    for (RegisterId::value_t r = 0; r < numRegs; ++r) {
      toKeep.back().emplace(r);
    }

    return RegisterInfos({}, {}, numRegs, numRegs, {}, std::move(toKeep));
  }

  auto MakeRemoveAllLinesExecutorInfos() -> LambdaExe::Infos {
    ProduceCall prod = [](AqlItemBlockInputRange& input,
                          OutputAqlItemRow& output)
        -> std::tuple<ExecutorState, LambdaExe::Stats, AqlCall> {
      EXPECT_TRUE(false) << "Should never be called";
      return {ExecutorState::DONE, NoStats{},
              AqlCall{0, true, 0, AqlCall::LimitType::HARD}};
    };

    SkipCall skip = [](AqlItemBlockInputRange& input, AqlCall& call)
        -> std::tuple<ExecutorState, LambdaExe::Stats, size_t, AqlCall> {
      std::ignore = input.skipAll();
      return {input.upstreamState(), NoStats{}, 0,
              AqlCall{0, true, 0, AqlCall::LimitType::HARD}};
    };

    return LambdaExe::Infos(prod, skip);
  }
};

template<size_t... vs>
const SplitType splitIntoBlocks = SplitType{std::vector<std::size_t>{vs...}};
template<size_t step>
const SplitType splitStep = SplitType{step};

INSTANTIATE_TEST_CASE_P(CountCollectExecutor, CountCollectExecutorTest,
                        ::testing::Values(SplitType{std::monostate()},
                                          splitStep<1>, splitIntoBlocks<2, 3>,
                                          splitStep<2>));

TEST_P(CountCollectExecutorTest, empty_input) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {{0}})
      .expectSkipped(0)
      .setCall(AqlCall{})
      .run();
}

TEST_P(CountCollectExecutorTest, count_input) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({{0}, {1}, {2}, {3}})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {{4}})
      .expectSkipped(0)
      .setCall(AqlCall{})
      .run();
}

TEST_P(CountCollectExecutorTest, empty_input_skip) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {})
      .expectSkipped(1)
      .setCall(AqlCall{10})
      .run();
}

TEST_P(CountCollectExecutorTest, count_input_skip) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({{0}, {1}, {2}, {3}})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {})
      .expectSkipped(1)
      .setCall(AqlCall{10})
      .run();
}

TEST_P(CountCollectExecutorTest, empty_input_fullCount) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {})
      .expectSkipped(1)
      .setCall(AqlCall{0, true, 0, AqlCall::LimitType::HARD})
      .run();
}

TEST_P(CountCollectExecutorTest, count_input_fullCount) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({{0}, {1}, {2}, {3}})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {})
      .expectSkipped(1)
      .setCall(AqlCall{0, true, 0, AqlCall::LimitType::HARD})
      .run();
}

TEST_P(CountCollectExecutorTest, count_input_softlimit) {
  makeExecutorTestHelper<1, 1>()
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({{0}, {1}, {2}, {3}})
      .expectedState(ExecutionState::DONE)
      .expectOutput({1}, {{4}})
      .expectSkipped(0)
      .setCall(AqlCall{0, false, 1, AqlCall::LimitType::SOFT})
      .run();
}

TEST_P(CountCollectExecutorTest, count_in_empty_subquery) {
  auto helper = makeExecutorTestHelper<1, 1>();

  helper
      .addConsumer<SubqueryStartExecutor>(MakeSubqueryStartRegisterInfos(),
                                          MakeSubqueryStartExecutorInfos(),
                                          ExecutionNode::SUBQUERY_START)
      .addConsumer<LambdaExe>(MakeRemoveAllLinesRegisterInfos(),
                              MakeRemoveAllLinesExecutorInfos(),
                              ExecutionNode::FILTER)
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .addConsumer<SubqueryEndExecutor>(MakeSubqueryEndRegisterInfos(1),
                                        MakeSubqueryEndExecutorInfos(1),
                                        ExecutionNode::SUBQUERY_END)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({{0}, {1}, {2}, {3}})
      .expectedState(ExecutionState::DONE)
      .expectOutput({2}, {{R"([0])"}, {R"([0])"}, {R"([0])"}, {R"([0])"}})
      .expectSkipped(0)
      .setCall(AqlCall{})
      .run();
}
TEST_P(CountCollectExecutorTest, count_in_subquery) {
  //   makeExecutorTestHelper<1, 1> helper();
  auto helper = makeExecutorTestHelper<1, 1>();

  helper
      .addConsumer<SubqueryStartExecutor>(MakeSubqueryStartRegisterInfos(),
                                          MakeSubqueryStartExecutorInfos(),
                                          ExecutionNode::SUBQUERY_START)
      .addConsumer<CountCollectExecutor>(MakeCountCollectRegisterInfos(1),
                                         MakeCountCollectExecutorInfos(1),
                                         ExecutionNode::COLLECT)
      .addConsumer<SubqueryEndExecutor>(MakeSubqueryEndRegisterInfos(1),
                                        MakeSubqueryEndExecutorInfos(1),
                                        ExecutionNode::SUBQUERY_END)
      .expectedStats(ExecutionStats{})
      .setInputSplitType(GetSplit())
      .setInputValue({{0}, {1}, {2}, {3}})
      .expectedState(ExecutionState::DONE)
      .expectOutput({2}, {{R"([1])"}, {R"([1])"}, {R"([1])"}, {R"([1])"}})
      .expectSkipped(0)
      .setCall(AqlCall{})
      .run();
}

}  // namespace aql
}  // namespace tests
}  // namespace arangodb
