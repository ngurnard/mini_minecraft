#include "deltariver.h"
#include "glm/gtx/matrix_transform_2d.hpp"

DeltaRiver::DeltaRiver(glm::vec2 position, glm::vec2 rotation) : axiom("FX"), current_state(State(position, rotation)), state_stack(),
    char_to_grammar(), char_to_rulefunction(), f_distance(35), rotate_angle(35)
{
    srand(1702);
    populateRules();
}

void DeltaRiver::populateRules()
{
    char_to_grammar['X'] = "[+FX][-FX]";
    char_to_rulefunction['F'] = &DeltaRiver::drawForward;
    char_to_rulefunction['-'] = &DeltaRiver::rotateRight;
    char_to_rulefunction['+'] = &DeltaRiver::rotateLeft;
    char_to_rulefunction['['] = &DeltaRiver::saveState;
    char_to_rulefunction[']'] = &DeltaRiver::restoreState;
    char_to_rulefunction['X'] = &DeltaRiver::doNothing;

}

// Apply valid rules to each symbol in our list for given number of iterations
void DeltaRiver::lsystemParse(int iterations)
{
    std::string new_axiom = "";
    for(int i = 0; i < iterations; i++)
    {
        for(int j = 0; j < axiom.length(); j++)
        {
            char literal = axiom[j];
            if(char_to_grammar.find(literal) != char_to_grammar.end())
            {
                if(rand() < 0.8 * RAND_MAX || i < 2)
                {
                    new_axiom += char_to_grammar[literal];
                }
                else if(rand() < 0.5 * RAND_MAX)
                {
                    new_axiom += "+FX";
                }
                else
                {
                    new_axiom += "-FX";
                }
            }
            else
            {
                new_axiom += literal;
            }
        }
        axiom = new_axiom;
    }
}

void DeltaRiver::saveState()
{
    state_stack.push(State(current_state));
    current_state.iteration++;
}

void DeltaRiver::restoreState()
{
    current_state = state_stack.top();
    state_stack.pop();
}

void DeltaRiver::rotateLeft()
{
    // Get random value between -0.25 to 0.25
    float rand_val = ((rand() / (float) RAND_MAX) - 0.5)/2.f;
    float turn_angle = rotate_angle * (1 + rand_val);
    glm::mat3 rotation_matrix = glm::rotate(glm::mat3(), glm::radians(turn_angle));
    current_state.rotation = glm::vec2(rotation_matrix * glm::vec3(current_state.rotation, 0.f));
}

void DeltaRiver::rotateRight()
{
    // Get random value between -0.25 to 0.25
    float rand_val = ((rand() / (float) RAND_MAX) - 0.5)/2.f;
    float turn_angle = -1 * rotate_angle * (1 + rand_val);
    glm::mat3 rotation_matrix = glm::rotate(glm::mat3(), glm::radians(turn_angle));
    current_state.rotation = glm::vec2(rotation_matrix * glm::vec3(current_state.rotation, 0.f));
}

void DeltaRiver::drawForward()
{
    float rand_val = (rand() / (float) RAND_MAX);
    if(rand_val < 0.5)
        rotateLeft();
    else
        rotateRight();
    float distance = std::pow(0.9, current_state.iteration) * f_distance;
    current_state.position += 0.5f * distance * current_state.rotation;
}

void DeltaRiver::doNothing()
{}
