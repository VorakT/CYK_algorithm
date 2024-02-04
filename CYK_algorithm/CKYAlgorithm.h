#pragma once
#include <vector>
#include <map>
#include <string>


const uint32_t kEmpty = 0;
const uint32_t kStartId = 1;
const uint32_t kNotTerminal = 0;
const uint32_t kTerminal = 1;


struct Symbol {
  uint32_t id;
  bool is_terminal;

  Symbol() = default;

  Symbol(uint32_t id, bool is_terminal);

  bool operator==(const Symbol&) const;
};


struct Grammar {
  Symbol left;
  std::vector<Symbol> right;

  bool operator==(const Grammar&) const;
};


class CKYAlgorithm {
private:
  std::vector<Grammar> grammar;
  std::map<char, uint32_t> terminal_id, not_terminal_id;
  uint32_t terminal_size, not_terminal_size;
  bool generates_empty;

  void convert_to_Chomsky_normal_form();

  void eliminate_non_generative_symbols();

  void eliminate_unreachable_symbols();

  void eliminate_mixed_rules();

  void eliminate_long_rules();

  void eliminate_empty_rules();

  void eliminate_single_rules();

  void eliminate_same_rules();

  //void debug();

public:
  void fit(const std::string& not_terminal_alphabet, const std::string& terminal_alphabet, 
            const std::vector<std::string>& given_grammar, char start_symbol);

  bool predict(const std::string& word_to_predict);
};