#include "Parser.h"

#include <cassert>
#include <iostream>

#include "TokenStream.h"

#include "../ast/Block.h"
#include "../ast/UnaryExpression.h"
#include "../ast/VariableExpression.h"
#include "../ast/ControlExpression.h"
#include "../ast/CallExpression.h"
#include "../ast/CastExpression.h"
#include "../ast/NoteExpression.h"
#include "../ast/NumberExpression.h"
#include "../ast/PostfixExpression.h"
#include "../ast/MathExpression.h"
#include "../ast/AssignExpression.h"
#include "../ast/TupleExpression.h"
#include "../util.h"

using namespace MaximParser;
using namespace MaximAst;
using namespace MaximCommon;

static std::string toUpperCase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Parser::Parser(std::unique_ptr<TokenStream> stream) : _stream(std::move(stream)) {

}

std::unique_ptr<Block> Parser::parse() {
    auto block = std::make_unique<Block>();

    while (stream()->peek().type != Token::Type::END_OF_FILE) {
        if (stream()->peek().type != Token::Type::END_OF_LINE) {
            auto expr = parseExpression();
            block->expressions.push_back(std::move(expr));
        }

        auto nextToken = stream()->next();
        if (nextToken.type == Token::Type::END_OF_FILE) break;
        expect(nextToken, Token::Type::END_OF_LINE);
    }

    return std::move(block);
}

std::unique_ptr<MaximAst::Expression> Parser::parseExpression() {
    return parseExpression(Precedence::ALL);
}

std::unique_ptr<MaximAst::Expression> Parser::parseExpression(Precedence precedence) {
    auto result = parsePrefix(precedence);
    MaximAst::Expression *lastResult;
    do {
        lastResult = result.get();
        result = parsePostfix(std::move(result), precedence);
    } while (result.get() != lastResult);
    return result;
}

std::unique_ptr<MaximAst::Expression> Parser::parsePrefix(Precedence precedence) {
    auto firstToken = stream()->peek();

    // parse basic expressions
    switch (firstToken.type) {
        case Token::Type::COLON:
            return parseColonTokenExpression(precedence);

        case Token::Type::OPEN_SQUARE:
            return parseOpenSquareTokenExpression();

        case Token::Type::NOTE:
            return parseNoteTokenExpression();

        case Token::Type::NUMBER:
            return parseNumberTokenExpression();

        case Token::Type::DOUBLE_STRING:
            return parseStringTokenExpression();

        case Token::Type::PLUS:
        case Token::Type::MINUS:
        case Token::Type::NOT:
        case Token::Type::INCREMENT:
        case Token::Type::DECREMENT:
            return parseUnaryTokenExpression();

        case Token::Type::IDENTIFIER:
            return parseIdentifierTokenExpression(precedence);

        case Token::Type::OPEN_BRACKET:
            return parseSubTokenExpression();

        default:
            throw fail(firstToken);
    }

    throw fail(firstToken);
}

std::unique_ptr<MaximAst::Expression> Parser::parsePostfix(std::unique_ptr<MaximAst::Expression> prefix,
                                                           Precedence precedence) {
    auto nextToken = stream()->peek();
    auto nextPrecedence = operatorToPrecedence(nextToken.type);
    if (precedence <= nextPrecedence) return prefix;

    switch (nextToken.type) {
        case Token::Type::CAST:
            return parseCastExpression(std::move(prefix));

        case Token::Type::INCREMENT:
        case Token::Type::DECREMENT:
            return parsePostfixExpression(std::move(prefix));

        case Token::Type::BITWISE_AND:
        case Token::Type::BITWISE_OR:
        case Token::Type::BITWISE_XOR:
        case Token::Type::LOGICAL_AND:
        case Token::Type::LOGICAL_OR:
        case Token::Type::EQUAL_TO:
        case Token::Type::NOT_EQUAL_TO:
        case Token::Type::LT:
        case Token::Type::GT:
        case Token::Type::LTE:
        case Token::Type::GTE:
        case Token::Type::PLUS:
        case Token::Type::MINUS:
        case Token::Type::TIMES:
        case Token::Type::DIVIDE:
        case Token::Type::MODULO:
        case Token::Type::POWER:
            return parseMathExpression(std::move(prefix));

        case Token::Type::ASSIGN:
        case Token::Type::PLUS_ASSIGN:
        case Token::Type::MINUS_ASSIGN:
        case Token::Type::TIMES_ASSIGN:
        case Token::Type::DIVIDE_ASSIGN:
        case Token::Type::MODULO_ASSIGN:
        case Token::Type::POWER_ASSIGN:
            return parseAssignExpression(std::move(prefix));

        default:
            return prefix;
    }
}

