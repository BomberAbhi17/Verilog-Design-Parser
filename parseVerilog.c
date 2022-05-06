
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
 
 

#define TOKENSIZE 999	 
#define LINESIZE 9999    
#define BUFSIZE 99999	 
#define SIZE 9999
#define INPUT	0
#define AND	1
#define NAND	2
#define OR	3
#define NOR	4
#define XOR	5
#define XNOR	6
#define BUF	7
#define NOT	8
#define INV	8
#define I	9
#define	RESERVEDNUM 107
#define	NO_OUT 0

typedef enum { false, true } bool;
typedef char * string;

struct wire_    {
    int id;		/*Wire ID number*/
    char *name;	/*Name of this wire*/
    char *type; 	/*Type of gate driving this wire*/
    int inputcount;		/*Number of wire inputs*/
    int inputs[LINESIZE];	/*Array of inputs*/
    int outputcount;
    int outputs[LINESIZE];	/*Array of outputs.*/
    bool primary;		/*Primary input flag*/
};
typedef struct wire_ *wire;

struct node_ {
    char type[99];	/*input, output, wire, regs*/
    char name[99];	/*node name*/
    int id;	/*node number*/
};
typedef struct node_ *node;

struct circuit_  {
    wire *wires;					/*Array of all wires */
    node *nodes;					/*Array of nodes*/
    char *name;					/*Name of the circuit. */
    int inputcount, outputcount;	   	/*Count of primary inputs and primary outputs. */
    int gatecount, wirecount, nodecount;				/*Number of wires, (gates)*/
    char *inputs[LINESIZE], *outputs[LINESIZE];	/*List of inputs and outputs in the netlist*/
    int size, id;		        		/*Circuit size and identifier*/
};
typedef struct circuit_ *circuit;

struct module_  {
    char *name;					/*Name of the module*/
    int inputcount, outputcount;	   		/*Count of primary inputs and primary outputs. */
    int wirecount, regcount, gatecount;	        /*Count of wires ,regs, gates*/
    char *inputs [LINESIZE], *outputs [LINESIZE];	/*List of inputs and outputs in the netlist*/
    char *wires [LINESIZE], *regs [LINESIZE]; 		/*List of wires, regs, gates in the netlist*/
    char *gates [LINESIZE];
    int id;
};
typedef struct module_ *module;

#define	GATESNUM 18
char* gate_name[] = {
    "and",
    "nand",
    "or",
    "nor",
    "xor",
    "xnor",
    "buf",
    "bufif0",
    "bufif1",
    "cmos",
    "nmos",
    "nor",
    "not",
    "fo",
    "fo3",
    "fo4",
    "fi",
    "inv"
};

char* reserved_word[] = {	/*Reserved Verilog keywords*/
    "always",
    "and",
    "assign",
    "begin",
    "buf",
    "bufif0",
    "bufif1",
    "case",
    "casex",
    "casez",
    "cmos",
    "deassign",
    "default",
    "defparam",
    "disable",
    "edge",
    "else",
    "end",
    "endcase",
    "endfunction",
    "endmodule",
    "endprimitive",
    "endspecify",
    "endtable",
    "endtask",
    "event",
    "for",
    "for",
    "force",
    "forever",
    "function",
    "highz0",
    "highz1",
    "if",
    "ifnone",
    "initial",
    "inout",
    "input",
    "integer",
    "join",
    "large",
    "macromodule",
    "medium",
    "module",
    "nand",
    "negedge",
    "nmos",
    "nor",
    "not",
    "notif0",
    "notif1",
    "or",
    "output",
    "parameter",
    "pmos",
    "posedge",
    "primitive",
    "pull0",
    "pull1",
    "pulldown",
    "pullup",
    "rcmos",
    "real",
    "realtime",
    "reg",
    "release",
    "repeat",
    "rnmos",
    "rpmos",
    "rtran",
    "rtranif0",
    "rtranif1",
    "scalared",
    "small",
    "specify",
    "specparam",
    "strong0",
    "strong1",
    "supply0",
    "supply1",
    "table",
    "task",
    "time",
    "tran",
    "tranif0",
    "tranif1",
    "tri",
    "tri0",
    "tri1",
    "triand",
    "trior",
    "trireg",
    "vectored",
    "wait",
    "wand",
    "weak0",
    "weak1",
    "while",
    "wire",
    "wor",
    "xnor",
    "xor",
    "fo",
    "fo3",
    "fo4",
    "fi",
    "inv"
};

