// THIS IS THE PROVIDED CODE FOR DSA, PROGRAM #3 (originally Fall 2026)

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cstring>
#include <cctype>
#include <cstdlib>

using namespace std;

// A simple class; each object holds two public fields
class Data {
public:
  unsigned int u;
  string s;
};

// Load the data from a specified input file
void loadDataList(list<Data *> &l, const string &filename) {

  ifstream input(filename);
  if (!input) {
    cerr << "Error: could not open " << filename << "\n";
    exit(1);
  }

  // The first line indicates the size
  string line;
  getline(input, line);
  stringstream ss(line);
  int size;
  ss >> size;

  // Load the data
  for (int i = 0; i < size; i++) {
    getline(input, line);
    stringstream ss2(line);
    Data *pData = new Data();
    ss2 >> pData->u >> pData->s;
    l.push_back(pData);
  }

  input.close();
}

// Output the data to a specified output file
void writeDataList(const list<Data *> &l, const string &filename) {

  ofstream output(filename);
  if (!output) {
    cerr << "Error: could not open " << filename << "\n";
    exit(1);
  }

  // Write the size first
  int size = l.size();
  output << size << "\n";

  // Write the data
  for (auto pData:l) {
    output << pData->u << " " 
	   << pData->s << "\n";
  }

  output.close();
}

// Sort the data according to a specified field
// (The implementation of this function will be later in this file)
void sortDataList(list<Data *> &, int);

// The main function calls routines to load the data, sort the data,
// and output the data. The sort is timed according to CPU time.
int main() {

  // Get the input filename from the user and load the data
  string filename;
  cout << "Enter name of input file: ";
  cin >> filename;
  list<Data *> theList;
  loadDataList(theList, filename);
  cout << "Data loaded.\n";

  // Ask about the criteria
  cout << "Which criterion is being used for the sort (1-4)? ";
    int criterion = 0;
  cin >> criterion;
  if (criterion < 1 || criterion > 4) {
    cerr << "Invalid criterion\n";
    exit(1);
  }
  
  // Sort the data, and output the CPU time used by the sort
  cout << "Executing sort...\n";
  clock_t t1 = clock();
  sortDataList(theList, criterion);
  clock_t t2 = clock();
  double timeDiff = ((double) (t2 - t1)) / CLOCKS_PER_SEC;
  cout << "Sort finished. CPU time was " << timeDiff << " seconds.\n";

  // Get the output filename from the user and write the sorted data
  cout << "Enter name of output file: ";
  cin >> filename;
  writeDataList(theList, filename);

  return 0;
}

// -------------------------------------------------
// YOU MAY NOT MODIFY OR ADD ANY CODE ABOVE HERE !!!
// -------------------------------------------------

// You may add global variables, functions, and/or class defintions
// here if you wish.
#include <sys/mman.h>

typedef std::__detail::_List_node_base Node;

struct Rec { unsigned long long key; Data *p; };

static unsigned char MAP[256];
static unsigned int countLo[65536], countHi[65536];

static void *bigAlloc(size_t bytes) {
  size_t P = 1 << 21;
  size_t rounded = (bytes + P - 1) / P * P;
  void *ptr = aligned_alloc(P, rounded);
#ifdef MADV_HUGEPAGE
  madvise(ptr, rounded, MADV_HUGEPAGE);
#endif
  return ptr;
}

static unsigned long long stringKey(const char *s) {
  unsigned long long c0 = MAP[(unsigned char)s[0]];
  unsigned long long c1 = MAP[(unsigned char)s[1]];
  unsigned long long c2 = MAP[(unsigned char)s[2]];
  unsigned long long c3 = MAP[(unsigned char)s[3]];
  unsigned long long key = (52 * c0 + c1) << 48 | (52 * c2 + c3) << 36;
  int shift = 30;
  for (int i = 4; i <= 9 && s[i] != '\0'; i++) {
    key |= (unsigned long long)(MAP[(unsigned char)s[i]] + 1) << shift;
    shift -= 6;
  }
  return key;
}

static bool compareString(const Rec &a, const Rec &b) {
  return a.p->s < b.p->s;
}

static bool compareInt(const Rec &a, const Rec &b) {
  if (a.p->s != b.p->s) {
    return a.p->s < b.p->s;
  }
  return a.p->u < b.p->u;
}

static void fixTies(Rec *a, size_t n, int criteria) {
  if (criteria == 1) {
    return;
  }
  size_t i = 0;
  while (i + 1 < n) {
    if (a[i].key != a[i + 1].key) {
      i++;
      continue;
    }
    size_t j = i + 1;
    while (j < n && a[j].key == a[i].key) {
      j++;
    }
    switch (criteria) {
      case 2:
        sort(a + i, a + j, compareString);
        break;
      case 3:
        stable_sort(a + i, a + j, compareString);
        break;
      case 4:
        sort(a + i, a + j, compareInt);
        break;
    }
    i = j;
  }
}

void sortDataList(list<Data *> &l, int criteria) {
  size_t n = l.size();
  size_t i;
  unsigned short shiftLo = criteria <= 2 ? 0 : 36;
  unsigned short shiftHi = criteria <= 2 ? 16 : 48;
  unsigned short mask = criteria <= 2 ? 0xFFFF : 0xFFF;

  for (i = 0; i < 26; i++) {
    MAP['A' + i] = i;
    MAP['a' + i] = 26 + i;
  }

  Rec *a = (Rec *)bigAlloc(n * sizeof(Rec));
  Node **nodes = new Node*[n];
  Node *head = l.end()._M_node;
  i = 0;
  for (Node *node = head->_M_next; node != head; node = node->_M_next) {
    Data *&slot = *(Data **)(node + 1);
    Data *data = slot;
    unsigned long long key;
    a[i].p = data;
    switch (criteria <= 2) {
      case true:
        key = data->u;
        break;
      case false:
        key = stringKey(data->s.c_str());
        break;
    }
    a[i].key = key;
    countLo[key >> shiftLo & mask]++;
    countHi[key >> shiftHi & mask]++;
    nodes[i] = node;
    i++;
  }

  size_t sumLo = 0, sumHi = 0;
  size_t c;
  for (i = 0; i < 65536; i++) {
    c = countLo[i];
    countLo[i] = sumLo;
    sumLo += c;
    c = countHi[i];
    countHi[i] = sumHi;
    sumHi += c;
  }

  Rec *b = (Rec *)bigAlloc(n * sizeof(Rec));
  size_t d;
  for (i = 0; i < n; i++) {
    d = a[i].key >> shiftLo & mask;
    b[countLo[d]] = a[i];
    countLo[d]++;
  }
  for (i = 0; i < n; i++) {
    d = b[i].key >> shiftHi & mask;
    a[countHi[d]] = b[i];
    countHi[d]++;
  }

  if (criteria >= 3) {
    Rec x;
    size_t j;
    for (i = 1; i < n; i++) {
      if (a[i - 1].key <= a[i].key) {
        continue;
      }
      x = a[i];
      j = i;
      while (j > 0 && a[j - 1].key > x.key) {
        a[j] = a[j - 1];
        j--;
      }
      a[j] = x;
    }
  }

  fixTies(a, n, criteria);
  for (i = 0; i < n; i++) {
    *(Data **)(nodes[i] + 1) = a[i].p;
  }
}
