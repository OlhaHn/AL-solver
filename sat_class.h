#ifndef SAT_CLASS_H
#define SAT_CLASS_H

#include "includes.h"

class SATclass {
public:
    std::unordered_set<int> unsigned_variables;
    std::unordered_map<int, Variable> variables;
    std::unordered_map<int, std::unordered_set<int>> formula;
    std::unordered_map<int, double> literal_weights;
    std::unordered_set<int> satisfied_clauses;
    std::unordered_map<int, int> literal_count;
    std::unordered_set<int> reducted_clauses; //reducted clauses after propagation
    std::unordered_set<int> new_binary_clauses;
    std::unordered_set<int> implicated_variables;
    int number_of_all_clauses;
    int decision_level;
    double trigger;
    double start_tigger;


    SATclass(std::unordered_set<int>& unsigned_variables, std::unordered_map<int, Variable>& variables,
             std::unordered_map<int, std::unordered_set<int>>& formula,
             int number_of_all_clauses, std::unordered_map<int, double>& literal_weights, std::unordered_map<int, int>& literal_count ) :
            unsigned_variables(unsigned_variables), variables(variables), formula(formula),
            number_of_all_clauses(number_of_all_clauses), literal_weights(literal_weights),
            literal_count(literal_count), decision_level(0) {
        satisfied_clauses = {};
    }
    // Copy constructor
    SATclass(const SATclass &p2) {
        number_of_all_clauses = p2.number_of_all_clauses;
        variables = p2.variables;
        unsigned_variables = p2.unsigned_variables;
        formula = p2.formula;
        reducted_clauses = p2.reducted_clauses;
        satisfied_clauses = p2.satisfied_clauses;
        literal_weights = p2.literal_weights;
        literal_count = p2.literal_count;
        decision_level = p2.decision_level;
        trigger = p2.trigger;
        start_tigger = p2.start_tigger;
    }

    // Copy assignment operator
    SATclass& operator=(SATclass p2)
    {
        variables = p2.variables;
        unsigned_variables = p2.unsigned_variables;
        formula = p2.formula;
        satisfied_clauses = p2.satisfied_clauses;
        reducted_clauses = p2.reducted_clauses;
        number_of_all_clauses = p2.number_of_all_clauses;
        literal_weights = p2.literal_weights;
        literal_count = p2.literal_count;
        decision_level = p2.decision_level;
        trigger = p2.trigger;
        start_tigger = p2.start_tigger;
        return *this;
    }

    std::unordered_set<int> preselect_propz() {
        if(decision_level < 5 || unsigned_variables.size() <= 10) {
            return unsigned_variables;
        }

        auto result_set = std::unordered_set<int>();
        for(auto var: unsigned_variables) {
            bool was_positive = false;
            bool was_negative = false;
            for(auto clause_hash: variables[var].clauses) {
                auto& clause = formula[clause_hash];
                if(clause.find(var) != clause.end()) {
                    was_positive = true;
                } 
                if(clause.find(-1*var) != clause.end()) {
                    was_negative = true;
                }
                if(was_negative && was_positive) {
                    result_set.insert(var);
                    break;
                }
            }
        }
        auto it = unsigned_variables.begin();
        while (result_set.size() < 10 && it != unsigned_variables.end()) {
            result_set.insert(*it);
            it++;
        }
        return result_set;
    }

