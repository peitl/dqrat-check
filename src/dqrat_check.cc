#include "dqrat_check.hh"
#include "parser.hh"
#include <fstream>
#include <ostream>

namespace DQRATCheck {

	string rule_name[RULE_COUNT] = {
		"NORULE",
		"LOCATE",
		"DEL",
		"UADD",
		"RUP",
		"UR",
		"DQRATE",
		"DQRATU",
		"DPURE"
	};

	string result_name[RESULT_COUNT] = {
	  "s VERIFIED",
	  "s FAILED",
	  "s UNKNOWN",
	  "s PROOF_NOT_EXISTS",
	  "s PROOF_OTHER"
	};

	void DQRATCheck::printResult() {
		std::cout << result_name[report.result] << std::endl;
	}

	bool DQRATCheck::readDQBF(string filename) {
	  std::ifstream ifs(filename);
	  if (!ifs.is_open()) {
		  read_result = FORMULA_NOT_EXISTS;
		  return false;
	  }
	  bool status = Parser().readQDIMACS(dqbf, ifs);
	  if (!status) {
		  read_result = UNSAT;
		  return false;
	  }
	  CRef cref = dqbf.propagate();
	  if (cref != CRef_Undef) {
		  read_result = UNSAT;
		  //std::cout << "Input formula propositionally unsatisfiable by unit propagation, nothing to prove" << std::endl;
	  }
	  read_result = READ;
	  return true;
	}

	void DQRATCheck::scanClause(std::istream& ifs, vector<Literal>& lits) {
		int numeric_token;
		ifs >> numeric_token;
		while (numeric_token != 0) {
			lits.push_back(dqbf.internalize_literal(numeric_token));
			ifs >> numeric_token;
		}
	}

	void DQRATCheck::readDQRAT(string filename) {
	  std::ifstream ifs(filename);
	  if (!ifs.is_open()) {
		  report = {PROOF_NOT_EXISTS, 0, {}, {}};
		  return;
	  }

	  string line;
	  vector<Literal> lits;

	  string token;
	  int numeric_token;
	  uint32_t line_ctr = 0;
	  while (ifs >> token) {
		  line_ctr++;
		  //std::cout << "line " << line_ctr << std::endl;
		  if (token == "a") {
			  // add universal variables
			  Variable current_var;
			  ifs >> current_var;
			  bool success = true;
			  while (current_var != 0) {
				  if (current_var < 0) {
					  if (dqbf.external_var_already_exists(-current_var)) {
						  success = false;
						  report = {FAILED, line_ctr, {UADD}, {current_var}};
						  break;
					  }
				  } else {
					  if (!dqbf.external_var_already_exists(current_var)) {
						  dqbf.addVarForall(current_var);
					  } else {
						  success = false;
						  report = {FAILED, line_ctr, {UADD}, {current_var}};
						  break;
					  }
				  }
				  ifs >> current_var;
			  }
			  if (!success)
				  break;
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
							  success = false;
							  report = {FAILED, line_ctr, {DPURE}, {-current_var, new_exi}};
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
			  // delete clause
			  scanClause(ifs, lits);
			  sort(lits.begin(), lits.end());
			  CRef cref = dqbf.constraint_database.retrieveSortedConstraint(lits);
			  if (cref == CRef_Undef) {
				  report = {FAILED, line_ctr, {LOCATE, DEL}, {}};
				  break;
			  }
			  dqbf.constraint_database.deleteClause(cref);
		  } else if (token == "u") {
			  // UR or DQRATU (same letter as used by the QRAT spec)
			  scanClause(ifs, lits);
			  Literal pivot = lits[0]; // remember pivot because we will sort
			  Variable pvar = var(pivot);
			  if (dqbf.is_var_exists(pvar)) {
				  report = {FAILED, line_ctr, {UR}, {dqbf.externalize(pivot)}};
				  break;
			  }
			  sort(lits.begin(), lits.end());
			  CRef cref = dqbf.constraint_database.retrieveSortedConstraint(lits);
			  if (cref == CRef_Undef) {
				  report = {FAILED, line_ctr, {LOCATE, UR}, {}};
				  break;
			  }
			  bool pivot_reducible = true;
			  for (Literal lit : lits) {
				  Variable v = var(lit);
				  if (lit == ~pivot) {
					  pivot_reducible = false;
					  break;
				  }
				  if (!dqbf.is_var_exists(v)) {
					  continue;
				  }
				  if (dqbf.is_var_outer_of_exivar(pvar, v)) {
					  pivot_reducible = false;
					  break;
				  }
			  }
			  if (pivot_reducible) {
				  statistics.lemmas_of_type[UR]++;
				  std::vector<Literal>::iterator pivot_idx = std::lower_bound(lits.begin(), lits.end(), pivot);
				  std::swap(*pivot_idx, lits.back());
				  lits.pop_back();
				  CRef cref = dqbf.addConstraint(lits);
				  if (cref == CRef_Undef) {
					  report = {VERIFIED, line_ctr , {}};
					  break;
				  }
			  } else {
				  if (check_DQRATU(lits, pivot)) {
					  statistics.lemmas_of_type[DQRATU]++;
					  CRef cref = dqbf.addConstraint(lits);
					  if (cref == CRef_Undef) {
						  report = {VERIFIED, line_ctr, {}};
						  break;
					  }
				  } else {
					  report = {FAILED, line_ctr, {UR, DQRATU}};
					  break;
				  }
			  }
		  } else {
			  // DQRAT clause addition
			  numeric_token = atoi(token.c_str());
			  while (numeric_token != 0) {
				  if (!dqbf.external_var_already_exists(abs(numeric_token))) {
					  if (lits.empty()) {
						  std::cout << "c WARNING: QRAT syntax on line " << line_ctr << std::endl;
						  std::cout << "c extension variables should be introduced with explicit dependency sets in DQRAT on lines starting with e" << std::endl;
						  Variable new_exi = abs(numeric_token);
						  dqbf.addVarExists(new_exi, dqbf.univars);
					  }
				  }
				  lits.push_back(dqbf.internalize_literal(numeric_token));
				  ifs >> numeric_token;
			  }
			  /*std::cout << "line " << line_ctr << " of the proof contains the literals ";
			  for (Literal l : lits) {
				  std::cout << get_dqbf().externalize(l) << " ";
			  }
			  std::cout << std::endl;*/
			  DQRATECheckResult res = check_DQRATE(lits);
			  if (res.success) {
				  CRef cref = dqbf.addConstraint(lits);
				  if (cref == CRef_Undef) {
					  report = {VERIFIED, line_ctr};
					  break;
				  }
			  } else {
				  report = {FAILED, line_ctr, {RUP, DQRATE}, {}, res.blocker};
				  break;
			  }
		  }
		  lits.clear();
	  }
	  ifs.close();
	}

