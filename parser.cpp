#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <algorithm>

using namespace std;

// ----------- Estructuras -----------

struct Production {
    string lhs;
    vector<string> rhs;
};

struct Item {
    int prodIndex;
    int dotPos;
    string lookahead; // LR(1) lookahead

    bool operator<(const Item& other) const {
        return tie(prodIndex, dotPos, lookahead) < tie(other.prodIndex, other.dotPos, other.lookahead);
    }
    bool operator==(const Item& other) const {
        return prodIndex == other.prodIndex && dotPos == other.dotPos && lookahead == other.lookahead;
    }
};

// ----------- Gramática de ejemplo -----------
// Adapta con tus tokens y reglas
std::vector<Production> productions = {
    {"S'", {"programa"}},
    {"programa", {"CREATE", "PAPER", "IN_LV", "INT", "COMMA", "INT", "OUT_LV", "sentencia_list"}},
    {"sentencia_list", {"sentencia", "sentencia_list"}},
    {"sentencia_list", {}}, // epsilon
    {"sentencia", {"PAPER", "IN_LV", "INT", "COMMA", "INT", "OUT_LV", "contenido_celda"}},
    {"contenido_celda", {"declaracion"}},
    {"contenido_celda", {"bloque_operacion"}},
    {"declaracion", {"tipo", "ASSIGN", "valor", "NOM", "IDENTIFIER"}},
    {"tipo", {"INT"}},
    {"tipo", {"STRING"}},
    {"tipo", {"FLOAT"}},
    {"tipo", {"BOOLV"}},
    {"tipo", {"BOOLF"}},
    {"valor", {"INT_VALUE"}},
    {"valor", {"STRING_VALUE"}},
    {"valor", {"FLOAT_VALUE"}},
    {"valor", {"BOOLV"}},
    {"valor", {"BOOLF"}},
    {"bloque_operacion", {"IN_OP", "op_interna", "OUT_OP"}},
    {"op_interna", {"if_stmt", "op_interna"}},
    {"op_interna", {"while_stmt", "op_interna"}},
    {"op_interna", {"from_stmt", "op_interna"}},
    {"op_interna", {"calculate_stmt", "op_interna"}},
    {"op_interna", {"sqrt_stmt", "op_interna"}},
    {"op_interna", {"qbic_stmt", "op_interna"}},
    {"op_interna", {"assign_stmt", "op_interna"}},
    {"op_interna", {"return_stmt"}},
    {"op_interna", {}},
    {"if_stmt", {"IF", "condicion", "THEN", ""}}
};

std::set<std::string> terminals = {
    "CREATE", "PAPER", "IN_LV", "INT", "COMMA", "OUT_LV", "EOF"
    // agrega los demás terminales
};
std::set<std::string> non_terminals = {
    "S'", "programa", "sentencia_list", "sentencia"
    // agrega los demás no terminales
};

// ----------- FIRST sets -----------
map<string, set<string>> first = {
    {"CREATE", {"CREATE"}},
    {"PAPER", {"PAPER"}},
    {"IN_LV", {"IN_LV"}},
    {"INT", {"INT"}},
    {"COMMA", {"COMMA"}},
    {"OUT_LV", {"OUT_LV"}},
    {"EOF", {"EOF"}},
    {"sentencia", {"PAPER"}}, // Ejemplo
    {"sentencia_list", {"PAPER", "e"}}, // Ejemplo
    // ...
};

// ----------- Función FIRST sobre secuencia -----------
set<string> FIRST_sequence(const vector<string>& seq) {
    set<string> result;
    bool has_epsilon = true;
    for (const string& symbol : seq) {
        if (first.count(symbol)) {
            for (const auto& f : first.at(symbol)) {
                if (f != "e") result.insert(f);
            }
            if (first.at(symbol).count("e") == 0) {
                has_epsilon = false;
                break;
            }
        } else {
            result.insert(symbol);
            has_epsilon = false;
            break;
        }
    }
    if (has_epsilon) result.insert("e");
    return result;
}

