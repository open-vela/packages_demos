/*
 * expression_calc.cpp
 *
 * MIT License
 * Copyright (c) 2022 XCLZ STUDIO@W-Mai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma clang diagnostic push
#pragma ide diagnostic   ignored "readability-convert-member-functions-to-static"

#include "expression_calc.h"

#include <utility>

#define ERROR(type_, msg_, tag_) \
    Error.type = (type_);        \
    Error.msg  = (msg_);         \
    goto tag_

namespace XCLZ {

eXpressionCalc::eXpressionCalc() {
    ErrorType2Name = {
        {ErrorType::Well,               "Well Done"          },
        { ErrorType::BracketNotMatched, "Bracket Not Matched"},
        { ErrorType::FunctionNotFound,  "Function Not Found" },
        { ErrorType::EvalError,         "Evaluate Error"     },
        { ErrorType::SyntaxError,       "Syntax Error"       }
    };

    TokenLevel = {
        {'+',   1},
        { '-',  1},
        { '*',  2},
        { '/',  2},
        { '^',  2},
        { '%',  2},
        { '\\', 2},
        { '(',  0},
        { ')',  0}
    };

    NoteTable = {
        {"+",        { 2, LAMBDA_EXPR(params[0] + params[1]) }                                        },
        { "-",       { 2, LAMBDA_EXPR(params[0] - params[1]) }                                        },
        { "*",       { 2, LAMBDA_EXPR(params[0] * params[1]) }                                        },
        { "/",       { 2, LAMBDA_EXPR(params[0] / params[1]) }                                        },
        { "^",       { 2, LAMBDA_EXPR(powf(params[0], params[1])) }                                   },
        { "%",       { 2, LAMBDA_EXPR(fmod(params[0], params[1])) }                                   },
        { "\\",      { 2, LAMBDA_EXPR((int)((int)params[0] / (int)params[1])) }                       },
        { "addFunc", { 2, LAMBDA_EXPR(params[0] + params[1]) }                                        },
        { "sub",     { 2, LAMBDA_EXPR(params[0] - params[1]) }                                        },
        { "mul",     { 2, LAMBDA_EXPR(params[0] * params[1]) }                                        },
        { "div",     { 2, LAMBDA_EXPR(params[0] / params[1]) }                                        },
        { "pow",     { 2, LAMBDA_EXPR(powf(params[0], params[1])) }                                   },
        { "mod",     { 2, LAMBDA_EXPR(fmod(params[0], params[1])) }                                   },
        { "divi",    { 2, LAMBDA_EXPR((int)((int)params[0] / (int)params[1])) }                       },
        { "sqrt",    { 1, LAMBDA_EXPR(sqrt(params[0])) }                                              },
        { "abs",     { 1, LAMBDA_EXPR(fabs(params[0])) }                                              },
        { "sin",     { 1, LAMBDA_EXPR(sin(params[0])) }                                               },
        { "cos",     { 1, LAMBDA_EXPR(cos(params[0])) }                                               },
        { "tan",     { 1, LAMBDA_EXPR(tan(params[0])) }                                               },
        { "asin",    { 1, LAMBDA_EXPR(asin(params[0])) }                                              },
        { "acos",    { 1, LAMBDA_EXPR(acos((params[0]))) }                                            },
        { "atan",    { 1, LAMBDA_EXPR(atan(params[0])) }                                              },
        { "ln",      { 1, LAMBDA_EXPR(log(params[0])) }                                               },
        { "log",     { 1, LAMBDA_EXPR(log10(params[0])) }                                             },
        { "log2",    { 1, LAMBDA_EXPR(log2(params[0])) }                                              },
        { "floor",   { 1, LAMBDA_EXPR(floor(params[0])) }                                             },
        { "ceil",    { 1, LAMBDA_EXPR(ceil(params[0])) }                                              },
        { "sign",    { 1, LAMBDA_EXPR(abs(params[0]) < 1e-10 ? 0 : params[0] > 0 ? 1
                                                                              : -1) }},
        { "PI",      { 0, LAMBDA_EXPR(M_PI) }                                                         },
        { "E",       { 0, LAMBDA_EXPR(M_E) }                                                          },
    };
}

void eXpressionCalc::resetError() {
    Error.type = ErrorType::Well;
    Error.msg  = "";
}

inline bool eXpressionCalc::checkNumber() {
    return isdigit(*ExprIt)
        || ((ExprIt == Expr.begin() || *(ExprIt - 1) == '(')
            && ((*ExprIt == '-' || *ExprIt == '+') && (ExprIt + 1) != Expr.end()))
        || *ExprIt == '.' && (ExprIt + 1) != Expr.end() && isdigit(*(ExprIt + 1));
}

std::string eXpressionCalc::readNumber() {
    int         dot_count = 0;
    std::string rtn;
    while (ExprIt != Expr.end() && checkNumber()) {
        if (*ExprIt == '.') dot_count++;
        if (dot_count > 1) break;
        if (*ExprIt != '+') rtn += *ExprIt++;
        else ++ExprIt;
    }
    return rtn;
}

inline bool eXpressionCalc::checkFunc() {
    return isalnum(*ExprIt);
}

std::string eXpressionCalc::readFunc() {
    std::string rtn;
    while (ExprIt != Expr.end() && checkFunc()) rtn += *ExprIt++;
    return rtn;
}

bool eXpressionCalc::eatWhitespace() {
    while (ExprIt != Expr.end() && (isspace(*ExprIt) || *ExprIt == ',')) ExprIt++;
    return ExprIt == Expr.end();
}

bool eXpressionCalc::isNumber(const Expression_t& expr) {
    return std::all_of(expr.begin(), expr.end(), [](char ch) -> bool {
        return isdigit(ch) || ch == '.';
    });
}

Notation_t eXpressionCalc::reversePolishNotation() {
    resetError();
    Notation_t                                                      result;
    std::vector<std::string::const_iterator>                        buffer;
    std::stack<std::pair<std::string, std::string::const_iterator>> tmpFunc;
    for (ExprIt = Expr.begin(); ExprIt != Expr.end();) {
        if (eatWhitespace())
            break;

        auto ch = *ExprIt;
        if (!checkNumber()) {
            if (checkFunc()) {
                tmpFunc.push({ readFunc(), ExprIt });
                continue;
            }
            if (ch == '(' || buffer.empty()) {
                buffer.push_back(ExprIt);
            } else if (ch == ')') {
                while (!buffer.empty() && *buffer.back() != '(') {
                    result.push_back(std::string { *buffer.back() });
                    buffer.pop_back();
                }

                if (!tmpFunc.empty() && buffer.back() == tmpFunc.top().second) {
                    result.push_back(tmpFunc.top().first);
                    tmpFunc.pop();
                }

                if (buffer.empty()) {
                    ERROR(ErrorType::BracketNotMatched, std::string({ '(' }), ERROR_BUT_RETURN_RESULT_TAG);
                } else {
                    buffer.pop_back();
                }

            } else {
                auto search_res_ch   = TokenLevel.find(ch);
                auto search_res_back = TokenLevel.find(*buffer.back());
                if (search_res_ch == TokenLevel.end()) {
                    ERROR(ErrorType::SyntaxError, std::string({ ch }), ERROR_BUT_RETURN_RESULT_TAG);
                }
                if (search_res_back == TokenLevel.end()) {
                    ERROR(ErrorType::SyntaxError, std::string({ *buffer.back() }), ERROR_BUT_RETURN_RESULT_TAG);
                }
                if (TokenLevel.at(ch) <= TokenLevel.at(*buffer.back())) {
                    while (!buffer.empty() && TokenLevel.at(ch) <= TokenLevel.at(*buffer.back())) {
                        result.push_back(std::string { *buffer.back() });
                        buffer.pop_back();
                    }
                }
                buffer.push_back(ExprIt);
            }
            ++ExprIt;
        } else {
            std::string num = readNumber();
            result.push_back(num);
        }
    }

    while (!buffer.empty()) {
        result.push_back(std::string { *buffer.back() });
        buffer.pop_back();
    }

ERROR_BUT_RETURN_RESULT_TAG:
    return result;
}

double eXpressionCalc::evalNotation(const Notation_t& notation) {
    resetError();
    if (notation.empty())
        return INFINITY;

    std::stack<double> resultStack;

    for (auto& note : notation) {
        if (isNumber(note)) {
            resultStack.push(strtod(note.c_str(), nullptr));
        } else {
            if (note == "(" || note == ")") {
                ERROR(ErrorType::BracketNotMatched, note, ERROR_BUT_RETURN_INFINITY_TAG);
            }

            auto nt_it = NoteTable.find(note);
            if (nt_it == NoteTable.end()) {
                ERROR(ErrorType::FunctionNotFound, note, ERROR_BUT_RETURN_INFINITY_TAG);
            }

            const auto pCount = nt_it->second.first;
            const auto param  = new double[pCount];
            for (auto i = pCount - 1; i >= 0; i--) {
                if (resultStack.empty()) {
                    delete[] param;
                    ERROR(ErrorType::SyntaxError, note, ERROR_BUT_RETURN_INFINITY_TAG);
                }
                param[i] = resultStack.top();
                resultStack.pop();
            }

            resultStack.push(nt_it->second.second(param, pCount));

            delete[] param;
        }
    }

    if (resultStack.size() > 1) {
        ERROR(ErrorType::EvalError, "Can't Evaluate Correctly", ERROR_BUT_RETURN_RESULT_TAG);
    }

ERROR_BUT_RETURN_RESULT_TAG:
    return resultStack.top();
ERROR_BUT_RETURN_INFINITY_TAG:
    return INFINITY;
}

void eXpressionCalc::setExpression(Expression_t expression) {
    Expr = std::move(expression);
}

const Error_t& eXpressionCalc::getError() {
    return Error;
}

std::string eXpressionCalc::errorToString() {
    return ErrorType2Name[getError().type];
}

void eXpressionCalc::addFunc(const std::string& note, int pNum, MyFunc_t func) {
    NoteTable[note] = {
        pNum,
        func
    };
}

void eXpressionCalc::addToken(char token, int level) {
    TokenLevel[token] = level;
}
}

#pragma clang diagnostic pop
