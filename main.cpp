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
int look_ahead(SATclass& instance) {
    if(instance.unsigned_variables.size() == 0) {
        return -1;
    }
    return *instance.unsigned_variables.begin();
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
