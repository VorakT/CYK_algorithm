#include "CKYAlgorithm.h"
#include <iostream>
#include <algorithm>

Symbol::Symbol(uint32_t id, bool is_terminal) : id(id), is_terminal(is_terminal) {}

bool Symbol::operator==(const Symbol&) const = default;

bool Grammar::operator==(const Grammar&) const = default;

void CKYAlgorithm::fit(const std::string& not_terminal_alphabet, const std::string& terminal_alphabet, 
                        const std::vector<std::string>& given_grammar, char start_symbol) {
    generates_empty = false;
    not_terminal_id.clear();
    terminal_id.clear();
    uint32_t id = 2;
    for (char c : not_terminal_alphabet)
        not_terminal_id[c] = id++;
    not_terminal_size = id;
    id = 0;
    for (char c : terminal_alphabet)
        terminal_id[c] = id++;
    terminal_size = id;
    grammar.assign(given_grammar.size(), {});
    for (uint32_t i = 0; i < given_grammar.size(); ++i) {
        const auto& str = given_grammar[i];
        std::size_t pos = str.find("->");
        if (pos == std::string::npos)
            throw std::invalid_argument("No -> in grammar rule");
        grammar[i].left.id = 1;
        grammar[i].left.is_terminal = false;
        for (uint32_t j = 0; j < pos; ++j) {
            if (str[j] == ' ')
                continue;
            if (grammar[i].left.id != 1)
                throw std::invalid_argument("Must be only one symbol in the left side");
            if (not_terminal_id.find(str[j]) == not_terminal_id.end())
                throw std::invalid_argument("Symbol is not in not terminal alphabet");
            grammar[i].left.id = not_terminal_id[str[j]];
        }
        for (uint32_t j = pos + 2; j < str.size(); ++j) {
            if (str[j] == ' ')
                continue;
            Symbol s;
            if (not_terminal_id.find(str[j]) != not_terminal_id.end()) {
                s.is_terminal = false;
                s.id = not_terminal_id[str[j]];
            }
            else if (terminal_id.find(str[j]) != terminal_id.end()) {
                s.is_terminal = true;
                s.id = terminal_id[str[j]];
            }
            else
                throw std::invalid_argument("Symbol is not in alphabet");
            grammar[i].right.push_back(s);
        }
    }
    if (not_terminal_id.find(start_symbol) == not_terminal_id.end())
        throw std::invalid_argument("Start symbol is not in not terminal alphabet");
    uint32_t start_id = not_terminal_id[start_symbol];
    Grammar start;
    start.left = Symbol(kStartId, false);
    start.right = { Symbol(start_id, false) };
    grammar.push_back(start);
    convert_to_Chomsky_normal_form();
}


void CKYAlgorithm::convert_to_Chomsky_normal_form() {
    eliminate_mixed_rules();
    eliminate_long_rules();
    eliminate_empty_rules();
    eliminate_single_rules();
    eliminate_non_generative_symbols();
    eliminate_unreachable_symbols();
    eliminate_same_rules();
}


void CKYAlgorithm::eliminate_non_generative_symbols() {
    std::vector<std::vector<uint32_t>> right(not_terminal_size);
    std::vector<uint32_t> non_generative(grammar.size(), 0);
    std::vector<uint32_t> is_generative(not_terminal_size, 0);
    std::vector<uint32_t> stack;
    stack.reserve(not_terminal_size);
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        const auto& gr = grammar[i];
        for (const auto& symb : gr.right) {
            if (!symb.is_terminal) {
                right[symb.id].push_back(i);
                non_generative[i]++;
            }
        }
        if (non_generative[i] == 0) {
            if (!is_generative[gr.left.id]) {
                stack.push_back(gr.left.id);
                is_generative[gr.left.id] = 1;
            }
        }
    }
    while (!stack.empty()) {
        uint32_t id = stack.back();
        stack.pop_back();
        for (uint32_t grammar_id : right[id]) {
            non_generative[grammar_id]--;
            const auto& gr = grammar[grammar_id];
            if (non_generative[grammar_id] == 0 && !is_generative[gr.left.id]) {
                stack.push_back(gr.left.id);
                is_generative[gr.left.id] = 1;
            }
        }
    }
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        if (non_generative[i] == 0) {
            new_grammar.push_back(std::move(grammar[i]));
        }
    }
    new_grammar.swap(grammar);
}


