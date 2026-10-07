

typedef struct{
	int (*split_string_n)(int max,char *str, char **result, const char *delimiters) ;
	

}stropt_t;

extern stropt_t* stropt;
