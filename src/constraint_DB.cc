//#include "variable_data.hh"
#include "watched_literal_propagator.hh"
#include "constraint_DB.hh"
#include <algorithm>
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
		occurrences_of(vector<vector<CRef>>(2)),
		ca_to(nullptr) {
	}

	ConstraintDB::~ConstraintDB() {
		delete dependency_manager;
	}

	void ConstraintDB::addVariable(bool is_existential) {
		occurrences_of.emplace_back();
		occurrences_of.emplace_back();
		propagator.addVariable();
		dependency_manager->addVariable(is_existential);
	}

	CRef ConstraintDB::addConstraint(vector<Literal>& literals) {
		//std::cout << "adding " << literals;
		// TODO figure out how exactly independencies can be preserved
		//universally_reduce(literals, CLEAR_INDEPENDENCIES);
		CRef constraint_reference = constraints.alloc(literals);
		constraint_list.push_back(constraint_reference);
		Constraint& constraint = getConstraint(constraint_reference);
		for (Literal l : constraint) {
			getOcc(l).push_back(constraint_reference);
		}
		if (propagator.addConstraint(constraint_reference)) {
			return constraint_reference;
		} else {
			// propagator says formula is now UNSAT after unit propagation
			return CRef_Undef;
		}
	}

	void ConstraintDB::relocVector(vector<CRef>& crefs) {
		size_t i, j;
		for (i = 0, j = 0; i < crefs.size(); i++) {
			CRef& constraint_reference = crefs[i];
			if (!getConstraint(constraint_reference).isMarked()) {
				relocate(constraint_reference); 
				crefs[j++] = crefs[i];
			}
		}
		crefs.resize(j);
	}

	void ConstraintDB::relocConstraintReferences() {
		for (vector<CRef>& occ_crefs : occurrences_of) {
			relocVector(occ_crefs);
		}
		relocVector(constraint_list);
	}

	void ConstraintDB::relocAll() {
		assert(ca_to == nullptr);
		ConstraintAllocator to(constraints.size() - constraints.wasted());
		ca_to = &to;

		propagator.relocConstraintReferences();
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
					dqbf.contains(vl, depset_union)
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

	CRef ConstraintDB::retrieveSortedConstraint(const vector<Literal>& literals) {
		//std::cout << "retrieve " << literals;
		Literal least_occ_lit = literals[0];
		size_t lolsz = getOcc(least_occ_lit).size();
		for (size_t i = 1; i < literals.size(); i++) {
			if (getOcc(literals[i]).size() < lolsz) {
				least_occ_lit = literals[i];
				lolsz = getOcc(least_occ_lit).size();
			}
		}
		// TODO fix this
		//std::cout << "about to search by literal " << dqbf.externalize(least_occ_lit) << " which occurs in " << getOcc(least_occ_lit).size() << " constraints" << std::endl;
		for (std::vector<CRef>::const_reverse_iterator rit = occ_rbegin(least_occ_lit);
				rit != occ_rend(least_occ_lit); rit++) {
			Constraint& clause = getConstraint(*rit);
			if (clause.size() == literals.size()) {
				bool equal = true;
				for (Literal lit : clause) {
					if (!std::binary_search(literals.begin(), literals.end(), lit)) {
						equal = false;
						break;
					}
				}
				if (equal) {
					return *rit;
				}
				continue;
			}
		}
		return CRef_Undef;
	}

	void ConstraintDB::deleteClause(CRef cref) {
		getConstraint(cref).mark();
		num_del_cons++;
		if (num_del_cons > DELETION_THRESHOLD) {
			relocAll();
			num_del_cons = 0;
		}
	}

}
