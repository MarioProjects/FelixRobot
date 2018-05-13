%{
#include <prhlt/constants.h>
#include <prhlt/trace.h>
#include <stdlib.h>
#include <string.h>
#include <viterbi/parsers/dict-parser/dict-flex.h>
#define YYERROR_VERBOSE 1

//Global
//Hash table to name the states of the model
static vocab_t *state_vocab;
static model_t *model;
float probability=0.0;
int source=0;
char *old_word;
%}

%name-prefix="dict_"
%defines
%parse-param { lex_t * dict }


// Symbols.
%union
{
  int          ival;
  double       dval;
  char	      *sval;
}

//Tokens
%token        END 0 "end of file"
%token        ENDL  "end of line"
%token <sval> IDENTIFIER STRING
%token <ival> INT
%token <dval> FLOAT
%token        UNEXPECTED SENTSTART BRACKET_IN BRACKET_OUT SENTEND

//Type of the no-terminals
%type  <dval> probability
%type  <sval> phones phone

%destructor { free($$); } IDENTIFIER STRING

//Axiom of the grammar
%start file

%%

/* the base case is empty line; @$ declares the lloc variable that stores the cursor position */
file: {@$.first_line = 0;}
	| dicts END {
	dict_postprocess(dict);
	vocab_delete(state_vocab);
      }



dicts: dict
     | dicts dict

dict: IDENTIFIER emission probability {
  		symbol_t word = vocab_find_symbol(dict->vocab, $1);

  		if (word == VOCAB_NONE || (word == UNK_WORD && (dict->num_models == 0 || dict->models[word] == NULL))) {
  		  model = lex_model_create($1);
  		  dict_append_model(dict, model, word);
  		  if(state_vocab!=NULL) vocab_delete(state_vocab);
  		  //Create hash table to name the states
  		  state_vocab = vocab_create(541, NULL);
		  if(old_word!=NULL) free(old_word);
		  old_word=(char *)calloc(MAX_LINE, sizeof(char));
		  strcpy(old_word, model->label);
  		}
  		else{
  		  model=dict->models[word];
		  if(strcmp(model->label, old_word)!=0){
		   PRINT("WARNING: The lexicon is not tidy. Last word %s and next word %s\n", old_word, model->label);
		   exit(1);
		  }
		  
  		}

  		free($1);

		//After I put the probability with log
		probability=$3;
		} phones ENDL {
		 model->end = model->num_states-1;
		 //Unifico los estados finales del modelo
		 join_end_states(&model);
		}
     | start ENDL
     | end ENDL
     | ENDL 

start: SENTSTART emission phone_special

end: SENTEND emission phone_special

emission: /* empty */
       | BRACKET_IN IDENTIFIER BRACKET_OUT {free($2);}
       | BRACKET_IN BRACKET_OUT

//Probability is necessary
probability: IDENTIFIER {
         $$ = strtod($1,NULL);  
         if (! (errno != ERANGE)) {
          fprintf(stderr, "float is out of range");
         }
}

phones: phone {

        source=0;
        //source=model->num_states;

        char source_aux[256];
        char destination[256];

        sprintf(source_aux, "%d", source);
        if(model->num_states>0) sprintf(destination, "%d", model->num_states);
        else sprintf(destination, "%d", model->num_states+1);

	//Create source state
        int num_source = vocab_find_symbol(state_vocab, source_aux);
        if (num_source == VOCAB_NONE) {
         num_source = vocab_insert_symbol(state_vocab, source_aux, -1);
         dict_append_state(dict, model, num_source);
        }
        model->initial = num_source;
	
        //Create destination state
        int num_destination = vocab_find_symbol(state_vocab, destination);
        if (num_destination == VOCAB_NONE || num_destination<=model->num_states) {
          num_destination = vocab_insert_symbol(state_vocab, destination, -1);
          dict_append_state(dict, model, num_destination);
        }

        state_lex_t *state = model->states[source];
        dict_create_edge(state, dict, $1, num_source, num_destination, probability);

        source=model->num_states-1;
        free($1);
	}
       | phones phone {

        char source_aux[256];
        char destination[256];

        sprintf(source_aux, "%d", source);
        sprintf(destination, "%d", source+1);

        //Create source state
        int num_source = vocab_find_symbol(state_vocab, source_aux);
        if (num_source == VOCAB_NONE){
          num_source = vocab_insert_symbol(state_vocab, source_aux, -1);
          dict_append_state(dict, model, num_source);
        }

        //Create destination state
        int num_destination = vocab_find_symbol(state_vocab, destination);
        if (num_destination == VOCAB_NONE || num_destination<=model->num_states) {
          num_destination = vocab_insert_symbol(state_vocab, destination, -1);
          dict_append_state(dict, model, num_destination);
        }

        state_lex_t *state = model->states[source];
        dict_create_edge(state, dict, $2, num_source, num_destination, probability);


        source=model->num_states-1;

        free($2);
	}

phone: IDENTIFIER {$$=$1;}

phone_special: /*empty*/
	| IDENTIFIER {free($1);}

/*
number: FLOAT { $$ = $1; }
      | INT   { $$ = (double)$1; }
*/
%%

