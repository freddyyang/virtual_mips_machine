#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"

// these are the structures used in this simulator


// global variables
// register file
int regfile[32];
// instruction memory
int instmem[100];  // only support 100 static instructions
// data memory
int datamem[1024];
// program counter
int pc;

/* load
 *
 * Given the filename, which is a text file that 
 * contains the instructions stored as integers 
 *
 * You will need to load it into your global structure that 
 * stores all of the instructions.
 *
 * The return value is the maxpc - the number of instructions
 * in the file
 */
int load(char *filename)
{
  FILE *file;
  file = fopen(filename,"r");
  char buf[100];
  int i=0;
  pc=0;
  while(fgets(buf,1000,file)!=NULL)
    {
      instmem[i]=atoi(buf);
      i++;
    }
  fclose(file);
  return i;
}

/* fetch
 *
 * This fetches the next instruction and updates the program counter.
 * "fetching" means filling in the inst field of the instruction.
 */
void fetch(InstInfo *instruction)
{
  instruction->inst=instmem[pc];
}

/* decode
 *
 * This decodes an instruction.  It looks at the inst field of the 
 * instruction.  Then it decodes the fields into the fields data 
 * member.  The first one is given to you.
 *
 * Then it checks the op code.  Depending on what the opcode is, it
 * fills in all of the signals for that instruction.
 */
