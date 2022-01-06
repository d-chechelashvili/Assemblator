#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string.h>
#include <time.h>

using namespace std; 

enum OperationType { ALU, LOAD, STORE, FUNCTION, NOTFOUND };

void introduce();
void reset();
void printManual();
void emulate(char VirtualStack[]);
void executeLine(char VirtualStack[], bool isLive);
OperationType getType(string line);
bool isALUtype(string command, string left);
bool isLOADtype(string command);
bool isSTOREtype(string command);
void getAngry();
void executeAsALU();
void executeAsLOAD(char VirtualStack[]);
void executeAsSTORE(char VirtualStack[]);
void executeAsFUNCTION(char VirtualStack[], bool isLive);
void executeBRANCH(bool isLive, char VirtualStack[]);
void executeJUMP(string num, bool isLive, char VirtualStack[]);
void executeRET(char VirtualStack[], bool isLive);
void executeDEFINE(bool isLive);
void executeCALL(char VirtualStack[], bool isLive);
void executePRINT(char VirtualStack[] );
void live(char VirtualStack[]);


const int STACK_SIZE = 4000000;

vector <string> Instructions; // when the file is parsed it's stored in here line by line. In file every line represents one instruction
unordered_map <string, int> Registers; //each element represents register - value 
unordered_map <string, int> Functions; // each element represents function name - definition start index 


int main(){
  char VirtualStack[STACK_SIZE]; //artificially created virtual memory
  srand(time(NULL)); // so that pseudo random generation feels less pseudolike
  introduce();
  string command;
  reset();
  while(true){
    cout << "#|Assemblator|#: "; 
    getline(cin, command);
    this_thread::sleep_for(chrono::milliseconds(350));
    if(command == "man") printManual();
    else if(command == "emulate -f") {
      reset();
      emulate(VirtualStack);
    }else if (command  == "emulate -l") live(VirtualStack);
    else if(command == "exit") break;
    else cout << "Oops, it appears you have entered wrong command, please try again" << endl;
  }
  cout << "\n                                    Farewell!\n" << endl;
  this_thread::sleep_for(chrono::milliseconds(500));
  return 0;
}

//just basic information and introduction
void introduce(){
  cout << "\n                                     Welcome!\n" << endl;
  this_thread::sleep_for(chrono::milliseconds(1200));
  cout<< "               I am an assembly emulator and my name is Assemblator\n" << endl;
  this_thread::sleep_for(chrono::milliseconds(2400));
  cout << "  for instructions type man" << endl;
  this_thread::sleep_for(chrono::milliseconds(1150));
  cout << "  to emulate assembly code from a file type emulate -f" << endl;
  this_thread::sleep_for(chrono::milliseconds(1650));
  cout << "  to emulate assembly code live, line by line, type emulate -l" << endl;
  this_thread::sleep_for(chrono::milliseconds(1400));
  cout << "  to exit the program type exit" << endl;
  this_thread::sleep_for(chrono::milliseconds(1350));
  cout << "\n                                    Have fun!\n" << endl;
  this_thread::sleep_for(chrono::milliseconds(1250));
}

//clears stored instructions and special purpose registers. other data structures don't need clearing just as memory
void reset(){
  Instructions.clear();
  Registers["PC"] = 0;
  Registers["SP"] = STACK_SIZE;
}

// prints the manual from a manual file into console
void printManual(){
  cout << "\n---------------------------------------------------------\n" << endl;
  ifstream file;
  file.open("manual");
  if (file.is_open())
      cout << file.rdbuf();
  file.close();
  cout << "\n\n---------------------------------------------------------\n" << endl;
}

//prompts the user to enter location/address of file
string getLoc(){
  string location;
  cout << "Please enter a file address: ";
  cin >> location;
  return location;
}

//makes sure that Assemblator has a valid file to work with
void getFile(ifstream &file){
  while(true){
    string location = getLoc();
    file.open(location.c_str());
    if(file) break;
    cerr << "Unable to open file " << location << " please try again" << endl;
  }
}

void parseLine(string &line){
  line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end()); //gets rid of whitespaces
  for(int i = 0; i <  line.size(); i++) line[i] = toupper(line[i]);  //converts string to uppercase
}

//parses the file. in this case, which means reading, modifying it line by line and storing each line in Instructions vector
void parseFile(ifstream &file){
  for(string line; getline(file, line);){
    parseLine(line);
    Instructions.push_back(line);
  }
  file.close();
}

