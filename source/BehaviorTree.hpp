#ifndef GAME_BEHAVIORTREE_HPP
#define GAME_BEHAVIORTREE_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * @file BehaviorTree.hpp
 * @brief A small, STATELESS behaviour tree that yields one command per tick.
 *
 * The tree is a pure function of a CONTEXT: a list of named integer facts about
 * the situation. On each tick it is re-evaluated from the root and returns a
 * single command string. It keeps no memory of a "running" node between ticks
 * and depends on nothing outside the standard library, so it can be tested in
 * isolation with a hand-built context.
 *
 * The caller is responsible for filling the context before each tick. By
 * convention the context may include facts such as "EnemyVisible",
 * "CollisionWall" and "CollisionEntity" (a value of 0 means false, non-zero
 * means true); the tree treats every condition name uniformly as a lookup and
 * attaches no special meaning to any particular name.
 *
 * ---------------------------------------------------------------------------
 * NODE SEMANTICS (each tick reports success/failure and may carry a command)
 * ---------------------------------------------------------------------------
 *   - Condition(name) : succeeds iff the context contains @p name with a
 *                       non-zero value. Carries no command.
 *   - Action(command) : always succeeds; carries its command string.
 *   - Sequence(a,b,..): left to right; fails at the first failing child;
 *                       succeeds iff all succeed. Carries the last non-empty
 *                       command produced by its children.
 *   - Selector(a,b,..): left to right; succeeds at the first succeeding child
 *                       and carries that child's command; fails iff all fail.
 * The public tick() returns the command carried by the root's result, or the
 * empty string if the root failed (meaning: no command this tick).
 */

/// A list of named integer facts describing the current situation.
using BTContext = std::vector<std::pair<std::string, int>>;

/// Outcome of ticking a node.
struct BTResult {
    bool        success{false};
    std::string command;   ///< Empty means "no command".
};

/// Base class for behaviour-tree nodes.
class BTNode {
public:
    virtual ~BTNode() = default;
    [[nodiscard]] virtual BTResult tick(const BTContext& context) const = 0;
};

using BTNodePtr = std::unique_ptr<BTNode>;

// --- Concrete node types ---------------------------------------------------

class SelectorNode : public BTNode {
public:
    explicit SelectorNode(std::vector<BTNodePtr> children);
    [[nodiscard]] BTResult tick(const BTContext& context) const override;
private:
    std::vector<BTNodePtr> m_children;
};

class SequenceNode : public BTNode {
public:
    explicit SequenceNode(std::vector<BTNodePtr> children);
    [[nodiscard]] BTResult tick(const BTContext& context) const override;
private:
    std::vector<BTNodePtr> m_children;
};

class ConditionNode : public BTNode {
public:
    explicit ConditionNode(std::string name);
    [[nodiscard]] BTResult tick(const BTContext& context) const override;
private:
    std::string m_name;
};

class ActionNode : public BTNode {
public:
    explicit ActionNode(std::string command);
    [[nodiscard]] BTResult tick(const BTContext& context) const override;
private:
    std::string m_command;
};

// --- Construction helpers --------------------------------------------------
// Enable the nested construction syntax, e.g.:
//   auto root = Selector(
//                   Sequence(Condition("EnemyVisible"), Action("AttackEnemy")),
//                   Action("WalkForward") );

[[nodiscard]] inline BTNodePtr Condition(std::string name) {
    return std::make_unique<ConditionNode>(std::move(name));
}

[[nodiscard]] inline BTNodePtr Action(std::string command) {
    return std::make_unique<ActionNode>(std::move(command));
}

template <class... Children>
[[nodiscard]] BTNodePtr Selector(Children&&... children) {
    std::vector<BTNodePtr> v;
    (v.emplace_back(std::forward<Children>(children)), ...);
    return std::make_unique<SelectorNode>(std::move(v));
}

template <class... Children>
[[nodiscard]] BTNodePtr Sequence(Children&&... children) {
    std::vector<BTNodePtr> v;
    (v.emplace_back(std::forward<Children>(children)), ...);
    return std::make_unique<SequenceNode>(std::move(v));
}

/// Owns a tree root and evaluates it each tick.
class BehaviorTree {
public:
    explicit BehaviorTree(BTNodePtr root);

    /**
     * @brief Evaluates the tree once against @p context.
     * @return The command carried by the root result, or "" if the root failed.
     */
    [[nodiscard]] std::string tick(const BTContext& context) const;

private:
    BTNodePtr m_root;
};

#endif // GAME_BEHAVIORTREE_HPP