#define NUM_PUNCTUATIONS 4
char* punctuations[] = {
    ";",
    ".",
    " ",
    "\n",
};


 


char assignments[1000][1000];
int track=0;
 

 
bool reserved (char* word)
{
    int i;
    for (i = 0; i < RESERVEDNUM; i++)
        if (strcmp(word, reserved_word[i])==0 || strstr(word, "endmodule")!= NULL)
            return true;
    return false;
}

 

bool gate (char* word)
{
    int i;
    for (i = 0; i < GATESNUM; i++)
        if (strcmp(word, gate_name[i])==0)
            return true;
    return false;
}

 

bool isFinalOutput (wire w, circuit c)
{
    int i;
    for(i = 0; i < c->outputcount; i++)
        if (strcmp(w->name, c->outputs[i])==0)
            return true;
    return false;
}

 

char* trim (char* source)
{
    int i=0, index=0;
    int sr_length = strlen(source);
    for(i=0; i<sr_length; i++)
    {
        if((source[i]=='x' && i!=0)) {
            source[index] = '\0';
            break;
        }
        else
            source[index++] = source[i];
    }
    source[index] = '\0';
    return source;
}

/*
  Determines if a string is a vector of signals
   the string to check
   whether the string is a vector of signals or not
 */
bool signal_vector (char* word)
{
    int i;
    for (i = 0; i < strlen(word); i++)
        if (word[i] == ':')
            return true;
    return false;
}

 
bool end_of_module (char* word)
{
    if (strstr(word, "endmodule")!= NULL)
        return true;
    return false;
}

 
bool end_of_line(char* source)
{
    char * pchar = strchr(source, ';');
    return (pchar == NULL) ? false : true;
}

 
int convert (char* gate)
{
    if (strcasecmp(gate, "INPUT")==0)
        return 0;
    else if (strcasecmp(gate, "AND")==0)
        return 1;
    else if (strcasecmp(gate, "NAND")==0)
        return 2;
    else if (strcasecmp(gate, "OR")==0)
        return 3;
    else if (strcasecmp(gate, "NOR")==0)
        return 4;
    else if (strcasecmp(gate, "XOR")==0)
        return 5;
    else if (strcasecmp(gate, "XNOR")==0)
        return 6;
    else if (strcasecmp(gate, "BUF")==0)
        return 7;
    else if (strcasecmp(gate, "NOT")==0 || strcasecmp(gate, "INV")==0)
        return 8;
    else if (strcasecmp(gate, "I")==0)
        return 9;
    else
        return 10;
}

 
void print_module_summary (module m)
{
    int i;
    printf("\n ---------------MODULE %s  DESIGN OVERVIEW ---------------\n", m->name);
    printf("Total inputs: %d\n", m->inputcount);
    for(i = 0; i < m->inputcount; i++)
        printf("%s ", m->inputs[i]);

    printf("\n\nTotal outputs: %d\n", m->outputcount);
    for(i = 0; i < m->outputcount; i++)
        printf("%s ", m->outputs[i]);

     printf("\n\nTotal wires: %d\n", m->wirecount);
    for(i = 0; i < m->wirecount; i++)
        printf("%s ", m->wires[i]);

    printf("\n\n Total gates: %d\n", m->gatecount);
    for(i = 0; i < m->gatecount; i++)
        printf("%s ", m->gates[i]);

   

    printf("\n\nTotal registers: %d\n", m->regcount);
    for(i = 0; i < m->regcount; i++)
        printf("%s ", m->regs[i]);
    
}
 
