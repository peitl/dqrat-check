#include "dqrat_check.hh"
#include "parser.hh"
#include <fstream>

namespace DQRATCheck {

	void DQRATCheck::readDQBF(string filename) {
	  std::ifstream ifs(filename);
	  Parser().readQDIMACS(dqbf, ifs);
	  //dqbf.propagate();
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
			  lits.clear();
		  } else {
			  lits.push_back(get_dqbf().internalize_literal(token));
		  }
	  }
	  ifs.close();
	}

}
