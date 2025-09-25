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
            if (s != "ε") result.insert(s);
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
    if (X == "ε") return {}; // Nunca construir GOTO para epsilon
    set<Item> J;
    for (const auto& item : I) {
        const produccion& prod = producciones[item.idx];
        if (prod.right.empty()) continue; // Evita acceso a prod.right si está vacío
        if (item.dot_pos < prod.right.size() && prod.right[item.dot_pos] == X) {
            Item moved_item{item.idx, item.dot_pos + 1, item.lookahead};
            J.insert(moved_item);
        }
    }
    return closure(J, producciones, noTerminales, memo);
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
        // Si la producción es ε, el vector right queda vacío
        if (right.size() == 1 && right[0] == "ε") right.clear();

        producciones.push_back({left, right});
    }

    // Construcción de no terminales
    set<string> noTerminales;
    for (const auto& prod : producciones) {
        noTerminales.insert(prod.left);
    }

    // Construcción automática de terminales y no terminales
    set<string> all_symbols;
    for (const auto& prod : producciones) {
        for (const auto& sym : prod.right)
            all_symbols.insert(sym);
    }

    // Elimina ε de los terminales si existe
    all_symbols.erase("ε");

    // Orden de columnas como la imagen deseada
    vector<string> term_order = {"c", "d", "$", "(", ")", "a", "b", "id"}; // puedes agregar más si tu gramática cambia
    vector<string> goto_order = {"S'", "S", "C", "E", "T", "F"}; // puedes agregar más si tu gramática cambia

    // Terminales en el orden deseado (y se agregan los adicionales)
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

    // No terminales en el orden deseado (y se agregan los adicionales)
    vector<string> no_terminales;
    for (const auto& nt : goto_order) {
        if (noTerminales.find(nt) != noTerminales.end())
            no_terminales.push_back(nt);
    }
    for (const auto& nt : noTerminales) {
        if (find(no_terminales.begin(), no_terminales.end(), nt) == no_terminales.end())
            no_terminales.push_back(nt);
    }

    // Mostrar producciones
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

    // --- INICIO BLOQUE PARA TABLA LR(1) ---
    // Estado LR(1) = conjunto de items
    vector<set<Item>> estados;
    map<set<Item>, int> estado_id;
    map<int, map<string, string>> action;
    map<int, map<string, int>> goto_table;

    // Item inicial
    map<string, set<string>> memo;
    set<Item> I0;
    I0.insert({0, 0, "$"});
    set<Item> closure0 = closure(I0, producciones, noTerminales, memo);
    estados.push_back(closure0);
    estado_id[closure0] = 0;

    // Fuerza el orden de creación de los primeros estados GOTO desde el estado 0
    for (const auto& X : goto_order) {
        set<Item> goto0 = goto_fn(closure0, X, producciones, noTerminales, memo);
        if (!goto0.empty() && !estado_id.count(goto0)) {
            int nuevo_id = estados.size();
            estados.push_back(goto0);
            estado_id[goto0] = nuevo_id;
        }
    }

    // Ahora sigue el ciclo BFS habitual para crear los demás estados
    for (size_t idx = 0; idx < estados.size(); ++idx) {
        set<Item> I = estados[idx];
        // Para todos los símbolos posibles
        set<string> simbolos;
        for (const auto& prod : producciones) {
            for (const auto& s : prod.right) simbolos.insert(s);
        }
        simbolos.insert("$"); // Asegura el terminal $
        simbolos.erase("ε");  // Elimina ε para evitar segmentation fault
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
        // Reductions and Accept
        for (const auto& it : I) {
            const produccion& prod = producciones[it.idx];
            if (it.dot_pos == prod.right.size()) { // Punto al final
                if (prod.left == "S'" && it.lookahead == "$") {
                    action[idx]["$"] = "acc";
                } else {
                    action[idx][it.lookahead] = "r" + to_string(it.idx);
                }
            }
        }
    }

    // --- Impresión de la tabla ---
    cout << "\nLR(1) PARSING TABLE:\n";
    cout << "State\t";
    for (const auto& t : terminales) cout << t << "\t";
    for (const auto& nt : goto_order) {
        if (find(no_terminales.begin(), no_terminales.end(), nt) != no_terminales.end())
            cout << nt << "\t";
    }
    // También imprime no terminales adicionales que no están en el orden deseado
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
    // --- FIN BLOQUE PARA TABLA LR(1) ---
}