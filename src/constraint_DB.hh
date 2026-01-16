#ifndef constraint_DB_hh
#define constraint_DB_hh

#include <vector>
#include <unordered_map>
#include <algorithm>
#include "solver_types.hh"
#include "constraint.hh"
#include "watched_literal_propagator.hh"

using std::vector;
using std::unordered_map;
using std::sort;

namespace DQRATCheck {

	class DQBF;

	class ConstraintDB {

		public:
			//ConstraintDB(DQRATCheck& checker);
			ConstraintDB(DQBF& dqbf);
			CRef addConstraint(vector<Literal>& literals);
			Constraint& getConstraint(CRef constraint_reference);
			vector<CRef>::const_iterator constraintReferencesBegin();
			vector<CRef>::const_iterator constraintReferencesEnd();
			inline vector<CRef>::const_iterator literalOccurrencesBegin(Literal l) {
				return literal_occurrences[l].begin();
			}
			inline vector<CRef>::const_iterator literalOccurrencesEnd(Literal l) {
				return literal_occurrences[l].end();
			}
			void bumpConstraintActivity(Constraint& constraint, ConstraintType constraint_type);
			virtual void notifyStart();
			void updateLBD(Constraint& constraint);
			void relocate(CRef& constraint_reference);

			inline void registerVariable() { propagator.addVariable(); }
			inline void new_decision_level() { propagator.new_decision_level(); }
			inline void backtrack_before(unsigned int decision_level) { propagator.backtrack_before(decision_level); }
			inline CRef propagate() { return propagator.propagate(); };
			inline bool assigned(Variable v) { return propagator.assigned(v); };
			inline bool satisfied(Literal l) { return propagator.satisfied(l); };
			inline void enqueue(Literal l) { return propagator.enqueue(l); };
			inline const vector<CRef>& occurrences_of(Literal l) { return propagator.occurrences_of[toInt(l)]; };
			

		protected:
			void relocConstraintReferences();
			void relocAll();
			void cleanConstraints();
			bool isLocked(Constraint& constraint, CRef constraint_reference);

			DQBF& dqbf;
			WatchedLiteralPropagator propagator;
			ConstraintAllocator constraints;
			vector<CRef> constraint_list;
			unordered_map<Literal, vector<CRef>> literal_occurrences;
			ConstraintAllocator* ca_to;
	};

	// Implementation of inline methods.

	inline Constraint& ConstraintDB::getConstraint(CRef constraint_reference) {
		return constraints[constraint_reference];
	}

	inline void ConstraintDB::notifyStart() {}

	inline void ConstraintDB::relocate(CRef& constraint_reference) {
		assert(ca_to != nullptr);
		constraints.reloc(constraint_reference, *ca_to);
	}

}

#endif
