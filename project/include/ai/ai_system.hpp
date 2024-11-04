#pragma once

#include "core/common.hpp"
#include <functional>

/*
    AI Node States
    SUCCESS: The node has completed its task
    FAILURE: The node has failed its task
    RUNNING: The node is still running
    READY: The node is ready to run
*/
enum class NodeState {
    SUCCESS,
    FAILURE,
    RUNNING,
    READY
};

/*
    AI Node Types
    CONTROL: A control node that can run other nodes
    ACTION: An action node that can perform an action
    CONDITION: A condition node that can check a condition
*/
enum class NodeType {
    CONTROL,
    ACTION,
    CONDITION
};


/*
    AI Node
    A node in the AI tree
*/
struct Node {
    NodeState state = NodeState::READY;
    NodeType type;
    std::vector<Node*> children;
    
    // Virtual destructor for proper cleanup
    virtual ~Node() = default;
    
    // Virtual tick function that all nodes must implement
    virtual NodeState tick(float elapsed_ms) = 0;
};


// Control nodes have strategies for running children
struct ControlNode : public Node {
    // How this control node handles children
    enum class ControlType {
        SEQUENCE,   // Run all children in order (AND)
        SELECTOR,   // Run children until one succeeds (OR)
        PARALLEL    // Run all children at once
    };
    
    ControlType controlType;
    size_t currentChild = 0;  // For sequence/selector
    
    ControlNode(ControlType type) {
        this->type = NodeType::CONTROL;
        this->controlType = type;
    }
    
    NodeState tick(float elapsed_ms) override {
        switch(controlType) {
            case ControlType::SEQUENCE:
                return tickSequence(elapsed_ms);
            case ControlType::SELECTOR:
                return tickSelector(elapsed_ms);
            case ControlType::PARALLEL:
                return tickParallel(elapsed_ms);
        }
        return NodeState::FAILURE;
    }

private:
    NodeState tickSequence(float elapsed_ms) {
        bool success = true;
        for (Node* child : children) {
            NodeState state = child->tick(elapsed_ms);
            if (state == NodeState::FAILURE) {
                success = false;
                break;
            }
        }
        return success ? NodeState::SUCCESS : NodeState::FAILURE;
    }
    
NodeState tickSelector(float elapsed_ms) {
    while (currentChild < children.size()) {
        NodeState state = children[currentChild]->tick(elapsed_ms);
        
        if (state == NodeState::RUNNING) {
            // Child isn't done yet, keep running this same child next frame
            return NodeState::RUNNING;
        }
        
        if (state == NodeState::SUCCESS) {
            // Found a successful child, we're done!
            currentChild = 0;  // Reset for next time
            return NodeState::SUCCESS;
        }
        
        // This child failed, try next one
        currentChild++;
    }
    
    // All children failed
    currentChild = 0;  // Reset for next time
    return NodeState::FAILURE;
}
    
    NodeState tickParallel(float elapsed_ms) {
        // Implementation for parallel...
        printd("Parallel tick - not implemented\n");
        return NodeState::SUCCESS;
    }
};

struct ConditionNode : public Node {
    std::function<bool(float)> condition;    // The condition to check
    bool expectedValue = true;          // What we're checking for
    
    ConditionNode(std::function<bool(float)> conditionFn, bool expected = true) {
        this->type = NodeType::CONDITION;
        this->condition = conditionFn;
        this->expectedValue = expected;
    }
    
    NodeState tick(float elapsed_ms) override {
        return (condition(elapsed_ms) == expectedValue) 
               ? NodeState::SUCCESS 
               : NodeState::FAILURE;
    }
};


// Action nodes have actual game behaviors
struct ActionNode : public Node {
    float duration = 0;         // How long this action should run
    float elapsedTime = 0;     // Current running time
    bool isInterruptible;      // Can this action be interrupted?
    
    // Function pointer or lambda for the actual behavior
    std::function<NodeState(float)> action;
    
    ActionNode(std::function<NodeState(float)> actionFn, 
              float duration = 0.0f, 
              bool interruptible = true) {
        this->type = NodeType::ACTION;
        this->action = actionFn;
        this->duration = duration;
        this->isInterruptible = interruptible;
    }
    
NodeState tick(float elapsed_ms) override {
    // First check if we're starting fresh
    if (state == NodeState::READY) {
        elapsedTime = 0;
    }

    // Handle duration tracking BEFORE running action
    if (duration > 0) {
        elapsedTime += elapsed_ms;
        // printf("Duration: %f\n", duration);
        // printf("Action state duration: %f\n", elapsedTime);
        // printf("Elapsed ms: %f\n", elapsed_ms);
        
        if (elapsedTime >= duration) {
            // printf("Ran too long\n");
            elapsedTime = 0;
            state = NodeState::READY;  // Reset for next time
            return NodeState::SUCCESS;
        }
        state = NodeState::RUNNING;  // Keep it running
    }

    // Run the action after updating elapsed time
    NodeState actionState = action(elapsed_ms);
    
    // If no duration specified, use action's state
    if (duration <= 0) {
        state = actionState;
        return actionState;
    }
    
        return NodeState::RUNNING;
    }
};


/*
    AI System
    A system that ticks the AI tree for an entity
*/
struct AIComponent {
    Node* root;
};



namespace AI_SYSTEM {
    AIComponent& initAIComponent(Entity* entity);
    void tickForEntity(Entity* entity, float elapsed_ms);
    void create_enemy_projectile(const Entity& enemy_ent);
    void invoke_enemy_cooldown(const Entity& enemy_ent);
}

