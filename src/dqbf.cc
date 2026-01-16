#include "dqbf.hh"

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

		constraint_database.registerVariable();
		return internal;
	}

	Variable DQBF::addVarForall(Variable original_name) {
		Variable internal = ++max_var;

		univars.push_back(internal);
		is_existential.push_back(false);

		external_name.push_back(original_name);
		internal_name[original_name] = internal;

		constraint_database.registerVariable();
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
	void DQBF::delDependency(Variable of, Variable on) {
		if (is_existential[of]) {
			depset[of].erase(on);
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
