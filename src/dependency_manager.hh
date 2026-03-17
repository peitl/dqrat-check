#ifndef dependency_manager_hh
#define dependency_manager_hh

#include "solver_types.hh"
#include <algorithm>

namespace DQRATCheck {
  
	class DependencyManager {

		protected:
		std::vector<std::vector<Variable>> variable_dependencies;
		std::vector<bool> independencies_known;

		public:
		virtual ~DependencyManager() {}
		virtual void addVariable(bool is_existential) = 0;
		virtual void notifyStart() = 0;
		virtual void getDeps(Variable v) = 0;
		virtual bool check_pathC(Literal l, const vector<Literal>& lits, size_t num_lits_vec, CRef target)=0;

		// will calculate resolution paths if necessary
		inline bool notDependsOn(Variable of, Variable on) {
			if (!independenciesKnown(on)) {
				getDeps(on);
			}
			return std::binary_search(variable_dependencies[on - 1].begin(),
					variable_dependencies[on - 1].end(),
					of);
		}

		inline bool independenciesKnown(Variable on) const {
			return independencies_known[on-1];
		}

		inline bool numIndependencies(Variable on) const {
			return variable_dependencies[on-1].size();
		}

		inline void makeIndependenciesUnknown(Variable on) {
			independencies_known[on-1] = false;
			variable_dependencies[on-1].clear();
		}

	};

}

#endif
