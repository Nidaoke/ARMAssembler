/* On my honor, I have neither given nor received unauthorized aid on this assignment. */
#include <iostream>                                                                                         //Includes
#include <fstream> 
#include <string>
#include <unordered_map>
#include <map>
#include <utility> 
using namespace std;

unordered_map<int, int> Registers;                                                                          //Stores the registers and their data
enum Operations{CBZ,CBNZ,ADDI,SUBI,ANDI,ORRI,EORI,ADD,SUB,AND,ORR,EOR,LSR,LSL,LDUR,STUR,DUMMY};             //Operation Types
unordered_map<Operations, string> OpMap = {                                                                 //Links operations with their name
    {CBZ, "CBZ"},{CBNZ, "CBNZ"},
    {ADDI, "ADDI"},{SUBI, "SUBI"},{ANDI, "ANDI"},{ORRI, "ORRI"},{EORI, "EORI"},
    {ADD, "ADD"},{SUB, "SUB"},{AND, "AND"},{ORR, "ORR"},{EOR, "EOR"},{LSR, "LSR"},{LSL, "LSL"},
    {LDUR, "LDUR"},{STUR, "STUR"} };

void Sign(const string& binaryLit, int& val){                                                               //Performs 2s compliment
    if(binaryLit[0] == '1'){
        if(binaryLit == "10000000000000000000000000000000"){
            val = -2147483648; return;
        }
        bool change = false;
        string newBinaryLit = binaryLit;
        for(int i = newBinaryLit.length()-1; i >= 0; i--)
            if(change)
                if(newBinaryLit[i] == '0') newBinaryLit[i] = '1';
                else newBinaryLit[i] = '0';
            else if(newBinaryLit[i] == '1') change = true;
        val = stoi(newBinaryLit, 0, 2) * -1; }
    else val = stoi(binaryLit, 0, 2); }

class Command{                                                                                              //Default command base, only used for DUMMY
    public:
        Command(const string& binaryText){
            text = binaryText;
            tag = binaryText.substr(0, 3);
            data = binaryText.substr(3, 29);
            operation = DUMMY; }
        virtual string PrintOutput(){ return "DUMMY\n"; }
        string GetText(){ return text; }
        Operations GetOperation(){ return operation; }
        void SetOperation(){                                                                                //Determine type of operation
            int op = stoi(opcode + '2');                                                                    //Add two to the end so data isn't lost
            switch(op){
                case 100002: operation = CBZ; break;
                case 100012: operation = CBNZ; break;
                case 10000002: operation = ORRI; break;
                case 10000012: operation = EORI; break;
                case 10000102: operation = ADDI; break;
                case 10000112: operation = SUBI; break;
                case 10001002: operation = ANDI; break;
                case 101000002: operation = EOR; break;
                case 101000102: operation = ADD; break;
                case 101000112: operation = SUB; break;
                case 101001002: operation = AND; break;
                case 101001012: operation = ORR; break;
                case 101001102: operation = LSR; break;
                case 101001112: operation = LSL; break;
                case 101010102: operation = LDUR; break;
                case 101010112: operation = STUR; break;
                default: cout<<"Improper opcode!\n"; } }
    protected:
        string text, tag, data, opcode;
        Operations operation; };

class Cat1: public Command{                                                                                 //Follows are the derived command classes, separated by category
    public:
        Cat1(const string& binaryText): Command(binaryText){
            opcode = data.substr(0, 5);
            src1 = data.substr(5, 5);
            branchOffsetLit = data.substr(10, 19);
            SetOperation();
            Sign(branchOffsetLit, branchOffset); }                                                          //Branches need signed offsets
        int GetSource(){ return stoi(src1, 0, 2); }
        int GetOffset(){ return branchOffset; }
        string PrintOutput(){
            string source = ((stoi(src1, 0, 2))==31)?"ZR":to_string(stoi(src1, 0, 2));                      //X32 -> XZR
            return OpMap[operation] + " X" + source + ", #" + to_string(branchOffset) + '\n'; }
    private:
        string src1, branchOffsetLit;
        int branchOffset; };

class Cat2: public Command{
    public:
        Cat2(const string& binaryText): Command(binaryText){
            opcode = data.substr(0, 7);
            dest = data.substr(7, 5);
            src1 = data.substr(12, 5);
            immediateLit = data.substr(17, 12);
            SetOperation();
            if(operation==ADDI||operation==SUBI) Sign(immediateLit, immediateValue);                        //Only ADDI and SUBI need signed immediate
            else immediateValue = stoi(immediateLit, 0, 2); }
        int GetDestination(){ return stoi(dest, 0, 2); }
        int GetSource(){ return stoi(src1, 0, 2); }
        int GetImmediate(){ return immediateValue; }
        string PrintOutput(){
            string destination = ((stoi(dest, 0, 2))==31)?"ZR":to_string(stoi(dest, 0, 2));
            string source = ((stoi(src1, 0, 2))==31)?"ZR":to_string(stoi(src1, 0, 2));
            return OpMap[operation]+" X"+destination+", X"+source+", #"+to_string(immediateValue)+'\n'; }
    private:
        string dest, src1, immediateLit;
        int immediateValue; };

