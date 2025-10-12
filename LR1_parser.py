import io
import itertools
from typing import List, Set, Dict, Tuple
import sys
import csv

class Produccion:
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def __eq__(self, other):
        return isinstance(other, Produccion) and self.left == other.left and self.right == other.right

    def __hash__(self):
        return hash((self.left, self.right))

class Item:
    def __init__(self, idx, dot_pos, lookahead):
        self.idx = idx
        self.dot_pos = dot_pos
        self.lookahead = lookahead

    def __eq__(self, other):
        return (self.idx, self.dot_pos, self.lookahead) == (other.idx, other.dot_pos, other.lookahead)

    def __hash__(self):
        return hash((self.idx, self.dot_pos, self.lookahead))

def unique_preserve_order(seq):
    seen = set()
    return [x for x in seq if not (x in seen or seen.add(x))]

def first(symbol, producciones, no_terminales, memo, visited=None):
    if visited is None:
        visited = set()
    if symbol == "ε":
        return {"ε"}
    if symbol in memo:
        return memo[symbol]
    if symbol in visited:
        return set()
    result = set()
    if symbol not in no_terminales:
        result.add(symbol)
        memo[symbol] = result
        return result
    visited.add(symbol)
    for p in producciones:
        if p.left != symbol:
            continue
        if not p.right:
            result.add('ε')
            continue
        all_nullable = True
        for sym in p.right:
            if sym == "ε":
                sym_first = {"ε"}
            else:
                sym_first = first(sym, producciones, no_terminales, memo, visited.copy())
            for t in sym_first:
                if t != 'ε':
                    result.add(t)
            if 'ε' not in sym_first:
                all_nullable = False
                break
        if all_nullable:
            result.add('ε')
    memo[symbol] = result
    return result

def beta(right, dot_pos, lookahead):
    beta_a = list(right[dot_pos + 1:])
    beta_a.append(lookahead)
    return beta_a

def first_sequence(seq, producciones, no_terminales, memo):
    result = set()
    all_nullable = True
    for sym in seq:
        f = first(sym, producciones, no_terminales, memo)
        for s in f:
            if s != 'ε':
                result.add(s)
        if 'ε' not in f:
            all_nullable = False
            break
    if all_nullable:
        result.add('ε')
    return result

def closure(I, producciones, no_terminales, memo):
    C = set(I)
    changed = True
    while changed:
        changed = False
        to_add = set()
        for item in C:
            prod = producciones[item.idx]
            if item.dot_pos < len(prod.right):
                B = prod.right[item.dot_pos]
                if B in no_terminales:
                    beta_a = beta(prod.right, item.dot_pos, item.lookahead)
                    lookaheads = first_sequence(beta_a, producciones, no_terminales, memo)
                    for j, p in enumerate(producciones):
                        if p.left == B:
                            for b in lookaheads:
                                new_item = Item(j, 0, b)
                                if new_item not in C and new_item not in to_add:
                                    to_add.add(new_item)
                                    changed = True
        C |= to_add
    return C

def goto_fn(I, X, producciones, no_terminales, memo):
    if X == 'ε':
        return set()
    J = set()
    for item in I:
        prod = producciones[item.idx]
        if not prod.right:
            continue
        if item.dot_pos < len(prod.right) and prod.right[item.dot_pos] == X:
            moved_item = Item(item.idx, item.dot_pos + 1, item.lookahead)
            J.add(moved_item)
    return closure(J, producciones, no_terminales, memo)

def parse_string(input_str, producciones, action, goto_table):
    state_stack = [0]
    symbol_stack = ["$"]
    pos = 0
    word = input_str[pos] if pos < len(input_str) else "$"
    while True:
        state = state_stack[-1]
        print(f"State stack: {state_stack}, Symbol stack: {symbol_stack}, Next word: {word}")
        if state not in action or word not in action[state]:
            print(f"Cadena rechazada (no hay acción para estado {state} y símbolo '{word}').")
            return False
        act = action[state][word]
        if act.startswith("r"):
            prod_idx = int(act[1:])
            prod = producciones[prod_idx]
            rhs_size = len(prod.right)
            for _ in range(rhs_size):
                symbol_stack.pop()
                state_stack.pop()
            top_state = state_stack[-1]
            symbol_stack.append(prod.left)
            if top_state not in goto_table or prod.left not in goto_table[top_state]:
                print(f"Cadena rechazada (no hay goto para estado {top_state} y símbolo '{prod.left}').")
                return False
            next_state = goto_table[top_state][prod.left]
            state_stack.append(next_state)
        elif act.startswith("s"):
            next_state = int(act[1:])
            symbol_stack.append(word)
            state_stack.append(next_state)
            pos += 1
            word = input_str[pos] if pos < len(input_str) else "$"
        elif act == "acc":
            print(f"Cadena aceptada.")
            return True
        else:
            print(f"Cadena rechazada (acción inválida: {act}).")
            return False

