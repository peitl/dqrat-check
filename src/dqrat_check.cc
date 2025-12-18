#include "dqrat_check.hh"
#include "parser.hh"
#include <fstream>

namespace DQRATCheck {

	void DQRATCheck::readDQBF(string filename) {
	  std::ifstream ifs(filename);
	  dqbf = Parser().readQDIMACS(ifs);
	}

}
