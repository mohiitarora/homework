MPICC  = mpicc

TARGETS = par_ge_basic par_ge_pipelined gen_non_sing_matrix

all : $(TARGETS)
	@echo Make Complete

par_ge_basic : par_ge_basic.c $(SPECDEP)
	$(MPICC) -lm par_ge_basic.c -o par_ge_basic

par_ge_pipelined : par_ge_pipelined.c $(SPECDEP)
	$(MPICC) -lm par_ge_pipelined.c -o par_ge_pipelined

gen_non_sing_matrix : gen_non_sing_matrix.c $(SPECDEP)
	gcc -lm gen_non_sing_matrix.c -o gen_non_sing_matrix
