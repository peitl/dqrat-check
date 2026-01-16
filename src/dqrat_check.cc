#include "dqrat_check.hh"
#include "parser.hh"
#include <fstream>

namespace DQRATCheck {

	bool DQRATCheck::readDQBF(string filename) {
	  std::ifstream ifs(filename);
	  bool status = Parser().readQDIMACS(dqbf, ifs);
	  if (!status) {
		  return false;
	  }
	  CRef cref = dqbf.propagate();
	  if (cref != CRef_Undef) {
		  std::cout << "Input formula propositionally unsatisfiable by unit propagation, nothing to prove" << std::endl;
	  }
	  return true;
	}

	void DQRATCheck::readDQRAT(string filename) {
	  std::ifstream ifs(filename);

	  string line;
	  vector<Literal> lits;

	  int token;
	  uint32_t line_ctr = 0;
	  while (ifs >> token) {
		  if (token == 0) {
			  line_ctr++;
			  std::cout << "line " << line_ctr << " of the proof contains the literals ";
			  for (Literal l : lits) {
				  std::cout << get_dqbf().externalize(l) << " ";
			  }
			  std::cout << std::endl;
			  if (check_DQRATA(lits)) {
				  CRef cref = dqbf.addConstraint(lits);
				  if (cref == CRef_Undef) {
					  std::cout << "after adding the lemma at line " << line_ctr << " unit propagation derives conflict, stopping the proof check here. The proof is valid" << std::endl;
					  break;
				  }
			  } else {
				  std::cout << "the lemma at line " << line_ctr << " has been checked for the following properties: RUP, RAT. The check has failed. The proof is invalid" << std::endl;
				  break;
			  }
			  lits.clear();
		  } else {
			  lits.push_back(get_dqbf().internalize_literal(token));
		  }
	  }
	  ifs.close();
	}

	bool DQRATCheck::negate_and_propagate(const Literal* lits, size_t num_lits, Literal skiplit = Literal_Undef) {
		dqbf.new_decision_level();
		bool conflict = false;
		for (size_t i = 0; i < num_lits; i++) {
			Literal l = lits[i];
			if (l == skiplit)
				continue;
			//std::cout << "check_RUP: processing " << dqbf.externalize(l) << std::endl;
			if (dqbf.satisfied(l)) {
				//std::cout << "literal satisfied " << std::endl;
				conflict = true;
				break;
			} else if (dqbf.satisfied(~l)) {
				//std::cout << "literal falsified " << std::endl;
				continue;
			} else {
				//std::cout << "variable unassigned " << std::endl;
				dqbf.enqueue(~l);
				CRef cref = dqbf.propagate();
				if (cref != CRef_Undef) {
					//std::cout << "conflict" << std::endl;
					conflict = true;
					break;
				}
			}
		}
		return conflict;
	}

	// negate and propagate the given literals, but skip lx
	bool DQRATCheck::negate_and_propagate(const vector<Literal>& lits, Literal lx = Literal_Undef) {
		return negate_and_propagate(&lits[0], lits.size(), lx);
	}

	bool DQRATCheck::check_RUP(const vector<Literal>& lits) {
		bool is_rup = negate_and_propagate(lits);
		if (is_rup) {
			std::cout << "clause found to be RUP" << std::endl;
		} else {
			std::cout << "clause is not RUP at all" << std::endl;
		}
		std::cout << "backtracking" << std::endl;
		dqbf.backtrack_before(1);
		return is_rup;
	}

	// check DQRAT property: assuming lits[0] is the RAT literal
	bool DQRATCheck::check_DQRATA(const vector<Literal>& lits) {
		bool is_rup = negate_and_propagate(lits);
		if (is_rup) {
			return true;
		}

		if (!lits.empty()) {
			bool is_rat = true;
			for (CRef cref : dqbf.constraint_database.occurrences_of(~lits[0])) {
				Constraint& constraint = dqbf.constraint_database.getConstraint(cref);
				if (!negate_and_propagate((Literal*) &constraint.data[0], constraint.size(), ~lits[0])) {
					is_rat = false;
					break;
				}
				dqbf.backtrack_before(2);
			}
			dqbf.backtrack_before(1);
			return is_rat;
		} else {
			return false;
		}
	}

}
