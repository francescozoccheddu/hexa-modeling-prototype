#include <hexa-modeling-prototype/undoredo.hpp>

CommandManager::CommandManager()
{

}

void CommandManager::execute(std::list<std::shared_ptr<Action>> actions)
{
    for(auto &action : actions){
        action->execute();
    }
    undo_stack.push(actions);
}

bool CommandManager::undo()
{
    if(undo_stack.empty()) return false;

    for(auto &action : undo_stack.top()) action->undo();
    redo_stack.push(undo_stack.top());
    undo_stack.pop();

    return true;
}

bool CommandManager::redo()
{
    if(redo_stack.empty()) return false;

    for(auto &action : redo_stack.top()) action->execute();
    undo_stack.push(redo_stack.top());
    redo_stack.pop();

    return true;

}

void CommandManager::collapse_last_n_actions(unsigned int n){

    std::list<std::shared_ptr<Action>> actions;
    for(unsigned int i=0; i<n; i++){
        auto &curr=undo_stack.top();
        actions.insert(actions.begin(), curr.begin(), curr.end());
        undo_stack.pop();
    }
    undo_stack.push(actions);
}

void CommandManager::clear_undo(){
    undo_stack = std::stack<std::list<std::shared_ptr<Action>>>();
}
void CommandManager::clear_redo(){
    redo_stack = std::stack<std::list<std::shared_ptr<Action>>>();

}

void CommandManager::clear()
{
    clear_undo();
    clear_redo();
}