std::unique_ptr<MaximAst::Expression> Parser::parseColonTokenExpression(Precedence precedence) {
    auto colon = stream()->peek();
    expect(colon, Token::Type::COLON);
    return parseControlExpression("", colon.startPos);
}

std::unique_ptr<MaximAst::Expression> Parser::parseOpenSquareTokenExpression() {
    auto form = parseForm();
    auto expr = parseExpression(Precedence::UNARY);
    auto formStart = form->startPos;
    auto exprEnd = expr->endPos;
    return std::make_unique<CastExpression>(std::move(form), std::move(expr), true, formStart, exprEnd);
}

std::unique_ptr<MaximAst::Form> Parser::parseForm() {
    expect(stream()->next(), Token::Type::OPEN_SQUARE);
    auto nameToken = stream()->next();
    expect(nameToken, Token::Type::IDENTIFIER);
    FormType formType;
    if (nameToken.content == "lin") formType = FormType::LINEAR;
    else if (nameToken.content == "osc") formType = FormType::OSCILLATOR;
    else if (nameToken.content == "control") formType = FormType::CONTROL;
    else if (nameToken.content == "freq") formType = FormType::FREQUENCY;
    else if (nameToken.content == "note") formType = FormType::NOTE;
    else if (nameToken.content == "db") formType = FormType::DB;
    else if (nameToken.content == "q") formType = FormType::Q;
    else if (nameToken.content == "secs") formType = FormType::SECONDS;
    else if (nameToken.content == "beats") formType = FormType::BEATS;
    else {
        throw MaximCommon::CompileError(
            "Come on man, I don't support " + nameToken.content + " forms.",
            nameToken.startPos, nameToken.endPos
        );
    }

    auto form = std::make_unique<Form>(formType, nameToken.startPos, SourcePos(0, 0));
    auto closeToken = stream()->next();
    expect(closeToken, Token::Type::CLOSE_SQUARE);
    form->endPos = closeToken.endPos;

    return form;
}

static std::regex noteRegex("([a-gA-G]#?)([0-9]+)", std::regex::ECMAScript | std::regex::optimize);
static std::array<std::string, 12> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

std::unique_ptr<MaximAst::Expression> Parser::parseNoteTokenExpression() {
    auto noteToken = stream()->next();
    expect(noteToken, Token::Type::NOTE);

    std::smatch match;
    assert(std::regex_match(noteToken.content, match, noteRegex));

    auto noteName = toUpperCase(match[1].str());
    auto noteNum = std::distance(noteNames.begin(), std::find(noteNames.begin(), noteNames.end(), noteName));
    if ((size_t) noteNum >= noteNames.size()) {
        throw MaximCommon::CompileError(
            "Ey my man, don't you know that " + noteName + " isn't a valid note?",
            noteToken.startPos, noteToken.endPos
        );
    }

    auto octave = std::stoi(match[2].str());
    auto midiNumber = noteNum + octave * noteNames.size();
    return std::make_unique<NoteExpression>(midiNumber, noteToken.startPos, noteToken.endPos);
}

