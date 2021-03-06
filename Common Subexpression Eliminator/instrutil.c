/**********************************************
        CS415  Project 3
        Spring  2018
        Author: Ulrich Kremer
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "instrutil.h"

static next_register = 1; /* register 0 is reserved */
static next_label = 0;
static next_offset = 0;
static first=0;
int NextRegister() 
{
  if (next_register < MAX_VIRTUAL_REGISTERS)
    return next_register++;
  else {
    printf("*** ERROR *** Reached limit of virtual registers: %d\n", next_register);
    exit(-1);
  }
}

int NextLabel() 
{
  return next_label++;
}

int NextOffset(int units) 
{ 
  int current_offset = next_offset;
  next_offset = next_offset + 4*units;
  return current_offset;
}

void
emitComment(char *comment)
{
  fprintf(outfile, "\t// %s\n", comment);  
}


/*
 * emit implements CSE
 */

int
emit(int label_index,
     Opcode_Name opcode, 
     int field1, 
     int field2, 
     int field3) 
{
  char *label = " ";
  char hashhold [500];
	char hashhold2 [500];
	int reg;
  if (label_index < NOLABEL) {
    printf("ERROR: \"%d\" is an illegal label index.\n", label_index);
    return -1;
  }
    
  if (label_index > NOLABEL) {
    label = (char *) malloc(100);
    sprintf(label, "L%d:", label_index);
  };


  if (!cse_optimization_flag) {
  
    switch (opcode) { /* ---------------------- NON OPTIMIZED ------------------------------- */
    case NOP: 
      fprintf(outfile, "%s\t nop \n", label);
      return -1;
      break;
    case ADD:
      fprintf(outfile, "%s\t add r%d, r%d \t=> r%d \n", label, field1, field2, field3);
      return field3;
      break;
    case SUB: 
      fprintf(outfile, "%s\t sub r%d, r%d \t=> r%d \n", label, field1, field2, field3);
      return field3;
      break;
    case MULT: 
      fprintf(outfile, "%s\t mult r%d, r%d \t=> r%d \n", label, field1, field2, field3);
      return field3;
      break;
    case LOADI: 
      /* Example: loadI 1024 => r1 */
      fprintf(outfile, "%s\t loadI %d \t=> r%d \n", label, field1, field2);
      return field2;
      break;
    case LOADAI: 
      /* Example: loadAI r1, 16 => r3 */
      fprintf(outfile, "%s\t loadAI r%d, %d \t=> r%d \n", label, field1, field2, field3);
      return field3;
      break;
    case STOREAI: 
      /* Example: storeAI r1 => r2, 16 */
      fprintf(outfile, "%s\t storeAI r%d \t=> r%d, %d \n", label, field1, field2, field3);
      break;
    case OUTPUTAI: 
      /* Example: outputAI r0, 16  */
      fprintf(outfile, "%s\t outputAI r%d, %d\n", label, field1, field2);
      break;
    default:
      fprintf(stderr, "Illegal instruction in \"emit\" \n");
    }
    return -1;
  }
	
  else {
	if(first==0)
	{
		//emitComment("Running this for the first and only time.\n");
		init();
		first++;
	}

    switch (opcode) { /* ---------------------- CSE OPTIMIZED ------------------------------- */
	

    case NOP: 
      fprintf(outfile, "%s\t nop \n", label);
      return -1;
      break;
    case ADD:

		sprintf(hashhold, "add r%d r%d",field1,field2);
		sprintf(hashhold2, "add r%d r%d",field2,field1);
		reg = search(hashhold);
		if( reg != -1)
		{
			return reg;
		}
		ins(hashhold2, field3);
		ins(hashhold,field3);
		hashhold[0]='\0';

      fprintf(outfile, "%s\t add r%d, r%d \t=> r%d \n", label, field1, field2, field3);


      return field3;
      break;
    case SUB: 

		sprintf(hashhold, "sub r%d r%d",field1,field2);
		sprintf(hashhold2, "sub r%d r%d",field2,field1);
		reg = search(hashhold);
		if( reg != -1)
		{
			return reg;
		}
		ins(hashhold,field3);
		ins(hashhold2, field3);
		hashhold[0]='\0';

      fprintf(outfile, "%s\t sub r%d, r%d \t=> r%d \n", label, field1, field2, field3);
		

      return field3;
      break;
    case MULT: 

		sprintf(hashhold, "mult r%d r%d",field1,field2);
		sprintf(hashhold2, "mult r%d r%d",field2,field1);
		reg = search(hashhold);
		if( reg != -1)
		{
			return reg;
		}
		ins(hashhold,field3);
		ins(hashhold2, field3);
		hashhold[0]='\0';

      fprintf(outfile, "%s\t mult r%d, r%d \t=> r%d \n", label, field1, field2, field3);
		

      return field3;
      break;
    case LOADI: 
      /* Example: loadI 1024 => r1 */

		sprintf(hashhold, "loadI r%d",field1);
		reg = search(hashhold);
		if( reg != -1)
		{
			return reg;
		}
		ins(hashhold,field2);
		hashhold[0]='\0';

      fprintf(outfile, "%s\t loadI %d \t=> r%d \n", label, field1, field2);
		

      return field2;
      break;
    case LOADAI: 
      /* Example: loadAI r1, 16 => r3 */
		sprintf(hashhold, "storeAI r%d r%d",field1,field2);
		reg = search(hashhold);
		if( reg != -1)
		{
			return reg;
		}
			ins(hashhold,field3);
			hashhold[0]='\0';

      fprintf(outfile, "%s\t loadAI r%d, %d \t=> r%d \n", label, field1, field2, field3);
		

      return field3;
      break;
    case STOREAI: 
      /* Example: storeAI r1 => r2, 16 */
		sprintf(hashhold, "storeAI r%d r%d",field2,field3);
		reg = search(hashhold);
		if( reg == -1)
		{	
			ins(hashhold,field1);
			fprintf(outfile, "%s\t storeAI r%d \t=> r%d, %d \n", label, field1, field2, field3);	
		}
		else
		{
			edit(hashhold,field1);
			fprintf(outfile, "%s\t storeAI r%d \t=> r%d, %d \n", label, field1, field2, field3);	
		}
      		
				

      break;
    case OUTPUTAI: 
      /* Example: outputAI r0, 16  */


      fprintf(outfile, "%s\t outputAI r%d, %d\n", label, field1, field2);


      break;
    default:
      fprintf(stderr, "Illegal instruction in \"emit\" \n");
    }
  return -1;
  }
}




