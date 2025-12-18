#ifndef dependency_manager_upure_hh
#define dependency_manager_upure_hh

#include "dependency_manager.hh"

namespace DQRATCheck {

	class DQRATCheck;

	class DependencyManagerUPure: public DependencyManager {

		public:
			DependencyManagerUPure(DQRATCheck& checker);

			bool notDependsOn(Variable of, Variable on) const;

		protected:
			DQRATCheck& checker;
			void getDepsUPure(Variable v);
			vector<bool> getReachable(Literal l);
			//bool checkDependency(Variable of, Literal lof);
			bool independenciesKnown(Variable of) const;
			bool numIndependencies(Variable of) const;
			void addNonDependency(Variable of, Variable on);

	};

}

#endif
