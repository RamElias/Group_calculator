#pragma once

#include <vector>
#include <memory>
#include <string>
#include <iosfwd>
#include <optional>
#include <sstream>
#include <fstream>
#include <istream>
#include <ostream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "Union.h"
#include "Intersection.h"
#include "Difference.h"
#include "Product.h"
#include "Comp.h"
#include "Identity.h"
#include "Exceptions.h"



class Operation;

class SetCalculator
{
public:
    SetCalculator(std::istream& istr, std::ostream& ostr);
    void run();

private:
    void resize();
    void read();
    void eval();
    void del();
    void help();
    void exit();

    template <typename FuncType>
    void binaryFunc()
    {
        if (auto f0 = readOperationIndex(), f1 = readOperationIndex(); f0 && f1)
        {
            m_operations.push_back(std::make_shared<FuncType>(m_operations[*f0], m_operations[*f1]));
        }
    }

    void printOperations() const;

    enum class Action
    {
        Invalid,
        Eval,
        Union,
        Intersection,
        Difference,
        Product,
        Comp,
        Resize,
        Read,
        Del,
        Help,
        Exit,
    };

    struct ActionDetails
    {
        std::string command;
        std::string description;
        Action action;
    };

    using ActionMap = std::vector<ActionDetails>;
    using OperationList = std::vector<std::shared_ptr<Operation>>;

    const ActionMap m_actions;
    OperationList m_operations;
    bool m_running = true;
    int m_numOfOper = 0;
    std::istream& m_istr;
    std::ostream& m_ostr;

    std::optional<int> readOperationIndex() const;
    Action readAction() const;
    void runAction(Action action);
    int getNumOfOper();
    void checkOperAmount(const int&, int);
    bool checkIfToContinue();

    static ActionMap createActions();
    static OperationList createOperations();
};
