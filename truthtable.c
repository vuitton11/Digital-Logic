#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/*Enum for all possible instructions*/
typedef enum {AND, OR, NOT, NAND, NOR, XOR, PASS, MULTIPLEXER, DECODER} OpType;

typedef struct node{
	struct node *next;
	int variable;
	char name[251];
}node;

typedef struct hashtable
{
	int size;
	struct node **table;
}hashtable;

typedef struct instruction{
	OpType op;
	int numOfVariables;	//Number of variables
	int numOfTemps;	//Number of temps
	char variableName[100][50];	//Holds the variable names
	int done;	//Determines if the instruction is done
	int n;	//Used for multiplexer -> represents n -> selectors
}instruction;

//allocates space for the hashtable
struct hashtable* create_table(int size) {
	int x;
	struct hashtable *varTable = (struct hashtable*) malloc(sizeof(struct hashtable));
	varTable->size = size;
	varTable->table = malloc(sizeof(struct node*) * size);
	for (x = 0; x < size; x++) {
		varTable->table[x] = NULL;
	}
	return varTable;
}

//FREES THE NODES
void eraseNode(struct node *ptr) {
	if (ptr == NULL) return;
	if (ptr->next != NULL) eraseNode(ptr->next);
	free(ptr);
	ptr = NULL;
}
//FREES THE HASHTABLE
void eraseTable(struct hashtable *hashTable) {
	int i;
	for (i = 0; i < hashTable->size; i++) {
		eraseNode(hashTable->table[i]);
	}
	free(hashTable->table);
	hashTable->table = NULL;
	free(hashTable);
	hashTable = NULL;
}
//FREES MEMORY
void cleanup(struct instruction *instructions[], int numOfInstr) {
	int i;
	for (i = 0; i < numOfInstr; i++) {
		struct instruction *ins = instructions[i];
		free(ins);
		ins = NULL;
	}
}

/*Takes in a string and the length of the string
 *Creates a hashkey by taking each char in the string
 *and converts it into its integer value and adds them all up
 *then uses the total value and mods it by 1000 and returns the integer 
 */
int hashKey(char buffer[],int x){
	int key, total = 0, i;
	for(i = 0; i < x; i++){
		total += ((int)buffer[i] *(10+i));
	}
	key = total % 1000;
	return abs(key);
}

/*Takes in a string, the hashkey and the hashtable
 *Checks to see if the string is in the hashtable by comparing the structs->name
 *checks the name to avoid the case that a strings value added is equal to another strings added value
 *returns 0 if not found, else returns 1 if found
 */
int opSearch(char buffer[], int hashKey, struct hashtable *matrix){
	int present;
	node *bucket = matrix->table[hashKey];
	while(1){
		if(bucket == NULL){	//End of valid numbers
			present = 0;
			break;
		}
		if(strcmp(bucket->name, buffer) == 0){	//Found
			present = 1;
			break;
		}
		bucket = bucket->next; //moves to the next item in the list
	}
	return present;
}

/*Takes in a string, hashkey and a hashtable
 *Searches the hashtable to check if the struct is already present in the table
 *If so the method ends, else it creates space for the node and it inserts the the name into the table
 *and inserts -1 as a placeholder value to -> variables to show that this value is empty
 */
void opInsert(char buffer[], int hashKey, struct hashtable *matrix){
	int present = opSearch(buffer, hashKey, matrix);
	if(present == 1){
		//printf("duplicate\n");
		return;
	}else{
		//printf("inserted\n");
		node *this;
		node *additional = malloc(sizeof(node));
		if(matrix->table[hashKey] == NULL){
			strcpy(additional->name, buffer);
			additional->variable = -1;
			additional->next = NULL;
			matrix->table[hashKey] = additional;
		}else{
			this = matrix->table[hashKey];
			strcpy(additional->name, buffer);
			additional->variable = -1;
			additional->next = this;
			matrix->table[hashKey] = additional;
		}
	}
}

/*Takes in a file pointer, number of inputs, hashtable, and an array of integers
 *for the number of inputs reads each variable and inserts it into the hashtable of puts and saves its hashkey 
 *to the int array accordingly
 */
