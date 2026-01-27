//#include "variable_data.hh"
#include "watched_literal_propagator.hh"
#include "constraint_DB.hh"
#include <unordered_set>
#include "dependency_manager_upure.hh"
#include "dqbf.hh"

namespace DQRATCheck {

	enum {
		KEEP_INDEPENDENCIES = false,
		CLEAR_INDEPENDENCIES = true
	};

	ConstraintDB::ConstraintDB(DQBF& dqbf):
		dqbf(dqbf),
		dependency_manager(new DependencyManagerUPure(dqbf)),
		propagator(*this),
		constraints(ConstraintAllocator()),
		ca_to(nullptr) {
	}

	ConstraintDB::~ConstraintDB() {
		delete dependency_manager;
	}

	void ConstraintDB::addVariable(bool is_existential) {
		propagator.addVariable();
		dependency_manager->addVariable(is_existential);
	}

	CRef ConstraintDB::addConstraint(vector<Literal>& literals) {
		universally_reduce(literals, CLEAR_INDEPENDENCIES);
		CRef constraint_reference = constraints.alloc(literals);
		constraint_list.push_back(constraint_reference);
		if (propagator.addConstraint(constraint_reference)) {
			return constraint_reference;
		} else {
			// propagator says formula is now UNSAT after unit propagation
			return CRef_Undef;
		}
	}

	void ConstraintDB::relocConstraintReferences() {
		for (auto it = literal_occurrences.begin(); it != literal_occurrences.end(); ++it) {
			vector<CRef>& occ_crefs = it->second;
			//size_t tainted_idx = occ_crefs.size();
			size_t i, j;
			for (i = 0, j = 0; i < occ_crefs.size(); i++) {
				CRef& constraint_reference = occ_crefs[i];
				/* Since literal occurrences only consider input constraints, there is no need to check whether the
				   corresponding constraint has been marked for removal. */
				// when considering tainting, we unfortunately have to check markedness again
				if (!getConstraint(constraint_reference).isMarked()) {
					relocate(constraint_reference); 
					occ_crefs[j++] = occ_crefs[i];
				}
			}
			occ_crefs.resize(j);
		}
	}

	void ConstraintDB::relocAll() {
		assert(ca_to == nullptr);
		ConstraintAllocator to(constraints.size() - constraints.wasted());
		ca_to = &to;

		//checker.propagator->relocConstraintReferences();
		//checker.variable_data_store->relocConstraintReferences();
		relocConstraintReferences();

		to.moveTo(constraints);
		ca_to = nullptr;
	}

	void ConstraintDB::cleanConstraints() {
		uint32_t removed_counter = 0;
		for (CRef constraint_reference : constraint_list) {
			Constraint& constraint = constraints[constraint_reference];
			if (constraint.isMarked()) {
				removed_counter++;
			}
		}
		std::cerr << "Removed " << removed_counter << " clauses" << std::endl;
		relocAll();
	}

	// TODO this is probably very inefficient
	// we are reusing the depset_union: any universal variable inside it potentially receives new
	// resolution-path dependencies, so we may want to clear them
	// TODO this is really not the right place to do it, I'm doing it here only to reuse the computation
	void ConstraintDB::universally_reduce(vector<Literal>& literals, bool clear_independencies) {
		std::unordered_set<Variable> depset_union;
		for (Literal l : literals) {
			Variable vl = var(l);
			if (dqbf.is_var_exists(vl)) {
				depset_union.insert(dqbf.depset[vl].begin(), dqbf.depset[vl].end());
			}
		}
		if (clear_independencies) {
			for (Variable u : depset_union) {
				dependency_manager->makeIndependenciesUnknown(u);
			}
		}
		size_t j = 0;
		for (size_t i = 0; i < literals.size(); i++) {
			Variable vl = var(literals[i]);
			if (
					dqbf.is_var_exists(vl) ||
					depset_union.find(vl) != depset_union.end()
				) {
				literals[j++] = literals[i];
				/*if (!dqbf.is_var_exists(vl)) {
					std::cout << "keeping the literal " << dqbf.externalize(literals[i]) << std::endl;
				}*/
			} else {
				//std::cout << "reducing the literal " << dqbf.externalize(literals[i]) << std::endl;
			}
		}
		literals.resize(j);
	}

}
