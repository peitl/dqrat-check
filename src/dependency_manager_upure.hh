#ifndef dependency_manager_upure_hh
#define dependency_manager_upure_hh

#include "dependency_manager.hh"

namespace DQRATCheck {

	class DQBF;

	class DependencyManagerUPure: public DependencyManager {

		public:
			DependencyManagerUPure(DQBF& dqbf);

			bool notDependsOn(Variable of, Variable on) const;

		protected:
			DQBF& dqbf;
			void getDepsUPure(Variable v);
			vector<bool> getReachable(Literal l);
			//bool checkDependency(Variable of, Literal lof);
			bool independenciesKnown(Variable of) const;
			bool numIndependencies(Variable of) const;
			void addNonDependency(Variable of, Variable on);

	};

}

#endif
