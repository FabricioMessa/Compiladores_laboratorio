import sys
import csv
from enum import Enum, auto

class TokenType(Enum):
    IDENTIFIER = auto()
    INT = auto()
    STRING = auto()
    FLOAT = auto()
    BOOLV = auto()
    BOOLF = auto()
    CREATE = auto()
    PAPER = auto()
    IF = auto()
    ELSE = auto()
    THEN = auto()
    FROM = auto()
    TO = auto()
    WHILE = auto()
    IS = auto()
    RETURN = auto()
    IN = auto()
    CALCULATE = auto()
    SQRT = auto()
    QBIC = auto()
    ASSIGN = auto()
    PLUS = auto()
    MINUS = auto()
    MULTI = auto()
    DIVISION = auto()
    IN_OP = auto()
    OUT_OP = auto()
    IN_LV = auto()
    OUT_LV = auto()
    SIMILAR = auto()
    LESS_THAN = auto()
    GREATER_THAN = auto()
    LESS_EQUAL = auto()
    GREATER_EQUAL = auto()
    NOT_EQUAL = auto()
    POSITION = auto()
    NOM = auto()
    INCREMENT = auto()
    DECREMENT = auto()
    POWER = auto()
    QUOTE = auto()
    END_OF_FILE = auto()
    SEPARATOR = auto()
    STRING_LITERAL = auto()
    UNKNOWN = auto()

class Token:
    def __init__(self, token_type, value, line, column):
        self.type = token_type
        self.value = value
        self.line = line
        self.column = column
    def __repr__(self):
        return f"Token: {self.type.name:<15}| Valor: '{self.value}'\t| Linea: {self.line}\t| Columna: {self.column}"

class Scanner:
    def __init__(self, source_code):
        self.source = source_code
        self.current_pos = -1
        self.current_char = ''
        self.line = 1
        self.column = 0
        self.keywords = { "int": TokenType.INT, 
                        "string": TokenType.STRING, 
                        "float": TokenType.FLOAT, 
                        "boolv": TokenType.BOOLV, 
                        "boolf": TokenType.BOOLF, 
                        "create": TokenType.CREATE, 
                        "paper": TokenType.PAPER, 
                        "if": TokenType.IF, 
                        "else": TokenType.ELSE, 
                        "then": TokenType.THEN, 
                        "from": TokenType.FROM,
                        "to": TokenType.TO, 
                        "while": TokenType.WHILE, 
                        "is": TokenType.IS, 
                        "return": TokenType.RETURN, 
                        "in": TokenType.IN, 
                        "calculate": TokenType.CALCULATE, 
                        "sqrt": TokenType.SQRT, 
                        "qbic": TokenType.QBIC 
                    }
        self.advance()

    def advance(self):
        self.current_pos += 1
        if self.current_pos < len(self.source):
            self.current_char = self.source[self.current_pos]
            self.column += 1
            if self.current_char == '\n': self.line += 1; self.column = 0
        else: self.current_char = None

    def peek(self):
        peek_pos = self.current_pos + 1
        return self.source[peek_pos] if peek_pos < len(self.source) else None

    def get_identifier_or_keyword(self):
        value, start_col = '', self.column
        while self.current_char is not None and (self.current_char.isalnum() or self.current_char == '_'):
            value += self.current_char
            self.advance()
        token_type = self.keywords.get(value.lower(), TokenType.IDENTIFIER)

        if token_type in [TokenType.INT, TokenType.STRING, TokenType.FLOAT, TokenType.BOOLV, TokenType.BOOLF] and self.peek() not in ['=', ' ']:
             return Token(TokenType.IDENTIFIER, value, self.line, start_col)
        return Token(token_type, value, self.line, start_col)

    def get_number(self):
        value = ''
        start_col = self.column
        is_float = False
        while self.current_char is not None and (self.current_char.isdigit() or self.current_char == '.'):
            if self.current_char == '.': 
                is_float = True
            value += self.current_char; self.advance()
        return Token(TokenType.FLOAT if is_float else TokenType.INT, value, self.line, start_col)
    
    def get_next_token(self):
        while self.current_char is not None:
            if self.current_char.isspace():
                self.advance()
                continue

            if self.current_char == '"':
                self.advance()
                value = ''
                start_col = self.column
                
                while self.current_char is not None and self.current_char != '"':
                    value += self.current_char
                    self.advance()
                
                if self.current_char == '"':
                    self.advance()
                    return Token(TokenType.STRING_LITERAL, value, self.line, start_col)
                else:
                    print(f"Error: Cadena de texto no cerrada en {self.line}:{start_col}")
                    return Token(TokenType.UNKNOWN, value, self.line, start_col)

            if self.current_char.isalpha() or self.current_char == '_':
                return self.get_identifier_or_keyword()

            if self.current_char.isdigit():
                return self.get_number()
            
            char, start_col = self.current_char, self.column
        
            if char == '=' and self.peek() == '=': 
                self.advance() 
                self.advance()
                return Token(TokenType.SIMILAR, '==', self.line, start_col)
            
            if char == '>' and self.peek() == '=': 
                self.advance() 
                self.advance() 
                return Token(TokenType.GREATER_EQUAL, '>=', self.line, start_col)
            
            if char == '<' and self.peek() == '=': 
                self.advance() 
                self.advance()
                return Token(TokenType.LESS_EQUAL, '<=', self.line, start_col)
            
            if char == '!' and self.peek() == '=': 
                self.advance()
                self.advance()
                return Token(TokenType.NOT_EQUAL, '!=', self.line, start_col)
            
            if char == '-' and self.peek() == '>': 
                self.advance()
                self.advance()
                return Token(TokenType.NOM, '->', self.line, start_col)
            
            if char == '+' and self.peek() == '+': 
                self.advance()
                self.advance()
                return Token(TokenType.INCREMENT, '++', self.line, start_col)
            
            if char == '-' and self.peek() == '-': 
                self.advance()
                self.advance()
                return Token(TokenType.DECREMENT, '--', self.line, start_col)


            single_char_map = { 
                '=': TokenType.ASSIGN, 
                '>': TokenType.GREATER_THAN, 
                '<': TokenType.LESS_THAN, 
                '-': TokenType.MINUS, 
                '+': TokenType.PLUS, 
                '*': TokenType.MULTI, 
                '/': TokenType.DIVISION, 
                '{': TokenType.IN_OP, 
                '}': TokenType.OUT_OP, 
                '[': TokenType.IN_LV, 
                ']': TokenType.OUT_LV, 
                ',': TokenType.POSITION, 
                '^': TokenType.POWER,
                '|': TokenType.SEPARATOR 
            }
            if char in single_char_map: 
                self.advance()
                return Token(single_char_map[char], char, self.line, start_col)

            print(f"Error: Caracter invalido '{char}' en línea {self.line}, columna {self.column}"); self.advance(); return Token(TokenType.UNKNOWN, char, self.line, start_col)
        return Token(TokenType.END_OF_FILE, '', self.line, self.column)

