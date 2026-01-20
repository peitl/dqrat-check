#include "dqbf.hh"
#include "dependency_manager.hh"

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
			depset[of].insert(on);
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

	bool DQBF::is_var_outer_of_exivar(Variable v, Variable exivar) {
		if (is_var_exists(v)) {
			bool is_subset = true;
			/*std::cout << "deps of " << externalize(exivar) << ":";
			for (Variable exidep : depset[exivar]) {
				std::cout << " " << externalize(exidep);
			}
			std::cout << std::endl;*/
			for (Variable x : depset[v]) {
				if (depset[exivar].find(x) == depset[exivar].end()) {
					is_subset = false;
					break;
				}
			}
			return is_subset;
		} else {
			return depset[exivar].find(v) != depset[exivar].end();
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