std::unique_ptr<MaximAst::Expression> Parser::parseNumberTokenExpression() {
    auto numberToken = stream()->next();
    expect(numberToken, Token::Type::NUMBER);

    auto numValue = std::stof(numberToken.content);

    auto endPos = numberToken.endPos;
    auto postMulToken = stream()->peek();
    auto valueForm = std::make_unique<Form>(FormType::LINEAR, postMulToken.startPos, postMulToken.endPos);
    if (postMulToken.type == Token::Type::IDENTIFIER) {
        auto postMulText = toUpperCase(postMulToken.content);

        auto didMatchMul = true;
        if (postMulText[0] == 'K') numValue *= 1e3;
        else if (postMulText[0] == 'M') numValue *= 1e6;
        else if (postMulText[0] == 'G') numValue *= 1e9;
        else if (postMulText[0] == 'T') numValue *= 1e12;
        else if (postMulText[0] == 'P') numValue *= 1e15;
        else didMatchMul = false;

        auto didMatchForm = true;
        auto formStart = didMatchMul ? 1u : 0u;
        if (!postMulText.compare(formStart, postMulText.npos, "HZ")) valueForm->type = FormType::FREQUENCY;
        else if (!postMulText.compare(formStart, postMulText.npos, "DB")) valueForm->type = FormType::DB;
        else if (!postMulText.compare(formStart, postMulText.npos, "Q")) valueForm->type = FormType::Q;
        else if (!postMulText.compare(formStart, postMulText.npos, "S")) valueForm->type = FormType::SECONDS;
        else if (!postMulText.compare(formStart, postMulText.npos, "B")) valueForm->type = FormType::BEATS;
        else didMatchForm = false;

        if ((didMatchMul && postMulText.size() == 1) || didMatchForm) {
            endPos = postMulToken.endPos;
            stream()->next();
        }
    }

    return std::make_unique<NumberExpression>(numValue, std::move(valueForm), numberToken.startPos, endPos);
}

std::unique_ptr<MaximAst::Expression> Parser::parseStringTokenExpression() {
    auto nameToken = stream()->next();
    expect(nameToken, Token::Type::DOUBLE_STRING);
    return parseControlExpression(nameToken.content, nameToken.startPos);
}

std::unique_ptr<MaximAst::Expression> Parser::parseUnaryTokenExpression() {
    auto typeToken = stream()->next();
    UnaryExpression::Type unaryType;
    switch (typeToken.type) {
        case Token::Type::PLUS:
            unaryType = UnaryExpression::Type::POSITIVE;
            break;
        case Token::Type::MINUS:
            unaryType = UnaryExpression::Type::NEGATIVE;
            break;
        case Token::Type::NOT:
            unaryType = UnaryExpression::Type::NOT;
            break;
        default:
            throw fail(typeToken);
    }

    auto expr = parseExpression(Precedence::UNARY);
    auto exprEnd = expr->endPos;
    return std::make_unique<UnaryExpression>(unaryType, std::move(expr), typeToken.startPos, exprEnd);
}

std::unique_ptr<MaximAst::Expression> Parser::parseIdentifierTokenExpression(Precedence precedence) {
    auto identifier = stream()->next();
    expect(identifier, Token::Type::IDENTIFIER);

    auto nextToken = stream()->peek();
    if (nextToken.type == Token::Type::OPEN_BRACKET) {
        return parseCallExpression(identifier.content, identifier.startPos);
    } else if (nextToken.type == Token::Type::COLON) {
        return parseControlExpression(identifier.content, identifier.startPos);
    } else {
        return std::make_unique<VariableExpression>(identifier.content, identifier.startPos, identifier.endPos);
    }
}