    std::unordered_set<int> preselect_cra() {
        if(unsigned_variables.size() < 20) {
            return unsigned_variables;
        }
        auto cra_map = std::unordered_map<int, int>();

        int unsigned_varialbes_count = unsigned_variables.size();
        for(auto var: unsigned_variables) {

            int positive_sum = 0;
            int negative_sum = 0;

            std::unordered_map<int, int> literal_number_of_binary_clauses = {};
            for(auto clause_hash: variables[var].clauses) {
                auto& clause = formula[clause_hash];
                if(clause.size() == 2) {
                    auto it = clause.begin(); 
                    if(abs(*it) == var) {
                        it++;
                    }
                    auto next_literal = *it;
                    if(literal_number_of_binary_clauses.find(-1*next_literal) == literal_number_of_binary_clauses.end()) {
                        int result = 0;
                        for(auto i: variables[abs(next_literal)].clauses) {
                            auto& clause_value = formula[i];
                            if(clause_value.size() != 2 && clause.find(-1*next_literal) != clause.end()) {
                                result ++;
                            }
                        }
                        literal_number_of_binary_clauses[-1*next_literal] = result;
                    }
                    if(clause.find(var) != clause.end()) {
                        positive_sum += literal_number_of_binary_clauses[-1*next_literal];
                    } else {
                        negative_sum += literal_number_of_binary_clauses[-1*next_literal];
                    }
                }
            }

            cra_map[var] = positive_sum*negative_sum;
        }

        auto size = std::max(20, unsigned_varialbes_count/10);
        std::vector<std::pair<int, int>> top(size);
        auto result_set = std::unordered_set<int>();
        std::partial_sort_copy(cra_map.begin(),
                            cra_map.end(),
                            top.begin(),
                            top.end(),
                            [](std::pair<int, int> const& l,
                                std::pair<int, int> const& r)
                            {
                                return l.second > r.second;
                            });
        std::transform(top.begin(), top.end(), std::inserter(result_set, result_set.begin()),
                   [](std::pair<int, int> c) { return c.first; });
        return result_set;
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
            #if DIRECTION_HEURISTIC == 0 || AUTARKY_REASONING == 1
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
        if(new_binary_clauses.find(clause_hash) != new_binary_clauses.end()) {
            new_binary_clauses.erase(clause_hash);
        }
    }

    bool propagation(int variable, bool value) {
        auto assigned_variables = std::stack<std::pair<int, bool>>();
        reducted_clauses = {};
        new_binary_clauses = {};
        #if LOCAL_LEARNING == 1
            implicated_variables = {};
        #endif
        assigned_variables.push(std::make_pair(variable, value));
        variables[variable].value = value;
        while(!assigned_variables.empty()) {

            auto var = assigned_variables.top();
            assigned_variables.pop();
            unsigned_variables.erase(var.first);
            auto newly_satisfied_clauses = std::vector<int>();

            for(auto clause_hash: variables[var.first].clauses) {
                if( is_clause_satisfied(clause_hash, var.first, var.second) ) {
                    newly_satisfied_clauses.push_back(clause_hash);
                } else {
                    // clause was reducted
                    auto literal = var.second ? -1*var.first : var.first;
                    reducted_clauses.insert(clause_hash);
                    formula[clause_hash].erase(literal);

                    #if DIRECTION_HEURISTIC == 0 || AUTARKY_REASONING == 1
                    literal_count[literal] -= 1;
                    #endif

                    if(formula[clause_hash].size() == 2) {
                        new_binary_clauses.insert(clause_hash);
                    }

                    if(formula[clause_hash].size() == 1) {
                        auto literal = *formula[clause_hash].begin();
                        if(variables[abs(literal)].value == -1) {
                            bool value = literal > 0;
                            variables[abs(literal)].value = value;
                            assigned_variables.push(std::make_pair(abs(literal), value));
                            #if LOCAL_LEARNING == 1
                            if(var.first != variable) { // not direct implication
                                implicated_variables.insert(abs(literal));
                            }
                            #endif
                        } else {
                            auto value = variables[abs(literal)].value;
                            if((literal > 0 && value ) || (literal < 0 && !value)) {
                                newly_satisfied_clauses.push_back(clause_hash);
                            } else {
                                return false;
                            }
                        }
                    }
                }
            }

            for(auto i: newly_satisfied_clauses) {
                prepare_satisfied_clause(i);
            }
        }

        #if LOCAL_LEARNING == 1
        implicated_variables.erase(variable);
        #endif

        return true;
    }
};


#endif