void CKYAlgorithm::eliminate_unreachable_symbols() {
    std::vector<std::vector<uint32_t>> left(not_terminal_size);
    std::vector<uint32_t> reachable(not_terminal_size, 0);
    std::vector<uint32_t> stack;
    stack.reserve(not_terminal_size);
    reachable[kStartId] = 1;
    stack.push_back(kStartId);
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        left[grammar[i].left.id].push_back(i);
    }
    while (!stack.empty()) {
        uint32_t symbol_id = stack.back();
        stack.pop_back();
        for (uint32_t grammar_id : left[symbol_id]) {
            for (const auto& right_symbol : grammar[grammar_id].right) {
                if (!right_symbol.is_terminal && !reachable[right_symbol.id]) {
                    reachable[right_symbol.id] = 1;
                    stack.push_back(right_symbol.id);
                }
            }
        }
    }
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        if (reachable[grammar[i].left.id]) {
            new_grammar.push_back(std::move(grammar[i]));
        }
    }
    new_grammar.swap(grammar);
}


void CKYAlgorithm::eliminate_mixed_rules() {
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        auto& gr = grammar[i];
        if (gr.right.size() <= 1) {
            new_grammar.push_back(std::move(gr));
            continue;
        }
        for (auto& symbol : gr.right) {
            if (!symbol.is_terminal) {
                continue;
            }
            Grammar new_rule;
            new_rule.left = Symbol(not_terminal_size, false);
            new_rule.right = { symbol };
            new_grammar.push_back(std::move(new_rule));
            symbol = Symbol(not_terminal_size, false);
            not_terminal_size++;
        }
        new_grammar.push_back(std::move(gr));
    }
    new_grammar.swap(grammar);
}


void CKYAlgorithm::eliminate_long_rules() {
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        auto& gr = grammar[i];
        if (gr.right.size() <= 2) {
            new_grammar.push_back(std::move(gr));
            continue;
        }
        Symbol prev = std::move(gr.left);
        for (uint32_t j = 0; j < gr.right.size() - 2; ++j) {
            Grammar new_rule;
            new_rule.left = std::move(prev);
            new_rule.right = { std::move(gr.right[j]), Symbol(not_terminal_size, false) };
            new_grammar.push_back(std::move(new_rule));
            prev = Symbol(not_terminal_size, false);
            not_terminal_size++;
        }
        Grammar new_rule;
        new_rule.left = std::move(prev);
        new_rule.right = { std::move(gr.right[gr.right.size() - 2]), std::move(gr.right.back()) };
        new_grammar.push_back(std::move(new_rule));
    }
    new_grammar.swap(grammar);
}


void CKYAlgorithm::eliminate_empty_rules() {
    std::vector<std::vector<uint32_t>> right(not_terminal_size);
    std::vector<uint32_t> not_empty_generative(grammar.size(), 0);
    std::vector<uint32_t> empty_generative(not_terminal_size, 0);
    std::vector<uint32_t> stack;
    stack.reserve(not_terminal_size);
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        const auto& gr = grammar[i];
        for (const auto& symb : gr.right) {
            if (!symb.is_terminal) {
                right[symb.id].push_back(i);
                not_empty_generative[i]++;
            }
        }
        if (gr.right.empty()) {
            if (!empty_generative[gr.left.id]) {
                stack.push_back(gr.left.id);
                empty_generative[gr.left.id] = 1;
            }
        }
    }
    while (!stack.empty()) {
        uint32_t id = stack.back();
        stack.pop_back();
        for (uint32_t grammar_id : right[id]) {
            not_empty_generative[grammar_id]--;
            const auto& gr = grammar[grammar_id];
            if (not_empty_generative[grammar_id] == 0 && !empty_generative[gr.left.id]) {
                stack.push_back(gr.left.id);
                empty_generative[gr.left.id] = 1;
            }
        }
    }
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (auto& gr : grammar) {
        if (gr.right.empty())
            continue;
        new_grammar.push_back(gr);
        if (gr.right.size() != 2) {
            continue;
        }
        if (empty_generative[gr.right[0].id]) {
            Grammar new_rule = gr;
            new_rule.right.erase(new_rule.right.begin());
            new_grammar.push_back(std::move(new_rule));
        }
        if (empty_generative[gr.right[1].id]) {
            Grammar new_rule = std::move(gr);
            new_rule.right.pop_back();
            new_grammar.push_back(std::move(new_rule));
        }
    }
    new_grammar.swap(grammar);
    if (empty_generative[kStartId]) {
        generates_empty = true;
    }
}