std::unique_ptr<MaximAst::AssignableExpression> Parser::parseControlExpression(std::string name, SourcePos startPos) {
    expect(stream()->next(), Token::Type::COLON);
    auto typeToken = stream()->next();
    expect(typeToken, Token::Type::IDENTIFIER);
    ControlType controlType;
    if (typeToken.content == "num") controlType = ControlType::NUMBER;
    else if (typeToken.content == "graph") controlType = ControlType::GRAPH;
    else if (typeToken.content == "midi") controlType = ControlType::MIDI;
    else if (typeToken.content == "roll") controlType = ControlType::ROLL;
    else if (typeToken.content == "num[]") controlType = ControlType::NUM_EXTRACT;
    else if (typeToken.content == "midi[]") controlType = ControlType::MIDI_EXTRACT;
    else {
        throw MaximCommon::CompileError(
            "Come on man, I don't support " + typeToken.content + " controls.",
            typeToken.startPos, typeToken.endPos
        );
    }

    std::string propertyName = "value";

    auto endPos = typeToken.endPos;
    auto propToken = stream()->peek();
    if (propToken.type == Token::Type::DOT) {
        stream()->next();
        auto propertyToken = stream()->next();
        expect(propertyToken, Token::Type::IDENTIFIER);
        propertyName = propertyToken.content;
        endPos = propertyToken.endPos;
    }

    return std::make_unique<ControlExpression>(name, controlType, propertyName, startPos, endPos);
}

std::unique_ptr<MaximAst::Expression> Parser::parseCallExpression(std::string name, SourcePos startPos) {
    auto callExpr = std::make_unique<CallExpression>(name, startPos, SourcePos(0, 0));

    expect(stream()->next(), Token::Type::OPEN_BRACKET);
    if (stream()->peek().type != Token::Type::CLOSE_BRACKET) {
        parseArguments(callExpr->arguments);
    }
    auto closeBracket = stream()->next();
    expect(closeBracket, Token::Type::CLOSE_BRACKET);
    callExpr->endPos = closeBracket.endPos;

    return std::move(callExpr);
}

std::unique_ptr<MaximAst::Expression> Parser::parseSubTokenExpression() {
    auto openBracket = stream()->next();
    expect(openBracket, Token::Type::OPEN_BRACKET);

    auto tupleExpr = std::make_unique<TupleExpression>(openBracket.startPos, SourcePos(0, 0));

    auto firstIter = true;
    auto nextToken = stream()->peek();
    while (true) {
        if (nextToken.type == Token::Type::CLOSE_BRACKET) break;
        if (!firstIter) expect(stream()->next(), Token::Type::COMMA);
        firstIter = false;

        tupleExpr->expressions.push_back(parseExpression());
        nextToken = stream()->peek();
    }
    stream()->next();

    if (tupleExpr->expressions.size() == 1) {
        return std::move(tupleExpr->expressions[0]);
    } else {
        tupleExpr->endPos = nextToken.endPos;
        return std::move(tupleExpr);
    }
}

void Parser::parseArguments(std::vector<std::unique_ptr<MaximAst::Expression>> &arguments) {
    auto firstIter = true;
    do {
        if (!firstIter) stream()->next();
        firstIter = false;

        arguments.push_back(std::move(parseExpression()));
    } while (stream()->peek().type == Token::Type::COMMA);
}

std::unique_ptr<MaximAst::Expression> Parser::parseCastExpression(std::unique_ptr<MaximAst::Expression> prefix) {
    expect(stream()->next(), Token::Type::CAST);
    auto form = parseForm();
    auto formEnd = form->endPos;
    return std::make_unique<CastExpression>(std::move(form), std::move(prefix), false, prefix->startPos, formEnd);
}

std::unique_ptr<MaximAst::Expression> Parser::parsePostfixExpression(std::unique_ptr<MaximAst::Expression> prefix) {
    auto postfixToken = stream()->next();
    PostfixExpression::Type postfixType;
    switch (postfixToken.type) {
        case Token::Type::INCREMENT:
            postfixType = PostfixExpression::Type::INCREMENT;
            break;
        case Token::Type::DECREMENT:
            postfixType = PostfixExpression::Type::DECREMENT;
            break;
        default:
            throw fail(postfixToken);
    }

    auto assignable = std::make_unique<LValueExpression>(prefix->startPos, prefix->endPos);
    pushAssignable(assignable->assignments, std::move(prefix));

    auto assignableStart = assignable->startPos;
    auto assignableEnd = assignable->endPos;
    return std::make_unique<PostfixExpression>(std::move(assignable), postfixType, assignableStart, assignableEnd);
}

