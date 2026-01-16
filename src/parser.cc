#include "parser.hh"
#include "dqbf.hh"
#include <assert.h>

using std::istream;
using std::string;

namespace DQRATCheck {

	const char QTYPE_FORALL = 'a';
	const char QTYPE_EXISTS = 'e';
	const char QTYPE_UNDEF = 0;

	const string QDIMACS_QTYPE_FORALL = "a";
	const string QDIMACS_QTYPE_EXISTS = "e";
	const string QDIMACS_QTYPE_DEPEND = "d";

	istream& Parser::getline(istream& ifs, std::string& str) {
		++current_line;
		return std::getline(ifs, str);
	}

	/*void Parser::readAUTO(istream& ifs) {
		int first_char = ifs.peek();
		while (isspace(first_char)) {
			ifs.get();
			first_char = ifs.peek();
		}
		if (first_char == 'p' || first_char == 'c') {
			readQDIMACS(ifs);
		} else {
			readQCIR(ifs);
		}
	}*/

	// returns false if matrix unsat by unit propagation
	bool Parser::readQDIMACS(DQBF& dqbf, istream& ifs) {

		string token;

		// discard leading comment lines
		ifs >> token;
		while (token == "c") {
			getline(ifs, token);
			ifs >> token;
		}

		string line;

		assert(token == "p");
		ifs >> token;
		assert(token == "cnf");

		// the declared number of clauses: when reading from the standard input, no more than this many will be read
		uint32_t num_clauses;
		// the declared bound on the number of variables
		int32_t max_var;

		ifs >> max_var;
		ifs >> num_clauses;

		dqbf.notifyMaxVarDeclaration(max_var);
		dqbf.notifyNumClausesDeclaration(num_clauses);

		// this calls the internal wrapper around std::getline, which also updates current_line
		current_line = 1;
		getline(ifs, line);

		// the map that converts old variable name to the new one
		vector<Variable> var_conversion_map(max_var+1, 0);

		char current_qtype = QTYPE_UNDEF;
		int32_t current_var;
		//int vars_seen = 0;

		bool empty_formula = true;

		// Read the prefix.
		vector<Variable> all_universals_so_far;
		while (ifs >> token) {
			if (token == QDIMACS_QTYPE_FORALL) {
				current_qtype = QTYPE_FORALL;
			} else if (token == QDIMACS_QTYPE_EXISTS) {
				current_qtype = QTYPE_EXISTS;
			} else if (token == QDIMACS_QTYPE_DEPEND) {
				Variable new_exi;
				ifs >> new_exi;
				if (var_conversion_map[new_exi] != 0) {
					duplicate_variable_error(new_exi);
				}
				vector<Variable> dependency_set;
				ifs >> current_var;
				while (current_var != 0) {
					if (current_var < 0 || current_var > max_var) {
						variable_out_of_bounds_error(current_var);
					}
					dependency_set.push_back(current_var);
					ifs >> current_var;
				}
				var_conversion_map[new_exi] = dqbf.addVarExists(new_exi, dependency_set);

				// skip the rest of the line and continue main loop
				getline(ifs, line);
				continue;
			} else {
				/* The prefix has ended.
				 * We have, however, already read the first literal of the first clause.
				 * Therefore, when we jump out of the loop, we must take that literal into account.
				 */
				empty_formula = false;
				break;
			}

			ifs >> current_var;
			while (current_var != 0) {
				//vars_seen++;
				if (current_var < 0 || current_var > max_var) {
					variable_out_of_bounds_error(current_var);
				}
				if (var_conversion_map[current_var] != 0) {
					duplicate_variable_error(current_var);
				}
				if (current_qtype == QTYPE_FORALL) {
					all_universals_so_far.push_back(current_var);
					var_conversion_map[current_var] = dqbf.addVarForall(current_var);
				} else if (current_qtype == QTYPE_EXISTS) {
					var_conversion_map[current_var] = dqbf.addVarExists(current_var, all_universals_so_far);
				}
				// TODO if linear prefix line, add variable with explicit dependencies that include all
				// universal variables so far
				ifs >> current_var;
			}
			getline(ifs, line);
		}

		// Handle the case of an empty formula
		if (!empty_formula) {
			int32_t literal = stoi(token);

			uint32_t clauses_seen = 0;

			// Read the matrix.
			do {
				vector<Literal> temp_clause;
				while (literal != 0) {
					// Convert the read literal into the corresponding literal according to the renaming of variables.
					int32_t variable = abs(literal);
					if (variable > max_var) {
						variable_out_of_bounds_error(variable);
					}
					if (var_conversion_map[variable] == 0) {
						free_variable_error(literal);
					}
					temp_clause.push_back(mkLiteral(var_conversion_map[variable], literal > 0));
					ifs >> literal;
				}
				clauses_seen++;
				sort(temp_clause.begin(), temp_clause.end());
				bool tautological = false;
				for (unsigned i = 1; i < temp_clause.size(); i++) {
					if (temp_clause[i] == ~temp_clause[i-1]) {
						// Tautological clause, do not add.
						tautological = true;
						break;
					}
				}
				if (!tautological) {
					if (dqbf.addConstraint(temp_clause) == CRef_Undef) {
						std::cout << "formula unsat by unit propagation after clause " << clauses_seen << std::endl;;
						return false;
					}
				}
				getline(ifs, line);
			} while ((&ifs != &std::cin || clauses_seen < num_clauses) && ifs >> literal);
		}
		return true;
	}

	char * Parser::uintToCharArray(uint32_t x) {
		uint32_t num_digits = 0;
		uint32_t t = x;
		do {
			t /= 10;
			num_digits++;
		} while(t);
		char * result = new char[num_digits + 2];
		sprintf(result, "%d ", x);
		return result;
	}

}
