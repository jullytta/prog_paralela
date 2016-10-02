#!/bin/bash
# Autora: Jullyana M. Vycas (jullytta)
# Ultima revisao: 02/10/2016
# Esse script testa um programa em MPI passado como argumento uma serie
# de vezes, com diferentes numeros de processos.
# A lista de testes deve ser passada como segundo argumento.
# Testes devem estar no formato: entrada teste.in e saida teste.out.

INPUT_PATH=`dirname $2`

if [ ${#} -ne 2 ] ; then
  echo "Usagem: $0 programa_MPI lista_de_testes" >/dev/stderr
  exit 1
fi

for test in `cat ${2}`; do
  # Testamos o programa com a mesma entrada, porem numero de processos
  # diferentes, para garantir que a resposta e' a mesma independente
  # do numero de processos escolhido.
  for n in 1 2 4 6 8 ; do
    tmpout=`mktemp /tmp/${test}outXXXXXX`

    echo "Executando: mpiexec ${1} -n ${n} < ${INPUT_PATH}/${test}.in"
    eval mpiexec ${1} -n ${n} < ${INPUT_PATH}/${test}.in > ${tmpout} 2>/dev/null
    diff ${INPUT_PATH}/${test}.out ${tmpout} >/dev/null 2>&1
    result=${?}
    if [ ${result} -eq 2 ] ; then
      echo "Erro: arquivo de entrada/saida nao encontrado ou invalido." >/dev/stderr
      exit 1
    fi
    if [ ${result} -eq 1 ] ; then
      echo -e "$(tput setaf 1)Fracasso$(tput sgr 0): ${test}\nEntrada:\n`cat ${test}.in`\nEsperado:\n`cat ${test}.out`\nSaida:\n`cat ${tmpout}`\n"
    else
      echo "$(tput setaf 2)Sucesso!$(tput sgr 0)"
    fi
    rm ${tmpout}
  done
done