//iterates over instructions and executes each one
void emulate(char VirtualStack[]){
  ifstream file;
  getFile(file);
  parseFile(file);
  this_thread::sleep_for(chrono::milliseconds(200));
  cout << "File parsed successfully" << endl;
  for(; Registers["PC"] / 4 < Instructions.size(); Registers["PC"] += 4)
    executeLine(VirtualStack, false);
  
  cout << "\n                       ---Emulation done successfuly\n" << endl;
}

//executes single instruction/line
void executeLine(char VirtualStack[], bool isLive){
  this_thread::sleep_for(chrono::milliseconds(150));
  cout << "\n  Executing Line " << Registers["PC"] / 4 + 1 << ": " << Instructions[Registers["PC"] / 4] << endl;
  this_thread::sleep_for(chrono::milliseconds(150));
  cout << "Detecting type..." << endl;
  OperationType type = getType(Instructions[Registers["PC"] / 4]);
  this_thread::sleep_for(chrono::milliseconds(150));
  switch (type){
  case ALU:
    cout << "Type detected : ALU" << endl;
    this_thread::sleep_for(chrono::milliseconds(150));
    executeAsALU();
    break;
  case LOAD:
    cout << "Type detected : LOAD" << endl;
    this_thread::sleep_for(chrono::milliseconds(150));
    executeAsLOAD(VirtualStack);
    break;
  case STORE:
    cout << "Type detected : STORE" << endl;
    this_thread::sleep_for(chrono::milliseconds(150));
    executeAsSTORE(VirtualStack);
    break;
  case FUNCTION:
    cout << "Type detected : FUNCTION" << endl;
    this_thread::sleep_for(chrono::milliseconds(150));
    executeAsFUNCTION(VirtualStack, isLive);
    break;
  default:
    cout << "Couldn't detect what type of operation you intended to do. You're probably doing something wrong" << endl;
    break;
  }
}

//detects the type of operation 
OperationType getType(string line){
  OperationType type = NOTFOUND;
  size_t eqPos = line.find('=');
  if(eqPos == string::npos) type = FUNCTION;
  else{
    bool typeFound = false;
    string right = line.substr(eqPos + 1);
    string left = line.substr(0, eqPos);
    if(isALUtype(right, left)) {
      typeFound = true;
      type = ALU;
    }
    if(isLOADtype(right)){
      if(typeFound) getAngry();
      typeFound = true;
      type = LOAD;
    }  
    if(isSTOREtype(left)){
      if(line.find('+') != string::npos) typeFound = true;
      if(typeFound) getAngry();
      type = STORE;
    }
  }
  return type;
}

//checks if instruction is ALU type
bool isALUtype(string command, string left){
  if(command[0] == '.'){
    cout << "Ilogical instruction" << endl;
    cout << "Exiting the program..." << endl;
    exit(1);
  }
  if(command[0] == 'R' || command[0] == 'S' || command[0] == 'P'){
    if(left[0] == 'R' || left[0] == 'S' || left[0] == 'P') return true;
  }
  for(int i = 0 ; i < command.length(); i++){
    if(command[i] == '+' || command[i] == '-' || command[i] == '*' || command[i] == '/') return true;
    if(isdigit(command[i])){
      if(left[0] == 'M') return false;
      if(i == 0) return true;
      if(command[i-1] == '.') continue;
      if(command[i-1] != 'R' && !(command[i-1] == '[' && command[i-2] == 'M')) return true;
      i++;
      while(isdigit(command[i])) i++;
      i--;
    }
  }
  return false;
}

bool isLOADtype(string command){
  if(command[0] == 'M') return true;
  if(command[0] == '.' && command[2] == 'M') return true;
  return false;
}

//ik same code, but different name. For clarity purposes 
bool isSTOREtype(string command){
   if(command[0] == 'M') return true;
   if(command[0] == '.' && command[2] == 'M') return true;
   return false;
}

//Day 3... I think my program developed emotions of its own...
void getAngry(){
  cout << "\n\nLINE " << Registers["PC"] / 4 + 1 << "-- YOU HAVE JUST BROKEN THE MOST SACRED LAW OF ASSEMBLY LANGUAGE - RULE N1" <<endl;
  this_thread::sleep_for(chrono::milliseconds(2000));
  cout << "\n         AND IT APPEARS YOU HAVEN'T READ THE MANUAL TOO. NOW I AM ANGRY." << endl;
  this_thread::sleep_for(chrono::milliseconds(1450));
  cout << "\n        LEAVE NOW AND COME BACK WHEN YOU ARE READY TO TAKE THIS SERIOUSLY" << endl;
  this_thread::sleep_for(chrono::milliseconds(1800));
  cout << "\n                                   DISCONNECTING... \n" << endl;
  this_thread::sleep_for(chrono::milliseconds(1500));
  exit(1);
}

