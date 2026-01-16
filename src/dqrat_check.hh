#ifndef __dqrat_check_hh__
#define __dqrat_check_hh__

#include "dqbf.hh"
#include <string>

using std::string;

namespace DQRATCheck {

  class DQRATCheck {

    DQBF dqbf;

	bool negate_and_propagate(const Literal* lits, size_t num_lits, Literal skiplit);
	bool negate_and_propagate(const vector<Literal>& lits, Literal lx);

    public:

	void readDQRAT(string filename);
	bool readDQBF(string filename);
    inline DQBF& get_dqbf() {
      return dqbf;
    }

	bool check_RUP(const vector<Literal>& lits);
	bool check_DQRATA(const vector<Literal>& lits);

    inline Constraint& get_clause(CRef cref) {
      return dqbf.constraint_database.getConstraint(cref);
    }

    struct {
      uint64_t propagations = 0;
      uint64_t watched_list_accesses = 0;
      uint64_t spurious_watch_events = 0;
    } statistics;

  };


}

#endif
