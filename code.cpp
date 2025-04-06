#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <cctype>
#include <cstring>

using namespace std;

// Structure to represent tokens: number, operator, variable, or parenthesis
struct Token {
    enum Type { NUMBER, OPERATOR, VARIABLE, PARENTHESIS } type;
    string value;
    Token(Type t, string v) : type(t), value(v) {}
};

// Structure to store intermediate instructions (TAC, quadruples, etc.)
struct Instruction {
    string op, arg1, arg2, result;

    Instruction(string o, string a1, string a2, string r)
        : op(o), arg1(a1), arg2(a2), result(r) {}

    // Convert instruction to Three Address Code format
    string toString() const {
        return op == "=" ? result + " = " + arg1 : result + " = " + arg1 + " " + op + " " + arg2;
    }

    // Convert instruction to quadruple format
    string toQuadruple() const {
        return "(" + op + ", " + arg1 + ", " + (op == "=" ? " " : arg2) + ", " + result + ")";
    }
};

// Function to tokenize and convert an expression into TAC instructions
vector<Instruction> processExpression(const string& expression, const string& resultVar) {
    vector<Token> tokens;
    string current;

    // Step 1: Tokenization - Split expression into tokens
    for (size_t i = 0; i < expression.length(); i++) {
        char c = expression[i];

        if (isspace(c)) continue; // Ignore whitespaces

        // Handle numeric literals
        if (isdigit(c)) {
            current = "";
            while (i < expression.length() && (isdigit(expression[i]) || expression[i] == '.'))
                current += expression[i++];
            i--;
            tokens.push_back(Token(Token::NUMBER, current));
        }
        // Handle variables (alphanumeric identifiers)
        else if (isalpha(c)) {
            current = "";
            while (i < expression.length() && (isalnum(expression[i]) || expression[i] == '_'))
                current += expression[i++];
            i--;
            tokens.push_back(Token(Token::VARIABLE, current));
        }
        // Handle operators and parentheses
        else if (strchr("+-*/%^()", c)) {
            tokens.push_back(Token(c == '(' || c == ')' ? Token::PARENTHESIS : Token::OPERATOR, string(1, c)));
        }
    }

    // Step 2: Evaluate expression using stacks and generate TAC
    vector<Instruction> instructions;
    stack<Token> operators;
    stack<string> operands;
    int tempCount = 1; // For generating temporary variables: t1, t2, ...

    // Define operator precedence
    auto getPrecedence = [](const string& op) {
        if (op == "+" || op == "-") return 1;
        if (op == "*" || op == "/" || op == "%") return 2;
        if (op == "^") return 3;
        return 0;
    };

    // Generate TAC instruction when an operator is processed
    auto processOperator = [&]() {
        string op = operators.top().value; operators.pop();
        string arg2 = operands.top(); operands.pop();
        string arg1 = operands.top(); operands.pop();
        string tempVar = "t" + to_string(tempCount++); // New temporary variable
        instructions.push_back(Instruction(op, arg1, arg2, tempVar));
        operands.push(tempVar); // Push result back to operand stack
    };

    // Step 3: Process all tokens using operator precedence (similar to Shunting Yard)
    for (const Token& token : tokens) {
        if (token.type == Token::NUMBER || token.type == Token::VARIABLE) {
            operands.push(token.value); // Push operand directly
        }
        else if (token.type == Token::OPERATOR) {
            bool isRightAssoc = token.value == "^"; // Exponent is right-associative
            // Handle precedence and associativity
            while (!operators.empty() && operators.top().type == Token::OPERATOR &&
                   ((!isRightAssoc && getPrecedence(operators.top().value) >= getPrecedence(token.value)) ||
                    (isRightAssoc && getPrecedence(operators.top().value) > getPrecedence(token.value)))) {
                processOperator();
            }
            operators.push(token);
        }
        else if (token.value == "(") {
            operators.push(token); // Open parenthesis
        }
        else if (token.value == ")") {
            // Process until matching '('
            while (!operators.empty() && operators.top().value != "(")
                processOperator();
            if (!operators.empty()) operators.pop(); // Discard '('
        }
    }

    // Step 4: Process any remaining operators
    while (!operators.empty())
        processOperator();

    // Final assignment of the result to user-defined variable
    if (!operands.empty())
        instructions.push_back(Instruction("=", operands.top(), "", resultVar));

    return instructions;
}

// Print instructions in Triple format: (op, arg1, arg2)
void printTriples(const vector<Instruction>& instructions) {
    cout << "\nTriples:\n";
    for (size_t i = 0; i < instructions.size(); i++) {
        cout << i << ": (" << instructions[i].op << ", " << instructions[i].arg1 << ", " << instructions[i].arg2 << ")\n";
    }
}

// Print instructions in Indirect Triple format
void printIndirectTriples(const vector<Instruction>& instructions) {
    cout << "\nIndirect Triples:\n";

    // Pointer Table simply points to index of instruction
    cout << "Pointer Table:\n";
    for (size_t i = 0; i < instructions.size(); i++) {
        cout << i << " -> " << i << "\n";
    }

    // Instruction Table (same as triples but accessed via pointer)
    cout << "\nInstruction Table:\n";
    for (size_t i = 0; i < instructions.size(); i++) {
        cout << i << ": (" << instructions[i].op << ", " << instructions[i].arg1 << ", " << instructions[i].arg2 << ")\n";
    }
}

int main() {
    string expression, resultVar;

    // Input expression from user
    cout << "Enter an expression: ";
    getline(cin, expression);

    // Input variable name to store the final result
    cout << "Enter the variable to store the result: ";
    getline(cin, resultVar);

    // Generate TAC and other representations
    auto instructions = processExpression(expression, resultVar);

    // Output: Three Address Code
    cout << "\nThree Address Code (TAC):" << endl;
    for (size_t i = 0; i < instructions.size(); i++)
        cout << i + 1 << ": " << instructions[i].toString() << endl;

    // Output: Quadruples
    cout << "\nQuadruples:" << endl;
    for (size_t i = 0; i < instructions.size(); i++)
        cout << i + 1 << ": " << instructions[i].toQuadruple() << endl;

    // Output: Triples
    printTriples(instructions);

    // Output: Indirect Triples
    printIndirectTriples(instructions);

    return 0;
}