std::unique_ptr<MaximAst::Expression> Parser::parseMathExpression(std::unique_ptr<MaximAst::Expression> prefix) {
    auto opToken = stream()->next();
    MaximCommon::OperatorType opType;
    switch (opToken.type) {
        case Token::Type::BITWISE_AND:
            opType = MaximCommon::OperatorType::BITWISE_AND;
            break;
        case Token::Type::BITWISE_OR:
            opType = MaximCommon::OperatorType::BITWISE_OR;
            break;
        case Token::Type::BITWISE_XOR:
            opType = MaximCommon::OperatorType::BITWISE_XOR;
            break;
        case Token::Type::LOGICAL_AND:
            opType = MaximCommon::OperatorType::LOGICAL_AND;
            break;
        case Token::Type::LOGICAL_OR:
            opType = MaximCommon::OperatorType::LOGICAL_OR;
            break;
        case Token::Type::EQUAL_TO:
            opType = MaximCommon::OperatorType::LOGICAL_EQUAL;
            break;
        case Token::Type::NOT_EQUAL_TO:
            opType = MaximCommon::OperatorType::LOGICAL_NOT_EQUAL;
            break;
        case Token::Type::LT:
            opType = MaximCommon::OperatorType::LOGICAL_LT;
            break;
        case Token::Type::GT:
            opType = MaximCommon::OperatorType::LOGICAL_GT;
            break;
        case Token::Type::LTE:
            opType = MaximCommon::OperatorType::LOGICAL_LTE;
            break;
        case Token::Type::GTE:
            opType = MaximCommon::OperatorType::LOGICAL_GTE;
            break;
        case Token::Type::PLUS:
            opType = MaximCommon::OperatorType::ADD;
            break;
        case Token::Type::MINUS:
            opType = MaximCommon::OperatorType::SUBTRACT;
            break;
        case Token::Type::TIMES:
            opType = MaximCommon::OperatorType::MULTIPLY;
            break;
        case Token::Type::DIVIDE:
            opType = MaximCommon::OperatorType::DIVIDE;
            break;
        case Token::Type::MODULO:
            opType = MaximCommon::OperatorType::MODULO;
            break;
        case Token::Type::POWER:
            opType = MaximCommon::OperatorType::POWER;
            break;
        default:
            fail(opToken);
    }
    auto postfix = parseExpression(operatorToPrecedence(opToken.type));
    auto prefixStart = prefix->startPos;
    auto postfixEnd = postfix->endPos;
    return std::make_unique<MathExpression>(std::move(prefix), opType, std::move(postfix), prefixStart, postfixEnd);
}

std::unique_ptr<MaximAst::Expression> Parser::parseAssignExpression(std::unique_ptr<MaximAst::Expression> prefix) {
    auto opToken = stream()->next();
    MaximCommon::OperatorType opType;
    switch (opToken.type) {
        case Token::Type::ASSIGN:
            opType = MaximCommon::OperatorType::IDENTITY;
            break;
        case Token::Type::PLUS_ASSIGN:
            opType = MaximCommon::OperatorType::ADD;
            break;
        case Token::Type::MINUS_ASSIGN:
            opType = MaximCommon::OperatorType::SUBTRACT;
            break;
        case Token::Type::TIMES_ASSIGN:
            opType = MaximCommon::OperatorType::MULTIPLY;
            break;
        case Token::Type::DIVIDE_ASSIGN:
            opType = MaximCommon::OperatorType::DIVIDE;
            break;
        case Token::Type::MODULO_ASSIGN:
            opType = MaximCommon::OperatorType::MODULO;
            break;
        case Token::Type::POWER_ASSIGN:
            opType = MaximCommon::OperatorType::POWER;
            break;
        default:
            fail(opToken);
    }

    auto assignable = std::make_unique<LValueExpression>(prefix->startPos, prefix->endPos);
    pushAssignable(assignable->assignments, std::move(prefix));

    auto postfix = parseExpression(operatorToPrecedence(opToken.type));

    auto assignableStart = assignable->startPos;
    auto postfixEnd = postfix->endPos;
    return std::make_unique<AssignExpression>(std::move(assignable), opType, std::move(postfix), assignableStart,
                                              postfixEnd);
}

