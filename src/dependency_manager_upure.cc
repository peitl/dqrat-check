#include "dependency_manager_upure.hh"
#include "dqrat_check.hh"
#include "constraint_DB.hh"
#include <stack>

namespace DQRATCheck {

	DependencyManagerUPure::DependencyManagerUPure(DQRATCheck& checker): DependencyManager(), checker(checker) {
	}

	/*
	 * computes the variables that depend *ON* v
	 * v MUST be universal
	 */
	void DependencyManagerUPure::getDepsUPure(Variable v) {
		if (independencies_known[v])
			return;
		if (!checker.get_dqbf().is_var_exists(v)) {
			return;
		}
		//clock_t t = clock();

		vector<bool> reachable_true = getReachable(mkLiteral(v, true));
		vector<bool> reachable_false = getReachable(mkLiteral(v, false));

		// TODO iterate over !vqtype variables only (using a next-of-type query?)
		for(int x = (v+1)*2; x <= checker.get_dqbf().get_max_var()*2; x += 2) {
			Variable xvar = x / 2;
			if (checker.get_dqbf().is_var_exists(xvar)) {
				if ((reachable_true[x] && reachable_false[x^1]) || (reachable_true[x^1] && reachable_false[x])) {
					//std::cout << "Found that " << solver.externalize(mkLiteral(xvar)) << "  *does*  depend on " << solver.externalize(mkLiteral(v)) << std::endl; 
				}
				else {
					// independence detected
					//std::cout << "Found that " << solver.externalize(mkLiteral(xvar)) << " does not depend on " << solver.externalize(mkLiteral(v)) << std::endl; 
					addNonDependency(xvar, v);
				}
			}
		}
		independencies_known[v] = true;
		//checker.solver_statistics.time_spent_computing_depscheme += clock()-t;
	}

	vector<bool> DependencyManagerUPure::getReachable(Literal l) {
		/* TODO optimizations?
		 *
		 * * (?) short-circuit once it is found that everything in the 
		 *	 dependency conflict is a dependency
		 * * (?) possible variant of the above: never run the second 'getReachable'
		 *	 if nothing from the dependency conflict is reachable the first time
		 *	 around (or in general, consider the results of the first run before
		 *	 the second one)
		 */

		uint32_t num_vars = checker.get_dqbf().get_max_var();
		uint32_t num_lits = num_vars * 2;

		Variable lvar = var(l);
		bool lqtype = !checker.get_dqbf().is_var_exists(var(l));
		bool target_qtype = 1 - lqtype;

		vector<bool> reachable(num_lits);
		vector<bool> explored(num_lits);
		std::stack<Literal> landing_literals;
		if (lqtype == 1) { // l is universal
			reachable.assign(num_lits, false);
			explored.assign(num_lits, false);
			landing_literals.push(l);
		} else {
			reachable.assign(num_lits, true);
		}

		Literal negl = ~l;

		//uint32_t max_target_lits = 2*solver.variable_data_store->countVarsOfTypeRightOf(target_qtype, lvar);
		uint32_t max_target_lits = 2*checker.get_dqbf().get_max_var();

		uint32_t target_lits_found = 0;
		while (!landing_literals.empty()) {
			Literal current_lit = landing_literals.top();
			landing_literals.pop();
			int current_lit_idx = toInt(current_lit);
			if (explored[current_lit_idx])
				continue;
			explored[current_lit_idx] = true;

			for (auto occit = checker.get_dqbf().constraint_database.literalOccurrencesBegin(current_lit);
					occit != checker.get_dqbf().constraint_database.literalOccurrencesEnd(current_lit);
					occit++) {
				//auto fel_ref = first_entry_literal.find(*occit);
				Constraint& clause = checker.get_dqbf().constraint_database.getConstraint(*occit);
				bool clause_has_negl = false;
				for (Literal lit : clause) {
					if (lit == negl) {
						// the path is not u-pure
						clause_has_negl = true;
						break;
					}
				}
				if (clause_has_negl)
					continue;
				//first_entry_literal[*occit] = current_lit;
				for (Literal lit : clause) {
					if (!explored[toInt(~lit)]) {
						Variable litvar = var(lit);
						int lit_idx = toInt(lit);
						bool litvarqtype = !checker.get_dqbf().is_var_exists(litvar);
						if (litvarqtype == 0 && l <= lit) {
							landing_literals.push(~lit);
						}
						if (litvarqtype == target_qtype && l < lit) {
							/* lit is validly reached by the current path */
							if (!reachable[lit_idx]) {
								reachable[lit_idx] = true;
								max_target_lits++;
							}
						}
					}
				}
				if (target_lits_found == max_target_lits) {
					/* clear the stack and break out of clause traversal,
					 * ensures a single return statement and RVO
					 */
					landing_literals = {};
					break;
				}
			}
		}

		return reachable;
	}

	void DependencyManagerUPure::addNonDependency(Variable of, Variable on) {
		//checker.statistics.nr_independencies++;
		variable_dependencies[on].push_back(of);
	}

}
