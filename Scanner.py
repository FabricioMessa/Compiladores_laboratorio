import sys
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
    POSITION = auto()   # ,
    NOM = auto()        # ->
    INCREMENT = auto()
    DECREMENT = auto()
    POWER = auto()
    QUOTE = auto()
    
    END_OF_FILE = auto()
    UNKNOWN = auto()

class Token:
    def __init__(self, token_type, value, line, column):
        self.type = token_type
        self.value = value
        self.line = line
        self.column = column

    def __repr__(self):
        """Representación en string del Token para imprimirlo fácilmente."""
        return f"Token: {self.type.name:<15}| Valor: '{self.value}'\t| Linea: {self.line}\t| Columna: {self.column}"

class Scanner:
    def __init__(self, source_code):
        self.source = source_code
        self.current_pos = -1
        self.current_char = ''
        self.line = 1
        self.column = 0
        self.keywords = {
            "int": TokenType.INT,
            "str": TokenType.STRING,
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
            "qbic": TokenType.QBIC,
        }
        self.advance()

    def advance(self):
        """Avanza al siguiente caracter en el código fuente."""
        self.current_pos += 1
        if self.current_pos < len(self.source):
            self.current_char = self.source[self.current_pos]
            self.column += 1
            if self.current_char == '\n':
                self.line += 1
                self.column = 0
        else:
            self.current_char = None

    def peek(self):
        """Mira el siguiente caracter sin consumirlo."""
        peek_pos = self.current_pos + 1
        if peek_pos < len(self.source):
            return self.source[peek_pos]
        return None

    def skip_whitespace_and_comments(self):
        while self.current_char is not None and (self.current_char.isspace() or self.current_char == '/'):
            if self.current_char.isspace():
                self.advance()
            elif self.current_char == '/':
                if self.peek() == '/':
                    while self.current_char is not None and self.current_char != '\n':
                        self.advance()
                elif self.peek() == '*':
                    self.advance() 
                    self.advance() 
                    while self.current_char is not None:
                        if self.current_char == '*' and self.peek() == '/':
                            self.advance()
                            self.advance()
                            break
                        self.advance()
                else:
                    break # Es una división, no un comentario

    def get_identifier_or_keyword(self):
        value = ''
        start_col = self.column
        while self.current_char is not None and self.current_char.isalnum():
            value += self.current_char
            self.advance()
        
        token_type = self.keywords.get(value, TokenType.IDENTIFIER)
        return Token(token_type, value, self.line, start_col)

    def get_number(self):
        value = ''
        start_col = self.column
        is_float = False
        while self.current_char is not None and (self.current_char.isdigit() or self.current_char == '.'):
            if self.current_char == '.':
                if is_float:
                    print(f"Error: Número flotante mal formado en {self.line}:{start_col}")
                    self.advance()
                    return Token(TokenType.UNKNOWN, value, self.line, start_col)
                is_float = True
            value += self.current_char
            self.advance()
        
        if is_float:
            return Token(TokenType.FLOAT, value, self.line, start_col)
        else:
            return Token(TokenType.INT, value, self.line, start_col)

    def get_next_token(self):
        self.skip_whitespace_and_comments()

        if self.current_char is None:
            return Token(TokenType.END_OF_FILE, '', self.line, self.column)

        if self.current_char.isalpha():
            return self.get_identifier_or_keyword()

        if self.current_char.isdigit():
            return self.get_number()
        
        start_col = self.column
        char = self.current_char
        
        if char == '=':
            self.advance()
            if self.current_char == '=':
                self.advance()
                return Token(TokenType.SIMILAR, '==', self.line, start_col)
            return Token(TokenType.ASSIGN, '=', self.line, start_col)
        
        if char == '>':
            self.advance()
            if self.current_char == '=':
                self.advance()
                return Token(TokenType.GREATER_EQUAL, '>=', self.line, start_col)
            return Token(TokenType.GREATER_THAN, '>', self.line, start_col)

        if char == '<':
            self.advance()
            if self.current_char == '=':
                self.advance()
                return Token(TokenType.LESS_EQUAL, '<=', self.line, start_col)
            return Token(TokenType.LESS_THAN, '<', self.line, start_col)
            
        if char == '!':
            self.advance()
            if self.current_char == '=':
                self.advance()
                return Token(TokenType.NOT_EQUAL, '!=', self.line, start_col)
            else:
                print(f"Error léxico: Carácter '!' no válido en {self.line}:{start_col}")
                return Token(TokenType.UNKNOWN, '!', self.line, start_col)

        if char == '-':
            self.advance()
            if self.current_char == '>':
                self.advance()
                return Token(TokenType.NOM, '->', self.line, start_col)
            elif self.current_char == '-':
                self.advance()
                return Token(TokenType.DECREMENT, '--', self.line, start_col)
            return Token(TokenType.MINUS, '-', self.line, start_col)

        if char == '+':
            self.advance()
            if self.current_char == '+':
                self.advance()
                return Token(TokenType.INCREMENT, '++', self.line, start_col)
            return Token(TokenType.PLUS, '+', self.line, start_col)
            
        single_char_tokens = {
            '*': TokenType.MULTI, '/': TokenType.DIVISION,
            '{': TokenType.IN_OP, '}': TokenType.OUT_OP,
            '[': TokenType.IN_LV, ']': TokenType.OUT_LV,
            ',': TokenType.POSITION, '^': TokenType.POWER,
            '"': TokenType.QUOTE
        }

        if char in single_char_tokens:
            token_type = single_char_tokens[char]
            self.advance()
            return Token(token_type, char, self.line, start_col)
        
        print(f"Error: Caracter invalido '{char}' en línea {self.line}, columna {self.column}")
        self.advance()
        return Token(TokenType.UNKNOWN, char, self.line, start_col)

def main():
    file_path = input("Ingrese la ruta del archivo: ")
    try:
        with open(file_path, 'r') as file:
            source_code = file.read()
    except FileNotFoundError:
        print(f"Error: no se puede abrir el archivo '{file_path}'.", file=sys.stderr)
        return

    scanner = Scanner(source_code)
    while True:
        token = scanner.get_next_token()
        if token.type != TokenType.UNKNOWN:
             print(token)
        if token.type == TokenType.END_OF_FILE:
            break
            
    print("Análisis completado.")

if __name__ == "__main__":
    main()