class Produccion:
    def __init__(self, left, right): 
        self.left = left
        self.right = tuple(right)

    def __eq__(self, other): 
        return isinstance(other, Produccion) and self.left == other.left and self.right == other.right
    
    def __hash__(self): 
        return hash((self.left, self.right))
    
    def __repr__(self): 
        return f"{self.left} -> {' '.join(self.right) if self.right else 'ε'}"

class Item:
    def __init__(self, idx, dot_pos, lookahead): 
        self.idx = idx
        self.dot_pos = dot_pos
        self.lookahead = lookahead

    def __eq__(self, other): 
        return (self.idx, self.dot_pos, self.lookahead) == (other.idx, other.dot_pos, other.lookahead)
    
    def __hash__(self): 
        return hash((self.idx, self.dot_pos, self.lookahead))
    
    def __repr__(self): 
        return f"Item(idx={self.idx}, dot_pos={self.dot_pos}, lookahead='{self.lookahead}')"

def first(symbol, producciones, no_terminales, memo):
    if symbol in memo: 
        return memo[symbol]
    
    if symbol not in no_terminales: 
        return {symbol}
    
    result = set()
    for p in producciones:
        if p.left == symbol:
            if not p.right: 
                result.add('ε')
                continue

            all_nullable = True
            for sym in p.right:
                sym_first = first(sym, producciones, no_terminales, memo)
                result.update(s for s in sym_first if s != 'ε')
                if 'ε' not in sym_first: 
                    all_nullable = False
                    break

            if all_nullable: 
                result.add('ε')

    memo[symbol] = result
    return result

def first_sequence(seq, producciones, no_terminales, memo):
    result = set()
    all_nullable = True

    for sym in seq:
        f = first(sym, producciones, no_terminales, memo)
        result.update(s for s in f if s != 'ε')
        if 'ε' not in f: 
            all_nullable = False
            break

    if all_nullable: 
        result.add('ε')

    return result

def closure(I, producciones, no_terminales, memo):
    C = set(I)

    while True:
        to_add = set()
        for item in C:
            prod = producciones[item.idx]
            if item.dot_pos < len(prod.right):
                B = prod.right[item.dot_pos]
                if B in no_terminales:
                    beta_a = list(prod.right[item.dot_pos + 1:]) + [item.lookahead]
                    lookaheads = first_sequence(beta_a, producciones, no_terminales, memo)
                    for j, p in enumerate(producciones):
                        if p.left == B:
                            for b in lookaheads:
                                new_item = Item(j, 0, b)
                                if new_item not in C: 
                                    to_add.add(new_item)

        if not to_add: 
            break

        C.update(to_add)
    return C

def goto_fn(I, X, producciones, no_terminales, memo):
    J = set()
    for item in I:
        prod = producciones[item.idx]
        if item.dot_pos < len(prod.right) and prod.right[item.dot_pos] == X:
            J.add(Item(item.idx, item.dot_pos + 1, item.lookahead))
    return closure(J, producciones, no_terminales, memo)