Parser::Precedence Parser::operatorToPrecedence(Token::Type type) {
    switch (type) {
        case Token::Type::CAST:
            return Precedence::CASTING;
        case Token::Type::INCREMENT:
        case Token::Type::DECREMENT:
            return Precedence::UNARY;
        case Token::Type::BITWISE_AND:
        case Token::Type::BITWISE_OR:
        case Token::Type::BITWISE_XOR:
            return Precedence::BITWISE;
        case Token::Type::PLUS:
            return Precedence::ADD;
        case Token::Type::MINUS:
            return Precedence::SUBTRACT;
        case Token::Type::TIMES:
            return Precedence::MULTIPLY;
        case Token::Type::DIVIDE:
            return Precedence::DIVIDE;
        case Token::Type::MODULO:
            return Precedence::MODULO;
        case Token::Type::POWER:
            return Precedence::POWER;
        case Token::Type::EQUAL_TO:
        case Token::Type::NOT_EQUAL_TO:
        case Token::Type::LT:
        case Token::Type::GT:
        case Token::Type::LTE:
        case Token::Type::GTE:
            return Precedence::EQUALITY;
        case Token::Type::LOGICAL_AND:
        case Token::Type::LOGICAL_OR:
            return Precedence::LOGICAL;
        case Token::Type::ASSIGN:
        case Token::Type::PLUS_ASSIGN:
        case Token::Type::MINUS_ASSIGN:
        case Token::Type::TIMES_ASSIGN:
        case Token::Type::DIVIDE_ASSIGN:
        case Token::Type::MODULO_ASSIGN:
        case Token::Type::POWER_ASSIGN:
            return Precedence::ASSIGNMENT;
        case Token::Type::END_OF_LINE:
        case Token::Type::END_OF_FILE:
        default:
            return Precedence::ALL;
    }
}

void Parser::pushAssignable(std::vector<std::unique_ptr<MaximAst::AssignableExpression>> &target,
                            std::unique_ptr<MaximAst::Expression> source) {
    auto tupleSource = AxiomUtil::dynamic_unique_cast<TupleExpression>(source);
    if (tupleSource) {
        for (auto &item : tupleSource->expressions) {
            auto assignableItem = AxiomUtil::dynamic_unique_cast<AssignableExpression>(item);
            if (assignableItem) target.push_back(std::move(assignableItem));
            else throw castFail(item.get());
        }
    } else {
        auto assignableItem = AxiomUtil::dynamic_unique_cast<AssignableExpression>(source);
        if (assignableItem) target.push_back(std::move(assignableItem));
        else throw castFail(source.get());
    }
}

void Parser::expect(const Token &token, Token::Type expectedType) {
    if (token.type != expectedType) {
        throw MaximCommon::CompileError(
            "Dude, why is there a " + Token::typeString(token.type) + "? I expected a " +
            Token::typeString(expectedType) + " (or something else) here.",
            token.startPos, token.endPos
        );
    }
}

MaximCommon::CompileError Parser::fail(const Token &token) {
    return MaximCommon::CompileError(
        "Hey man, not cool. I didn't expect this " + Token::typeString(token.type) + "!",
        token.startPos, token.endPos
    );
}

MaximCommon::CompileError Parser::castFail(MaximAst::Expression *expr) {
    return MaximCommon::CompileError(
        "Hey! I need something I can assign to here, not this silly fudge you're giving me.",
        expr->startPos, expr->endPos
    );
}
