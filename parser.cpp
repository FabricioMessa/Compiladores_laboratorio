#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

struct produccion {
    string left;
    vector<string> right; 
};

struct Item {
    int idx;
    int dot_pos;
    string lookahead;

    bool operator<(const Item& o) const {
        return tie(idx, dot_pos, lookahead) < tie(o.idx, o.dot_pos, o.lookahead);
    }
};

vector<set<Item>> kernels;
vector<set<Item>> closures;
map<set<Item>, int> estado_id;

set<string> first(const string& symbol, const vector<produccion>& producciones, const set<string>& noTerminales, map<string, set<string>>& memo) {
    if (memo.count(symbol)) {
        return memo[symbol];
    }

    set<string> result;

    if(noTerminales.find(symbol) == noTerminales.end()) {
        result.insert(symbol);
        memo[symbol] = result;
        return result;
    }

    for (const auto& prod : producciones) {
        if (prod.left != symbol) {
            continue;
        }

        if (prod.right.empty()) {
            result.insert("ε");
            continue;
        }

        bool allNullable = true;
        for (const auto& sym : prod.right) {
            set<string> symFirst = first(sym, producciones, noTerminales, memo);
            for(const auto& t : symFirst) {
                if (t != "ε") {
                    result.insert(t);
                }
            }
            if (symFirst.find("ε") == symFirst.end()) {
                allNullable = false;
                break;
            }
        }
        if (allNullable) {
            result.insert("ε");
        }   
    }
    memo[symbol] = result;
    return result;
}

vector<string> beta(const vector<string>& right, int dot_pos, const string& lookahead) {
    vector<string> beta_a;
    for (int i = dot_pos + 1; i < right.size(); ++i) {
        beta_a.push_back(right[i]);
    }
    beta_a.push_back(lookahead);
    return beta_a;
}

set<string> first_sequence(const vector<string>& seq, const vector<produccion>& producciones, const set<string>& noTerminales, map<string, set<string>>& memo) {
    set<string> result;
    bool allNullable = true;
    
    for (const auto& sym : seq) {
        set<string> f = first(sym, producciones, noTerminales, memo);
        for (const auto& s : f)
            if (s != "ε") {
                result.insert(s);
            }
        if (f.find("ε") == f.end()) {
            allNullable = false;
            break;
        }
    }

    if (allNullable) {
        result.insert("ε");
    }
    return result;
}

