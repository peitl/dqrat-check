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

	  string token;
	  int numeric_token;
	  uint32_t line_ctr = 0;
	  while (ifs >> token) {
		  line_ctr++;
		  //std::cout << "line " << line_ctr << std::endl;
		  if (token == "a") {
			  // TODO add universal variables
		  } else if (token == "e") {
			  /* adds a new existential variable or updates the dependency set of an existing one
			   * expects a positive integer followed by a sequence of possibly negative integers
			   * negative numbers mean dependency removal
			   * if the first variable is unknown, it is automatically added (as existential)
			   * further unknown variables are automatically added as universal variables
			   */
			  Variable new_exi, new_exi_internal;
			  ifs >> new_exi;
			  if (!dqbf.external_var_already_exists(new_exi)) {
				  //std::cout << "creating extension variable " << new_exi << std::endl;
				  new_exi_internal = dqbf.addVarExists(new_exi, {});
			  } else {
				  new_exi_internal = dqbf.internalize(new_exi);
			  }
			  vector<Variable> dependency_set;
			  Variable current_var;
			  ifs >> current_var;
			  bool success = true;
			  while (current_var != 0) {
				  if (current_var < 0) {
					  if (dqbf.external_var_already_exists(-current_var)) {
						  // TODO check if valid
						  // delDependency should probably check itself and return bool indicator of validity
						  if (!dqbf.delDependency(new_exi_internal, dqbf.internalize(-current_var))) {
							  std::cout << "line " << line_ctr << ": removal of the dependency " << -current_var << " " << new_exi << " failed. The proof is not valid" << std::endl;
							  success = false;
							  break;
						  } else {
							  //std::cout << "successfully removed the dependency " << -current_var << " " << new_exi << std::endl;
						  }
					  }
				  } else {
					  Variable current_var_internal;
					  if (!dqbf.external_var_already_exists(current_var)) {
						  current_var_internal = dqbf.addVarForall(current_var);
					  } else {
						  current_var_internal = dqbf.internalize(current_var);
					  }
					  //std::cout << "adding dependency of " << new_exi << " on " << current_var << std::endl;
					  dqbf.addDependency(new_exi_internal, current_var_internal);
				  }
				  ifs >> current_var;
			  }
			  if (!success) {
				  break;
			  }
		  } else if (token == "d") {
			  // TODO delete clause
		  } else {
			  // DQRAT clause addition
			  numeric_token = atoi(token.c_str());
			  while (numeric_token != 0) {
				  lits.push_back(dqbf.internalize_literal(numeric_token));
				  ifs >> numeric_token;
			  }
			  /*std::cout << "line " << line_ctr << " of the proof contains the literals ";
			  for (Literal l : lits) {
				  std::cout << get_dqbf().externalize(l) << " ";
			  }
			  std::cout << std::endl;*/
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
		  }
	  }
	  ifs.close();
	}

	bool DQRATCheck::negate_and_propagate(const Literal* lits, size_t num_lits, std::function<bool(Literal)> whichlits) {
		dqbf.new_decision_level();
		bool conflict = false;
		for (size_t i = 0; i < num_lits; i++) {
			Literal l = lits[i];
			if (!whichlits(l)) {
				continue;
			}
			if (dqbf.satisfied(l)) {
				conflict = true;
				break;
			} else if (dqbf.satisfied(~l)) {
				continue;
			} else {
				dqbf.enqueue(~l);
				CRef cref = dqbf.propagate();
				if (cref != CRef_Undef) {
					conflict = true;
					break;
				}
			}
		}
		return conflict;
	}

	// negate and propagate the given literals
	bool DQRATCheck::negate_and_propagate(const vector<Literal>& lits) {
		return negate_and_propagate(&lits[0], lits.size(), [] (Literal) -> bool {return true;});
	}

	// obsolete, is checked as part of DQRATA
	bool DQRATCheck::check_RUP(const vector<Literal>& lits) {
		bool is_rup = negate_and_propagate(lits);
		std::cout << "backtracking" << std::endl;
		dqbf.backtrack_before(1);
		return is_rup;
	}

	// check DQRAT property: assuming lits[0] is the RAT literal
	bool DQRATCheck::check_DQRATA(const vector<Literal>& lits) {
		bool is_rup = negate_and_propagate(lits);
		if (is_rup) {
			dqbf.backtrack_before(1);
			return true;
		}

		if (!lits.empty() && dqbf.is_var_exists(var(lits[0]))) {
			bool is_rat = true;
			for (CRef cref : dqbf.constraint_database.occurrences_of(~lits[0])) {
				Constraint& constraint = dqbf.constraint_database.getConstraint(cref);
				auto isouter = [&lits, this] (Literal l) -> bool {
					return l != ~lits[0] && dqbf.is_var_outer_of_exivar(var(l), var(lits[0]));
				};
				if (!negate_and_propagate((Literal*) &constraint.data[0], constraint.size(), isouter)) {
					is_rat = false;
					break;
				}
				dqbf.backtrack_before(2);
			}
			/*if (is_rat) {
				std::cout << "clause is RAT" << std::endl;
			}*/
			dqbf.backtrack_before(1);
			return is_rat;
		} else {
			return false;
		}
	}

}