void makePuts(FILE *fp, int numOfPuts, struct hashtable *puts, int *keys, int extra){
	int i;
    char buffer[251];
    for(i = 0; i < numOfPuts; i++){
    	fscanf(fp, "%s", buffer);
    	if(strcmp(buffer, ":") == 0){
			i--;
			continue;
		}
		if(extra == 1 && i == numOfPuts-1){
			printf("%s",buffer);
		}else if(extra == 1 && i != numOfPuts-1){
			printf("%s ",buffer);
		}
    	int length = strlen(buffer);
    	int key = hashKey(buffer, length);
    	keys[i] = key;
    	//printf("String: %s	length:	%d	key: %d\n",buffer, length, key);
    	opInsert(buffer, key, puts);
    }
}

/*Allocates space for struct instruction*/
struct instruction *create_instruction() {
	struct instruction *ins = (struct instruction*)malloc(sizeof(struct instruction));
	return ins;
}

/*Takes in a string, hashkey, hashtable, and a value
 *searches the hashtable for the string name and if found inserts the value into the struct
 */
void setValue(char buffer[], int hashKey, struct hashtable *matrix, int value){
	node *bucket = matrix->table[hashKey];
	while(1){
		if(strcmp(bucket->name, buffer) == 0){	//Found
			matrix->table[hashKey]->variable = value;
			break;
		}
		bucket = bucket->next; //moves to the next item in the list
	}
}

/*Takes in a integer(index of row), hashtable of inputs, array of hashkeys, number of inputs, int extra (for the extra credit)
 *Converts the decimal number into binary and breaks it up using them as inputs to set into struct->variables
 *If the extra credit is present it will print the correct spacinf by determining the amount of letters a string has and space it by that -1
 */
void binaryCode(int gray, struct hashtable *inputs, int * keys, int numOfIn, int extra){
	int i, j, holder, indexHolder;
	for(i = numOfIn-1; i >= 0; i--){
		holder = gray >> i;
		if(holder & 1){
 	    	indexHolder = 1;
        }else{
        	indexHolder = 0;
        }
        if(extra == 1){
        	for(j = 0; j < strlen(inputs->table[keys[numOfIn-i-1]]->name)-1; j++){
        		printf(" ");
        	}
        	printf("%d ", indexHolder);
        	setValue(inputs->table[keys[numOfIn-i-1]]->name, keys[numOfIn-i-1], inputs, indexHolder);
    	}else{
    		printf("%d ", indexHolder);
        	setValue(inputs->table[keys[numOfIn-i-1]]->name, keys[numOfIn-i-1], inputs, indexHolder);
    	}
	}
}

/*Does AND operation*/
int ANDgate(int p, int q){
	return p && q;
}

/*Does OR operation*/
int ORgate(int p, int q){
	return p || q;
}

/*Does NOT operation*/
int NOTgate(int p){
	return !p;
}

/*Does XOR operation*/
int XORgate(int p, int q){
	return p ^ q;
}

/*Searches the hashtable and retrieves the value of the struct wanted
 *by comparing the names to the wanted string input
 */
int getValue(char buffer[], int hashKey, struct hashtable *matrix){
	int value;
	node *bucket = matrix->table[hashKey];
	while(1){
		if(bucket == NULL){	//End of valid numbers
			value = -1;
			break;
		}
		if(strcmp(bucket->name, buffer) == 0){	//Found
			value = matrix->table[hashKey]->variable;
			break;
		}
		bucket = bucket->next; //moves to the next item in the list
	}
	return value;
}

/*Prints out the outputs*/
void outPutPrint(struct hashtable *outputs, int *keys, int numOfout, int extra){
	int i,j;
	if(extra == 1){
		for(i = 0; i < numOfout; i++){
			for(j = 0; j < strlen(outputs->table[keys[i]]->name)-1; j++){
        		printf(" ");
        	}
			printf(" %d", outputs->table[keys[i]]->variable);
			outputs->table[keys[i]]->variable = -1;
		}
		printf("\n");
	}else{
		for(i = 0; i < numOfout; i++){
			printf(" %d", outputs->table[keys[i]]->variable);
			outputs->table[keys[i]]->variable = -1;
		}
		printf("\n");
	}
}

/*Resets the temp variables to -1 to work on the next line of inputs*/
void resetTempVariable(struct hashtable *inputs, int * tempKeyHolder, int numOftempsVars){
	int i;
	for(i = 0; i < numOftempsVars; i++){
		//printf("%s\n", inputs->table[tempKeyHolder[i]]->name);
		inputs->table[tempKeyHolder[i]]->variable = -1;
	}
}

/*Resets the ->done = 0, so we can work on the next line of inputs*/
void resetDones(struct instruction **instructions, int numOfInstr){
	int i; 
	for(i = 0; i < numOfInstr; i++){
		instructions[i]->done = 0;
	}
}

