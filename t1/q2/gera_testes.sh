echo "Compilando gerador..."
echo "g++ gerador_testes.cc -o gerador_testes"
g++ gerador_testes.cc -o gerador_testes
echo "Compilado."
echo "Gerando teste com N = 10e1"
./gerador_testes -entrada 10 > testes/10e1.in
./gerador_testes -saida testes/10e1.in > testes/10e1.out
echo "Concluido."
echo "Gerando teste com N = 10e2"
./gerador_testes -entrada 100 > testes/10e2.in
./gerador_testes -saida testes/10e2.in > testes/10e2.out
echo "Concluido."
echo "Gerando teste com N = 10e3"
./gerador_testes -entrada 1000 > testes/10e3.in
./gerador_testes -saida testes/10e3.in > testes/10e3.out
echo "Concluido."
echo "Gerando teste com N = 10e4"
./gerador_testes -entrada 10000 > testes/10e4.in
./gerador_testes -saida testes/10e4.in > testes/10e4.out
echo "Concluido."
echo "Gerando teste com N = 10e5"
./gerador_testes -entrada 100000 > testes/10e5.in
./gerador_testes -saida testes/10e5.in > testes/10e5.out
echo "Concluido."
echo "Gerando teste com N = 10e6"
./gerador_testes -entrada 1000000 > testes/10e6.in
./gerador_testes -saida testes/10e6.in > testes/10e6.out
echo "Concluido."
echo "Gerando teste com N = 10e7"
./gerador_testes -entrada 10000000 > testes/10e7.in
./gerador_testes -saida testes/10e7.in > testes/10e7.out
echo "Concluido."
echo "Gerando teste com N = 10e8"
./gerador_testes -entrada 100000000 > testes/10e8.in
./gerador_testes -saida testes/10e8.in > testes/10e8.out
echo "Concluido."
