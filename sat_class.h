#ifndef SAT_CLASS_H
#define SAT_CLASS_H

class SATclass {
public:
    std::unordered_set<int> unsigned_variables;
    std::unordered_map<int, Variable> variables;
    std::unordered_map<int, std::unordered_set<int>> formula;
    std::unordered_map<int, PairsSet> binary_clauses;
    std::unordered_map<int, double> literal_weights;
    std::unordered_set<int> satisfied_clauses;
    std::unordered_map<int, int> literal_count;
    std::unordered_set<int> reducted_clauses; //reducted clauses after propagation
    int number_of_all_clauses;


    SATclass(std::unordered_set<int>& unsigned_variables, std::unordered_map<int, Variable>& variables,
             std::unordered_map<int, std::unordered_set<int>>& formula, std::unordered_map<int, PairsSet>& binary_clauses,
             int number_of_all_clauses, std::unordered_map<int, double>& literal_weights, std::unordered_map<int, int>& literal_count ) :
            unsigned_variables(unsigned_variables), variables(variables), formula(formula), binary_clauses(binary_clauses),
            number_of_all_clauses(number_of_all_clauses), literal_weights(literal_weights),
            literal_count(literal_count) {
        satisfied_clauses = {};
    }
    // Copy constructor
    SATclass(const SATclass &p2) {
        number_of_all_clauses = p2.number_of_all_clauses;
        variables = p2.variables;
        unsigned_variables = p2.unsigned_variables;
        formula = p2.formula;
        binary_clauses = p2.binary_clauses;
        reducted_clauses = p2.reducted_clauses;
        satisfied_clauses = p2.satisfied_clauses;
        literal_weights = p2.literal_weights;
        literal_count = p2.literal_count;
    }

    // Copy assignment operator
    SATclass& operator=(SATclass p2)
    {
        variables = p2.variables;
        unsigned_variables = p2.unsigned_variables;
        formula = p2.formula;
        binary_clauses = p2.binary_clauses;
        satisfied_clauses = p2.satisfied_clauses;
        reducted_clauses = p2.reducted_clauses;
        number_of_all_clauses = p2.number_of_all_clauses;
        literal_weights = p2.literal_weights;
        literal_count = p2.literal_count;
        return *this;
    }

    bool is_satisfied() {
        return number_of_all_clauses == satisfied_clauses.size();
    }

    int get_clause_size(int clause_hash) {
        return formula[clause_hash].size();
    }

    int get_satified_literal(std::pair<int, int> new_value) {
        if(new_value.second == 0) {
            return -1*new_value.first;
        }
        return new_value.first;
    }

    bool is_clause_satisfied(int clause_hash, int variable, bool value) {
        auto& clause = formula[clause_hash];
        bool contains_positive_literal = clause.find(variable) != clause.end();
        if( (contains_positive_literal && value) || (!contains_positive_literal && !value)) {
            return true;
        }
        return false;
    }

    void prepare_satisfied_clause(int clause_hash) {
        auto& clause = formula[clause_hash];
        #if DIFF_HEURISTIC >= 1
        double coeff = powers[clause.size()];
        #endif
        for(auto literal: clause) {
            variables[abs(literal)].clauses.erase(clause_hash);
            #if DIRECTION_HEURISTIC == 0
            literal_count[literal] -= 1;
            #endif
            #if DIFF_HEURISTIC >= 1
            literal_weights[literal] -= coeff;
            #endif
        }
        formula.erase(clause_hash);
        remove_from_reducted_if_there(clause_hash);
        satisfied_clauses.insert(clause_hash);
    }

    void remove_from_reducted_if_there(int clause_hash) {
        if(reducted_clauses.find(clause_hash) != reducted_clauses.end()) {
            reducted_clauses.erase(clause_hash);
        }
    }

    void prepare_new_binary_clause(int clause_hash) {
        auto it = formula[clause_hash].begin();
        auto first_literal = *it; it++;
        auto second_literal = *it;

        binary_clauses[first_literal].insert(std::make_pair(second_literal, clause_hash));
        binary_clauses[second_literal].insert(std::make_pair(first_literal, clause_hash));
        for(auto literal: formula[clause_hash]) {
            variables[abs(literal)].clauses.erase(clause_hash);
        }
    }

    void prepare_binary_satisfied_clauses(int literal) {

        #if DIFF_HEURISTIC >= 1
        double coeff = powers[2];
        literal_weights[literal] -= coeff*binary_clauses[literal].size();
        #endif

        #if DIRECTION_HEURISTIC == 0
        literal_count[literal] -= binary_clauses[literal].size();
        #endif

        for(auto i: binary_clauses[literal]) {
            satisfied_clauses.insert(i.second);
            formula.erase(i.second);
            remove_from_reducted_if_there(i.second);

            #if DIFF_HEURISTIC >= 1
            literal_weights[i.first] -= coeff;
            #endif

            #if DIRECTION_HEURISTIC == 0
            literal_count[literal] -= 1;
            #endif
        }
    }

    bool prepare_birary_reducted_clauses(int literal_to_check, std::stack<std::pair<int, bool>>& assigned_variables) {
        for(auto i: binary_clauses[literal_to_check]) {
            int literal = i.first;
            auto variable_value = variables[abs(literal)].value;
            if(variable_value > -1) {
                if(!((literal < 0 && !variable_value) || (literal > 0 && variable_value))) { // is satisfied
                    return false;
                }
                // Already has value
            } else {
                bool satisfying_value = literal > 0;
                variables[abs(literal)].value = satisfying_value;
                assigned_variables.push(std::make_pair(abs(literal), satisfying_value));
            }
        }
        return true;
    }

    bool propagation(int variable, bool value) {
        auto assigned_variables = std::stack<std::pair<int, bool>>();
        reducted_clauses = {};
        assigned_variables.push(std::make_pair(variable, value));
        variables[variable].value = value;
        while(!assigned_variables.empty()) {

            auto var = assigned_variables.top();
            assigned_variables.pop();
            unsigned_variables.erase(var.first);

            auto newly_satisfied_clauses = std::vector<int>();
            auto newly_created_binary_clauses = std::vector<int>();

            for(auto clause_hash: variables[var.first].clauses) {
                if( is_clause_satisfied(clause_hash, var.first, var.second) ) {
                    newly_satisfied_clauses.push_back(clause_hash);
                } else {
                    // clause was reducted
                    auto literal = var.second ? -1*var.first : var.first;
                    reducted_clauses.insert(clause_hash);
                    formula[clause_hash].erase(literal);

                    #if DIRECTION_HEURISTIC == 0
                    literal_count[literal] -= 1;
                    #endif
                    
                    if(formula[clause_hash].size() == 2) {
                        newly_created_binary_clauses.push_back(clause_hash);
                    }
                }
            }

            for(auto i: newly_satisfied_clauses) {
                prepare_satisfied_clause(i);
            }
            for(auto i: newly_created_binary_clauses) {
                prepare_new_binary_clause(i);
            }

            if(var.second) { // positive value, positive -> satisfied, negative -> unit;
                prepare_binary_satisfied_clauses(var.first);
                if(!prepare_birary_reducted_clauses(-1*var.first, assigned_variables)) {
                    return false;
                }
            } else {
                prepare_binary_satisfied_clauses(-1*var.first);
                if(!prepare_birary_reducted_clauses(var.first, assigned_variables)) {
                    return false;
                }
            }

        }

        return true;
    }
};


#endif