/*Evaluates the instructions and does all the work of putting all the methods together*/
void evalCirc(struct hashtable *inputs, struct hashtable *outputs, struct instruction **instructions, int numOfInstr, int numOfLines, int *inKeys, int *outKeys, int numOfIn, int numOfout, int * tempKeyHolder, int numOftempsVars, int extra){
	int i, j, u, isdone, count, tempVar, hKey;
	for(i = 0; i < numOfLines; i++){	//Runs the loop for the amount of input lines there must be
		binaryCode(i, inputs, inKeys, numOfIn, extra);
		printf("|");
		count = 0;
		isdone = 0;
		for(;;count++){	//Runs until all instructions are complete
			tempVar = 0;
			if(isdone == numOfInstr) break; //INstruction is complete
			if(count == numOfInstr) count = 0;//Has passed all instruction but not all are done so resets the instruction counter
			if(instructions[count]->done == 0){	//checks if the instruction is complete or not
				if(instructions[count]->op == MULTIPLEXER){
					//printf("\n");
					int contin = 0;
					for(u = 0; u < instructions[count]->numOfVariables-1; u++){	//Checks to see if we have all the variables needed to complete this instruction
						hKey = hashKey(instructions[count]->variableName[u], strlen(instructions[count]->variableName[u]));
						if(getValue(instructions[count]->variableName[u], hKey, inputs) != -1) continue; //Data found
						else if(getValue(instructions[count]->variableName[u], hKey, outputs) == -1){
							contin = 1;	//Data not found
							//printf("DATA NOT FOUND\n");
							break;
						}else continue;
					}
					if(contin == 1) continue;	//Not enough info -> move to next instruction
					int bin, num, varPlacer;	//enough data is found
					/*Turns the selctors input into a binary number then converts it into a decimal and then determines which input it is as if its an index*/
					for(bin = 0, j = pow(2,instructions[count]->n); j < instructions[count]->numOfVariables-1;++j){
						hKey = hashKey(instructions[count]->variableName[j], strlen(instructions[count]->variableName[j]));
						if(getValue(instructions[count]->variableName[j], hKey, inputs) != -1) num = getValue(instructions[count]->variableName[j], hKey, inputs);
						else num = getValue(instructions[count]->variableName[j], hKey, outputs);
						//printf("num: %d\n", num);
						bin = bin << 1;
						bin = bin + num;
					}
					/*Places the value in the output by checking where it might be in the input or output hashtable*/
					varPlacer = getValue(instructions[count]->variableName[bin], hashKey(instructions[count]->variableName[bin], strlen(instructions[count]->variableName[bin])), inputs);
					if(varPlacer == -1) varPlacer = getValue(instructions[count]->variableName[bin], hashKey(instructions[count]->variableName[bin], strlen(instructions[count]->variableName[bin])), outputs);
					hKey = hashKey(instructions[count]->variableName[instructions[count]->numOfVariables-1], strlen(instructions[count]->variableName[instructions[count]->numOfVariables-1]));
					if(opSearch(instructions[count]->variableName[instructions[count]->numOfVariables-1], hKey, inputs) == 1){
						setValue(instructions[count]->variableName[instructions[count]->numOfVariables-1], hKey, inputs, varPlacer);
					}else{
						setValue(instructions[count]->variableName[instructions[count]->numOfVariables-1], hKey, outputs, varPlacer);
					}
					instructions[count]->done = 1;
					isdone++;
				}
				else if(instructions[count]->op == DECODER){
					int contin = 0;
					for(u = 0; u < instructions[count]->n; u++){	//Checks to see if we have all the variables needed to complete this instruction
						hKey = hashKey(instructions[count]->variableName[u], strlen(instructions[count]->variableName[u]));
						if(getValue(instructions[count]->variableName[u], hKey, inputs) != -1) continue; //Data found
						else if(getValue(instructions[count]->variableName[u], hKey, outputs) == -1){
							contin = 1;	//Data not found
							//printf("DATA NOT FOUND\n");
							break;
						}else continue;
					}
					if(contin == 1) continue;	//Not enough info -> move to next instruction
					int bin, num;	//enough data is found
					/*Turns the selctors input into a binary number then converts it into a decimal and then determines which input it is as if its an index*/
					for(bin = 0, j = 0; j < instructions[count]->n;++j){
						hKey = hashKey(instructions[count]->variableName[j], strlen(instructions[count]->variableName[j]));
						if(getValue(instructions[count]->variableName[j], hKey, inputs) != -1) num = getValue(instructions[count]->variableName[j], hKey, inputs);
						else num = getValue(instructions[count]->variableName[j], hKey, outputs);
						bin = bin << 1;
						bin = bin + num;
					}
					//printf("BINARY: %d\n", bin);
					int counterONE = 0; //When thr counter == bin then that spot will be a 1 and everything else is a zero
					for(j = instructions[count]->n; j < instructions[count]->numOfVariables; j++){
						if(counterONE == bin){
							hKey = hashKey(instructions[count]->variableName[j], strlen(instructions[count]->variableName[j]));
							if(opSearch(instructions[count]->variableName[j], hKey, inputs) == 1){
								setValue(instructions[count]->variableName[j], hKey, inputs, 1);
							}else{
								setValue(instructions[count]->variableName[j], hKey, outputs, 1);
							}
						}else{
							hKey = hashKey(instructions[count]->variableName[j], strlen(instructions[count]->variableName[j]));
							if(opSearch(instructions[count]->variableName[j], hKey, inputs) == 1){
								setValue(instructions[count]->variableName[j], hKey, inputs, 0);
								//printf("0\n");
							}else{
								setValue(instructions[count]->variableName[j], hKey, outputs, 0);
							}
						}
						counterONE++;
					}
					instructions[count]->done = 1;
					isdone++;
				}
				else if(instructions[count]->op == AND || instructions[count]->op == OR || instructions[count]->op == NAND || instructions[count]->op == NOR || instructions[count]->op == XOR){
					/*Checks to see if we have enough info to complete this instruction*/
					int p = -1, q = -1, k;
					for(j = 0; j < instructions[count]->numOfVariables-1; j++){
						hKey = hashKey(instructions[count]->variableName[j], strlen(instructions[count]->variableName[j]));
						k = getValue(instructions[count]->variableName[j], hKey, inputs);
						if(k != -1){
							tempVar++;
							if(p == -1){
								p = k;
							}else{
								q = k;
							}
						}else{
							k = getValue(instructions[count]->variableName[j], hKey, outputs);
							if(k != -1){
								tempVar++;
								if(p == -1){
									p = k;
								}else{
									q = k;
								}
							}else{
								break;
							}
						}
					}
					if(tempVar == instructions[count]->numOfVariables-1){	//There is enough info to complete the instruction
						switch(instructions[count]->op){
							case 0: //AND
								hKey = hashKey(instructions[count]->variableName[2], strlen(instructions[count]->variableName[2]));
								if(opSearch(instructions[count]->variableName[2], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[2], hKey, inputs, ANDgate(p, q));
								}else{
									setValue(instructions[count]->variableName[2], hKey, outputs, ANDgate(p, q));
								}
								break;
							case 1:	//OR
								hKey = hashKey(instructions[count]->variableName[2], strlen(instructions[count]->variableName[2]));
								if(opSearch(instructions[count]->variableName[2], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[2], hKey, inputs, ORgate(p, q));
								}else{
									setValue(instructions[count]->variableName[2], hKey, outputs, ORgate(p, q));
								}
								break;
							case 3: //NAND
								hKey = hashKey(instructions[count]->variableName[2], strlen(instructions[count]->variableName[2]));
								if(opSearch(instructions[count]->variableName[2], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[2], hKey, inputs, NOTgate(ANDgate(p, q)));
								}else{
									setValue(instructions[count]->variableName[2], hKey, outputs, NOTgate(ANDgate(p, q)));
								}
								break;
							case 4: //NOR
								hKey = hashKey(instructions[count]->variableName[2], strlen(instructions[count]->variableName[2]));
								if(opSearch(instructions[count]->variableName[2], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[2], hKey, inputs, NOTgate(ORgate(p, q)));
								}else{
									setValue(instructions[count]->variableName[2], hKey, outputs, NOTgate(ORgate(p, q)));
								}
								break;
							case 5: //XOR
								hKey = hashKey(instructions[count]->variableName[2], strlen(instructions[count]->variableName[2]));
								if(opSearch(instructions[count]->variableName[2], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[2], hKey, inputs, XORgate(p, q));
								}else{
									setValue(instructions[count]->variableName[2], hKey, outputs, XORgate(p, q));
								}
								break;
							default:
								printf("Error not a valid instruction\n");
						}
						instructions[count]->done = 1;
						isdone++;
					}else{
						continue;
					}
				}
				else if(instructions[count]->op == NOT || instructions[count]->op == PASS){ //Evaluates NOT and PASS
					int k;
					hKey = hashKey(instructions[count]->variableName[0], strlen(instructions[count]->variableName[0]));
					k = getValue(instructions[count]->variableName[0], hKey, inputs);
					if(k != -1){
						switch(instructions[count]->op){
							case 2:	//NOT
								hKey = hashKey(instructions[count]->variableName[1], strlen(instructions[count]->variableName[1]));
								if(opSearch(instructions[count]->variableName[1], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[1], hKey, inputs, NOTgate(k));
								}else{
									setValue(instructions[count]->variableName[1], hKey, outputs, NOTgate(k));
								}
								break;
							case 6:	//PASS
								hKey = hashKey(instructions[count]->variableName[1], strlen(instructions[count]->variableName[1]));
								if(opSearch(instructions[count]->variableName[1], hKey, inputs) == 1){
									setValue(instructions[count]->variableName[1], hKey, inputs, ANDgate(k, 1));
								}else{
									setValue(instructions[count]->variableName[1], hKey, outputs, ANDgate(k, 1));
								}
								break;
							default:
								printf("Error not a valid instruction\n");
						}
						instructions[count]->done = 1;
						isdone++;
					}
				}
				else{
					continue;
				}
			}
		}
		/*Resets all values to get ready for the next lines of instruction and inputs*/
		outPutPrint(outputs, outKeys, numOfout,extra);
		resetTempVariable(inputs, tempKeyHolder, numOftempsVars);
		resetDones(instructions, numOfInstr);
	}
}

