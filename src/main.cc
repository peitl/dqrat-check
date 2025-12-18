#include <string>
#include "dqrat_check.hh"

using std::string;

int main(int argc, char** argv) {
	string filename = "";
	for (int i = 1; i < argc; i++) {
		filename = string(argv[i]);
	}
	if (filename == "") {
		std::cout << "no filename given" << std::endl;
		return 1;
	}
	DQRATCheck::DQRATCheck checker = DQRATCheck::DQRATCheck();
	checker.readDQBF(filename);

	return 0;
}