// ----------- closure LR(1) -----------
set<Item> closure(const set<Item>& I) {
    set<Item> C = I;
    bool changed;
    do {
        changed = false;
        set<Item> newItems;
        for (const auto& item : C) {
            const Production& prod = productions[item.prodIndex];
            if (item.dotPos < prod.rhs.size()) {
                string B = prod.rhs[item.dotPos];
                if (non_terminals.count(B)) {
                    // Calcula beta
                    vector<string> beta;
                    if (item.dotPos + 1 < prod.rhs.size()) {
                        beta.insert(beta.end(), prod.rhs.begin() + item.dotPos + 1, prod.rhs.end());
                    }
                    beta.push_back(item.lookahead);
                    set<string> lookAheads = FIRST_sequence(beta);
                    for (int j = 0; j < productions.size(); ++j) {
                        if (productions[j].lhs == B) {
                            for (const auto& la : lookAheads) {
                                Item newItem{j, 0, la};
                                if (C.find(newItem) == C.end() && newItems.find(newItem) == newItems.end()) {
                                    newItems.insert(newItem);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        C.insert(newItems.begin(), newItems.end());
    } while (changed);
    return C;
}

// ----------- goto LR(1) -----------
set<Item> goto_set(const set<Item>& I, const string& X) {
    set<Item> result;
    for (const auto& item : I) {
        const Production& prod = productions[item.prodIndex];
        if (item.dotPos < prod.rhs.size() && prod.rhs[item.dotPos] == X) {
            result.insert(Item{item.prodIndex, item.dotPos + 1, item.lookahead});
        }
    }
    return closure(result);
}

// ----------- Construcción de estados -----------
vector<set<Item>> states;
map<pair<int, string>, int> transitions;

void build_states() {
    set<Item> start = { {0, 0, "EOF"} }; // S' → •programa, EOF
    states.push_back(closure(start));
    vector<bool> marked;
    marked.push_back(false);

    while (any_of(marked.begin(), marked.end(), [](bool m){return !m;})) {
        for (int i = 0; i < states.size(); ++i) {
            if (marked[i]) continue;
            marked[i] = true;
            set<std::string> symbols;
            for (const auto& item : states[i]) {
                const Production& prod = productions[item.prodIndex];
                if (item.dotPos < prod.rhs.size()) {
                    symbols.insert(prod.rhs[item.dotPos]);
                }
            }
            for (const auto& x : symbols) {
                set<Item> temp = goto_set(states[i], x);
                if (!temp.empty()) {
                    auto it = std::find(states.begin(), states.end(), temp);
                    int idx;
                    if (it == states.end()) {
                        states.push_back(temp);
                        marked.push_back(false);
                        idx = states.size() - 1;
                    } else {
                        idx = std::distance(states.begin(), it);
                    }
                    transitions[{i, x}] = idx;
                }
            }
        }
    }
}

// ----------- Construcción de tablas ACTION y GOTO -----------
map<pair<int, string>, string> ACTION;
map<pair<int, string>, int> GOTO;

void build_tables() {
    for (int x = 0; x < states.size(); ++x) {
        for (const auto& item : states[x]) {
            const Production& prod = productions[item.prodIndex];
            // SHIFT
            if (item.dotPos < prod.rhs.size()) {
                string a = prod.rhs[item.dotPos];
                if (terminals.count(a)) {
                    auto trans_it = transitions.find({x, a});
                    if (trans_it != transitions.end()) {
                        int k = trans_it->second;
                        ACTION[{x, a}] = "shift " + std::to_string(k);
                    }
                }
            }
            // ACCEPT
            if (prod.lhs == "S'" && item.dotPos == prod.rhs.size() && item.lookahead == "EOF") {
                ACTION[{x, "EOF"}] = "accept";
            }
            // REDUCE
            if (item.dotPos == prod.rhs.size() && prod.lhs != "S'") {
                string reduce_str = "reduce " + prod.lhs + " →";
                for (const auto& sym : prod.rhs) reduce_str += " " + sym;
                ACTION[{x, item.lookahead}] = reduce_str;
            }
        }
        // GOTO
        for (const auto& n : non_terminals) {
            auto trans_it = transitions.find({x, n});
            if (trans_it != transitions.end()) {
                int k = trans_it->second;
                GOTO[{x, n}] = k;
            }
        }
    }
}

// ----------- Impresión -----------
void print_tables() {
    cout << "\n--- ACTION Table ---\n";
    for (const auto& entry : ACTION) {
        cout << "ACTION[" << entry.first.first << ", " << entry.first.second << "] = " << entry.second << "\n";
    }
    cout << "\n--- GOTO Table ---\n";
    for (const auto& entry : GOTO) {
        cout << "GOTO[" << entry.first.first << ", " << entry.first.second << "] = " << entry.second << "\n";
    }
}

void print_states() {
    for (int i = 0; i < states.size(); ++i) {
        cout << "\nEstado " << i << ":\n";
        for (const auto& item : states[i]) {
            const Production& prod = productions[item.prodIndex];
            cout << prod.lhs << " → ";
            for (int j = 0; j < prod.rhs.size(); ++j) {
                if (j == item.dotPos) cout << "•";
                cout << prod.rhs[j] << " ";
            }
            if (item.dotPos == prod.rhs.size()) cout << "•";
            cout << ", " << item.lookahead << "\n";
        }
    }
}

// ----------- MAIN -----------
int main() {
    build_states();
    print_states();
    build_tables();
    print_tables();
    return 0;
}