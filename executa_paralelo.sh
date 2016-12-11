#!/bin/bash
# Autora: Jullyana M. Vycas (jullytta)
# Ultima revisao: 02/10/2016
# Script que executa um programa paralelo em OpenMPI uma serie de vezes
# com diferentes quantidades de processos (1, 2, 4, 6 e 8) e salva o output de
# todas as execucoes em um unico arquivo (nome_do_programa_output.txt).

# Formate o output para facil transferencia para planilhas.

OPEN_MPI="mpiexec"
N_EXECUCOES=3
ENTRADA=""

# Verifica o formato do comando.
# TODO(jullytta): transformar argumentos em flags para viabilizar o uso
# em qualquer ordem.
if [ ${#} -lt 1 ] ; then
  echo "Numero invalido de argumentos."
  echo "Usagem correta: $0 executavel [numero_de_execucoes] [entrada]" >/dev/stderr
  exit 1
fi

# Le o segundo argumento opcional (numero de execucoes, default = 3)
if [ ${#} -ge 2 ] ; then
  N_EXECUCOES=${2}
fi

# Le o terceiro argumento opcional (entrada, default = nenhuma)
# TODO(jullytta): mensagem de erro caso a entrada especificada nao seja encontrada
if [ ${#} -ge 3 ] ; then
  ENTRADA="< ${3}.in"
fi

# TODO(jullytta): Verificar se o executavel existe mesmo
# OU adicionar compilacao antes da execucao.

# Esvazia o conteudo do arquivo output.txt
> ${1}_output.txt

# TODO(jullytta): Adicionar passagem de argumentos.

# Diferentes numeros de processos
for n in 1 2 4 8 16
do
  i=0
  # Adiciona a quantidade de processos a linha do output para uso posterior
  printf "${n}\t" >> ${1}_output.txt

  # N_EXECUCOES para cada quantidade de processos
  while [ ${i} -lt ${N_EXECUCOES} ]
  do
    echo "Executando: ${OPEN_MPI} -n ${n} ${1} ${ENTRADA} >> ${1}_output.txt" 
    eval ${OPEN_MPI} -n ${n} ${1} ${ENTRADA} >> ${1}_output.txt
    echo "Concluido."
    let i=i+1
  done

  # Pula linha entre quantidades diferentes de processos
  printf "\n" >> ${1}_output.txt
done
