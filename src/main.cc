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
	if (checker.readDQBF(formula_filename)) {
		std::cout << "formula '" << formula_filename << "' read successfully" << std::endl;
		if (proof_filename == "") {
			std::cout << "no proof given" << std::endl;
			return 1;
		}
		checker.readDQRAT(proof_filename);
	} else {
		std::cout << "formula found unsat during reading in" << std::endl;
	}

	return 0;
}
