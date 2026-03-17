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

#define DELETION_THRESHOLD 1000

namespace DQRATCheck {

	class DQBF;
	class DependencyManager;

	class ConstraintDB {
		
		friend class DQBF;	
		friend class WatchedLiteralPropagator;	

		public:
			//ConstraintDB(DQRATCheck& checker);
			ConstraintDB(DQBF& dqbf);
			~ConstraintDB();
			CRef addConstraint(vector<Literal>& literals);
			CRef retrieveSortedConstraint(const vector<Literal>& literals);
			Constraint& getConstraint(CRef constraint_reference);
			vector<CRef>::const_iterator constraintReferencesBegin();
			vector<CRef>::const_iterator constraintReferencesEnd();
			inline vector<CRef>::const_iterator occ_begin(Literal l) {
				return getOcc(l).begin();
			}
			inline vector<CRef>::const_iterator occ_end(Literal l) {
				return getOcc(l).end();
			}
			inline vector<CRef>::const_reverse_iterator occ_rbegin(Literal l) {
				return getOcc(l).rbegin();
			}
			inline vector<CRef>::const_reverse_iterator occ_rend(Literal l) {
				return getOcc(l).rend();
			}
			void bumpConstraintActivity(Constraint& constraint, ConstraintType constraint_type);
			virtual void notifyStart();
			void updateLBD(Constraint& constraint);
			void relocate(CRef& constraint_reference);

			void addVariable(bool is_existential); 
			inline void new_decision_level() { propagator.new_decision_level(); }
			inline void backtrack_before(unsigned int decision_level) { propagator.backtrack_before(decision_level); }
			inline CRef propagate() { return propagator.propagate(); };
			inline bool assigned(Variable v) { return propagator.assigned(v); };
			inline bool satisfied(Literal l) { return propagator.satisfied(l); };
			inline void enqueue(Literal l) { return propagator.enqueue(l); };
			inline vector<CRef>& getOcc(Literal l) { return occurrences_of[toInt(l)]; };

			void universally_reduce(vector<Literal>& literals, bool clear_independencies = false);
			void deleteClause(CRef cref);
			
			inline bool check_pathC(Literal l, const vector<Literal>& lits, size_t num_lits_vec, CRef target) {
				return dependency_manager->check_pathC(l, lits, num_lits_vec, target);
			}

		protected:
			void relocConstraintReferences();
			void relocVector(vector<CRef>& crefs);
			void relocAll();
			void cleanConstraints();
			bool isLocked(Constraint& constraint, CRef constraint_reference);

			DQBF& dqbf;
			DependencyManager* dependency_manager;
			WatchedLiteralPropagator propagator;
			ConstraintAllocator constraints;
			vector<CRef> constraint_list;
			vector<vector<CRef>> occurrences_of;
			ConstraintAllocator* ca_to;

			unsigned int num_del_cons = 0;
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
