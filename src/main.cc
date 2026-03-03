#include <string>
#include "dqrat_check.hh"

using std::string;

int main(int argc, char** argv) {
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
		std::cout << "no formula given" << std::endl;
		return 1;
	}
	//std::cout << proof_filename << std::endl;
	DQRATCheck::DQRATCheck checker = DQRATCheck::DQRATCheck();
	checker.readDQBF(formula_filename);
	if (checker.read_result == DQRATCheck::FORMULA_NOT_EXISTS) {
		std::cout << "file " << formula_filename << " not found" << std::endl;
	} else if (checker.read_result == DQRATCheck::READ) {
		std::cout << "formula '" << formula_filename << "' read successfully" << std::endl;
		if (proof_filename == "") {
			std::cout << "no proof given" << std::endl;
			return 1;
		}
		checker.readDQRAT(proof_filename);
		switch (checker.report.result) {
			case DQRATCheck::VERIFIED:
				std::cout << "after adding the lemma at line " << checker.report.line_no << " unit propagation derives conflict, stopping the proof check here. The proof is valid" << std::endl;
				std::cout << "VERIFIED" << std::endl;
				break;
			case DQRATCheck::FAILED:
				if (checker.report.rule[0] == DQRATCheck::DPURE || checker.report.rule[1] == DQRATCheck::DPURE) {
					std::cout << "line " << checker.report.line_no << ": removal of the dependency " << checker.report.vars[0] << " " << checker.report.vars[1] << " failed. The proof is not valid" << std::endl;
				} else if (checker.report.rule[0] == DQRATCheck::LOCATE) {
					std::cout << "lemma at line " << checker.report.line_no << " does not exist. The proof is invalid (lemma attempted for rule " << DQRATCheck::rule_name[checker.report.rule[1]] << ")" << std::endl;

				} else if (checker.report.rule[0] == DQRATCheck::UR && checker.report.rule[1] == DQRATCheck::NORULE) {
				    std::cout << "UR lemma at line " << checker.report.line_no << " failed due to variable " << checker.report.vars[0] << std::endl;

				} else {
					std::cout << "the lemma at line " << checker.report.line_no << " has been checked for the following properties: " << DQRATCheck::rule_name[checker.report.rule[0]];
					if (checker.report.rule[1] != DQRATCheck::NORULE) {
						std::cout << ", " <<  DQRATCheck::rule_name[checker.report.rule[1]];
					}
					std:: cout << ". The check has failed. The proof is invalid" << std::endl;
					}
				std::cout << "FAILED" << std::endl;
				break;
			case DQRATCheck::UNKNOWN:
				std::cout << "lemmas are correct, but there is no conflict at the end" << std::endl;
				std::cout << "UNKNOWN" << std::endl;
				break;
			case DQRATCheck::PROOF_NOT_EXISTS:
				std::cout << "file " << proof_filename << " not found";
				break;
			case DQRATCheck::PROOF_OTHER:
				std::cout << "unknown error";
				break;
		}
	} else if (checker.read_result == DQRATCheck::UNSAT) {
		std::cout << "formula found unsat during reading in" << std::endl;
		std::cout << "VERIFIED" << std::endl;
	}

	return 0;
}
