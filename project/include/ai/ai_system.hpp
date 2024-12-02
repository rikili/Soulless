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
    
    virtual ~Node() {
        // for (Node* child : children) {
        //     delete child;
        // }
        // children.clear();
    }
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
            // Child isn't done yet, keep running
            return NodeState::RUNNING;
        }
        
        if (state == NodeState::SUCCESS) {
            currentChild = 0; 
            return NodeState::SUCCESS;
        }
        
        currentChild++;
    }
    
    currentChild = 0;  
    return NodeState::FAILURE;
}
    
NodeState tickParallel(float elapsed_ms) {
    // not sure how it would work tbh but i think it's a common thing
    printd("Parallel tick - not implemented\n");
    return NodeState::SUCCESS;
    }
};

struct ConditionNode : public Node {
    std::function<bool(float)> condition;   
    bool expectedValue = true;         
    
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


struct ActionNode : public Node {
    float duration = 0;         
    float elapsedTime = 0;     
    bool isInterruptible;     
    
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
    if (state == NodeState::READY) {
        elapsedTime = 0;
    }

    if (duration > 0) {
        elapsedTime += elapsed_ms;
        // printf("Duration: %f\n", duration);
        // printf("Action state duration: %f\n", elapsedTime);
        // printf("Elapsed ms: %f\n", elapsed_ms);
        
        if (elapsedTime >= duration) {
            // printf("Ran too long\n");
            elapsedTime = 0;
            state = NodeState::READY;  
            return NodeState::SUCCESS;
        }
        state = NodeState::RUNNING; 
    }

    NodeState actionState = action(elapsed_ms);
    
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
    void create_enemy_projectile(const Entity& enemy_ent, bool mainSpell);
    void invoke_enemy_cooldown(const Entity& enemy_ent, bool first);
    void slash(const Entity& enemy_ent);
    void cleanNodeTree(Node* node);
}