//finds first occurrence of a sign in expression. Returns -1 if not found
int findSignIndex(string expr){
  for (int i = 0; i < expr.size(); i++){
    if(expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') return i;
  }
  return -1;
}

//evaluates the expression. given expression is a number or a register name
int getValue(string expr){
  if(!expr.length()) return 0;

  if(isdigit(expr[0])) return stoi(expr);

  if(expr == "SP" || expr == "RV" || expr == "PC" ||(expr[0] == 'R' && isdigit(expr[1]))) {
    if(Registers.find(expr) == Registers.end()){
      Registers[expr] = rand() - rand(); //generate virtual random number that should be located in unitialized memory
    }
    return Registers[expr];
  }

  cout << "Couldn't get value out of " << expr << endl;
  cout << "Exiting the program..." << endl;
  exit(1);
  return -1;
}

bool ALUchecker(string command){
  if(command  == "RV") return false;
  if(command[0] == 'R' || isdigit(command[0])){
    for(int i = 1; i < command.length(); i++){
      if(!isdigit(command[i])) return true;  
    }
  }else if(command != "SP" && command != "PC") return true;
  return false;
}

void checkSyntaxALU(string left, string right, int signPos){
  bool wrongSyntaxLeft = ALUchecker(left);
  if(wrongSyntaxLeft){
    cout << "Wrong Instruction, check the left side of equation" << endl;
    cout << "Exiting the program..." << endl;
    exit(1); 
  }

  bool wrongSyntaxRight = false;
  if(signPos == -1) wrongSyntaxRight = ALUchecker(right);
  else if(findSignIndex(right.substr(signPos+1)) != -1) {
    wrongSyntaxRight = true;
  }

  if(wrongSyntaxRight){
    cout << "Wrong Instruction, check the right side of equation" << endl;
    cout << "Exiting the program..." << endl;
    exit(1); 
  }   
}

//executes ALU operations
void executeAsALU(){
  size_t eqPos = Instructions[Registers["PC"] / 4].find('=');
  string left = Instructions[Registers["PC"] / 4].substr(0, eqPos);
  string right = Instructions[Registers["PC"] / 4].substr(eqPos + 1);
  
  int value;
  int signPos = findSignIndex(right);  
  checkSyntaxALU(left, right, signPos);
  
  if(signPos == -1) { // it's a number then
    value = getValue(right);
    cout << "Writing " << right << " into register " << left << "..." << endl;
  }else{
    cout << "Evaluating " << right << "..." << endl;
    int leftValue = getValue(right.substr(0, signPos));
    int rightValue = getValue(right.substr(signPos+1));
    switch (right[signPos]){
    case '+':
      value = leftValue + rightValue;
      break;
    case '-':
      value = leftValue - rightValue;
      break;
    case '*':
      value = leftValue * rightValue;
      break;
    case '/':
      value = leftValue / rightValue;
      break;
    default:
      cout << "Sorry, something seems wrong. couldn't recognize operand in ALU" << endl;
      cout << "Exiting the program..." << endl;
      exit(1);
      break;
    }
    
    this_thread::sleep_for(chrono::milliseconds(150));
    cout << "Writing answer into register " << left << "..." << endl;
  }

  Registers[left] = value;
}


//executes LOAD operations
void executeAsLOAD(char VirtualStack[]){
  size_t eqPos = Instructions[Registers["PC"] / 4].find('=');
  string left = Instructions[Registers["PC"] / 4].substr(0, eqPos);
  string right = Instructions[Registers["PC"] / 4].substr(eqPos + 1);
  int start = right.find('[') + 1;
  string source = right.substr(start, (right.find(']') - start));

  int bytesToWrite = 4;
  if(right[0] == '.') {
    bytesToWrite = right[1] - '0';
    right = right.substr(2);
  }

  int index = getValue(source);
  cout << "Writing information from location " << source << " into register " << left << "..." << endl;
  
  unsigned int value;
  switch (bytesToWrite){
  case 4:
    value = *((unsigned int*)(&VirtualStack[index]));
    break;
  case 1:
    value = *((unsigned char*)(&VirtualStack[index]));
    cout << "only took 1 byte out of " << right << endl;
    break;
  case 2:
    value = *((unsigned short*)(&VirtualStack[index]));
    cout << "only took 2 bytes out of " << right << endl;
    break;
  default:
    cout << "\nIllegal type of cast" << endl;
    exit(1);
    break;
  }

  Registers[left] = value;
}


//executes STORE operations
void executeAsSTORE(char VirtualStack[]){
  size_t eqPos = Instructions[Registers["PC"] / 4].find('=');
  string left = Instructions[Registers["PC"] / 4].substr(0, eqPos);
  string right = Instructions[Registers["PC"] / 4].substr(eqPos + 1);
  int start = left.find('[') + 1;
  string dest = left.substr(start, (left.find(']') - start));

  int bytesToWrite = 4;
  if(right[0] == '.') {
    bytesToWrite = right[1] - '0';
    right = right.substr(2);
  }

  int index = getValue(dest);

  int value;
  if(isdigit(right[0])) {
    value = stoi(right);
    cout << "Writing " << value << " to location " << dest << "..." << endl;
  }else{
    if(Registers.find(right) == Registers.end()){
      Registers[right] = rand() - rand(); //generate virtual random number that should be located in unitialized memory
    }
    value = Registers[right];
    cout << "Writing information from " << right <<  " to location " << dest << "..." << endl;
  }

  switch (bytesToWrite){
  case 4:
    *((unsigned int*)(&VirtualStack[index])) = value;   
    break;
  case 1:
    *((unsigned char*)(&VirtualStack[index])) = (char)value;
    cout << "only took 1 byte out of " << right << endl;
    break;
  case 2:
    *((unsigned short*)(&VirtualStack[index])) = (short)value;
    cout << "only took 2 bytes out of " << right << endl;
    break;
  default:
    cout << "\nIllegal type of cast" << endl;
    exit(1);
    break;
  }
}

//controls the execution of functions
void executeAsFUNCTION(char VirtualStack[], bool isLive){
  string command = Instructions[Registers["PC"] / 4];
  string firstThree = command.substr(0, 3);

  if(firstThree == "BEQ" || firstThree == "BNE" || firstThree == "BGE" || firstThree == "BGT" || 
                                                            firstThree == "BLE" || firstThree == "BLT"){
    executeBRANCH(isLive, VirtualStack);
    return;
  }
  
  if(command.compare(0, 5, "JMPPC") == 0){
    executeJUMP(command.substr(5), isLive, VirtualStack);
    return;
  }

  if(command == "RET"){
    executeRET(VirtualStack, isLive);
    return;
  }

  if(command.compare(0, 4, "CALL") == 0){
    executeCALL(VirtualStack, isLive);
    return;
  }

  if(command.compare(0, 6, "DEFINE") == 0){
    executeDEFINE(isLive);
    return;
  }

  if(command == "RESET"){
    reset();
    return;
  }

  if(command.compare(0, 5, "PRINT") == 0){
    executePRINT(VirtualStack);
    return;
  }

  cout << "Couldn't find a function with that name" << endl;
  cout << "Exiting the program..." << endl;
  exit(1);
}

//executes branch commands
void executeBRANCH(bool isLive, char VirtualStack[]){
  string command = Instructions[Registers["PC"] / 4];
  string firstThree = command.substr(0, 3);
  int commaPos = command.find(',');
  string left = command.substr(3, commaPos - 3);
  int plusPos = command.find('+');
  string right = command.substr(commaPos + 1, (plusPos - 3) - commaPos);
  string jumpNum = command.substr(plusPos);
  int jumpLines;
  
  if(isLive){
    jumpLines = stoi(jumpNum.substr(1));
    string line;
    for(int i = 0; i < jumpLines / 4; i++){
      getline(cin, line);
      parseLine(line);
      Instructions.push_back(line);
    }
  }

  int leftVal = getValue(left);
  int rightVal = getValue(right);

  if((firstThree == "BEQ" && leftVal == rightVal) || (firstThree == "BNE" && leftVal != rightVal) || 
      (firstThree == "BGE" && leftVal >= rightVal) || (firstThree == "BGT" && leftVal > rightVal) || 
      (firstThree == "BLE" && leftVal <= rightVal) || (firstThree == "BLT" && leftVal < rightVal))
  {
    cout << "Branch instruction passed... ";
    this_thread::sleep_for(chrono::milliseconds(150));
    executeJUMP(jumpNum, false, VirtualStack);
  }else{ 
    cout << "Branch instruction failed..." << endl;
    if(isLive){
      for(; Registers["PC"] / 4 < Instructions.size(); Registers["PC"] += 4){
        executeLine(VirtualStack, false);
      }
    }
  }
}

//jumps by given bytes of instructions in given direction
void executeJUMP(string num, bool isLive, char VirtualStack[]){
  char sign = num[0];
  num = num.substr(1);
  int jumpNum = stoi(num);
  if(sign == '-') jumpNum *= -1;
  else if (sign != '+'){
    cout << "Something went wrong with the jump instruction" << endl;
    cout << "Exiting the program..." << endl;
    exit(1);
  }

  Registers["PC"] += (jumpNum - 4);
  
  cout << "shifting PC forward by " << jumpNum << " bytes" << endl;
  this_thread::sleep_for(chrono::milliseconds(100));
  
  if(isLive){
    for(; Registers["PC"] / 4 < Instructions.size(); Registers["PC"] += 4){
      executeLine(VirtualStack, false);
    }
  }
}

//returns to place indicated in saved PC 
void executeRET(char VirtualStack[], bool isLive){
  if(Registers["PC"] / 4 >= Instructions.size() - 1) {
    cout << "Ending the program..." << endl;
    return;
  }
  unsigned int returnPlace = *((unsigned int*)(&VirtualStack[Registers["SP"]]));
  Registers["PC"] = returnPlace; 
  Registers["SP"] += 4;
  cout << "Returning to line " << (returnPlace / 4) + 1 << "..." << endl;
  
  if(isLive){
    for(; Registers["PC"] / 4 < Instructions.size(); Registers["PC"] += 4){
      executeLine(VirtualStack, false);
    }
  }

}

//allows user to define custom function 
void executeDEFINE(bool isLive){
  int index = Registers["PC"] ;
  string command = Instructions[index  / 4];
  string functionName = command.substr(7, command.length() - 8);
  Functions[functionName] = index;

  if(isLive){
    string line;
    while(line != "END_DEF"){
      getline(cin, line);
      parseLine(line);
      Instructions.push_back(line);
      Registers["PC"] += 4;
    }
  }else{
    while(Instructions[Registers["PC"] / 4] != "END_DEF"){
      Registers["PC"] += 4;
    }
  }
  cout << "Function - " << functionName <<  " has been defined" << endl;
}

//calls the function indicated in <> parenthesis then returns to place where execution should continue
void executeCALL(char VirtualStack[], bool isLive){
  string command = Instructions[Registers["PC"] / 4];
  string functionName = command.substr(5, command.length() - 6);
  if(Functions.find(functionName) == Functions.end()){
    cout << "There is no function with that name defined" << endl;
    cout << "Exiting the program..." << endl;
    exit(1);
  }
  Registers["SP"] -= 4;
  *((unsigned int*)(&VirtualStack[Registers["SP"]])) = Registers["PC"];
  Registers["PC"] = Functions[functionName];
  cout << "Function - " << functionName << " has been called" << endl;

  if(isLive){
    string line;
    while(line != "RET"){
      Registers["PC"] += 4;
      line = Instructions[Registers["PC"] / 4];
      executeLine(VirtualStack, false);
    }
  }

}

//prints the value of given register or memory .12048
void executePRINT(char VirtualStack[]){
  string command = Instructions[Registers["PC"] / 4];
  string toPrint = command.substr(6, command.length() - 7);
  
  int bytesToWrite = 4;
  if(toPrint[0] == '.') {
    bytesToWrite = toPrint[1] - '0';
    toPrint = toPrint.substr(2);
  }

  int value;

  //in case of getting value out of raw memory
  if(toPrint[0] == 'M' && toPrint[1] == '[' && toPrint[toPrint.length() - 1] == ']'){
    toPrint = toPrint.substr(2, toPrint.length() - 3);
    int index = getValue(toPrint);
    switch (bytesToWrite){
    case 4:
      value = *((unsigned int*)(&VirtualStack[index]));
      break;
    case 1:
      value = *((unsigned char*)(&VirtualStack[index]));
      break;
    case 2:
      value = *((unsigned short*)(&VirtualStack[index]));
      break;
    default:
      cout << "\nIllegal type of cast" << endl;
      exit(1);
      break;
    }
  }else{
    //in case of getting infromation from a register
    value = getValue(toPrint);
    switch (bytesToWrite){
    case 4:
      break;
    case 1:
      value = (unsigned char)value;
      break;
    case 2:
      value = (unsigned short)value;
      break;
    default:
      cout << "\nIllegal type of cast" << endl;
      exit(1);
      break;
    }
    value = getValue(toPrint);
  }
  cout << value << endl;
}

//controls live, line by line emulation
void live(char VirtualStack[]){
  string command;
  while(true){
    cout << "\n#|Assemblator -l|#: "; 
    getline(cin, command);
    if(command == "exit") {
      reset();
      break;
    }
    parseLine(command);
    Instructions.push_back(command);
    executeLine(VirtualStack, true);
    Registers["PC"] += 4;
  }
}