class Cat3: public Command{
    public:
        Cat3(const string& binaryText): Command(binaryText){
            opcode = data.substr(0, 8);
            dest = data.substr(8, 5);
            src1 = data.substr(13, 5);
            src2 = data.substr(18, 5);
            SetOperation(); }
        int GetDestination(){ return stoi(dest, 0, 2); }
        int GetSource1(){ return stoi(src1, 0, 2); }
        int GetSource2(){ return stoi(src2, 0, 2); }
        string PrintOutput(){
            string destination = ((stoi(dest, 0, 2))==31)?"ZR":to_string(stoi(dest, 0, 2));
            string source = ((stoi(src1, 0, 2))==31)?"ZR":to_string(stoi(src1, 0, 2));
            string source2 = ((stoi(src2, 0, 2))==31)?"ZR":to_string(stoi(src2, 0, 2));
            return OpMap[operation]+" X"+destination+", X"+source+", X"+source2+'\n'; }
    private:
        string dest, src1, src2; };

class Cat4: public Command{
    public:
        Cat4(const string& binaryText): Command(binaryText){
            opcode = data.substr(0, 8);
            srcdst = data.substr(8, 5);
            src1 = data.substr(13, 5);
            immediateLit = data.substr(18, 11);
            SetOperation();
            Sign(immediateLit, immediateVal); }                                                             //LDUR and STUR need signed immediates
        int GetDestination(){ return stoi(srcdst, 0, 2); }
        int GetSource(){ return stoi(src1, 0, 2); }
        int GetImmediate(){ return immediateVal; }
        string PrintOutput(){
            string destination = ((stoi(srcdst, 0, 2))==31)?"ZR":to_string(stoi(srcdst, 0, 2));
            string source = ((stoi(src1, 0, 2))==31)?"ZR":to_string(stoi(src1, 0, 2));
            return OpMap[operation]+" X"+destination+", [X"+source+", #"+to_string(immediateVal)+"]"+'\n'; }
    private:
        string srcdst, src1, immediateLit;
        int immediateVal; };

void WriteDisassembly(const map<int, Command*>& commands, const map<int, pair<string, int>>& dataBlocks){   //Output disassembly.txt
    ofstream disassembly ("disassembly.txt");
    if(disassembly){
        for(pair<int, Command*> p : commands)
            disassembly << p.second->GetText()+'\t'+to_string(p.first)+'\t'+p.second->PrintOutput();
        for(pair<int, pair<string, int>> p : dataBlocks)
            disassembly << p.second.first+'\t'+to_string(p.first)+'\t'+to_string(p.second.second)+'\n';
        disassembly.close(); }
    else cout<<"Couldnt open disassembly.txt!\n";
}

