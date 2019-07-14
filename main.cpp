#include "includes.h"
#include "variable.h"
#include "reader.h"
#include "sat_class.h"
#include <fstream>
/*
    Returns:
    0 -> instance is unsatisfable
    -1 -> decision variable was not found
    > 0 -> decision varible

*/

double decision_heuristic(SATclass& instance, SATclass& true_instace, SATclass& false_instance) {
    int number_of_binary = true_instace.number_of_all_clauses - true_instace.satisfied_clauses.size() - true_instace.formula.size();
    int number_of_binary_f = false_instance.number_of_all_clauses - false_instance.satisfied_clauses.size() - false_instance.formula.size();
    return number_of_binary*number_of_binary_f;
}


int look_ahead(SATclass& instance) {
    auto preselect = instance.unsigned_variables;
    int selected_var = -1;
    double decision_heuristic_value = -100;
    for(auto i: preselect) {
        if(instance.variables[i].value == -1) {

            auto result_of_true_instance = instance;
            auto result_of_false_instance = instance;

            bool true_propagation = result_of_true_instance.propagation(i, 1);
            bool false_propagation = result_of_false_instance.propagation(i, 0);
            if(!true_propagation && !false_propagation) {
                return 0;
            } else if(!true_propagation) {
                instance = result_of_false_instance;
            } else if(!false_propagation) {
                instance = result_of_true_instance;
            } else {
                auto new_decision = decision_heuristic(instance, result_of_true_instance, result_of_false_instance);
                if(new_decision > decision_heuristic_value) {
                    decision_heuristic_value = new_decision;
                    selected_var = i;
                }
            }
        }
    }

    if(selected_var > 0 && instance.variables[selected_var].value != -1) {
        return -1;
    }
    return selected_var;
}


bool dpll(SATclass instance) {
    if (instance.is_satisfied()) {
        return true;
    } else {
        int decision_variable = look_ahead(instance);
        if (decision_variable == 0) {
            return false;
        } else if (decision_variable == -1) {
            return dpll(instance);
        } else {
            auto instance_copy = instance;
            bool propagarion_res = instance.propagation(decision_variable, 0);
            if(propagarion_res && dpll(instance)) {
                return true;
            } else {
                propagarion_res = instance_copy.propagation(decision_variable, 1);
                return propagarion_res && dpll(instance_copy);
            }
        }
    }
}

int main() {

    //std::ifstream in("/Users/marek/Desktop/licencjat_in_progress/input/jnh/jnh1.cnf");
    //std::cin.rdbuf(in.rdbuf());

    auto formula = std::unordered_map<int, std::unordered_set<int>>();
    auto variables = std::unordered_map<int, Variable>();
    auto unasigned_variables = std::unordered_set<int>();
    auto binary_clauses = std::unordered_map<int, PairsSet>();
    int number_of_clauses = 0;

    read_input(formula, variables, unasigned_variables, binary_clauses, number_of_clauses);
    auto sat_instance = SATclass(unasigned_variables, variables, formula, binary_clauses, number_of_clauses);
    auto result = dpll(sat_instance);
    std::cout << "Result: " << result << '\n';

    return 0;
}
