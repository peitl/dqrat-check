#ifndef _dqbf_hh_
#define _dqbf_hh_

#include <vector>
#include <string>
#include "solver_types.hh"

using std::string;
using std::vector;

namespace DQRATCheck {

	class DQBF {

		public:
			void addVarExists(const string& original_name);
			void addVarExists(const string& original_name, const vector<string>& dependency_set);
			void addVarForall(const string& original_name);
			void delVar(const string& original_name);

			void addDependency(const string& of_name, const string& on_name);
			void delDependency(const string& of_name, const string& on_name);

			CRef addConstraint(vector<Literal>& literals, ConstraintType constraint_type);

			void notifyMaxVarDeclaration(Variable max_var);
			void notifyNumClausesDeclaration(uint32_t num_clauses);
	};
}

#endif
