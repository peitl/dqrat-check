//#include "variable_data.hh"
#include "watched_literal_propagator.hh"
#include "constraint_DB.hh"

namespace DQRATCheck {

	//ConstraintDB::ConstraintDB(DQRATCheck& checker):
	ConstraintDB::ConstraintDB():
		//checker(checker),
		constraints(ConstraintAllocator()),
		ca_to(nullptr) {}

	CRef ConstraintDB::addConstraint(vector<Literal>& literals) {
		CRef constraint_reference = constraints.alloc(literals);
		constraint_list.push_back(constraint_reference);
		return constraint_reference;
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

}
