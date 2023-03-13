/************************************************************************
 * Submitted by: Jean Claude Turambe, jnn56
 * Project 1 Disassembler
 * Copyright 2022, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. 
 ************************************************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <iomanip>
using namespace std;

const int NREGS = 32;
const string regNames[NREGS] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

///////////////////////////////////////////////////////////////////////////
// Disassemble the 32-bit MIPS instruction (assumed to be at the given PC)
// and print the assembly instruction to screen
// ALL YOUR CHANGES GO IN THIS FUNCTION
///////////////////////////////////////////////////////////////////////////
void disassembleInstr(uint32_t pc, uint32_t instr) {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t simm;         // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

  opcode = (instr >> 26) & 0x3f; //The opcode field is 6 bits long
  rs = (instr >> 21) & 0x1f;    //Value of 5bits long
  rt = (instr >> 16) & 0x1f;   //Value of 5bits long
  rd = (instr >> 11) & 0x1f;  //Value of 5bits long
  shamt = (instr >> 6) & 0x1f; //value of 5bits long(6 to 10)
  funct = instr & 0x3f; //Value of 6 bits long (0 to 5).
  uimm = instr & 0xffff; //The 16 bit immediate value(0-15)
  simm = ((signed)uimm << 16) >> 16;//The 16 bit signed immediate value
  addr = instr & 0x3ffffff;//A 26-bit shortened address of the destination(0-25).

  cout << hex << setw(8) << pc << ": ";
  switch(opcode) {
    case 0x00:
      switch(funct) {
      	//This switch block will perform the R-type instructions opcode/funct,
		//the opcode characterize the statements for switch cases.
      	//Acces regNames[] for printing register specifier
      	//and the default statement prints unimplemented case.
        case 0x00: cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt; break;
        case 0x03: cout << "sra " << regNames[rd]<< ", " << regNames[rs] << ", " << dec << shamt; break;
        case 0x08: cout << "jr " << regNames[rs]; break;
        case 0x10: cout << "mfhi " << regNames[rd]; break;
        case 0x12: cout << "mflo " << regNames[rd]; break;
        case 0x18: cout << "mult " << regNames[rd] << ", " << regNames[rt]; break;
        case 0x1a: cout << "div " << regNames[rd] << ", " << regNames[rt]; break;
        case 0x21: cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]; break;
        case 0x22: cout << "sub " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]; break;
        case 0x23: cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]; break;
        case 0x2a: cout <<"slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]; break;
        default: cout << "unimplemented";
      }
      break;
    //These cases will perform the I or J-type instructions(only opcode),
	//Acces regNames[] for printing register specifier
	//Implement instruction for BranchAddr and SignExtImm.
    //default statement prints unimplemented case.
    case 0x02: cout << "j " << hex << ((pc + 4) & 0xf0000000) + addr * 4; break;
    case 0x03: cout << "jal " << hex << ((pc + 4) & 0xf0000000) + addr * 4; break;
    case 0x04: cout << "beq " << regNames[rs] << ", " <<regNames[rt] << ", " << (pc + 4) + (simm << 2); break;
    case 0x05: cout << "bne " << regNames[rs] << ", " <<regNames[rt] << ", " << (pc + 4) + (simm << 2); break;
    case 0x09: cout << "addiu " << regNames[rt] << ", " <<regNames[rs] << ", " << dec << simm; break;
    case 0x0c: cout << "andi " << regNames[rt] << ", " <<regNames[rs] << ", " << dec << simm; break;
    case 0x0f: cout << "lui " << regNames[rt] << ", " << dec << simm; break;
    case 0x1a: cout << "trap " << hex << addr; break;
    case 0x23: cout << "lw " << regNames[rt] << ", " << dec << simm << "(" <<regNames[rs] << ")"; break;
    case 0x2b: cout << "sw " << regNames[rt] << ", " << dec << simm << "(" <<regNames[rs] << ")"; break;
    default: cout << "unimplemented";
  }
  cout << endl;
}

////////////////////////////////////////////
// Swizzle all bytes of the input word
////////////////////////////////////////////
uint32_t swizzle(uint8_t *bytes) {
  return (bytes[0] << 24) | (bytes[1] << 16) | bytes[2] << 8 | bytes[3];
}

//////////
// MAIN
//////////
int main(int argc, char *argv[]) {
  int count, start;
  ifstream exeFile;
  uint8_t bytes[4];

  uint32_t *instructions;

  cout << "CS 3339 MIPS Disassembler" << endl;
  if(argc != 2) {
    cerr << "usage: " << argv[0] << " mips_executable" << endl;
    return -1;
  }

  // open the executable
  exeFile.open(argv[1], ios::binary | ios::in);
  if(!exeFile) {
    cerr << "error: could not open executable file " << argv[1] << endl;
    return -1;
  }

  // BE->LE swap: Executable files are stored 0A0B0C0D => addr 00,01,02,03
  //              Read into bytes[] as b[0],b[1],b[2],b[3] = 0A,0B,0C,0D
  //              Need to swizzle bytes back to Little-endian
  // read number of words in the input file
  if(!exeFile.read((char *)&bytes, 4)) {
    cerr << "error: could not read count from file " << argv[1] << endl;
    return -1;
  }
  count = swizzle(bytes);
  // read start address from file
  if(!exeFile.read((char *)&bytes, 4)) {
    cerr << "error: could not read start addr from file " << argv[1] << endl;
    return -1;
  }
  start = swizzle(bytes);

  // make space and read instructions
  instructions = new uint32_t[count];
  if(!instructions) {
    cerr << "error: out of memory" << endl;
    return -1;
  }
  for(int i = 0; i < count; i++) {
    if(!exeFile.read((char *)&bytes, 4)) {
      cerr << "error: could not read instructions from filename " << argv[1] << endl;
      return -1;
    }
    instructions[i] = swizzle(bytes);
  }

  exeFile.close();
  
  // disassemble
  for(int i = 0; i < count; i++) {
    disassembleInstr(start + i * 4, instructions[i]);
  }
  return 0;
}
