#include "dqbf.hh"
#include "dependency_manager.hh"
#include <unordered_set>

namespace DQRATCheck {

	DQBF::DQBF() : constraint_database(*this) {
		// padding so that internal variable names can be used as indices
		external_name.push_back(0);
		is_existential.push_back(0); 
	}

	Variable DQBF::addVarExists(Variable original_name) {
		return addVarExists(original_name, {});
	}

	Variable DQBF::addVarExists(Variable original_name, const vector<Variable>& dependency_set) {
		Variable internal = ++max_var;

		exivars.push_back(internal);
		is_existential.push_back(true);

		external_name.push_back(original_name);
		internal_name[original_name] = internal;
		
		depset[internal] = std::unordered_set<Variable>(dependency_set.begin(), dependency_set.end());

		constraint_database.addVariable(true);
		return internal;
	}

	Variable DQBF::addVarForall(Variable original_name) {
		Variable internal = ++max_var;

		univars.push_back(internal);
		is_existential.push_back(false);

		external_name.push_back(original_name);
		internal_name[original_name] = internal;

		constraint_database.addVariable(false);
		return internal;
	}

	void DQBF::delVar(Variable) {
		// TODO
	}

	void DQBF::addDependency(Variable of, Variable on) {
		if (is_existential[of]) {
			if (is_existential[on]) {
				for (Variable u : depset[on]) {
					depset[on].insert(u);
				}
			} else {
				depset[of].insert(on);
			}
		}
	}
	bool DQBF::delDependency(Variable of, Variable on) {
		if (is_existential[of]) {
			if (constraint_database.dependency_manager->notDependsOn(of, on)) {
				depset[of].erase(on);
				return true;
			} else {
				return false;
			}
		} else {
			return true;
		}
	}

	bool DQBF::is_subset_of(const std::unordered_set<Variable>& subset, const std::unordered_set<Variable>& bigset) {
		if (subset.size() > bigset.size())
			return false;
		bool is_subset = true;
		for (Variable x : subset) {
			if (bigset.find(x) == bigset.end()) {
				is_subset = false;
				break;
			}
		}
		return is_subset;
	}

	std::unordered_set<Variable> DQBF::union_of(const std::vector<std::unordered_set<Variable>*>& sets) {
		std::unordered_set<Variable> set_union;
		for (const std::unordered_set<Variable>* set : sets) {
			for (Variable elem : *set) {
				set_union.insert(elem);
			}
		}
		return set_union;
	}

	std::unordered_set<Variable> DQBF::intersection_of(const std::vector<std::unordered_set<Variable>*>& sets) {
		std::unordered_set<Variable> set_intersection;
		if (sets.empty())
			return set_intersection;
		// copy sets[0] to set_intersection
		for (Variable v : *sets[0]) {
			bool in_intersection = true;
			for (size_t i = 1; i < sets.size(); i++) {
				if (!contains(v, *sets[i])) {
					in_intersection = false;
					break;
				}
			}
			if (in_intersection) {
				set_intersection.insert(v);
			}
		}
		return set_intersection;
	}

	bool DQBF::is_var_outer_of_univar(Variable v, Variable univar) {
		/* 1. compute kernel of univar
		 * 2. if v is universal
		 * 		return (v is in kernel)
		 *    if v is existential
		 *      return (depset[v] <= kernel)
		 */
		std::vector<std::unordered_set<Variable>*> inner_depsets;
		std::vector<Variable> non_inner_vars;
		for (Variable x : exivars) {
			if (contains(univar, depset[x])) {
				inner_depsets.push_back(&depset[x]);
			}
		}
		if (inner_depsets.empty()) {
			return true;
		}
		std::unordered_set<Variable> kernel = intersection_of(inner_depsets);
		if (!is_var_exists(v)) {
			return contains(v, kernel);
		}
		kernel.erase(univar);
		return is_subset_of(depset[v], kernel);
	}

	bool DQBF::is_var_outer_of_exivar(Variable v, Variable exivar) {
		if (is_var_exists(v)) {
			return is_subset_of(depset[v], depset[exivar]);
		} else {
			return contains(v, depset[exivar]);
		}
	}

	// expects internal names
	CRef DQBF::addConstraint(vector<Literal>& literals) {
		return constraint_database.addConstraint(literals);
	}

	void DQBF::notifyMaxVarDeclaration(Variable) {
	}

	void DQBF::notifyNumClausesDeclaration(uint32_t) {
	}
	
}