void print_circuit_summary (circuit c)
{
    int i,j,row,col;
    printf("\n--------------- CIRCUIT %s  DESIGN OVERVIEW ---------------\n", c->name);
    printf(" Overall Circuit size: %d\n", c->size);
    printf("Total primary inputs: %d\n", c->inputcount);
    for(i = 0; i < c->inputcount; i++)
        printf("%s ", c->inputs[i]);

    printf("\n\nTotal outputs: %d\n", c->outputcount);
    for(i = 0; i < c->outputcount; i++)
        printf("%s ", c->outputs[i]);

    printf("\n\nTotal gates: %d\n", c->gatecount);

    printf("\n\n");
    i=0;


    printf("This is how Input is converted to Output through Datapath (Combination of Gates)\n");
    

    while (i<c->wirecount && c->wires[i] != NULL) {
        printf ("c->wire[%d]->type: %s, ",i, c->wires[i]->type);
        printf ("ID: %d,  ", c->wires[i]->id);
        printf ("name: %s, ", c->wires[i]->name);

        printf ("\nInputs (%d): ", c->wires[i]->inputcount);/*Wire inputs*/
        for(j=0; j<c->wires[i]->inputcount; j++)
            printf ("%d ",c->wires[i]->inputs[j]);

        printf ("\nOutputs (%d): ", c->wires[i]->outputcount);/*Wire outputs*/
        for(j=0; j<c->wires[i]->outputcount; j++)
            printf ("%d ",c->wires[i]->outputs[j]);

        i++;
        printf ("\n");
    }
    printf("Following connections of wires are also Performed using Assign Keyword \n");
    i=0;
    while(i<track){
        printf("%s\n" , assignments[i]);
        i++;
    }
    
}

 
int getID (char* name, circuit c)
{
    int i;
    for(i=0; i<c->size; i++) {
        if (strcmp(name, c->nodes[i]->name)==0) { 
            return c->nodes[i]->id;
        }
    }
    return 0;
}

 
wire getWire (int id, circuit c)
{
    int i=0;
    while (i<c->wirecount && c->wires[i] != NULL) {
        if (c->wires[i]->id == id) 
            return c->wires[i];
        i++;
    }
    return 0;
}

 
wire getWireByName (char* name, circuit c)
{
    int i=0;
    while (c->wires[i] != NULL) {
        if (strcmp(name, c->wires[i]->name)==0)  
            return c->wires[i];
        i++;
    }
    return 0;
}


 
void setNode ( node n, char* type, char* name, int id)
{
    strcpy(n->type, type);
    strcpy(n->name, name);
    n->id = id;			/*Store node id*/
}

 
void build_wire (circuit c, wire w, char* type, char* name)
{
    int i;
    w->id = getID (name, c);   
    w->type = strdup(type);	 
    w->name = strdup(name);		 
    w->inputcount = 0;		 
    w->outputcount = 0;			 

    for(i = 0; i < c->inputcount; i++) { /*Circuit primary inputs*/
        if (strcmp(w->name, c->inputs[i])==0) {
            w->primary = true;
        }
    }

    printf(" wire %d : %s \n", w->id, w->name);
}

 
bool defined (circuit c, char* name)
{
    int i=0;
    while (c->wires[i] != NULL) {
        if (strcmp(c->wires[i]->name, name)==0)
            return true;
        i++;
    }
    return false;
}


 



