#ifndef __dqrat_check_hh__
#define __dqrat_check_hh__

#include "dqbf.hh"
#include <string>

using std::string;

namespace DQRATCheck {

  class DQRATCheck {

    DQBF dqbf;

    public:

	void readDQBF(string filename);
    inline DQBF& get_dqbf() {
      return dqbf;
    }

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
