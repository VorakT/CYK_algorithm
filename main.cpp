#include <iostream>
#include <sstream>
#include "CYK_algorithm/CKYAlgorithm.cpp"


int main() {
    std::string stream, not_terminal_alphabet, terminal_alphabet;
    getline(std::cin, stream);
    getline(std::cin, not_terminal_alphabet);
    getline(std::cin, terminal_alphabet);
    std::stringstream ss(stream);
    int grammar_size;
    ss >> grammar_size;
    ss >> grammar_size;
    ss >> grammar_size;
    std::vector<std::string> grammar(grammar_size);
    for (uint32_t i = 0; i < grammar_size; ++i) {
        getline(std::cin ,grammar[i]);
    }
    std::string start_symbol_str;
    getline(std::cin, start_symbol_str);

    CKYAlgorithm parser;
    parser.fit(not_terminal_alphabet, terminal_alphabet, grammar, start_symbol_str[0]);
    std::string word_count_string;
    getline(std::cin, word_count_string);
    int word_count = std::stoi(word_count_string);
    for (uint32_t i = 0; i < word_count; ++i) {
        std::string word;
        getline(std::cin, word);
        if (parser.predict(word))
            std::cout << "Yes\n";
        else 
            std::cout << "No\n";
    }
}
