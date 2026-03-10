#ifndef __dqrat_check_hh__
#define __dqrat_check_hh__

#include "dqbf.hh"
#include <string>

using std::string;

namespace DQRATCheck {

  enum ReadResult {
	  READ,
	  UNSAT,
	  FORMULA_NOT_EXISTS,
	  READ_OTHER
  };

  enum CheckResult {
	  VERIFIED,
	  FAILED,
	  UNKNOWN,
	  PROOF_NOT_EXISTS,
	  PROOF_OTHER,
	  RESULT_COUNT
  };

  enum DQRATRule {
	  NORULE,
	  LOCATE,
	  DEL,
	  UADD,
	  RUP,
	  UR,
	  DQRATE,
	  DQRATU,
	  DPURE,
	  RULE_COUNT
  };

  struct DQRATECheckResult {
	  bool success;
	  CRef blocker;
  };

  extern string rule_name[DQRATRule::RULE_COUNT];
  extern string result_name[CheckResult::RESULT_COUNT];

  struct DQRATCheckReport {
	  CheckResult result = UNKNOWN;
	  uint32_t line_no; // line number which failed to verify or which completed the proof
	  DQRATRule rule[2] = {NORULE, NORULE}; // which rules were attempted in case of failure
	  Variable vars[2] = {0, 0}; // which variables conflicted in case of dependency failure
	  CRef cref = CRef_Undef; // which variables conflicted in case of dependency failure
  };

  class DQRATCheck {

    DQBF dqbf;

	bool negate_and_propagate(const Literal* lits, size_t num_lits, std::function<bool(Literal)> whichlits);
	bool negate_and_propagate(const vector<Literal>& lits);

    public:

	ReadResult read_result = READ_OTHER;
	DQRATCheckReport report;

	void readDQRAT(string filename);
	void scanClause(std::istream& ifs, vector<Literal>& lits);
	bool readDQBF(string filename);
    inline DQBF& get_dqbf() {
      return dqbf;
    }

	bool check_RUP(const vector<Literal>& lits);
	DQRATECheckResult check_DQRATE(const vector<Literal>& lits);
	bool check_DQRATU(const vector<Literal>& lits, Literal pivot);

    inline Constraint& get_clause(CRef cref) {
      return dqbf.constraint_database.getConstraint(cref);
    }

	void printResult();

    struct {
      uint64_t propagations = 0;
      uint64_t watched_list_accesses = 0;
	  uint64_t lemmas_of_type[DQRATRule::RULE_COUNT];
    } statistics;

  };


}

#endif