set<Item> closure(const set<Item>& I, const vector<produccion>& producciones, const set<string>& noTerminales, map<string, set<string>>& memo) {
    set<Item> C = I;
    bool changed = true;

    while (changed) {
        changed = false;
        set<Item> to_add;
        
        for (const auto& item : C) {
            const produccion& prod = producciones[item.idx];
            if (item.dot_pos < prod.right.size()) {
                string B = prod.right[item.dot_pos];
                if (noTerminales.count(B)) {
                    vector<string> beta_a = beta(prod.right, item.dot_pos, item.lookahead);
                    set<string> lookaheads = first_sequence(beta_a, producciones, noTerminales, memo);
                    for (int j = 0; j < producciones.size(); ++j) {
                        if (producciones[j].left == B) {
                            for (const auto& b : lookaheads) {
                                Item new_item{j, 0, b};
                                if (C.find(new_item) == C.end() && to_add.find(new_item) == to_add.end()) {
                                    to_add.insert(new_item);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        C.insert(to_add.begin(), to_add.end());
    }
    return C;
}

set<Item> goto_fn(const set<Item>& I, const string& X, const vector<produccion>& producciones, const set<string>& noTerminales, map<string, set<string>>& memo) {
    if (X == "ε") {
        return {};
    }

    set<Item> J;
    for (const auto& item : I) {
        const produccion& prod = producciones[item.idx];
        if (prod.right.empty()) { 
            continue;
        }
        if (item.dot_pos < prod.right.size() && prod.right[item.dot_pos] == X) {
            Item moved_item{item.idx, item.dot_pos + 1, item.lookahead};
            J.insert(moved_item);
        }
    }
    return closure(J, producciones, noTerminales, memo);
}

bool parse_string(const vector<string>& input, const vector<produccion>& producciones, const map<int, map<string, string>>& action, const map<int, map<string, int>>& goto_table) {
    vector<int> state_stack;
    vector<string> symbol_stack;

    state_stack.push_back(0);    // Estado inicial
    symbol_stack.push_back("$"); // Símbolo $

    size_t pos = 0;
    string word = (pos < input.size()) ? input[pos] : "$";

    while (true) {
        int state = state_stack.back();

        // Busca la acción para el estado y el símbolo actual
        auto it = action.find(state);
        if (it == action.end() || it->second.find(word) == it->second.end()) {
            cout << "Cadena rechazada (no hay acción para estado " << state << " y símbolo '" << word << "')." << endl;
            return false;
        }

        string act = it->second.at(word);

        if (act[0] == 'r') { // reduce
            int prod_idx = stoi(act.substr(1));
            const produccion& prod = producciones[prod_idx];
            int rhs_size = prod.right.size();

            // Pop 2 × |β| elementos (símbolos y estados alternados)
            for (int i = 0; i < rhs_size; ++i) {
                symbol_stack.pop_back();
                state_stack.pop_back();
            }

            int top_state = state_stack.back();
            symbol_stack.push_back(prod.left);

            // Empuja el estado según la tabla GOTO
            if (goto_table.at(top_state).find(prod.left) == goto_table.at(top_state).end()) {
                cout << "Cadena rechazada (no hay goto para estado " << top_state << " y símbolo '" << prod.left << "')." << endl;
                return false;
            }
            int next_state = goto_table.at(top_state).at(prod.left);
            state_stack.push_back(next_state);
        }
        else if (act[0] == 's') { // shift
            int next_state = stoi(act.substr(1));
            symbol_stack.push_back(word);
            state_stack.push_back(next_state);

            ++pos;
            word = (pos < input.size()) ? input[pos] : "$";
        }
        else if (act == "acc") {
            cout << "Cadena aceptada." << endl;
            return true;
        }
        else {
            cout << "Cadena rechazada (acción inválida: " << act << ")." << endl;
            return false;
        }
    }
}

int main() {
    vector<produccion> producciones;

    ifstream infile("gramatica.txt");
    if (!infile.is_open()) {
        cerr << "Error al abrir el archivo de gramatica." << endl;
        return 1;
    }

    string line;
    while(getline(infile, line)) {
        if (line.empty()) {
            continue;
        }

        size_t flecha = line.find("->");

        if (flecha == string::npos) {
            cerr << "Error: la linea no contiene '->': " << line << endl;
            continue;
        }

        string left = line.substr(0, flecha);
        string right_side = line.substr(flecha + 2);

        left.erase(0, left.find_first_not_of(" \t"));
        left.erase(left.find_last_not_of(" \t") + 1);

        vector<string> right;
        stringstream ss(right_side);
        string simbolo;

        while (ss >> simbolo) {
            right.push_back(simbolo);
        }

        if (right.size() == 1 && right[0] == "ε") {
            right.clear();
        }

        producciones.push_back({left, right});
    }

    set<string> noTerminales;
    for (const auto& prod : producciones) {
        noTerminales.insert(prod.left);
    }

    set<string> all_symbols;
    for (const auto& prod : producciones) {
        for (const auto& sym : prod.right)
            all_symbols.insert(sym);
    }

    all_symbols.erase("ε");

    vector<string> term_order = {"c", "d", "$", "(", ")", "a", "b", "id"}; 
    vector<string> goto_order = {"S'", "S", "C", "E", "T", "F"};

    vector<string> terminales;
    for (const auto& t : term_order) {
        if (all_symbols.find(t) != all_symbols.end() || t == "$")
            terminales.push_back(t);
    }
    for (const auto& sym : all_symbols) {
        if (noTerminales.find(sym) == noTerminales.end() && find(terminales.begin(), terminales.end(), sym) == terminales.end())
            terminales.push_back(sym);
    }
    if (find(terminales.begin(), terminales.end(), "$") == terminales.end())
        terminales.push_back("$");

    vector<string> no_terminales;
    for (const auto& nt : goto_order) {
        if (noTerminales.find(nt) != noTerminales.end())
            no_terminales.push_back(nt);
    }
    for (const auto& nt : noTerminales) {
        if (find(no_terminales.begin(), no_terminales.end(), nt) == no_terminales.end())
            no_terminales.push_back(nt);
    }

    for (int i = 0; i < producciones.size(); ++i) {
        cout << i << ": " << producciones[i].left << " -> ";
        if (producciones[i].right.empty()) {
            cout << "ε";
        } else {
            for (const auto& symbol : producciones[i].right) {
                cout << symbol << " ";
            }
        }
        cout << endl;
    }

    vector<set<Item>> estados;
    map<set<Item>, int> estado_id;
    map<int, map<string, string>> action;
    map<int, map<string, int>> goto_table;
    map<string, set<string>> memo;
    set<Item> I0;
    I0.insert({0, 0, "$"});
    set<Item> closure0 = closure(I0, producciones, noTerminales, memo);
    estados.push_back(closure0);
    estado_id[closure0] = 0;
    kernels.push_back(I0);
    closures.push_back(closure0);
    
    for (size_t idx = 0; idx < estados.size(); ++idx) {
        set<Item> I = estados[idx];
        set<string> simbolos;
        for (const auto& prod : producciones) {
            for (const auto& s : prod.right) simbolos.insert(s);
        }
        simbolos.insert("$");
        simbolos.erase("ε");

        for (const auto& X : simbolos) {
            set<Item> goto_I_X_kernel;
            for (const auto& item : I) {
                const produccion& prod = producciones[item.idx];
                if (prod.right.empty()) continue;
                if (item.dot_pos < prod.right.size() && prod.right[item.dot_pos] == X) {
                    goto_I_X_kernel.insert({item.idx, item.dot_pos + 1, item.lookahead});
                }
            }
            if (!goto_I_X_kernel.empty()) {
                set<Item> goto_I_X = closure(goto_I_X_kernel, producciones, noTerminales, memo);
                if (!estado_id.count(goto_I_X)) {
                    int nuevo_id = estados.size();
                    estados.push_back(goto_I_X);
                    estado_id[goto_I_X] = nuevo_id;
                    kernels.push_back(goto_I_X_kernel);
                    closures.push_back(goto_I_X);
                }
            }
        }
    }

    // Imprimir la tabla de closure
    cout << "\nLR(1) CLOSURE TABLE:\n";
    cout << "State\tKernel\t\tClosure\n";
    for (size_t i = 0; i < kernels.size(); ++i) {
        cout << i << "\t{";
        // Imprime kernel
        bool first = true;
        for (const auto& item : kernels[i]) {
            if (!first) cout << ", ";
            const produccion& prod = producciones[item.idx];
            cout << "[";
            cout << prod.left << " → ";
            for (int k = 0; k < prod.right.size(); ++k) {
                if (k == item.dot_pos) cout << ". ";
                cout << prod.right[k] << " ";
            }
            if (item.dot_pos == prod.right.size()) cout << ". ";
            cout << ", " << item.lookahead << "]";
            first = false;
        }
        cout << "}\t{";
        first = true;
        for (const auto& item : closures[i]) {
            if (!first) cout << ", ";
            const produccion& prod = producciones[item.idx];
            cout << "[";
            cout << prod.left << " → ";
            for (int k = 0; k < prod.right.size(); ++k) {
                if (k == item.dot_pos) cout << ". ";
                cout << prod.right[k] << " ";
            }
            if (item.dot_pos == prod.right.size()) cout << ". ";
            cout << ", " << item.lookahead << "]";
            first = false;
        }
        cout << "}\n";
    }

    for (const auto& X : goto_order) {
        set<Item> goto0 = goto_fn(closure0, X, producciones, noTerminales, memo);
        if (!goto0.empty() && !estado_id.count(goto0)) {
            int nuevo_id = estados.size();
            estados.push_back(goto0);
            estado_id[goto0] = nuevo_id;
        }
    }

    for (size_t idx = 0; idx < estados.size(); ++idx) {
        set<Item> I = estados[idx];
        set<string> simbolos;
        for (const auto& prod : producciones) {
            for (const auto& s : prod.right) simbolos.insert(s);
        }
        simbolos.insert("$"); 
        simbolos.erase("ε"); 
        for (const auto& X : simbolos) {
            set<Item> goto_I_X = goto_fn(I, X, producciones, noTerminales, memo);
            if (!goto_I_X.empty()) {
                if (!estado_id.count(goto_I_X)) {
                    int nuevo_id = estados.size();
                    estados.push_back(goto_I_X);
                    estado_id[goto_I_X] = nuevo_id;
                }
                int to_id = estado_id[goto_I_X];
                if (find(terminales.begin(), terminales.end(), X) != terminales.end()) {
                    action[idx][X] = "s" + to_string(to_id);
                } else if (find(no_terminales.begin(), no_terminales.end(), X) != no_terminales.end()) {
                    goto_table[idx][X] = to_id;
                }
            }
        }

        for (const auto& it : I) {
            const produccion& prod = producciones[it.idx];
            if (it.dot_pos == prod.right.size()) { 
                if (prod.left == "S'" && it.lookahead == "$") {
                    action[idx]["$"] = "acc";
                } else {
                    action[idx][it.lookahead] = "r" + to_string(it.idx);
                }
            }
        }
    }

    cout << "\nLR(1) PARSING TABLE:\n";
    cout << "State\t";
    for (const auto& t : terminales) cout << t << "\t";
    for (const auto& nt : goto_order) {
        if (find(no_terminales.begin(), no_terminales.end(), nt) != no_terminales.end())
            cout << nt << "\t";
    }

    for (const auto& nt : no_terminales) {
        if (find(goto_order.begin(), goto_order.end(), nt) == goto_order.end())
            cout << nt << "\t";
    }
    cout << endl;
    for (size_t i = 0; i < estados.size(); ++i) {
        cout << i << "\t";
        for (const auto& t : terminales) {
            if (action[i].count(t)) cout << action[i][t] << "\t";
            else cout << "\t";
        }
        for (const auto& nt : goto_order) {
            if (find(no_terminales.begin(), no_terminales.end(), nt) != no_terminales.end()) {
                if (goto_table[i].count(nt)) cout << goto_table[i][nt] << "\t";
                else cout << "\t";
            }
        }
        for (const auto& nt : no_terminales) {
            if (find(goto_order.begin(), goto_order.end(), nt) == goto_order.end()) {
                if (goto_table[i].count(nt)) cout << goto_table[i][nt] << "\t";
                else cout << "\t";
            }
        }
        cout << endl;
    }

    cout << "\nIngrese la cadena: ";
    string input_line;
    getline(cin, input_line);
    stringstream ss(input_line);
    vector<string> input;
    string tok;
    while (ss >> tok) {
        input.push_back(tok);
    }

    parse_string(input, producciones, action, goto_table);

    return 0;
}