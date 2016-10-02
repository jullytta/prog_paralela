// Gera entradas para o problema de ordenacao de n numeros distintos.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>

#define MAX_N 1000

using namespace std;

void geraEntrada(int tamanho){
  vector<int> v;
  int i = 0;
  while(i < tamanho){
    v.push_back(i++);
  }

  int n = tamanho;
  while(n > 0){
    int pos = rand()%n;
    cout << v[pos] << endl;
    v[pos] = v[n-1];
    n--;
  }

}

void geraSaida(vector<double> &v){

  sort(v.begin(), v.end());

  int i = 0;
  while(i < v.size()){
    cout << v[i++] << endl;
  }

}

int main (int argc, char *argv[]) {

  srand(time(NULL));

  // Se nenhuma flag foi especificada, voce esta usando esse programa errado
  if(argc == 1){
    cout << "Erro: nenhuma flag especificada." << endl
         << "Flags: " << endl
         << "--entrada n \t\t\t// para gerar entrada de tamanho n" << endl
         << "--saida arquivo_de_entrada \t// para gerar saida para a entrada especificada" << endl;

  }
  else {
    // Comeco a iteracao por 1, ja que o primeiro argumento e' o
    // nome do executavel (desinteressante)
    int i = 1;
    while(42){
      stringstream ss(argv[i]);
      string token;
      ss >> token;
      
      if(token == "-entrada" || token == "--entrada"){
        // Ler o proximo argumento
        if(++i == argc){
          cout << "Erro: flag --entrada precisa ser seguida de um numero inteiro n" << endl;
          break;
        }
        ss.clear();
        ss.str(argv[i]);

        // Se e' um numero inteiro, prossegue
        int n;
        if(ss >> n){
          geraEntrada(n);
        }
        // Caso contrario, erro
        else {
          cout << "Erro: flag --entrada precisa ser seguida de um numero inteiro n" << endl;
          break;
        }

      }

      if(token == "--saida" || token == "-saida"){
        // Ler o proximo argumento
        if(++i == argc){
          cout << "Erro: flag --saida precisa ser seguida por um arquivo de entrada" << endl;
          break;
        }
        ss.clear();
        ss.str(argv[i]);

        // Tenta abrir o arquivo
        ifstream entrada(ss.str().c_str());
        if(entrada.is_open()){
          // Le o vetor do arquivo
          double temp;
          vector<double> v;
          while(entrada >> temp){
            v.push_back(temp);
          }

          // Gera a saida esperada para o dado vetor
          geraSaida(v);
        }
        else {
          cout << "Erro: flag --saida precisa ser seguida por um arquivo de entrada existente" << endl;
        }
      }

      if(++i == argc) break;
    }
  }

  return 0;

}