def parse_string(input_tokens, producciones, action, goto_table):
    state_stack = [0]
    symbol_stack = ["$"]
    pos = 0
    word = input_tokens[pos] if pos < len(input_tokens) else "$"
    print("\n--- INICIO DEL PARSEO ---")
    
    while True:
        state = state_stack[-1]
        print(f"Pila de Estados: {state_stack}, Pila de Símbolos: {symbol_stack}, Token Actual: {word}")
        
        if state not in action or word not in action[state]: 
            print(f"Error: Cadena rechazada. No hay acción para estado {state} y símbolo '{word}'.")
            return False
        
        act = action[state][word]
        if act.startswith("s"):
            next_state = int(act[1:])
            symbol_stack.append(word); state_stack.append(next_state)
            pos += 1; word = input_tokens[pos] if pos < len(input_tokens) else "$"
        
        elif act.startswith("r"):
            prod_idx = int(act[1:])
            prod = producciones[prod_idx]
            print(f"Reduciendo por la regla: {prod}")
            for _ in prod.right: 
                symbol_stack.pop()
                state_stack.pop()
            
            top_state = state_stack[-1]
            symbol_stack.append(prod.left)

            if top_state not in goto_table or prod.left not in goto_table[top_state]: 
                print(f"Error: Cadena rechazada. No hay goto para estado {top_state} y símbolo '{prod.left}'.")
                return False
            
            next_state = goto_table[top_state][prod.left]
            state_stack.append(next_state)

        elif act == "acc": 
            print("\n--- CADENA ACEPTADA ---\n")
            return True

        else: 
            print(f"Error: Cadena rechazada. Acción inválida: {act}.")
            return False

def read_grammar(filename):
    producciones = []
    with open(filename, "r", encoding="utf-8") as infile:
        for line in infile:
            line = line.split('#', 1)[0]
            line = line.strip()
            if not line:
                continue
            
            left, right_side = map(str.strip, line.split("->"))
            right = right_side.split() if right_side != "ε" else []
            producciones.append(Produccion(left, right))
    return producciones

def main():
    grammar_file = "gramatica_lenguaje.txt"
    input_file = input("Ingrese nombre del archivo: ")

    producciones = read_grammar(grammar_file)

    start_symbol_augmented = producciones[0].left
    
    no_terminales = sorted(list(set(p.left for p in producciones)))
    todos_simbolos = set(no_terminales) | set(s for p in producciones for s in p.right)
    terminales = sorted(list(todos_simbolos - set(no_terminales)))
    
    if '$' not in terminales: 
        terminales.append('$')

    #print("\nPRODUCCIONES:")
    #for i, prod in enumerate(producciones): 
        #print(f"{i}: {prod}")

    #print(f"\nTERMINALES: {terminales}")
    #print(f"\nNO TERMINALES: {no_terminales}")

    memo_first = {}
    I0 = {Item(0, 0, "$")}
    C0 = closure(I0, producciones, no_terminales, memo_first)
    
    estados = [C0]
    estado_id = {frozenset(C0): 0}
    action = {}
    goto_table = {}
    worklist = [0]

    while worklist:
        i = worklist.pop(0)
        I = estados[i]
        for X in todos_simbolos:
            goto_I_X = goto_fn(I, X, producciones, no_terminales, memo_first)
            if goto_I_X:
                if (f_goto := frozenset(goto_I_X)) not in estado_id:
                    estado_id[f_goto] = len(estados)
                    estados.append(goto_I_X)
                    worklist.append(len(estados) - 1)
                
                to_id = estado_id[f_goto]
                if X in terminales:
                    action.setdefault(i, {})[X] = f"s{to_id}"
                else:
                    goto_table.setdefault(i, {})[X] = to_id
    
    for i, I in enumerate(estados):
        for item in I:
            if item.dot_pos == len(producciones[item.idx].right):
                prod = producciones[item.idx]
                if prod.left == start_symbol_augmented: 
                    if item.lookahead == '$': 
                        action.setdefault(i, {})['$'] = "acc"
                else:
                    action.setdefault(i, {})[item.lookahead] = f"r{item.idx}"

    header = ["State"] + terminales + no_terminales
    with open("LR1_table.csv", "w", newline='', encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        for i, I in enumerate(estados):
            row = [i]
            for t in terminales: 
                row.append(action.get(i, {}).get(t, ""))
            
            for nt in no_terminales: 
                row.append(goto_table.get(i, {}).get(nt, ""))
            writer.writerow(row)
    #print("\nTabla LR(1) generada y exportada a LR1_table.csv")

    try:
        with open(input_file, 'r') as file:
            source_code = file.read()
    except FileNotFoundError:
        print(f"Error: No se pudo abrir el archivo '{input_file}'")
        return
        
    scanner = Scanner(source_code)
    input_tokens = []
    print("\nTOKENS GENERADOS POR EL SCANNER")
    while True:
        token = scanner.get_next_token()
        print(token)
        if token.type == TokenType.END_OF_FILE:
            break
        elif token.type != TokenType.UNKNOWN:
            input_tokens.append(token.type.name)
    
    parse_string(input_tokens, producciones, action, goto_table)

if __name__ == "__main__":
    main()