void Simulate(map<int, pair<string, int>>& dataBlocks, const map<int, Command*>& commands){                 //Run main simulation, and output simulation.txt
    ofstream simulation ("simulation.txt");
    if(simulation){
        int pc = 64;
        int cycle = 1;
        bool testPurposes = true;
        while(testPurposes){
            int pcAdd = 4;
            Command* com = commands.at(pc);
            simulation << "--------------------\nCycle "+to_string(cycle)+":\t"+to_string(pc)+'\t'+com->PrintOutput()+'\n'; //Output cycle and operation
            switch(com->GetOperation()){                                                                    //Handle simulation based on operation
                case CBZ: {                                                                                 //Scope bindings necessary because of variable declarations
                    Cat1* c = dynamic_cast<Cat1*>(com);
                    pcAdd = Registers.at(c->GetSource())==0?c->GetOffset()*4:4; break; }
                case CBNZ: {
                    Cat1* c = dynamic_cast<Cat1*>(com);
                    pcAdd = Registers.at(c->GetSource())!=0?c->GetOffset()*4:4; break; }
                case ORRI: {
                    Cat2* c = dynamic_cast<Cat2*>(com);
                    Registers.at(c->GetDestination()) = (Registers.at(c->GetSource()) | c->GetImmediate()); break; }
                case EORI: {
                    Cat2* c = dynamic_cast<Cat2*>(com);
                    Registers.at(c->GetDestination()) = (Registers.at(c->GetSource()) ^ c->GetImmediate()); break; }
                case ADDI: {
                    Cat2* c = dynamic_cast<Cat2*>(com);
                    Registers.at(c->GetDestination()) = Registers.at(c->GetSource()) + c->GetImmediate(); break; }
                case SUBI: {
                    Cat2* c = dynamic_cast<Cat2*>(com);
                    Registers.at(c->GetDestination()) = Registers.at(c->GetSource()) - c->GetImmediate(); break; }
                case ANDI: {
                    Cat2* c = dynamic_cast<Cat2*>(com);
                    Registers.at(c->GetDestination()) = (Registers.at(c->GetSource()) & c->GetImmediate()); break; }
                case EOR: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = (Registers.at(c->GetSource1()) ^ Registers.at(c->GetSource2())); break; }
                case ADD: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = Registers.at(c->GetSource1()) + Registers.at(c->GetSource2()); break; }
                case SUB: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = Registers.at(c->GetSource1()) - Registers.at(c->GetSource2()); break; }
                case AND: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = (Registers.at(c->GetSource1()) & Registers.at(c->GetSource2())); break; }
                case ORR: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = (Registers.at(c->GetSource1()) | Registers.at(c->GetSource2())); break; }
                case LSR: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = ((unsigned int)(Registers.at(c->GetSource1()))>>((Registers.at(c->GetSource2())<<27)>>27)); break; }
                case LSL: {
                    Cat3* c = dynamic_cast<Cat3*>(com);
                    Registers.at(c->GetDestination()) = ((unsigned int)(Registers.at(c->GetSource1()))<<((Registers.at(c->GetSource2())<<27)>>27)); break; }
                case LDUR: {
                    Cat4* c = dynamic_cast<Cat4*>(com);
                    Registers.at(c->GetDestination()) = dataBlocks.at(Registers.at(c->GetSource()) + c->GetImmediate()).second; break; }
                case STUR: {
                    Cat4* c = dynamic_cast<Cat4*>(com);
                    dataBlocks.at(Registers.at(c->GetSource()) + c->GetImmediate()).second = Registers.at(c->GetDestination()); break; }
                default: { testPurposes = false; } }
            Registers.at(31) = 0;
            simulation<<"Registers";                                                                        //Output registers
            for(int i = 0; i < 32; i++){
                if(i%8==0){
                    string s = to_string(i);
                    s.insert(s.begin(), 2-s.length(), '0');
                    simulation<<"\nX"+s+':'; }
                simulation<<"\t"+to_string(Registers.at(i)); }
            simulation<<"\n\nData";                                                                         //Output data
            int dataSpaceCounter = 8;
            for(pair<int, pair<string, int>>p:dataBlocks){
                if(dataSpaceCounter==8){
                    simulation<<'\n'+to_string(p.first)+':';
                    dataSpaceCounter=0; }
                simulation<<'\t'+to_string(p.second.second);
                dataSpaceCounter++; }
            simulation<<"\n\n";
            pc+=pcAdd;                                                                                      //Incremend PC and cycle
            cycle++; }
        simulation.close(); }
    else cout<<"Couldn't open simulation.txt!\n"; }

int main(int argc, char* argv[]){                                                                                                //Main Program
    map<int, pair<string, int>> dataBlocks;                                                                 //Initialize data and command space
    map<int, Command*> commands;
    string commandText;
    bool inData = false;                                                                                    //Variable initialization
    int pc = 64;                                                                            
    for(int i = 0; i < 32; i++) Registers[i] = 0; 
    ifstream inFile;                                                                                        //Open sample.txt
    inFile.open (argv[1]);
    if(!inFile) cout << "Unable to open " << argv[1];
    while(inFile >> commandText){                                                                           //Read sample.txt and set commands/data
        if(!inData){
            int tag = stoi(commandText.substr(0, 3));
            switch(tag){
                case 1: commands[pc] = new Cat1(commandText); break;
                case 10: commands[pc] = new Cat2(commandText); break;
                case 11: commands[pc] = new Cat3(commandText); break;
                case 100: commands[pc] = new Cat4(commandText); break;
                case 101: commands[pc] = new Command(commandText);
                    inData = true; break;
                default: cout<<"Improper tag in binary!\n"; } }
        else{
            int dataVal = 0;
            Sign(commandText, dataVal);
            dataBlocks[pc] = make_pair(commandText, dataVal); }
        pc+=4; }
    inFile.close();
    WriteDisassembly(commands, dataBlocks);                                                                 //Write disassembly then perform simulation
    Simulate(dataBlocks, commands); }