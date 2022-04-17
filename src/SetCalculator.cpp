#include "SetCalculator.h"

namespace rng = std::ranges;
 //-------CTOR----------------------------------------------------------
SetCalculator::SetCalculator(std::istream& istr, std::ostream& ostr)
    : m_actions(createActions()), m_operations(createOperations()), m_istr(istr), m_ostr(ostr)
{
    m_numOfOper = getNumOfOper();
}

//the main function that runs the calculator
void SetCalculator::run()
{
    do
    {
        try
        {
            m_ostr << "-------------------------------------------------\n";
            m_ostr << "num of available operations: " << m_numOfOper << '\n';
            printOperations();
            m_ostr << "Enter command ('help' for the list of available commands): ";
            const auto action = readAction();
            runAction(action);
        }
        catch (std::istream::failure e) //for fails in read
        {
            m_ostr << "Bad input\n";
        }
        catch (std::out_of_range e) //for fails due to lack of space in calculator
        {
            m_ostr << "Not enough space for new operation\n";
            m_istr.clear();
            m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        catch (InvalidPath e)
        {
            m_ostr << e.what();
        }
    } while (m_running);
}

//A function that receives from the user the number of operations that will be in the calculator 
int SetCalculator::getNumOfOper()
{
    auto num = 0;
    
    while (true)
    {
        try
        {
            m_ostr << "please enter the number of operation (3-100):\n";
            m_istr >> num; 
            if (m_istr.fail() || num < 3 || num > 100)
            {
                m_istr.clear();
                m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::invalid_argument("");
            }
            break;
        }
        catch (std::invalid_argument)
        {
            m_ostr << "must be number between 3-100\n";
            if (m_numOfOper != 0)
                return m_numOfOper;
        }
    }
    return num;
}

// A function that checks if an operation can be added to the
// calculator and throws an error if not
void SetCalculator::checkOperAmount(const int& numOfOper, int action)
{
    if (m_operations.size() == numOfOper)
        if (action > 1 && action < 7)
            throw std::out_of_range("");
}

// A function that compute the result of operation in the caclulator
void SetCalculator::eval()
{
    if (auto index = readOperationIndex(); index)
    {
        const auto& operation = m_operations[*index];
        auto inputs = std::vector<Set>();
        
        for (auto i = 0; i < operation->inputCount(); ++i)
        {
            inputs.push_back(Set(m_istr));
        }
            
        operation->print(m_ostr, inputs);
        m_ostr << " = " << operation->compute(inputs) << '\n';
    }
}

// A function that resize the number of operation that can be added to the calculator
// if the number is small than the exiest amount of operation the function ask
// the user if he wants to cancel or delete the spair functions
void SetCalculator::resize()
{
    int newSize = getNumOfOper();
   
    if (newSize < m_operations.size()) //if the number is small than the current amount
    {
        m_ostr << "Some operations will need to be deleted\n"
                  "Press Y if you want to continue the process"
                  " or N if you want to cancel it\n";
        m_istr.get();
        while (1)
        {
            try
            {
                std::string answer;
                std::getline(m_istr, answer);
                if (answer == "Y" || answer == "y")
                {
                    m_operations.erase(m_operations.begin() + newSize, m_operations.end());
                    m_numOfOper = newSize;
                    return;
                }
                else if (answer == "N" || answer == "n")
                    return;
                else
                    throw std::istream::failure("");
            }
            catch (std::istream::failure e)
            {
                m_ostr << "Bad input\n"
                          "Some operations will need to be deleted\n"
                          "Press Y if you want to continue the process"
                          " or N if you want to cancel it\n";
            }
        }
    }
    else //if the number is bigger that the cuurent amount
        m_numOfOper = newSize;
}

//A function that read operations from file
void SetCalculator::read()
{
    auto path = std::string();
    m_ostr << "please enter the path to the file:\n";
    std::getline(m_istr >> std::ws, path); //read the file path from user
    auto file = std::ifstream(path);
    auto line = std::string();
    auto readAnything = false, contRead = true;
    std::streambuf* cinbuf = m_istr.rdbuf(); //save the old buffer
    while (std::getline(file, line) && contRead)
    {
        try
        {
            readAnything = true;
            auto iss = std::istringstream(line);
            iss.exceptions(std::ios::failbit | std::ios::badbit);
            m_istr.rdbuf(iss.rdbuf()); //redirect std::cin to file!
            m_ostr << line << std::endl; //print the operation read
            const auto action = readAction();
            runAction(action);
            if (action == Action::Invalid) //if the funtion doesnt exiest
                throw FileError();
        }        
        catch (std::out_of_range e) //for error due to lack of space in calculator 
        {
            m_ostr << "Not enough space for new operation\n";
            m_istr.clear();
            m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        catch (FileError e) //for errors in the file
        {
            m_ostr << e.what() << line << std::endl;
            m_istr.rdbuf(cinbuf);
            contRead = checkIfToContinue();
        }
        catch (std::exception& ex) //for errors in the arguments operation 
        {
            m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            m_ostr << "Exception thrown-in read func: " << ex.what() << std::endl;
            m_ostr << "problem in line : " << line << std::endl;
            m_istr.rdbuf(cinbuf);
            contRead = checkIfToContinue();
        }

        if (!file.eof() && contRead) //print the operation
        {
            m_ostr << "-------------------------------------------------\n";
            m_ostr << "num of available operations: " << m_numOfOper << '\n';
            printOperations();
            m_ostr << "Enter command ('help' for the list of available commands): ";
        }
    }
    if (!readAnything) //if the file didnt opened
        throw InvalidPath();
    
    m_istr.rdbuf(cinbuf); //redirect back to std::cin
}

// A function the checks if user wants to continue reading from the file
bool SetCalculator::checkIfToContinue()
{
    while (1)
    {
        try {
            m_ostr <<
                "\nwould you like to continue reading the file?\n"
                "press Y/N \n";
            auto answer = std::string();
            m_istr >> answer;
            if (answer == "Y" || answer == "y")
                return true;
            else if (answer == "N" || answer == "n")
                return false;

            throw std::invalid_argument(answer + " not valid option");
        }
        catch (std::invalid_argument& e) //For an answer that does not fit the options
        {
            m_istr.clear();
            m_ostr << "Bad Input: " << e.what() << std::endl;
        }
    }
}

void SetCalculator::del()
{
    if (auto i = readOperationIndex(); i)
    {
        m_operations.erase(m_operations.begin() + *i);
    }
}

void SetCalculator::help()
{
    m_ostr << "The available commands are:\n";
    for (const auto& action : m_actions)
    {
        m_ostr << "* " << action.command << action.description << '\n';
    }
    m_ostr << '\n';
}

void SetCalculator::exit()
{
    m_ostr << "Goodbye!\n";
    m_running = false;
}

void SetCalculator::printOperations() const
{
    m_ostr << "List of available set operations:\n";
    for (decltype(m_operations.size()) i = 0; i < m_operations.size(); ++i)
    {
        m_ostr << i << ".\t";
        auto gen = NameGenerator();
        m_operations[i]->print(m_ostr, gen);
        m_ostr << '\n';
    }
    m_ostr << '\n';
}

std::optional<int> SetCalculator::readOperationIndex() const
{
    auto i = 0;
    try
    {
        m_istr >> i;
        if (m_istr.fail())
        {
            m_istr.clear();
            m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            throw std::istream::failure("");
        }

        if (i >= m_operations.size() || i < 0 )
            throw std::out_of_range("");
        return i;
    }
    
    catch(std::out_of_range)
    {
        m_ostr << "Operation #" << i << " doesn't exist\n";
        return {};
    }

}

SetCalculator::Action SetCalculator::readAction() const
{
    auto action = std::string();
    m_istr >> action;

    try
    {
        const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
        if (i == m_actions.end())
        {
            throw std::istream::failure("");
        }
        return i->action;
    }
    catch(std::istream::failure)
    {
        std::getline(m_istr,action);
        return Action::Invalid;
    }
}

void SetCalculator::runAction(Action action)
{
    checkOperAmount(m_numOfOper, int(action));

    switch (action)
    {
        default:
            m_ostr << "Unknown enum entry used!\n";
            break;

        case Action::Invalid:
            m_ostr << "Command not found\n";
            break;

        case Action::Eval:         eval();                     break;
        case Action::Union:        binaryFunc<Union>();        break;
        case Action::Intersection: binaryFunc<Intersection>(); break;
        case Action::Difference:   binaryFunc<Difference>();   break;
        case Action::Product:      binaryFunc<Product>();      break;
        case Action::Comp:         binaryFunc<Comp>();         break;
        case Action::Resize:       resize();                   break;
        case Action::Read:         read();                     break;
        case Action::Del:          del();                      break;
        case Action::Help:         help();                     break;
        case Action::Exit:         exit();                     break;
    }
}

SetCalculator::ActionMap SetCalculator::createActions()
{
    return ActionMap
    {
        {
            "eval",
            "(uate) num ... - compute the result of function #num on the "
            "following set(s); each set is prefixed with the count of numbers to"
            " read",
            Action::Eval
        },
        {
            "uni",
            "(on) num1 num2 - Creates an operation that is the union of "
            "operation #num1 and operation #num2",
            Action::Union
        },
        {
            "inter",
            "(section) num1 num2 - Creates an operation that is the "
            "intersection of operation #num1 and operation #num2",
            Action::Intersection
        },
        {
            "diff",
            "(erence) num1 num2 - Creates an operation that is the "
            "difference of operation #num1 and operation #num2",
            Action::Difference
        },
        {
            "prod",
            "(uct) num1 num2 - Creates an operation that returns the product of"
            " the items from the results of operation #num1 and operation #num2",
            Action::Product
        },
        {
            "comp",
            "(osite) num1 num2 - creates an operation that is the composition "
            "of operation #num1 and operation #num2",
            Action::Comp
        },
        {
            "resize",
            "resize num of available operations",
            Action::Resize
        },
        {
            "read",
            "Read operations from file",
            Action::Read
        },
        {
            "del",
            "(ete) num - delete operation #num from the operation list",
            Action::Del
        },
        {
            "help",
            " - print this command list",
            Action::Help
        },
        {
            "exit",
            " - exit the program",
            Action::Exit
        }
    };
}

SetCalculator::OperationList SetCalculator::createOperations()
{
    return OperationList
    {
        std::make_shared<Union>(std::make_shared<Identity>(), std::make_shared<Identity>()),
        std::make_shared<Intersection>(std::make_shared<Identity>(), std::make_shared<Identity>()),
        std::make_shared<Difference>(std::make_shared<Identity>(), std::make_shared<Identity>())
    };
}
