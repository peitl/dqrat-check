#include "watched_literal_propagator.hh"
#include "constraint_DB.hh"

namespace DQRATCheck {

	WatchedLiteralPropagator::WatchedLiteralPropagator(ConstraintDB& constraint_database):
	   	trail(vector<vector<Literal>>(1)),
		constraint_database(constraint_database),
		constraints_watched_by(vector<vector<CRef>>(2)),
		occurrences_of(vector<vector<CRef>>(2)) {}

	CRef WatchedLiteralPropagator::propagate() {
		while (!propagation_queue.empty()) {
			Literal to_propagate = propagation_queue.back();
			propagation_queue.pop_back();
			//LOG(trace) << "Propagating literal: " << (sign(to_propagate) ? "" : "-") << var(to_propagate) << std::endl;
			Literal watcher = ~to_propagate;
			vector<CRef>& record_vector = constraints_watched_by[toInt(watcher)];
			vector<CRef>::iterator i, j;
			for (i = j = record_vector.begin(); i != record_vector.end(); ++i) {
				//++checker.statistics.watched_list_accesses;
				CRef& constraint_reference = *i;
				bool watcher_changed = false;
				Constraint& constraint = constraint_database.getConstraint(constraint_reference);
				if (constraintIsWatchedByLiteral(constraint, watcher) && !constraint.isMarked()) { // if we want to allow for removal, add "&& !constraint.isMarked()"
					//std::cout << "processing clause " << constraint << std::endl;
					if (!updateWatchedLiterals(constraint, constraint_reference, watcher_changed)) {
						// Constraint is empty: clean up, return constraint_reference.
						for (; i != record_vector.end(); i++, j++) {
							*j = *i;
						}
						record_vector.resize(j - record_vector.begin(), CRef_Undef);
						return constraint_reference;
					}
				} else {
					//++checker.statistics.spurious_watch_events;
					watcher_changed = true;
				}
				if (!watcher_changed) {
					*j++ = constraint_reference;
				}
			}
			record_vector.resize(j - record_vector.begin(), CRef_Undef);
		}
		//assert(propagationCorrect());
		return CRef_Undef;
	}

	// returns false if the formula becomes unsatisfiable under unit propagation
	bool WatchedLiteralPropagator::addConstraint(CRef constraint_reference) {
		Constraint& constraint = constraint_database.getConstraint(constraint_reference);
		if (constraint.size() == 0) {
			return false;
		} else if (constraint.size() == 1) {
			// TODO make sure that unit constraints are delete-able
			enqueue(constraint[0]);
			return propagate() == CRef_Undef;
		} else {
			for (Literal l : constraint) {
				occurrences_of[toInt(l)].push_back(constraint_reference);
			}
			constraints_watched_by[toInt(constraint[0])].push_back(constraint_reference);
			constraints_watched_by[toInt(constraint[1])].push_back(constraint_reference);
			bool watcher_changed;
			return updateWatchedLiterals(constraint_database.getConstraint(constraint_reference), constraint_reference, watcher_changed) && propagate();
		}
	}

	void WatchedLiteralPropagator::relocConstraintReferences() {
		for (unsigned literal_int = Min_Literal_Int; literal_int < constraints_watched_by.size(); literal_int++) {
			vector<CRef>& watched_records = constraints_watched_by[literal_int];
			vector<CRef>::iterator i, j;
			for (i = j = watched_records.begin(); i != watched_records.end(); ++i) {
				CRef constraint_reference = *i;
				Constraint& constraint = constraint_database.getConstraint(constraint_reference);
				if (!constraint.isMarked()) {
					constraint_database.relocate(constraint_reference);
					*j++ = *i;
				}
			}
			watched_records.resize(j - watched_records.begin(), CRef_Undef);
		}
	}

	bool WatchedLiteralPropagator::updateWatchedLiterals(Constraint& constraint, CRef constraint_reference, bool& watcher_changed) {
		watcher_changed = false;
		if (isDisabled(constraint)) {
			return true;
		}

		if (!assigned(var(constraint[1]))) {
			std::swap(constraint[0], constraint[1]);
		}

		unsigned int i = 2; 
		if (assigned(var(constraint[0]))) {
			for (; i < constraint.size(); i++) {
				if (!assigned(var(constraint[i]))) {
					std::swap(constraint[0], constraint[i]);
					constraints_watched_by[toInt(constraint[0])].emplace_back(constraint_reference);
					watcher_changed = true;
					break;
				}
			}
			if (!watcher_changed) {
				//assert(solver.debug_helper->isEmpty(constraint, constraint_type));
				return false;
			}
		}

		if (assigned(var(constraint[1]))) {
			for (; i < constraint.size(); i++) {
				if (!assigned(var(constraint[i]))) {
					std::swap(constraint[1], constraint[i]);
					constraints_watched_by[toInt(constraint[1])].emplace_back(constraint_reference);
					watcher_changed = true;
					break;
				}
			}

			if (assigned(var(constraint[1]))) {
				enqueue(constraint[0]);
			}
		}

		return true;
	}


	bool WatchedLiteralPropagator::constraintIsWatchedByLiteral(Constraint& constraint, Literal l) {
		return (l == constraint[0]) || (l == constraint[1]);
	}

	bool WatchedLiteralPropagator::satisfied(Literal literal) {
		//std::cout << "is_assigned,value of variable " << var(literal) << " is " << is_assigned[var(literal)-1] << "," << value[var(literal)-1] << std::endl;
		return is_assigned[var(literal)-1] && (value[var(literal)-1] == sign(literal));
	}

	bool WatchedLiteralPropagator::assigned(Variable v) {
		return is_assigned[v-1];
	}


	bool WatchedLiteralPropagator::isDisabled(Constraint& constraint) {
		for (unsigned i = 0; i < constraint.size(); i++) {
			if (satisfied(constraint[i])) {
				return true;
			}
		}
		return false;
	}

}
