#ifndef DELTARIVER_H
#define DELTARIVER_H
#include "glm_includes.h"
#include <iterator>
#include <stack>
#include <unordered_map>
struct State
{
    glm::vec2 position;
    glm::vec2 rotation;
    int iteration;
    State(glm::vec2 position, glm::vec2 rotation) : position(position), rotation(rotation), iteration(1) {}
    State(const State& s) : position(s.position), rotation(s.rotation), iteration(s.iteration) {}
};

class DeltaRiver
{
public:
    typedef void (DeltaRiver::*Rule)(void);
    std::string axiom;
    State current_state;
    std::stack<State> state_stack;
    std::unordered_map<char, std::string> char_to_grammar;
    std::unordered_map<char, Rule> char_to_rulefunction;
    DeltaRiver();
    DeltaRiver(glm::vec2 position, glm::vec2 rotation);
    float f_distance, rotate_angle;
    // Functions
    void populateRules();
    void lsystemParse(int iterations);
    void drawForward();
    void saveState();
    void restoreState();
    void rotateLeft();
    void rotateRight();
    void doNothing();
};

#endif // DELTARIVER_H
