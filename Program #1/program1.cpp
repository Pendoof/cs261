#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

/* 
This program reads a list of objects from an input file guarenteed to be formatted with foobar type and name, creates a vector of Foobar objects (or its derived classes Foo and Bar), sets their positions, and writes their name and power to an output file.

Coded by: Derek Chen CS160 Spring 2026
*/

class Foobar {
    public:
        Foobar(const std::string& name, size_t position) : name(name), position(position) {}

        // virtual destructor to ensure proper cleanup of derived classes
        virtual ~Foobar() = default;

        std::string getName() const {
            return name;
        }

        // normal foobar returns its position as its power
        // type is size_t because position should always be a non-negative integer
        virtual size_t getPower() const {
            return position;
        }
        
        // Overload the << operator to write Foobar objects to an output file
        friend std::ofstream& operator<<(std::ofstream& ofs, const Foobar& foobar) {
            ofs << foobar.getName() << " " << foobar.getPower() << "\n";
            return ofs;
        }

        const Foobar& setPosition(size_t position) {
            this->position = position;
            return *this;
        }
    
    protected:
        size_t getPosition() const {
            return position;
        }
        
    // Accessible to derived classes
    private:
        std::string name;
        size_t position;
};

class Foo: public Foobar {
    public:
        Foo(const std::string& name, size_t position) : Foobar(name, position) {}

        // foo returns position * 3 as its power
        size_t getPower() const override {
            return this->getPosition()  * 3;
        }
};

class Bar: public Foobar {
    public:
        Bar(const std::string& name, size_t position) : Foobar(name, position) {}

        // bar returns position + 15 as its power
        size_t getPower() const override {
            return this->getPosition() + 15;
        }
};

// variables passed by reference to store the input and output file names
void getFileNames (std::string& inputFileName, std::string& outputFileName) {
    std::cout << "Enter the name of the input file: ";
    std::cin >> inputFileName;
    std::cout << "Enter the name of the output file: ";
    std::cin >> outputFileName;
}

bool createVectorFromFile(std::vector<std::unique_ptr<Foobar>>& foobarVector, const std::string& inputFileName) {
    std::ifstream inputFile(inputFileName);

    // check if the input file was opened successfully
    if (!inputFile) {
        std::cerr << "Error opening input file: " << inputFileName << "\n";
        return false;
    }

    std::string line;
    std::istringstream ss;
    std::string type;
    std::string name;

    // guarenteed line format: <type> <name>
    while (getline(inputFile, line)) {
        ss = std::istringstream(line);
        ss >> type >> name;

        // smart pointers used to delete objects automatically when vector is destroyed
        if (type == "foo") {
            foobarVector.push_back(std::make_unique<Foo>(name, 0));
        } else if (type == "bar") {
            foobarVector.push_back(std::make_unique<Bar>(name, 0));
        } else {
            foobarVector.push_back(std::make_unique<Foobar>(name, 0));
        }
    }

    inputFile.close();
    return true;
}

void setPositions(std::vector<std::unique_ptr<Foobar>>& foobarVector) {
    // traverse vector in reverse order to set positions
    for (size_t i = foobarVector.size(), pos = 1; i-- > 0; pos++) {
        foobarVector[i]->setPosition(pos);
    }
}

bool writeVectorToFile(const std::vector<std::unique_ptr<Foobar>>& foobarVector, const std::string& outputFileName) {
    std::ofstream outputFile(outputFileName);
    // check if the output file was opened successfully
    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFileName << "\n";
        return false;
    }

    for (const std::unique_ptr<Foobar>& foobar : foobarVector) {
        // dereference the unique_ptr to access the Foobar object and write it to the output file
        outputFile << *foobar;
    }

    outputFile.close();
    return true;
}

int main() {
    std::string inputFileName;
    std::string outputFileName;

    getFileNames(inputFileName, outputFileName);

    std::vector<std::unique_ptr<Foobar>> foobarVector;

    if (!createVectorFromFile(foobarVector, inputFileName)) {
        return 1;
    }
    setPositions(foobarVector);
    if (!writeVectorToFile(foobarVector, outputFileName)) {
        return 1;
    }
    return 0;
}