void CKYAlgorithm::eliminate_single_rules() {
    std::vector<std::vector<uint32_t>> graph(not_terminal_size);
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        const auto& gr = grammar[i];
        graph[gr.left.id].push_back(i);
    }
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (uint32_t symbol_id = 0; symbol_id < not_terminal_size; ++symbol_id) {
        std::vector<uint32_t> used(not_terminal_size, 0);
        std::vector<uint32_t> stack;
        stack.reserve(not_terminal_size);
        stack.push_back(symbol_id);
        used[symbol_id] = 1;
        while (!stack.empty()) {
            uint32_t now_id = stack.back();
            stack.pop_back();
            for (uint32_t grammar_id : graph[now_id]) {
                auto gr = grammar[grammar_id];
                if (gr.right.size() != 1 || gr.right[0].is_terminal) {
                    gr.left = Symbol(symbol_id, false);
                    new_grammar.push_back(std::move(gr));
                    continue;
                }
                if (!used[gr.right[0].id]) {
                    used[gr.right[0].id] = 1;
                    stack.push_back(gr.right[0].id);
                }
            }
        }
    }
    new_grammar.swap(grammar);
}


void CKYAlgorithm::eliminate_same_rules() {
    std::vector<Grammar> new_grammar;
    new_grammar.reserve(grammar.size());
    for (auto& gr : grammar) {
        if (find(new_grammar.begin(), new_grammar.end(), gr) == new_grammar.end())
            new_grammar.push_back(std::move(gr));
    }
    new_grammar.swap(grammar);
    grammar.shrink_to_fit();
}



bool CKYAlgorithm::predict(const std::string& word_to_predict) {
    if (word_to_predict == "") {
        return generates_empty;
    }
    std::vector<uint32_t> word(word_to_predict.size());
    for (uint32_t i = 0; i < word_to_predict.size(); ++i) {
        if (terminal_id.find(word_to_predict[i]) == terminal_id.end())
            throw std::invalid_argument("Symbol not in terminal alphabet");
        word[i] = terminal_id[word_to_predict[i]];
    }
    std::vector<std::vector<std::vector<uint32_t>>> dp(word.size(), std::vector<std::vector<uint32_t>>(word.size() + 1,
                                                         std::vector<uint32_t>(not_terminal_size)));
    for (uint32_t i = 0; i < word.size(); ++i) {
        for (uint32_t j = 0; j < grammar.size(); ++j) {
            if (grammar[j].right.size() == 1 && grammar[j].right[0].is_terminal && grammar[j].right[0].id == word[i]) {
                dp[i][i + 1][grammar[j].left.id] = 1;
            }
        }
    }
    for (uint32_t len = 2; len <= word.size(); ++len) {
        for (uint32_t beg = 0; beg + len <= word.size(); ++beg) {
            uint32_t en = beg + len;
            for (const auto& gr : grammar) {
                if (gr.right.size() == 1)
                    continue;
                for (uint32_t mid = beg + 1; mid < en; ++mid) {
                    if (dp[beg][mid][gr.right[0].id] && dp[mid][en][gr.right[1].id]) {
                        dp[beg][en][gr.left.id] = 1;
                    }
                }
            }
        }
    }
    return dp[0][word.size()][kStartId];
}


/*void CKYAlgorithm::debug() {
    std::cout << "Grammar" << grammar.size() << '\n';
    for (const auto& gr : grammar) {
        std::cout << "left symbol: ";
        if (gr.left.is_terminal)
            std::cout << "terminal ";
        else
            std::cout << "not terminal ";
        std::cout << gr.left.id << '\n';
        std::cout << "right symbols:\n";
        for (const auto& s : gr.right) {
            if (s.is_terminal)
                std::cout << "terminal ";
            else
                std::cout << "not terminal ";
            std::cout << s.id << '\n';
        }
    }
    std::cout << generates_empty << std::endl;
}*/
