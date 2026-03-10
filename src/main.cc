#include <string>
#include "dqrat_check.hh"
#include <time.h>

using std::string;

int main(int argc, char** argv) {
	clock_t clock_begin = clock();
	string formula_filename = "";
	string proof_filename = "";
	for (int i = 1; i < argc; i++) {
		if (formula_filename == "") {
			formula_filename = string(argv[i]);
		} else if (proof_filename == "") {
			proof_filename = string(argv[i]);
		}
	}
	if (formula_filename == "") {
		std::cout << "c no formula given" << std::endl;
		return 1;
	}
	//std::cout << proof_filename << std::endl;
	DQRATCheck::DQRATCheck checker = DQRATCheck::DQRATCheck();
	checker.readDQBF(formula_filename);
	if (checker.read_result == DQRATCheck::FORMULA_NOT_EXISTS) {
		std::cout << "c file " << formula_filename << " not found" << std::endl;
	} else if (checker.read_result == DQRATCheck::READ) {
		std::cout << "c formula '" << formula_filename << "' read successfully" << std::endl;
		if (proof_filename == "") {
			std::cout << "no proof given" << std::endl;
			return 1;
		}
		checker.readDQRAT(proof_filename);
		switch (checker.report.result) {
			case DQRATCheck::VERIFIED:
				std::cout << "c line " << checker.report.line_no << ": unit propagation derived conflict, stopping the proof check here. The proof is valid" << std::endl;
				break;
			case DQRATCheck::FAILED:
				switch (checker.report.rule[0]) {
					case DQRATCheck::DPURE:
						std::cout << "c line " << checker.report.line_no << ": " <<
							"removal of the dependency " << checker.report.vars[0] << " " << checker.report.vars[1] << " failed" << std::endl;
						break;
					case DQRATCheck::LOCATE:
						std::cout << "c line " << checker.report.line_no << ": " <<
							"lemma does not exist (attempted rule: " << DQRATCheck::rule_name[checker.report.rule[1]] << ")" << std::endl;
						break;
					case DQRATCheck::UADD:
						if (checker.report.vars[0] < 0) {
							std::cout << "c line " << checker.report.line_no << ": " <<
								"cannot add negative literal " << checker.report.vars[0] << std::endl;;
						} else {
							std::cout << "c line " << checker.report.line_no << ": " <<
								"cannot re-add existing variable " << checker.report.vars[0] << std::endl;;
						}
						break;
					case DQRATCheck::UR:
						if(checker.report.rule[1] == DQRATCheck::NORULE) {
							std::cout << "c line " << checker.report.line_no << ": " <<
								"cannot reduce existential literal " << checker.report.vars[0] << std::endl;
							break;
						} else {
							//fallthrough
						}
					default:
						std::cout << "c line " << checker.report.line_no << ": " <<
							"lemma was checked for: " << DQRATCheck::rule_name[checker.report.rule[0]];
							if (checker.report.rule[1] != DQRATCheck::NORULE) {
								std::cout << ", " <<  DQRATCheck::rule_name[checker.report.rule[1]];
							}
							std::cout << std::endl;
						if (checker.report.cref != DQRATCheck::CRef_Undef) {
							DQRATCheck::Constraint& clause = checker.get_dqbf().constraint_database.getConstraint(checker.report.cref);
							std::cout << "c offending side clause for DQRATE:";
							for (DQRATCheck::Literal l : clause) {
								std::cout << " " << checker.get_dqbf().externalize(l);
							}
							std::cout << std::endl;
						}
				}
				std:: cout << "c the check has failed. The proof is invalid" << std::endl;
				break;
			case DQRATCheck::UNKNOWN:
				std::cout << "c lemmas are correct, but there is no conflict at the end" << std::endl;
				break;
			case DQRATCheck::PROOF_NOT_EXISTS:
				std::cout << "c file " << proof_filename << " not found";
				break;
			case DQRATCheck::PROOF_OTHER:
				std::cout << "c unknown error";
				break;
			default:
				break;
		}
		checker.printResult();
	} else if (checker.read_result == DQRATCheck::UNSAT) {
		std::cout << "c formula found unsat during reading in" << std::endl;
		std::cout << "s VERIFIED" << std::endl;
	}

	clock_t clock_end = clock();
	double dur = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;

	std::cout << "c DQRAT-check time: " <<  dur << std::endl;
	return 0;
}
