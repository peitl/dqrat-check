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
		virtual void addVariable(bool auxiliary, bool is_existential) = 0;
		virtual void notifyStart() = 0;

		DependencyManager() {
			variable_dependencies.resize(1);
			independencies_known.resize(1);
		}

		// Implementation of inline methods.
		inline bool notDependsOn(Variable of, Variable on) const {
			return std::binary_search(variable_dependencies[on - 1].begin(),
					variable_dependencies[on - 1].end(),
					of);
		}

		inline bool independenciesKnown(Variable on) const {
			return independencies_known[on];
		}

		inline bool numIndependencies(Variable on) const {
			return variable_dependencies[on].size();
		}

	};

}

#endif
