#ifndef _dqbf_hh_
#define _dqbf_hh_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "solver_types.hh"
#include "constraint_DB.hh"

using std::string;
using std::vector;

namespace DQRATCheck {

	class DQBF {
		Variable max_var = 0;

		std::unordered_map<Variable, Variable> internal_name;
		std::vector<Variable> external_name;
		std::vector<bool> is_existential;
		std::vector<Variable> univars;
		std::vector<Variable> exivars;
		std::unordered_map<Variable, std::unordered_set<Variable>> depset;

		public:

		ConstraintDB constraint_database;

		DQBF();
		void addVarExists(Variable original_name);
		void addVarExists(Variable original_name, const vector<Variable>& dependency_set);
		void addVarForall(Variable original_name);
		void delVar(Variable original_name);

		void addDependency(Variable of_name, Variable on_name);
		void delDependency(Variable of_name, Variable on_name);

		CRef addConstraint(vector<Literal>& literals);

		void notifyMaxVarDeclaration(Variable max_var);
		void notifyNumClausesDeclaration(uint32_t num_clauses);

		inline bool is_var_exists(Variable v) {
			return is_existential[v];
		}

		inline Variable get_max_var() {
			return max_var;
		}
	};
}

#endif