void decode(InstInfo *instruction)
{
	// fill in the signals and fields
	int val = instruction->inst;
	int op, func;
	instruction->fields.op = (val >> 26) & 0x03f;

	// fill in the rest of the fields here
	instruction->fields.func = (val>>0)& 0x03f;
	int format_num = 0; // 0 for R format;  1 for I format; 2 for J format;

	// now fill in the signals
	op = instruction->fields.op;
	func = instruction->fields.func;
	
	instruction->fields.rd = (val >> 11) & 0x01f;
	instruction->fields.rt = (val >> 16) & 0x01f;
	instruction->fields.rs = (val >> 21) & 0x01f;
	instruction->fields.imm = (val >> 0) & 0xFFFF;

	if(instruction->fields.imm>>15==1) //This means the immediate is negative. 
	  { 
	    instruction->fields.imm=((0xFFFF0000)|(instruction->fields.imm));
	    //This is the same as adding 16 1's to the beginning of fields.imm. 32 bits total. 
	  }
	else 
	  { 
	    instruction->fields.imm = ((0x0000FFFF)&(instruction->fields.imm)); 
	    //This is the same as adding 16 0's to the beginning of fields.imm. 32 bits total. 
	  }



	if (op == 48)// ADDI
	  {
		instruction->signals.aluop = 0;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = 0;
		instruction->signals.asrc = 1;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 0;
		instruction->signals.rw = 1;

		sprintf(instruction->string,"addi $%d, $%d, %d",
			instruction->fields.rt, instruction->fields.rs, 
			instruction->fields.imm);
		instruction->destreg = instruction->fields.rd;
	  }
	else if (op == 33)
	  {
	  if ( func == 40 ) // AND
	  {
		instruction->signals.aluop = 2;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = 0;
		instruction->signals.asrc = 0;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 1;
		instruction->signals.rw = 1;

		sprintf(instruction->string,"and $%d, $%d, $%d",
			instruction->fields.rd, instruction->fields.rs, 
			instruction->fields.rt);
		instruction->destreg = instruction->fields.rd;
	  }
	  else if ( func == 24 ) // SUB
	  {
		instruction->signals.aluop = 1;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = 0;
		instruction->signals.asrc = 0;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 1;
		instruction->signals.rw = 1;
	
		sprintf(instruction->string,"sub $%d, $%d, $%d",
			instruction->fields.rd, instruction->fields.rs, 
			instruction->fields.rt);
		instruction->destreg = instruction->fields.rd;

	  }
	  else if ( func == 10) // SGT
	  {
		instruction->signals.aluop = 6;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = 0;
		instruction->signals.asrc = 0;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 1;
		instruction->signals.rw = 1;

		sprintf(instruction->string,"sgt $%d, $%d, $%d",
			instruction->fields.rd, instruction->fields.rs, 
			instruction->fields.rt);
		instruction->destreg = instruction->fields.rd;

	  }
	  }
	else if (op == 18) // LW
	  {
		instruction->signals.aluop = 0;
		instruction->signals.mw = 0;
		instruction->signals.mr = 1;
		instruction->signals.mtr = 1;
		instruction->signals.asrc = 1;
		instruction->signals.btype = 0;
		instruction->signals.rdst = 0;
		instruction->signals.rw = 1;

		sprintf(instruction->string,"lw $%d,%d($%d)",
			instruction->fields.rt, instruction->fields.imm, 
			instruction->fields.rs);
		instruction->destreg = instruction->fields.rd;
	  }  
	else if (op == 19) // SW
	  {
		instruction->signals.aluop = 0;
		instruction->signals.mw = 1;
		instruction->signals.mr = 0;
		instruction->signals.mtr = -1;
		instruction->signals.asrc = 1;
		instruction->signals.btype = 0;
		instruction->signals.rdst = -1;
		instruction->signals.rw = 0;

		sprintf(instruction->string,"sw $%d,%d($%d)",
			instruction->fields.rt, instruction->fields.imm, 
			instruction->fields.rs);
		instruction->destreg = instruction->fields.rd;
	  }
	else if (op == 14) // BLT
	  {
		instruction->signals.aluop = 7;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = -1;
		instruction->signals.asrc = 0;
		instruction->signals.btype = 3;
		instruction->signals.rdst = -1;
		instruction->signals.rw = 0;

		sprintf(instruction->string,"blt $%d, $%d, %d",
			instruction->fields.rs, instruction->fields.rt, 
			instruction->fields.imm);
		instruction->destreg = instruction->fields.rd;
	  }
	else if (op == 41) // JR
	  {
		instruction->signals.aluop = -1;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = -1;
		instruction->signals.asrc = -1;
		instruction->signals.btype = 2;
		instruction->signals.rdst = -1 ;
		instruction->signals.rw = 0;

		sprintf(instruction->string,"jr $%d",
			instruction->fields.rs);
		instruction->destreg = instruction->fields.rd;
	  }
	else if (op == 10 ) // JAL
	  {
	        format_num = 2;
		instruction->signals.aluop = -1;
		instruction->signals.mw = 0;
		instruction->signals.mr = 0;
		instruction->signals.mtr = 2;
		instruction->signals.asrc = -1;
		instruction->signals.btype = 1;
		instruction->signals.rdst = 2;
		instruction->signals.rw = 1;

		sprintf(instruction->string,"jal %d",
			instruction->fields.imm);
		instruction->destreg = instruction->fields.rd;
	   }
	
	// fill up the rest of fields in intruction 
	// read from the register file
	// fill in s1data and input2

	instruction->s1data = regfile[instruction->fields.rs];
	instruction->s2data = regfile[instruction->fields.rt];
	instruction->input1 = instruction->s1data;
	instruction->input2 = (instruction->signals.asrc == 1) ? instruction->s2data = instruction->fields.imm : instruction->s2data;

}

