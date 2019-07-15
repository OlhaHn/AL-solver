#include "includes.h"
#include "settings.h"
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


double count_crh(SATclass& instance) {
    double result = 0;
    for(auto clause_hash: instance.reducted_clauses) {
        if(instance.formula.find(clause_hash) != instance.formula.end()) {
            result += powers[instance.get_clause_size(clause_hash)];
        } else {
            result += 1; // binary clause;
        }
        
    }
    return result;
}

void recount_weights(SATclass& instance, SATclass& new_instace) {
    for(auto hash: new_instace.reducted_clauses) {
        double new_coeff = powers[new_instace.get_clause_size(hash)];
        double old_coeff = powers[instance.get_clause_size(hash)];
        double update = new_coeff - old_coeff;
        for(auto literal: new_instace.formula[hash]) {
            new_instace.literal_weights[literal] += update;
        }
    }
}

double count_wbh(SATclass& instance, SATclass& new_instace) {
    recount_weights(instance, new_instace);
    double wbh = 0;
    for(auto i: new_instace.reducted_clauses) {
        if(new_instace.get_clause_size(i) == 2) {
            for(auto literal: new_instace.formula[i]) {
                wbh += new_instace.literal_weights[-1*literal];
            }
        }
    }
    return wbh;
}

double count_bsh(SATclass& instance, SATclass& new_instace) {
    recount_weights(instance, new_instace);
    double bsh = 0;
    for(auto i: new_instace.reducted_clauses) {
        if(new_instace.get_clause_size(i) == 2) {
            double result = 1;
            for(auto literal: new_instace.formula[i]) {
                result *= new_instace.literal_weights[-1*literal];
            }
            bsh += result;
        }
    }
    return bsh;
}


double decision_heuristic(SATclass& instance, SATclass& true_instace, SATclass& false_instance) {
    #if DIFF_HEURISTIC == 0
    return count_crh(true_instace)*count_crh(false_instance);
    #elif DIFF_HEURISTIC == 1
    return count_wbh(instance, true_instace)*count_wbh(instance, false_instance);
    #elif DIFF_HEURISTIC == 2
    return count_bsh(instance, true_instace)*count_bsh(instance, false_instance);
    #endif
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
    auto literal_wieghts = std::unordered_map<int, double>();
    int number_of_clauses = 0;

    read_input(formula, variables, unasigned_variables, binary_clauses, number_of_clauses, literal_wieghts);
    auto sat_instance = SATclass(unasigned_variables, variables, formula, binary_clauses, number_of_clauses, literal_wieghts);
    auto result = dpll(sat_instance);
    std::cout << "Result: " << result << '\n';

    return 0;
}
