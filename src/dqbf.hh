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
		friend class ConstraintDB;

		protected:

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
		// methods return internal variable name
		Variable addVarExists(Variable original_name);
		Variable addVarExists(Variable original_name, const vector<Variable>& dependency_set);
		Variable addVarForall(Variable original_name);
		void delVar(Variable original_name);

		void addDependency(Variable of_name, Variable on_name);
		// returns true if deletion allowed by Dupure
		bool delDependency(Variable of_name, Variable on_name);

		// set operations
		std::unordered_set<Variable> union_of(const std::vector<std::unordered_set<Variable>*>&);
		std::unordered_set<Variable> intersection_of(const std::vector<std::unordered_set<Variable>*>&);
		inline bool contains(Variable elem, const std::unordered_set<Variable>& set) {
			return set.find(elem) != set.end();
		}
		bool is_subset_of(const std::unordered_set<Variable>& subset, const std::unordered_set<Variable>& bigset);

		bool is_var_outer_of_exivar(Variable v, Variable exivar);
		bool is_var_outer_of_univar(Variable v, Variable univar);

		CRef addConstraint(vector<Literal>& literals);

		void notifyMaxVarDeclaration(Variable max_var);
		void notifyNumClausesDeclaration(uint32_t num_clauses);

		inline CRef propagate() { return constraint_database.propagate(); }
		inline void new_decision_level() { constraint_database.new_decision_level(); }
		inline void backtrack_before(unsigned int decision_level) { constraint_database.backtrack_before(decision_level); }
		inline bool assigned(Variable v) { return constraint_database.assigned(v); }
		inline bool satisfied(Literal l) { return constraint_database.satisfied(l); }
		inline void enqueue(Literal l) { return constraint_database.enqueue(l); };

		inline bool external_var_already_exists(Variable v) {
			return internal_name.find(v) != internal_name.end();
		}

		inline bool is_var_exists(Variable v) {
			return is_existential[v];
		}

		inline Variable get_max_var() {
			return max_var;
		}

		inline Variable internalize(Variable v) {
			return internal_name[v];
		}

		inline Literal internalize_literal(int x) {
			return mkLiteral(internal_name[abs(x)], x > 0);
		}

		inline Variable externalize(Variable v) {
			return external_name[v];
		}

		inline Variable externalize(Literal l) {
			return external_name[var(l)] * (2*sign(l) - 1);
		}
	};
}

#endif
