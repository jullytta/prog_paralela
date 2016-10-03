// Gera entradas para o problema de minimos quadrados.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>

#define DEFAULT_N 10
#define MAX_RAND_X 1000
#define MAX_RAND_M 100
#define MAX_RAND_B 1000

using namespace std;

void leArgumentos(int argc, char *argv[], int &n, double &m, double &b){
  n = DEFAULT_N;
  m = numeric_limits<double>::max();
  b = numeric_limits<double>::max();
 
  int i = 1;
  while(i < argc){
    stringstream ss(argv[i]);
    string token;
    ss >> token;
      
    if(token == "-n"){
      // Ler o proximo argumento
      if(++i == argc){
        cout << "Erro: flag -n precisa ser seguida de um numero inteiro n" << endl;
        break;
      }
      ss.clear();
      ss.str(argv[i]);

      // Se nao temos um numero inteiro, erro
      if(!(ss >> n)){
        cout << "Erro: flag -n precisa ser seguida de um numero inteiro n" << endl;
        break;
      }
    }

    if(token == "-m"){
      // Ler o proximo argumento
      if(++i == argc){
        cout << "Erro: flag -m precisa ser seguida por um numero m" << endl;
        break;
      }
      ss.clear();
      ss.str(argv[i]);

      // Se nao temos um double, erro
      if(!(ss >> m)){
        cout << "Erro: flag -m precisa ser seguida de um numero m" << endl;
        break;
      }
    }

    if(token == "-b"){
      // Ler o proximo argumento
      if(++i == argc){
        cout << "Erro: flag -b precisa ser seguida por um numero b" << endl;
        break;
      }
      ss.clear();
      ss.str(argv[i]);

      // Se nao temos um double, erro
      if(!(ss >> b)){
        cout << "Erro: flag -b precisa ser seguida de um numero b" << endl;
        break;
      }
    }

    i++;
  }
}

void geraParametros(double &m, double &b){

	if(m == numeric_limits<double>::max()){
		m = rand()%MAX_RAND_M;
	}
	
	if(b == numeric_limits<double>::max()){
		b = rand()%MAX_RAND_B;
	}

}

int main (int argc, char *argv[]) {

	int n;
  double m, b;

  // Prepara para a criacao de numeros aleatorios
  srand(time(NULL));

  // Le argumentos recebidos
  leArgumentos(argc, argv, n, m, b);

  // Caso m ou b nao tenham sido especificados, eles se sao
  // gerados aleatoriamente
  geraParametros(m, b);

  // Abre o arquivo para o qual escreveremos os dados
	ofstream ofs;
  ofs.open("xydata");

	// Primeira coisa a ser impressa no arquivo: o valor de n
  ofs << n << endl;

  #ifdef DEBUG_FLAG
  cout << "n = " << n << "\tm = " << m << "\tb = " << b << endl;
  #endif

	// Seguido de n linhas de pares x, y que facam
  // parte da equacao y = xm + b
	for(int i = 0; i < n; i++){
    // Gera x aleatoriamente
    double x = rand()%MAX_RAND_X;

    // Encontra o y correto
		double y = x*m + b;

		#ifdef DEBUG_FLAG
    cout << "x = " << x << "\ty = " << y << endl;
    #endif

		// Adiciona um erro aleatorio 'a coordenada y
		// para justicar a utilizacao de minimos quadrados
		// para encontrar a equacao
		y += rand()/(double)RAND_MAX;

    // Adiciona o par gerado ao arquivo de saida
    ofs << x << " " << y << endl;
  }

	ofs.close();

  return 0;

}
