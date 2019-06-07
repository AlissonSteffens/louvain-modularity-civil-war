#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

using MatrizAdj = vector<vector<int>>;
using Comunidades = vector<int>;
using Vertices = vector<int>;
using Names = vector<string>;

Names nomes;

/*
	Esta função divide uma string 's' utilizando o caracter 'c' como separador
	
	Args:
	s -> A String a fer dividada
	c -> O caracter de separação

	Return:
	um vetor de substrings de 's'
 */
const vector<string> splitString(const string& s, const char& c)
{
	string buff{ "" };
	vector<string> v;

	for (auto n : s)
	{
		if (n != c) buff += n; else
			if (n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if (buff != "") v.push_back(buff);

	return v;
}

/*
	Esta função Lê um arquivo no estilo .net e gera um grafo para o algoritmo
	
	Args:
	instancia -> O nome do arquivo a ser lido

	Return:
	a matriz de adjacência do grafo lido
 */
MatrizAdj obterInstancia(string instancia) {
	MatrizAdj matriz;
	cout << instancia << endl;
	ifstream arquivo(instancia);
	string linha;

	getline(arquivo, linha);
	vector<string> separado{ splitString(linha, ' ') };

	int N = stoi(separado[1]);
	nomes = Names(N);
	for (int i = 0; i < N; i++) {
		getline(arquivo, linha);
		// Aqui são lidos os labels do grafo, eles não são utilizados no algorítmo, mas são úteis na exibição das resostas
		vector<string> nodo{ splitString(linha, ' ') };
		nomes[i] = nodo[1];

		matriz.push_back(vector<int>());
		for (int j = 0; j < N; j++)
			matriz[i].push_back(0);
	}

	getline(arquivo, linha);
	while (getline(arquivo, linha)) {
		separado = { splitString(linha, ' ') };
		int i = stoi(separado[0]) - 1;
		int j = stoi(separado[1]) - 1;
		matriz[i][j] = 1;
		matriz[j][i] = 1;
	}

	return matriz;
}

/*
	Este procedimento renderiza uma matriz  no console
	
	Args:
	matriz -> A matriz a ser exibida
	
 */
void exibirMatriz(MatrizAdj matriz) {
	int N = matriz.size();
	for (int i = 0; i < N; i++) {
		printf("\n|");
		for (int j = 0; j < N; j++) {
			printf("%d|", matriz[i][j]);
		}
	}
}

// Esta estrutura é referente ao algoritmo, e representa seus atributos
struct Louvain {
	MatrizAdj matriz;
	Vertices vertices;
	Comunidades comunidades;
	double modularidade;
	
	// O construtor do algortmo recebe como parâmetro o nome do arquivo onde está descrito o grafo
	Louvain(string instancia) {
		matriz = obterInstancia(instancia);

		const int N = matriz.size();
		comunidades = Comunidades(N);
		vertices = Vertices(N);
		for (int i = 0; i < N; i++) {
			comunidades[i] = i;
			vertices[i] = i;
		}
	}

	Louvain(MatrizAdj matriz, Vertices vertices, Comunidades comunidades, double modularidade) :
		matriz(matriz), vertices(vertices), comunidades(comunidades), modularidade(modularidade) {};
};

/*
	Esta função obtem os vertices de uma comunidade
	
	Args:
	louvain -> A instancia do algoritmo
	comunidade -> O id referenta à comunidade

	Return:
	uma lista com todos os vertices da 'comunidade'
 */
Vertices obterVerticesDaComunidade(Louvain louvain, int comunidade) {
	Vertices vertices = Vertices();
	for (int i = 0; i < louvain.vertices.size(); i++)
		if (louvain.vertices[i] == comunidade)
			vertices.push_back(i);
	return vertices;
}

/*
	Esta função obtem as comunidades vizinhas à  uma 'comunidade'
	
	Args:
	louvain -> A instancia do algoritmo
	comunidade -> O id referenta à comunidade

	Return:
	Uma lista com as comunidades vizinhas à 'comunidade'
 */
Comunidades obterComunidadesVizinhas(Louvain louvain, int comunidade) {	
	Vertices vertices = obterVerticesDaComunidade(louvain, comunidade);

	Comunidades comunidades = Comunidades();
	for(int v : vertices) {
		for (int j = 0; j < louvain.vertices.size(); j++) {
			int cv = louvain.vertices[v];
			int cj = louvain.vertices[j];
			if (cv != cj && louvain.matriz[v][j] == 1)
				comunidades.push_back(cj);
		}
	}

	sort(comunidades.begin(), comunidades.end());
	comunidades.erase(unique(comunidades.begin(), comunidades.end()), comunidades.end());

	return comunidades;
}

/*
	Esta função obtem o id de uma comunidade
	
	Args:
	louvain -> A instancia do algoritmo
	comunidade -> O id referenta à comunidade

	Return:
	o id da 'comunidade'
 */
int obterIndiceComunidade(Louvain louvain, int comunidade) {	
	for (int c = 0; c < louvain.comunidades.size(); c++)
		if (louvain.comunidades[c] == comunidade)
			return c;
	return -1;
}

/*
	Esta função obtem o grau de um vertice
	
	Args:
	louvain -> A instancia do algoritmo
	vertice -> O id do vertice

	Return:
	O grau do vertice 'id'
 */
int obterGrau(Louvain louvain, int vertice) {
	int grau = 0;
	for (int i = 0; i < louvain.vertices.size(); i++)
		grau += louvain.matriz[vertice][i];
	return grau;
}

/*
	Esta função obtem a quantidade total de arestas em um grafo
	
	Args:
	matriz -> a matriz de adjacência de um grafo

	Return:
	O número de arestas no grafo
 */
int obterQuantidadeTotalArestas(MatrizAdj matriz) {
	int total = 0;
	for (int i = 0; i < matriz.size(); i++)
		for (int j = 0; j < matriz.size(); j++)
			total += matriz[i][j];
	return total;
}

/*
	Esta função obtem a modularidadeQ de do grafo com suas atuais modularidades
	Esta é a função objetivo do algoritmo
	
	Args:
	louvain -> A instancia do algoritmo
	
	Return:
	Um valor que indica a modularidade
 */
double obterModularidadeQ(Louvain louvain) {
	double Q = 0.0;
	double E = obterQuantidadeTotalArestas(louvain.matriz);
	for (int comunidade : louvain.comunidades) {
		Vertices vertices = obterVerticesDaComunidade(louvain, comunidade);
		for (int i : vertices) {
			double di = obterGrau(louvain, i);
			for (int j : vertices) {
				if (i != j) {
					double a = louvain.matriz[i][j];
					double dj = obterGrau(louvain, j);
					Q += a - (di * dj) / E;
				}
			}
		}
	}
	return (1 / E) * Q;
}

/*
	Esta função tenta unir dua comunidades e ver se isto aumenta sua modulariade
	
	Args:
	louvain -> A instancia do algoritmo
	comunidadeOrigem -> id da comunidade base
	comunidadeDestino -> id da comunidade a ser unificada
	
	Return:
	Uma nova instância do algoritmo
 */
Louvain migrarVertices(Louvain louvain, int comunidadeOrigem, int comunidadeDestino) {
	for (int v = 0; v < louvain.vertices.size(); v++)
		if (louvain.vertices[v] == comunidadeOrigem)
			louvain.vertices[v] = comunidadeDestino;

	louvain.modularidade = obterModularidadeQ(louvain);
	
	int verticesNaAntigaComunidade = obterVerticesDaComunidade(louvain, comunidadeOrigem).size();

	int indiceComunidadeOrigem = obterIndiceComunidade(louvain, comunidadeOrigem);
	
	if (indiceComunidadeOrigem == -1 && verticesNaAntigaComunidade == 0)
		louvain.comunidades.erase(louvain.comunidades.begin(), louvain.comunidades.begin() + indiceComunidadeOrigem);
	return louvain;
}

/*	
	Esta é a função principal
	
	Args:
	Ela deve receber o arquivo .net com a descrição do grafo como parametro em sua execução
 */
int main(int argc, char** argv)
{	
	// Caso um valor menor que dois, entõa o arquivo.net não foi passado como parâmetro para o programa
	if (argc < 2)
        cerr << "no input file's name\n"<< endl;

	Louvain louvain = Louvain(argv[1]);	
	
	louvain.modularidade = obterModularidadeQ(louvain);
	bool houveMelhora;
	
	do {
		houveMelhora = false;
		for (int comunidade : louvain.comunidades) {
			Louvain melhorAlteracao = louvain;			
			Comunidades vizinhas = obterComunidadesVizinhas(louvain, comunidade);			

			for (int vizinha : vizinhas) {
				Louvain alteracao = migrarVertices(louvain, comunidade, vizinha);
				if (alteracao.modularidade > melhorAlteracao.modularidade) {
					melhorAlteracao = alteracao;
					houveMelhora = true;
				}
			}

			louvain = melhorAlteracao;
		}
	} while (houveMelhora);

	for (int c = 0; c < louvain.comunidades.size(); c++){
		
		
		Vertices vertices = obterVerticesDaComunidade(louvain, c);
		if(vertices.size() > 0){
			cout<< "\nComunidade "<< c <<'\n';
			for(int v : vertices)
				cout << nomes[v];
		}
		
		//cout << nomes[v] << '\t' << louvain.vertices[v] << '\n';
		// printf("V[%d] -> C[%d] \n", v, louvain.vertices[v]);
	}
		
}