void execute(InstInfo *instruction)
{
  
  int op1 = instruction->fields.op;
  int func1 = instruction->fields.func;
  if (op1 == 48) //ADDI
    {                                                                                                                                                                                            
      instruction->aluout = regfile[instruction->fields.rs]+ instruction->fields.imm;
      // instruction->aluout = instruction->fields.rt;
    }
  else if (op1 == 33)
    {
      if( func1 == 40)//AND                                                                                                                                                                                                                                                                       
        {
          instruction->aluout = regfile[instruction->fields.rt] & regfile[instruction->fields.rs];
          //instruction->aluout = instruction->fields.rd;
        }

      else if( func1 == 24)//SUB                                                                                                                                                                                                                                                                  
        {
          instruction->aluout = regfile[instruction->fields.rt] -  regfile[instruction->fields.rs];
	  // instruction->aluout = instruction->fields.rd;
        }
      else if (func1 ==10)//SGT                                                                                                                                                                                                                                                                   
        {
          if(regfile[instruction->fields.rs] > regfile[instruction->fields.rt])
            {
              instruction->aluout =1;
            }
          else
            {
            instruction->aluout = 0;
            }
          //instruction->aluout = instruction->fields.rd;
          
        }
    }
  else if(op1 ==18)//LW                                                                                                                                                                                                                                                           
    {
      regfile[instruction->fields.rs] = datamem[instruction->aluout];
      //instruction->input1 = regfile[rs];              
      //instruction->input2 = instruction->fields.imm;                                                           
    }
  else if(op1 == 19) //SW                                                                                    
      {

        instruction->aluout = regfile[instruction->fields.rs] + instruction->fields.imm;

        //regfile[rs] = instruction->s1data;                                                                   
        //regfile[rt] = instruction->s2data;                                                                                                                       
      }
  else if (op1 ==14) //BLT 
    { 
      if(regfile[instruction->fields.rs] < regfile[instruction->fields.rt]) 
	{
	  instruction->aluout = 1;
	} 
    } 
  else if(op1 == 41)//JR 
    { 
      instruction->aluout = regfile[instruction->fields.rs];
    } 
  else if(op1 == 10)//JAL
    { 
      instruction->aluout = (instruction->inst)&(0x3FFFFFF);
    }

if (instruction->signals.mtr == 0)
  {
    instruction->destdata = instruction -> aluout;
  }
}


/* memory                                                                        * If this is a load or a store, perform the memory operation                    */
void memory(InstInfo *instruction)
{
 
  if(instruction->signals.mw == 1)
    {
      datamem[instruction->aluout] = instruction->s2data;

    }
  else if(instruction->signals.mr == 1)
    {
      instruction->memout = datamem[instruction->aluout];
    }
  
}


/* writeback
*
* If a register file is supposed to be written, write to it now
*/

void writeback(InstInfo *instruction)
{
	if(instruction->signals.rw==1)
	{
		if(instruction->signals.rdst==0)
		{			
			//if(op==48)	//this is addi;
			if(instruction->signals.mtr==0)				
			{
				regfile[instruction->fields.rt]= instruction->aluout;
			}
			//if(op==18)	//this is lw.
			if(instruction->signals.mtr==1)
			{
				regfile[instruction->fields.rt]= instruction->memout; 
			}
		}

		if(instruction->signals.rdst==1)
		{
			//This is operations  and, sub, sgt.
		       	regfile[instruction->fields.rd] = instruction->aluout;
		}

		if(instruction->signals.rdst==2)
		{
			//This is operation jal.
			regfile[31]= pc+1;
		}


	}
}

void setPCWithInfo (InstInfo *instruction)
{ 
  int jumpaddress;
  switch (instruction->signals.btype)
    {
    case 0: 
      //This is not a branch or jump, move the PC forward one integer.
      pc++;
      break;
    case 1: //Jump to an address encoded in instruction; jal
      //regfile[31]= PC; , already taken care of in writeBack());
      jumpaddress=instruction->aluout;
      pc = (pc&(0xFC000000))+jumpaddress; 
      //FC000000 is 6 1's and 26 0's. 3FFFFFF is 6 0's and 26 1's. ( can also be thought of as just 26 1's.)
      break; 
    case 2: //Jump to a register-specified location; jr
      //PC = register-specified location;
      jumpaddress=instruction->aluout; 
      pc = jumpaddress;
      break;
    case 3: //Branch if less than, relative addressing; blt
      if(instruction->aluout==1) 
	{ 
	  jumpaddress = instruction->fields.imm; 
	  // if(jumpaddress>>15==1) //This means the immediate is negative. 
	  // { 
	  pc= pc+1+ jumpaddress;
	  //This is the same as adding 16 1's to the beginning of fields.imm. 32 bits total. 
	  // }
	  //else 
	  // { 
	  // pc = pc+1+((0x0000FFFF)&(jumpaddress)); 
	  //This is the same as adding 16 0's to the beginning of fields.imm. 32 bits total. 
	  //}
	}
      else
	{
	  pc++;
	}
      break;
    }
}