int main(int argc, char *argv[]){
	FILE * fp;
	int extra = 0;
	switch(argc){	//Checks the argument
		case 1:	//No arguments are placed -> read from stdin
			fp = stdin;
			break;
		case 2:	//One arguement is placed -> "-h" or textfile
			if(strcmp(argv[1], "-h") == 0){
				extra = 1;
				fp = stdin;
			}else{
				fp = fopen(argv[1], "r");
			}
			break;
		case 3:	//Both "-h" and textfile are given
			if(strcmp(argv[1], "-h")==0){
				extra = 1;
				fp = fopen(argv[2], "r");
			}else{
				extra = 1;
				fp = fopen(argv[1], "r");
			}
			break;
	}

	int numOfInputs, numOfOutputs, numOfInstr = 0, inputForMult, inputForDecoder, i;
	struct instruction *instructions[50];
	struct hashtable *inputs = create_table(1000);	//creates a hashtable for inputs
	struct hashtable *outputs = create_table(1000); //creates a hashtable for outputs
	int *inKeys, *outKeys, tempKeyHolder[300];	//An int array that holds hash keys for each input and output -> might not need outkeys
	char var[15];  //Holds the string of "input" or "output"
	char buffer[250];
	int tempKey, numOftempsVars = 0;


    while(fscanf(fp, "%s", var) != EOF){	//Reads the file untile the end
    	struct instruction *ins;
    	if(strcmp(var, "INPUT") == 0){
    		fscanf(fp, "%d", &numOfInputs); //Scans the type of variables and the number of variables
    		inKeys = (int*) malloc(sizeof(int) * numOfInputs);
		    makePuts(fp, numOfInputs, inputs, inKeys, extra);	//Places all input variables into the hashtable
		    if(extra == 1){	//Extra credit format
		    	printf(" | ");
			}
    	}
    	else if(strcmp(var, "OUTPUT") == 0){
    		fscanf(fp, "%d", &numOfOutputs); //Scans the type of variables and the number of variables
    		outKeys = (int*) malloc(sizeof(int) * numOfOutputs);
		    makePuts(fp, numOfOutputs, outputs, outKeys, extra); //Places all output variables into the hashtable
		    if(extra == 1){	//extra credit format
		    	printf("\n");
			}
		}
		else if(strcmp(var, "MULTIPLEXER") == 0){	//Reads the inputs and outputs and selector and places them in a hashtable
			ins = create_instruction();
			ins->op = MULTIPLEXER;
			ins->done = 0;
			fscanf(fp, "%d", &inputForMult);
			ins->numOfVariables = pow(2, inputForMult) +inputForMult + 1;
			ins->n = inputForMult;	//Number of selectors
			//printf("%d\n", ins->numOfVariables);
			for(i = 0; i < ins->numOfVariables; i++){
				fscanf(fp, "%s", buffer);
				if(strcmp(buffer, ":") == 0){	//Ignores ":"
					i--;
					continue;
				}
				tempKey = hashKey(buffer, strlen(buffer));	//Hashkey
				if(isdigit(buffer[0]) != 0){ //Checks to see if the input is a var or an integer
					opInsert(buffer, tempKey, inputs);
					setValue(buffer, tempKey, inputs, buffer[0] - '0');
				}
				else if(opSearch(buffer, tempKey, outputs) == 0 && opSearch(buffer, tempKey, inputs) == 0){
					tempKeyHolder[numOftempsVars] = tempKey;
					numOftempsVars++;
					opInsert(buffer, tempKey, inputs);
				}
				strcpy(ins->variableName[i], buffer);
			}
			instructions[numOfInstr] = ins;
			numOfInstr++;
		}
		else if(strcmp(var, "DECODER") == 0){ //reads the inputs and outputs of the decoder 
			//printf("PASS\n");
			ins = create_instruction();
			ins->op = DECODER;
			ins->done = 0;	//Sets not done
			fscanf(fp, "%d", &inputForDecoder); // n
			ins->numOfVariables = pow(2, inputForDecoder) + inputForDecoder;
			ins->n = inputForDecoder;
			for(i = 0; i < ins->numOfVariables; i++){
				fscanf(fp, "%s", buffer);
				if(strcmp(buffer, ":") == 0){	//Ignores ":"
					i--;
					continue;
				}
				tempKey = hashKey(buffer, strlen(buffer));
				//printf("%d\n", tempKey);
				if(isdigit(buffer[0]) != 0){ //Checks to see if the input is a var or an integer
					opInsert(buffer, tempKey, inputs);
					setValue(buffer, tempKey, inputs, buffer[0] - '0');
				}
				else if(opSearch(buffer, tempKey, outputs) == 0 && opSearch(buffer, tempKey, inputs) == 0){
					tempKeyHolder[numOftempsVars] = tempKey;
					numOftempsVars++;
					opInsert(buffer, tempKey, inputs);
				}
				strcpy(ins->variableName[i], buffer);
			}
			instructions[numOfInstr] = ins;
			numOfInstr++;
		}
		else if(strcmp(var, "AND") == 0 || strcmp(var, "NAND") == 0 || strcmp(var, "NOR") == 0 || strcmp(var, "NOT") == 0 || strcmp(var, "OR") == 0 || strcmp(var, "PASS") == 0 || strcmp(var, "XOR") == 0){
			ins = create_instruction();
			if (var[0] == 'A') ins->op = AND;
			else if (var[0] == 'P') ins->op = PASS;
			else if (var[1] == 'A') ins->op = NAND;
			else if (var[0] == 'X') ins->op = XOR;
			else if (var[2] == 'T') ins->op = NOT;
			else if (var[0] == 'O') ins->op = OR;
			else if (var[0] == 'N' && var[2] == 'R') ins->op = NOR;
			
			if(ins->op == NOT || ins->op == PASS) ins-> numOfVariables = 2;
			else ins->numOfVariables = 3;

			for(i = 0; i < ins->numOfVariables; i++){
				fscanf(fp, "%s", buffer);
				if(strcmp(buffer, ":") == 0){	//Ignores :
					i--;
					continue;
				}
				tempKey = hashKey(buffer, strlen(buffer));
				if(isdigit(buffer[0]) != 0){ //Checks to see if the input is a var or an integer
					opInsert(buffer, tempKey, inputs);
					setValue(buffer, tempKey, inputs, buffer[0] - '0');
				}
				else if(opSearch(buffer, tempKey, outputs) == 0 && opSearch(buffer, tempKey, inputs) == 0){
					tempKeyHolder[numOftempsVars] = tempKey;
					numOftempsVars++;
					opInsert(buffer, tempKey, inputs);
				}
				strcpy(ins->variableName[i], buffer);
			}
			ins->done = 0;
			instructions[numOfInstr] = ins;
			numOfInstr++;
		}
	}
	
	//Evaluates all instruction given
	evalCirc(inputs, outputs, instructions, numOfInstr, pow(2, numOfInputs), inKeys, outKeys, numOfInputs, numOfOutputs, tempKeyHolder, numOftempsVars, extra);
	
	/*Frees memory*/
	eraseTable(inputs);
	eraseTable(outputs);
	cleanup(instructions, numOfInstr);
	free(inKeys);
	free(outKeys);
	
	return 0;
}