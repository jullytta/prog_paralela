// Gera entradas para o problema de ordenacao de n numeros distintos.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

// Valores pequenos na matriz para garantir que nao vai ter overflow
// na multiplicacao de matrizes
#define MAX_ELEMENT 100

using namespace std;

void geraEntrada(int n){
  // Primeira linha tem a ordem das matrizes
  cout << n << endl << endl;

  // Gera duas matrizes: A e B
  for(int k = 0; k < 2; k++){
    for(int i = 0; i < n; i++){
      for(int j = 0; j < n; j++){
        cout << rand()%MAX_ELEMENT << "\t";
      }
      cout << endl;
    } 
    cout << endl;
  }

}

int main (int argc, char *argv[]) {

  srand(time(NULL));

  // Se nenhuma flag foi especificada, voce esta usando esse programa errado
  if(argc == 1){
    cout << "Erro: nenhuma flag especificada." << endl
         << "Flags: " << endl
         << "-n <ordem> \t\t\t// para gerar entrada de ordem n" << endl;
  }
  else {
    // Comeco a iteracao por 1, ja que o primeiro argumento e' o
    // nome do executavel (desinteressante)
    int i = 1;
    while(42){
      stringstream ss(argv[i]);
      string token;
      ss >> token;
      
      if(token == "-n" || token == "--n"){
        // Ler o proximo argumento
        if(++i == argc){
          cout << "Erro: flag -n precisa ser seguida de um numero inteiro" << endl;
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
          cout << "Erro: flag -n precisa ser seguida de um numero inteiro" << endl;
          break;
        }

      }

      if(++i == argc) break;
    }
  }

  return 0;

}