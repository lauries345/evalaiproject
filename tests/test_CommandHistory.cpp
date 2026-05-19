#include "CommandHistory.h"
#include "ICommand.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>

/**
 * @file test_CommandHistory.cpp
 * @brief Unit tests for eval::CommandHistory. Targets 100% line and branch coverage.
 */

namespace
{

class CountingCommand final : public eval::ICommand
{
public:
    CountingCommand(int& executions, int& undos)
        : m_executions(executions)
        , m_undos(undos)
    {
    }

    void execute() override { ++m_executions; }
    void undo()    override { ++m_undos; }
    [[nodiscard]] std::string description() const override { return "Counting"; }

private:
    int& m_executions;
    int& m_undos;
};

class CommandHistoryTest : public ::testing::Test
{
protected:
    eval::CommandHistory history;
};

TEST_F(CommandHistoryTest, ExecuteRunsCommandAndIncreasesUndoDepth)
{
    int execs = 0;
    int undos = 0;
    history.execute(std::make_unique<CountingCommand>(execs, undos));
    EXPECT_EQ(execs, 1);
    EXPECT_EQ(history.undoDepth(), 1u);
    EXPECT_EQ(history.redoDepth(), 0u);
    EXPECT_TRUE (history.canUndo());
    EXPECT_FALSE(history.canRedo());
}

TEST_F(CommandHistoryTest, NullCommandThrows)
{
    EXPECT_THROW(history.execute(nullptr), std::invalid_argument);
}

TEST_F(CommandHistoryTest, UndoOnEmptyReturnsFalse)
{
    EXPECT_FALSE(history.undo());
}

TEST_F(CommandHistoryTest, RedoOnEmptyReturnsFalse)
{
    EXPECT_FALSE(history.redo());
}

TEST_F(CommandHistoryTest, UndoMovesCommandToRedoStack)
{
    int execs = 0;
    int undos = 0;
    history.execute(std::make_unique<CountingCommand>(execs, undos));
    EXPECT_TRUE(history.undo());
    EXPECT_EQ(undos, 1);
    EXPECT_EQ(history.undoDepth(), 0u);
    EXPECT_EQ(history.redoDepth(), 1u);
}

TEST_F(CommandHistoryTest, RedoReexecutesAndMovesBackToUndoStack)
{
    int execs = 0;
    int undos = 0;
    history.execute(std::make_unique<CountingCommand>(execs, undos));
    history.undo();
    EXPECT_TRUE(history.redo());
    EXPECT_EQ(execs, 2);
    EXPECT_EQ(history.undoDepth(), 1u);
    EXPECT_EQ(history.redoDepth(), 0u);
}

TEST_F(CommandHistoryTest, NewExecuteClearsRedoStack)
{
    int execsA = 0;
    int undosA = 0;
    int execsB = 0;
    int undosB = 0;
    history.execute(std::make_unique<CountingCommand>(execsA, undosA));
    history.undo();
    ASSERT_EQ(history.redoDepth(), 1u);
    history.execute(std::make_unique<CountingCommand>(execsB, undosB));
    EXPECT_EQ(history.redoDepth(), 0u);
}

TEST_F(CommandHistoryTest, ClearEmptiesBothStacks)
{
    int execs = 0;
    int undos = 0;
    history.execute(std::make_unique<CountingCommand>(execs, undos));
    history.undo();
    history.clear();
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(history.undoDepth(), 0u);
    EXPECT_EQ(history.redoDepth(), 0u);
}

} // namespace
