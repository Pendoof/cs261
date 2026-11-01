#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

/* 
This program reads a list of commands from an input file guaranteed to be formatted with command, name and parameters. Creates, pushes, and pops values onto the appropriate lists. Writes the results to an output file.

Coded by: Derek Chen CS261 Spring 2026
*/


template <typename T>
class SimpleList {
    public:
        SimpleList(const std::string& name) : head(nullptr), tail(nullptr), name(name) {}

        // virtual destructor to ensure proper cleanup of derived classes
        virtual ~SimpleList() {
            while (head) {
                // Dropped head node will be automatically deleted because of unique_ptr to avoid stack overflow from recursive destructor calls
                head = std::move(head->next);
            }
        }

        const std::string& getName() const { return name; }
        // Check if the list is empty
        bool isEmpty() const { return head == nullptr; }

        virtual void push(const T& value) = 0;
        virtual T pop() = 0;

    // Protected methods for derived classes to insert and remove nodes
    protected:
        void insertFront(const T& value) {
            std::unique_ptr<Node> node = std::make_unique<Node>(value);
            if (!head) {
                // If the list is empty, set tail to the new node
                tail = node.get();
            } else {
                node->next = std::move(head);
            }
            head = std::move(node);
        }

        void insertBack(const T& value) {
            std::unique_ptr<Node> node = std::make_unique<Node>(value);
            Node* newTail = node.get();
            if (!tail) {
                // If the list is empty, set head to the new node
                head = std::move(node);
            } else {
                tail->next = std::move(node);
            }
            tail = newTail;
        }
        
        // Does not check if list is empty
        T removeFront() {
            std::unique_ptr<Node> oldHead = std::move(head);
            head = std::move(oldHead->next);
            if (!head) {
                // If the list is now empty, set tail to nullptr
                tail = nullptr;
            }
            return oldHead->value;
        }

    private:
        struct Node {
            T value;
            // Unique_ptr used to automatically manage memory of the next node
            std::unique_ptr<Node> next;
            Node(const T& value) : value(value), next(nullptr) {}
        };
        std::unique_ptr<Node> head;
        // Non-owning raw pointer used for tail to avoid circular reference with unique_ptr
        Node* tail;
        std::string name;
};

template <typename T>
class Stack : public SimpleList<T> {
    public:
        Stack(const std::string& name) : SimpleList<T>(name) {}
        // Stack uses LIFO, so push and pop operate on the front of the list
        void push(const T& value) override {
            this->insertFront(value);
        }
        T pop() override {
            return this->removeFront();
        }
};

template <typename T>
class Queue : public SimpleList<T> {
    public:
        Queue(const std::string& name) : SimpleList<T>(name) {}
        // Queue uses FIFO, so push operates on the back and pop operates on the front of the list
        void push(const T& value) override {
            this->insertBack(value);
        }
        T pop() override {
            return this->removeFront();
        }
};

bool openFiles(std::ifstream& inputFile, std::ofstream& outputFile) {
    std::string inputFileName, outputFileName;

    std::cout << "Enter the name of the input file: ";
    std::cin >> inputFileName;
    inputFile.open(inputFileName);
    if (!inputFile) {
        std::cerr << "Error opening input file: " << inputFileName << "\n";
        return false;
    }

    std::cout << "Enter the name of the output file: ";
    std::cin >> outputFileName;
    outputFile.open(outputFileName);
    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFileName << "\n";
        return false;
    }

    return true;
}

// Function templates to create lists, push values, and pop values
template <typename T>
void createList(
                std::unordered_map<std::string, std::unique_ptr<SimpleList<T>>>& lists,
                const std::string& name,
                const std::string& kind,
                std::ofstream& outputFile
              ) {
    if (lists.find(name) != lists.end()) {
        outputFile << "ERROR: This name already exists!\n";
        return;
    }
    if (kind == "stack") {
        lists[name] = std::make_unique<Stack<T>>(name);
    } else {
        lists[name] = std::make_unique<Queue<T>>(name);
    }
}

template <typename T>
void pushValue(
               std::unordered_map<std::string, std::unique_ptr<SimpleList<T>>>& lists,
               const std::string& name,
               const T& value,
               std::ofstream& outputFile
              ) {
    auto it = lists.find(name);
    if (it == lists.end()) {
        outputFile << "ERROR: This name does not exist!\n";
        return;
    }
    it->second->push(value);
}

template <typename T>
void popValue(
              std::unordered_map<std::string, std::unique_ptr<SimpleList<T>>>& lists,
              const std::string& name,
              std::ofstream& outputFile
             ) {
    auto it = lists.find(name);
    if (it == lists.end()) {
        outputFile << "ERROR: This name does not exist!\n";
        return;
    }
    if (it->second->isEmpty()) {
        outputFile << "ERROR: This list is empty!\n";
        return;
    }
    outputFile << "Value popped: " << it->second->pop() << "\n";
}

void processLine(const std::string& line,
                 std::ofstream& outputFile,
                 std::unordered_map<std::string, std::unique_ptr<SimpleList<int>>>& intMap,
                 std::unordered_map<std::string, std::unique_ptr<SimpleList<double>>>& doubleMap,
                 std::unordered_map<std::string, std::unique_ptr<SimpleList<std::string>>>& stringMap) {
    outputFile << "PROCESSING COMMAND: " << line << "\n";

    std::istringstream ss(line);
    std::string command, listName, param;
    // Pop has no paramter, so param stays empty
    ss >> command >> listName >> param;
    
    // Determine the type of list based on the first character of the list name and run command
    if (command == "create") {
        switch (listName[0]) {
            case 'i':
                createList(intMap, listName, param, outputFile);
                break;
            case 'd':
                createList(doubleMap, listName, param, outputFile);
                break;
            case 's':
                createList(stringMap, listName, param, outputFile);
                break;
        }
    } else if (command == "push") {
        switch (listName[0]) {
            case 'i':
                pushValue(intMap, listName, std::stoi(param), outputFile);
                break;
            case 'd':
                pushValue(doubleMap, listName, std::stod(param), outputFile);
                break;
            case 's':
                pushValue(stringMap, listName, param, outputFile);
                break;
        }
    } else if (command == "pop") {
        switch (listName[0]) {
            case 'i':
                popValue(intMap, listName, outputFile);
                break;
            case 'd':
                popValue(doubleMap, listName, outputFile);
                break;
            case 's':
                popValue(stringMap, listName, outputFile);
                break;
        }
    }


}

int main() {
    std::ifstream inputFile;
    std::ofstream outputFile;
    if (!openFiles(inputFile, outputFile)) {
        return 1;
    }
    
    {
        std::unordered_map<std::string, std::unique_ptr<SimpleList<int>>> intMap;
        std::unordered_map<std::string, std::unique_ptr<SimpleList<double>>> doubleMap;
        std::unordered_map<std::string, std::unique_ptr<SimpleList<std::string>>> stringMap;
        
        std::string line;
        while (std::getline(inputFile, line)) {
            processLine(line, outputFile, intMap, doubleMap, stringMap);
        }
    }

    inputFile.close();
    outputFile.close();
    return 0;
}