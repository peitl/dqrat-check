#ifndef watched_literal_propagator_hh
#define watched_literal_propagator_hh

#include <vector>
#include "propagator.hh"
#include "solver_types.hh"
#include "constraint.hh"

using std::vector;

namespace DQRATCheck {

  class ConstraintDB;

  class WatchedLiteralPropagator: public Propagator {

	friend ConstraintDB;

    public:
    WatchedLiteralPropagator(ConstraintDB& constraint_database);
    virtual void addVariable();
    virtual CRef propagate();
    virtual bool addConstraint(CRef constraint_reference);
    virtual void notifyAssigned(Literal l);
    virtual void notifyBacktrack(uint32_t decision_level_before);
    virtual void relocConstraintReferences();
    virtual bool satisfied(Literal literal);
	virtual void enqueue(Literal literal);
	virtual bool assigned(Variable v);

	void new_decision_level();
	inline void backtrack_before(unsigned int decision_level) {
		while (trail.size() > decision_level) {
			for (auto it = trail.back().rbegin(); it != trail.back().rend(); it++) {
				Variable v = var(*it);
				//std::cout << "unassign " << v << std::endl;
				is_assigned[v-1] = false;
			}
			trail.pop_back();
		}
	};

    protected:
    vector<vector<Literal>> trail;
    vector<bool> is_assigned;
    vector<bool> value;

    uint32_t findFirstWatcher(Constraint& constraint);
    uint32_t findSecondWatcher(Constraint& constraint);
    //bool isUnassignedOrDisablingPrimary(Literal literal);
    //bool isBlockedOrDisablingSecondary(Literal literal, Literal primary);
    bool constraintIsWatchedByLiteral(Constraint& constraint, Literal l);
    //bool propagateUnwatched(CRef constraint_reference, ConstraintType constraint_type, bool& watchers_found);
    bool isDisabled(Constraint& constraint);
    //bool isUnassignedPrimary(Literal literal, ConstraintType constraint_type);
    //bool isBlockedSecondary(Literal literal, ConstraintType constraint_type, Literal primary);
    bool updateWatchedLiterals(Constraint& constraint, CRef constraint_reference, bool& watcher_changed);
    bool propagationCorrect();

    /*struct WatchedRecord
    {
      CRef constraint_reference;
      Literal blocker;

      WatchedRecord(CRef constraint_reference, Literal blocker): constraint_reference(constraint_reference), blocker(blocker) {}

      WatchedRecord& operator=(const WatchedRecord& other) {
        constraint_reference = other.constraint_reference;
        blocker = other.blocker;
        return *this;
      }
    };*/

    ConstraintDB& constraint_database;

    vector<Literal> propagation_queue;
    vector<vector<CRef>> constraints_watched_by;
	vector<vector<CRef>> occurrences_of;
    //vector<vector<WatchedRecord>> constraints_watched_by;
    //vector<CRef> constraints_without_two_watchers;

  };

  // Implementation of inline methods.
  inline void WatchedLiteralPropagator::addVariable() {
      constraints_watched_by.emplace_back();
      constraints_watched_by.emplace_back();
      //occurrences_of.emplace_back();
      //occurrences_of.emplace_back();
	  is_assigned.push_back(false);
	  value.push_back(false);
  }

  inline void WatchedLiteralPropagator::new_decision_level() {
	  trail.push_back({});
  }

  // inline void WatchedLiteralPropagator::removeConstraint(CRef constraint_reference, ConstraintType constraint_type) {
  //   Constraint& constraint = solver.constraint_database->getConstraint(constraint_reference, constraint_type);
  //   constraint.mark();
  // }

  inline void WatchedLiteralPropagator::notifyAssigned(Literal l) {
    propagation_queue.push_back(l);
  }

  inline void WatchedLiteralPropagator::notifyBacktrack(uint32_t) {
    propagation_queue.clear();
  }

}

#endif
