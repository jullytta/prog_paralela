MPICC  = mpicc
EXEC = rank_sort

all: ${EXEC}

${EXEC}: ${EXEC}.c
	${MPICC} ${EXEC}.c -lm -o ${EXEC}

clean:
	rm ${EXEC}
