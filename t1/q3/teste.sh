#!/bin/bash
# Autora: Jullyana M. Vycas (jullytta)
# Ultima revisao: 03/10/2016
# Simples script que compila dois programas, gera uma nova entrada
# de tamanho $1 e compara os resultados.
# TODO(jullytta): organizar essas mensagens de feedback de um jeito
# mais bonito e menos manual. Talvez adicionar um override para que
# as mensagens vao mudando conforme os passos sao concluidos.

if [ ${#} -ne 1 ] ; then
  echo "Usagem: $0 N" >/dev/stderr
  exit 1
fi

echo -e "Compilando o programa original..."
echo -n "mpicc least_squares.c -o least_squares"
mpicc least_squares.c -o least_squares
echo "Concluido"

echo "Compilando a nossa solucao..."
echo "mpicc min_quad.c -o min_quad"
mpicc min_quad.c -o min_quad
echo "Concluido"

echo "Compilando o produtor de entrada..."
echo "g++ produz_entrada.cc -o produz_entrada"
g++ produz_entrada.cc -o produz_entrada
echo "Concluido"

echo "Produzindo entrada de tamanho N = $1..."
echo "Note que esse processo pode demorar alguns minutos dependendo de N"
echo "./produz_entrada -n $1"
./produz_entrada -n $1
echo "Concluido"

echo "Gerando saida da solucao original..."
echo "mpiexec -n 4 least_squares > least_squares.out"
mpiexec -n 4 least_squares > least_squares.out
echo "Concluido"

echo "Gerando saida da nossa solucao..."
echo "mpiexec -n 4 min_quad > min_quad.out"
mpiexec -n 4 min_quad > min_quad.out
echo "Concluido"

echo "Verificando a igualdade entre ambas..."
echo "diff least_squares.out min_quad.out"
diff least_squares.out min_quad.out >/dev/null 2>&1

result=${?}
if [ ${result} -eq 1 ] ; then
  echo "$(tput setaf 1)Fracasso$(tput sgr 0) :("
else
  echo "$(tput setaf 2)Sucesso$(tput sgr 0) :)"
fi