	bool DQRATCheck::negate_and_propagate(const Literal* lits, size_t num_lits, std::function<bool(Literal)> whichlits) {
		//std::cout << "calling n&p" << std::endl;
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
				//std::cout << "dec ";
				dqbf.enqueue(~l);
				CRef cref = dqbf.propagate();
				if (cref != CRef_Undef) {
					conflict = true;
					break;
				}
			}
		}
		//std::cout << "n&p finished" << std::endl;
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

	/* check DQRAT property: assuming lits[0] is the RAT literal
	 * return values:
	 *   0 RUP
	 *   1 RAT
	 *   2 RAT
	 */
	DQRATECheckResult DQRATCheck::check_DQRATE(const vector<Literal>& lits) {
		// TODO create var(lits[0]) to depend on all universal variables
		// but print warning

		bool is_rup = negate_and_propagate(lits);
		if (is_rup) {
			statistics.lemmas_of_type[RUP]++;
			dqbf.backtrack_before(1);
			return {true, CRef_Undef};
		}

		if (!lits.empty() && dqbf.is_var_exists(var(lits[0]))) {
			CRef blocker = CRef_Undef;
			for (CRef cref : dqbf.constraint_database.getOcc(~lits[0])) {
				Constraint& constraint = dqbf.constraint_database.getConstraint(cref);
				std::function<bool(Literal)> isouter = [&lits, this] (Literal l) -> bool {
					return l != ~lits[0] && dqbf.is_var_outer_of_exivar(var(l), var(lits[0]));
				};
				if (!negate_and_propagate((Literal*) &constraint.data[0], constraint.size(), isouter)) {
					//std::cout << "c failing RAT against constraint " << constraint << std::endl;
					blocker = cref;
					break;
				}
				dqbf.backtrack_before(2);
			}
			/*if (is_rat) {
				std::cout << "clause is RAT" << std::endl;
			}*/
			dqbf.backtrack_before(1);
			if (blocker == CRef_Undef) {
				statistics.lemmas_of_type[DQRATE]++;
			}
			return {blocker == CRef_Undef, blocker};
		} else {
			return {false, CRef_Undef};
		}
	}

	// check DQRATU property: pivot is the RAT literal
	bool DQRATCheck::check_DQRATU(const vector<Literal>& lits, Literal pivot) {
		bool is_rup = negate_and_propagate(&lits[0], lits.size(), [pivot] (Literal l) -> bool {return l != pivot;});
		if (is_rup) {
			dqbf.backtrack_before(1);
			return true;
		}

		CRef cref_of_lits = dqbf.constraint_database.retrieveSortedConstraint(lits);
		bool is_rat = true;
		for (CRef cref : dqbf.constraint_database.getOcc(~pivot)) {

			Constraint& constraint = dqbf.constraint_database.getConstraint(cref);
			// check if disconnected
			// if !check_pathC(pivot, lits, cref){ //need to make check_pathC(u,original_clause, target_clause) can use Dpure of Drrs if easier/fatsre
			// rest of check...
			if (dqbf.constraint_database.check_pathC(pivot, lits, lits.size(), cref)|| cref==cref_of_lits ){

				std::function<bool(Literal)> isouter = [pivot, this] (Literal l) -> bool {
					return l != ~pivot && dqbf.is_var_outer_of_univar(var(l), var(pivot));
				};
				if (!negate_and_propagate((Literal*) &constraint.data[0], constraint.size(), isouter)) {
					is_rat = false; 
					break;
				}
				dqbf.backtrack_before(2);
			}
		}
		/*if (is_rat) {
			std::cout << "clause is RAT" << std::endl;
		}*/
		dqbf.backtrack_before(1);
		return is_rat;
	}

}