def read_grammar(filename):
    producciones = []
    with open(filename, "r", encoding="utf-8") as infile:
        for line in infile:
            line = line.strip()
            if not line:
                continue
            flecha = line.find("->")
            if flecha == -1:
                print(f"Error: la línea no contiene '->': {line}", file=sys.stderr)
                continue
            left = line[:flecha].strip()
            right_side = line[flecha + 2:].strip()
            right = right_side.split()
            if len(right) == 1 and right[0] == 'ε':
                right = []
            producciones.append(Produccion(left, tuple(right)))
    return producciones

def main():
    producciones = read_grammar("gramatica.txt")
    noTerminales = set(prod.left for prod in producciones)
    all_symbols = set(sym for prod in producciones for sym in prod.right)
    all_symbols.discard("ε")

    # --- Para esta gramática, fija el orden y limpia duplicados ---
    terminales = ["c", "d", "$", "a", "(", ")"]
    goto_order = ["S'", "S", "C"]
    no_terminales = [nt for nt in goto_order if nt in noTerminales]

    terminales = unique_preserve_order(terminales)
    goto_order = unique_preserve_order(goto_order)
    no_terminales = unique_preserve_order(no_terminales)
    header = ["State"] + terminales + [nt for nt in goto_order if nt in no_terminales] + [nt for nt in no_terminales if nt not in goto_order]

    print("\nPRODUCCIONES:")
    for i, prod in enumerate(producciones):
        right_str = " ".join(prod.right) if prod.right else "ε"
        print(f"{i}: {prod.left} -> {right_str}")

    estados = []
    estado_id = {}
    action = {}
    goto_table = {}
    memo = {}

    I0 = {Item(0, 0, "$")}
    closure0 = closure(I0, producciones, noTerminales, memo)
    estados.append(closure0)
    estado_id[frozenset(closure0)] = 0
    kernels = [I0]
    closures = [closure0]
    idx = 0
    while idx < len(estados):
        I = estados[idx]
        simbolos = set(sym for prod in producciones for sym in prod.right)
        simbolos.add("$")
        simbolos.discard("ε")
        for X in simbolos:
            goto_I_X_kernel = set()
            for item in I:
                prod = producciones[item.idx]
                if not prod.right:
                    continue
                if item.dot_pos < len(prod.right) and prod.right[item.dot_pos] == X:
                    goto_I_X_kernel.add(Item(item.idx, item.dot_pos + 1, item.lookahead))
            if goto_I_X_kernel:
                goto_I_X = closure(goto_I_X_kernel, producciones, noTerminales, memo)
                if frozenset(goto_I_X) not in estado_id:
                    nuevo_id = len(estados)
                    estados.append(goto_I_X)
                    estado_id[frozenset(goto_I_X)] = nuevo_id
                    kernels.append(goto_I_X_kernel)
                    closures.append(goto_I_X)
        idx += 1
    for X in goto_order:
        goto0 = goto_fn(closure0, X, producciones, noTerminales, memo)
        if goto0 and frozenset(goto0) not in estado_id:
            nuevo_id = len(estados)
            estados.append(goto0)
            estado_id[frozenset(goto0)] = nuevo_id
    for idx, I in enumerate(estados):
        simbolos = set(sym for prod in producciones for sym in prod.right)
        simbolos.add("$")
        simbolos.discard("ε")
        for X in simbolos:
            goto_I_X = goto_fn(I, X, producciones, noTerminales, memo)
            if goto_I_X:
                if frozenset(goto_I_X) not in estado_id:
                    nuevo_id = len(estados)
                    estados.append(goto_I_X)
                    estado_id[frozenset(goto_I_X)] = nuevo_id
                to_id = estado_id[frozenset(goto_I_X)]
                if X in terminales:
                    action.setdefault(idx, {})[X] = f"s{to_id}"
                elif X in no_terminales:
                    goto_table.setdefault(idx, {})[X] = to_id
        for it in I:
            prod = producciones[it.idx]
            if it.dot_pos == len(prod.right):
                if prod.left == "S'" and it.lookahead == "$":
                    action.setdefault(idx, {})["$"] = "acc"
                else:
                    action.setdefault(idx, {})[it.lookahead] = f"r{it.idx}"

    # Exportar la tabla LR(1) a archivo CSV
    with open("LR1_table.csv", "w", newline='', encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        for i in range(len(estados)):
            row = [str(i)]
            for t in terminales:
                val = action.get(i, {}).get(t, "")
                if isinstance(val, list):
                    val = ";".join(val)
                row.append(val)
            for nt in goto_order:
                if nt in no_terminales:
                    row.append(str(goto_table.get(i, {}).get(nt, "")))
            for nt in no_terminales:
                if nt not in goto_order:
                    row.append(str(goto_table.get(i, {}).get(nt, "")))
            writer.writerow(row)
    print("\nTabla LR(1) exportada a LR1_table.csv")

    input_line = input("\nIngrese la cadena: ")
    input_tokens = input_line.strip().split()
    parse_string(input_tokens, producciones, action, goto_table)

if __name__ == "__main__":
    main()