void build_module_circuit (FILE *verilog, module m, circuit c)
{   

    printf("\n --------------- LINE BY LINE ANALYSIS ---------------\n");
    int i=0, j=0, in=0, out=0, id=0, index=0;		/*Indexes*/
    char linebuf[LINESIZE], tmpbuf[LINESIZE], buffer[BUFSIZE];   
    char *token[TOKENSIZE]; 				 

    memset(linebuf, 0, sizeof(char) * LINESIZE);
    memset(tmpbuf, 0, sizeof(char) * LINESIZE);
    memset(buffer, 0, sizeof(char) * BUFSIZE);
    memset(token, 0, sizeof(char*) * TOKENSIZE);

    /*Initialization of the circuit*/
    c->inputcount = m->inputcount;			 
    c->outputcount = m->outputcount;		 
    c->wirecount = m->wirecount;			 
    c->gatecount = m->gatecount;			 
    c->size = c->inputcount+c->outputcount+c->wirecount+c->gatecount; 
    c->wires = (wire *)calloc((c->wirecount) * 10,sizeof(wire));	 
    c->nodes = (node *)calloc((c->size) * 5,sizeof(node));	 

    for (i=0; i < c->inputcount; i++) {			 
        c->inputs[i] = calloc (strlen(m->inputs [i]) + 1, sizeof (char));
        strcpy (c->inputs[i], m->inputs [i]);
        c->nodes[id] = calloc (1,sizeof(struct node_));
        setNode (c->nodes[id], "input", m->inputs [i], id);  
        id++;
    }
    for (i=0; i < c->outputcount; i++) {		 
        c->outputs[i] = calloc (strlen(m->outputs [i]) + 1, sizeof(char));
        strcpy (c->outputs[i], m->outputs [i]);
        c->nodes[id] = calloc (1,sizeof(struct node_));
        setNode (c->nodes[id], "output", m->outputs [i], id); 
        id++;
    }
    for (i=0; i < c->wirecount; i++) {				 
        c->nodes[id] = calloc (1,sizeof(struct node_));
        setNode (c->nodes[id], "wire", m->wires [i], id);  
        id++;
    }
    for (i=0; i < c->gatecount; i++) {				 
        c->nodes[id] = calloc (1,sizeof(struct node_));
        setNode (c->nodes[id], "gate", m->gates [i], id); 
        id++;
    }

    c->nodecount = id;

    while (fgets(linebuf,LINESIZE-1,verilog) != NULL) {	 
        char *keyword = NULL;					 

        i=0;
        in=0;
        memset (buffer,0,sizeof(char) * BUFSIZE);				/*Clear the buffer*/
        strcpy (tmpbuf,linebuf);
        token[0] = strtok(tmpbuf, " [()],;"); 		/*Get 1st keyword from the line*/
        keyword = trim(token[0]);

        if (!reserved (keyword)) continue;		/*Skip any comment lines, empty lines or spaces*/
        if (end_of_module (linebuf)) break;		/*If end of module is reached then break*/
        if (!gate (keyword) && strcmp(keyword, "assign")!=0  ) continue;		 
        strcat (buffer,linebuf);

        while (!end_of_line(linebuf)) {			 
            if (fgets(linebuf,LINESIZE,verilog) != NULL)	 
                strcat (buffer,linebuf);
        }

        token[0] = strtok(buffer, " [()],;"); 	 
        while(token[i]!= NULL) {
            i++;
            token[i] = strtok(NULL, " [()],;\r\n");
        }
        
        if ( strcmp(keyword, "assign")==0){
           
           i=1;
          printf("Performing wire assignments as : ");
          
           while(token[i]!=NULL){

               strcat(assignments[track], " ");
               strcat(assignments[track], token[i]);
               
                i++;
           }
            printf("%s\n" , assignments[track]);
        //   printf("\n");
           track++;
           memset(linebuf, 0, sizeof(char) * LINESIZE);
           memset(tmpbuf, 0, sizeof(char) * LINESIZE);
           memset(buffer, 0, sizeof(char) * BUFSIZE);
           memset(token, 0, sizeof(char*) * TOKENSIZE);
         
            continue;
        }

        /*A. Create wires for all the gate inputs*/
        printf(" %s = ",token[1]);
        if (strcmp(keyword, "not")==0 || strcmp(keyword, "inv")==0  ){
            printf("not %s",token[2]);
        }
        else{
            printf("%s ( ",keyword);
            for(j=2;j<i;j++){
                printf("%s, ",token[j]);
            }
            printf(")\n");
        }

        for(j = 2; j < i-1; j++) {
            if (!defined (c,token[j])) { /*If wire is not already defined*/
                c->wires[index] = (wire)calloc(1,sizeof(struct wire_));
                build_wire (c, c->wires[index], "I", token[j]);
                c->wires[index]->outputs[0] = getID (token[1], c); 
                c->wires[index]->outputcount = 1;
                index++;
            }
        }

         
        c->wires[index] = (wire)calloc(1,sizeof(struct wire_));
        build_wire (c, c->wires[index], keyword, token[1]);
       
        in = 0;
        for(j = 2; j < i-1; j++) {
            c->wires[index]->inputs[in] = getID (token[j], c);
            c->wires[index]->inputcount++;
            in++;
        }
       
        c->wires[index]->outputs[0] = getID (token[i-1], c);
        c->wires[index]->outputcount = 1;
        index++;

         
        if (!defined (c,token[i-1])) { /*If wire is not already defined*/
            c->wires[index] = (wire)calloc(1,sizeof(struct wire_));
            build_wire (c, c->wires[index], "I", token[i-1]);
            c->wires[index]->inputs[0] = getID (token[1], c);/*1/29/15 assign in to I wires*/
            c->wires[index]->inputcount = 1;
            index++;
        }

        else { /*If wire is already defined*/
            getWireByName(token[i-1],c)->inputs[0] = getID (token[1], c); 
            getWireByName(token[i-1],c)->inputcount = 1;
        }

        memset(linebuf, 0, sizeof(char) * LINESIZE);
        memset(tmpbuf, 0, sizeof(char) * LINESIZE);
        memset(buffer, 0, sizeof(char) * BUFSIZE);
        memset(token, 0, sizeof(char*) * TOKENSIZE);

    } 
    c->wirecount = index;

} 



 
void parse_signal_vector (char *signal_arr[], char *token[], int *index, int *count)
{
    int v,v1,v2;
    char *sig_vector; 					 
    sig_vector = strtok (token[*index],":");	 
    v1 = atoi(sig_vector);  			 
    sig_vector = strtok (NULL, ":");
    v2 = atoi(sig_vector);				 
    (*index)++;					 
    for(v = v2; v <= v1; v++) {				 
        int wordsize = strlen(token[*index]);		 
        signal_arr [*count] = calloc (wordsize + 1, sizeof(char)); 	 
        strcpy (signal_arr [*count], token[*index]); 	 
        (*count)++;				 	 
    }
}

 
void build_module (FILE *verilog, module m)
{
    int i=0, j=0;						/*Indexes*/
    char linebuf[LINESIZE], tmpbuf[LINESIZE], buffer[BUFSIZE]; 	/*Buffer for lines of the verilog code*/
    char *token[TOKENSIZE]; 				 
    char *keyword;						 

  
    while (fgets(linebuf,LINESIZE,verilog) != NULL) {
        i=0;
        strcpy (buffer,"");				/*Clear the buffer*/
        strcpy (tmpbuf,linebuf);
        token[0] = strtok(tmpbuf, " [()],;"); 		/*Get 1st keyword from the line*/
        keyword = trim(token[0]);
        if (!reserved (keyword)) continue;		/*skip comment lines, empty lines or spaces*/

        strcat (buffer,linebuf);
        while (!end_of_line(linebuf)) {			 
            if (end_of_module (linebuf)) break;		 
            if (fgets(linebuf,LINESIZE,verilog) != NULL)	 
                strcat (buffer,linebuf);
        }

        token[0] = strtok(buffer, " [()],;"); 	 
        while(token[i]!= NULL) {
            i++;
            token[i] = strtok(NULL, " [()],;\r\n");
        }

        if (strcmp(keyword, "module")==0) {		/*MODULES*/
            m->name = calloc (strlen(token[1]) + 1, sizeof(char)); 	 
            strcpy (m->name,token[1]);		 
        }

        else if (strcmp(keyword, "input")==0) {	/*INPUTS*/
            for(j = 1; j < i; j++) {			 
                if (signal_vector(token[j])) 		 
                    parse_signal_vector (m->inputs, token, &j, &m->inputcount);
                else {						 	 
                    m->inputs [m->inputcount] = calloc (strlen(token[j]) + 1, sizeof(char)); 	 
                    strcpy (m->inputs [m->inputcount],token[j]);	 	 
                    m->inputcount ++;				 		 
                }
            }
        }

        else if (strcmp(keyword, "output")==0) { 
            for(j = 1; j < i; j++) {			 
                if (signal_vector(token[j]))  		 
                    parse_signal_vector (m->outputs, token, &j, &m->outputcount);
                else {						 	  
                    m->outputs [m->outputcount] = calloc (strlen(token[j]) + 1, sizeof(char)); 
                    strcpy (m->outputs [m->outputcount],token[j]);	 
                    m->outputcount ++;				 	 
                }
            }
        }

        else if (strcmp(keyword, "wire")==0) {		/*WIRES*/
            for(j = 1; j < i; j++) {			 
                if (signal_vector(token[j])) 		 
                    parse_signal_vector (m->wires, token, &j, &m->wirecount);
                else {						 		 
                    m->wires [m->wirecount] = calloc (strlen(token[j]) + 1, sizeof(char)); 	 
                    strcpy (m->wires [m->wirecount],token[j]);	 		 
                    m->wirecount ++;				 		 
                }
            }
        }

        else if (strcmp(keyword, "reg")==0) {		 
            for(j = 1; j < i; j++) {			 
                if (signal_vector(token[j])) 	 
                    parse_signal_vector (m->regs, token, &j, &m->regcount);
                else {						  
                    m->regs [m->regcount] = calloc (strlen(token[j]) + 1, sizeof(char)); 	 
                    strcpy (m->regs [m->regcount],token[j]);	 	 
                    m->regcount ++;				 	 
                }
            }
        }

        else if (gate (keyword)) {			/*GATES*/
            m->gates[m->gatecount] = calloc (strlen(token[1]) + 1, sizeof(char)); 
            strcpy (m->gates [m->gatecount], token[1]);	 
            m->gatecount ++;				 
        }

        else if (end_of_module (linebuf))		 
        {
            print_module_summary(m);			 
            break;					 
        }
    }  

} 

 
void parse_verilog_file (circuit c, char *filename)
{
    FILE *verilog; 		 
    int i = 0;
    verilog = fopen(filename, "r"); 	/* Open Verilog file */
    if (!verilog) {
        fprintf(stderr,"transfer:  cannot open file \"%s\"\n",filename);
        exit(1);
    }

    module m = (module)calloc(1, sizeof(struct module_));	/*Declare an instance of a module*/
    build_module (verilog, m);		 
    rewind(verilog);				 
    build_module_circuit (verilog, m, c); 	/*Create circuit object using the module*/

    /*Free Memory*/

    for (i = 0; i < m->outputcount; i++)
        free (m->outputs[i]);

    for (i = 0; i < m->inputcount; i++)
        free (m->inputs[i]);

    for (i = 0; i < m->wirecount; i++)
        free (m->wires[i]);

    for (i = 0; i < m->gatecount; i++)
        free (m->gates[i]);

    free (m->name);
    free (m);  

    fclose(verilog);
 
}

 
int main (int argc, char *argv[])
{
    int i;
    if (argc != 2) {
        printf("Use the command: ./veri <verilog_file.v>\n");    
        exit(1);
    }
    circuit c = (circuit)calloc(1,sizeof(struct circuit_));	 

    c->name = strdup(argv[1]); 			 

    parse_verilog_file (c, c->name);	  

    print_circuit_summary (c);			 

    /*Free memory*/
    for (i=0; i < c->outputcount; i++)
        free (c->outputs[i]);

    for (i=0; i < c->inputcount; i++)
        free (c->inputs[i]);

    for (i=0; i < c->nodecount; i++)
        free (c->nodes[i]);
    free(c->nodes);

    for (i=0; i < c->wirecount; i++) {
        free (c->wires[i]->name);
        free (c->wires[i]->type);
        free (c->wires[i]);
    }
    free(c->wires);

    free (c->name);
    free (c);  

    return 0;
}
