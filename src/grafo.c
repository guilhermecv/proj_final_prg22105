/*
 * grafo.c
 *
 *  Created on: Jul 5, 2016
 *      Author: Renan Augusto Starke
 *
 *	Modified on: Oct 13, 2019
 *			Guilherme Camargo Valese
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "grafo.h"
#include "vertice.h"
#include "lista_enc.h"
#include "no.h"
// #include "fila.h"
#include "pilha.h"

#define FALSE 0
#define TRUE 1

// #define DEBUG
#define DEBUG_ON_KRUSKAL

#define INFINITO INT_MAX

struct grafos {
	int id;                    /*!< Identificação numérica do grafo  */
	lista_enc_t *vertices;     /*!< Lista encadeada dos vértices: conjunto V  */
};

/**
  * @brief  Cria uma novo grafo
  * @param	id: Identificação numérica do grafo
  *
  * @retval grafo_t: ponteiro para um novo grafo
  */
grafo_t *cria_grafo(int id)
{
	grafo_t *p = NULL;

	p = (grafo_t*) malloc(sizeof(grafo_t));

	if (p == NULL)	{
		perror("cria_grafo:");
		exit(EXIT_FAILURE);
	}

	p->id = id;
	p->vertices = cria_lista_enc();

	return p;
}

/**
  * @brief  Adicionar um vértice no grafo (conjunto V)
  * @param	grafo: ponteiro do grafo que se deseja adicionar um vértice
  * @param  id: identificação da vértice
  *
  * @retval vertice_t: ponteiro do vértice criado e adicionada no grafo
  */
vertice_t* grafo_adicionar_vertice(grafo_t *grafo, int id)
{
	vertice_t *vertice;
	no_t *no;

#ifdef DEBUG
	printf("grafo_adicionar_vertice: %d\n", id);
#endif

	if (grafo == NULL)	{
			fprintf(stderr,"grafo_adicionar_vertice: grafo invalido!");
			exit(EXIT_FAILURE);
	}

	if (procura_vertice(grafo, id) != NULL) {
		fprintf(stderr,"grafo_adicionar_vertice: vertice duplicado!\n");
		exit(EXIT_FAILURE);
	}

	vertice = cria_vertice(id);
	no = cria_no(vertice);

	add_cauda(grafo->vertices, no);

	return vertice;
}

/**
  * @brief  Procura um vértice com id específico no grafo
  * @param	grafo: ponteiro do grafo que se deseja busca o vértice
  * @param  id: identificação da aresta
  *
  * @retval vertice_t: ponteiro do vértice. NULL se não encontrado
  */
vertice_t* procura_vertice(grafo_t *grafo, int id)
{
	no_t *no_lista;
	vertice_t *vertice;
	int meu_id;

	if (grafo == NULL)	{
		fprintf(stderr,"procura_vertice: grafo invalido!");
		exit(EXIT_FAILURE);
	}

	if (lista_vazia(grafo->vertices) == TRUE)
		return FALSE;

	no_lista = obter_cabeca(grafo->vertices);

	while (no_lista)
	{
		//obtem o endereco da lista
		vertice = obter_dado(no_lista);

		//obterm o id do vertice
		meu_id = vertice_get_id(vertice);

		if (meu_id == id) {
			return vertice;
		}

		no_lista = obtem_proximo(no_lista);
	}

	return NULL;
}

/**
  * @brief  Cria adjacências.
  * @param	grafo: ponteiro do grafo que contém o vértice (V pertence a G)
  * @param  vertice: vértice fonte da(s) adjacências
  * @param  n: número de parâmetros após n
  * @param  ...: pares ordenados dos vertices destino e peso da aresta: (id vertice destino, peso aresta)
  *
  * @retval Nenhum
  *
  * Ex: adicionar uma aresta para o vertice 2 e 3 com respectivos pesos 9 e 15
  * adiciona_adjacentes(grafo, vertice, 4(n), 2, 9, 3, 15);
  */

void adiciona_adjacentes(grafo_t *grafo, vertice_t *vertice, int n, ...)
{
	va_list argumentos;
	vertice_t *sucessor;
	arestas_t *aresta;

	int id_sucessor;
	int peso;
    int x;

	/* Initializing arguments to store all values after num */
	va_start (argumentos, n);

	for (x = 0; x < n; x=x+2 )
	{
		id_sucessor = va_arg(argumentos, int);
		peso = va_arg(argumentos, int);

		sucessor = procura_vertice(grafo, id_sucessor);

		if (sucessor == NULL) {
			fprintf(stderr, "adiciona_adjacentes: sucessor nao encontrado no grafo\n");
			exit(EXIT_FAILURE);
		}

		aresta = cria_aresta(vertice, sucessor,peso);
		adiciona_aresta(vertice, aresta);

#ifdef DEBUG
		printf("\tvertice: %d\n", vertice_get_id(vertice));
		printf("\tsucessor: %d\n", id_sucessor);
		printf("\tpeso: %d\n", peso);
#endif

	}

	va_end (argumentos);
}

/**
  * @brief  Exporta o grafo em formato dot.
  * @param	filename: nome do arquivo dot gerado
  * @param  grafo: ponteiro do grafo a ser exportado
  *
  * @retval Nenhum
  */
void exportar_grafo_dot(const char *filename, grafo_t *grafo)
{
	FILE *file;

	no_t *no_vert;
	no_t *no_arest;
	vertice_t *vertice;
	vertice_t *adjacente;
	arestas_t *aresta;
	arestas_t *contra_aresta;
	lista_enc_t *lista_arestas;

	int peso;

	if (filename == NULL || grafo == NULL){
		fprintf(stderr, "exportar_grafo_dot: ponteiros invalidos\n");
		exit(EXIT_FAILURE);
	}

	file = fopen(filename, "w");

	if (file == NULL){
		perror("exportar_grafo_dot:");
		exit(EXIT_FAILURE);
	}

	fprintf(file, "graph {\n");

	//obtem todos os nos da lista
	no_vert = obter_cabeca(grafo->vertices);
	while (no_vert){
		vertice = obter_dado(no_vert);

		//obtem todos as arestas
		lista_arestas = vertice_get_arestas(vertice);

		no_arest = obter_cabeca(lista_arestas);
		while (no_arest) {
			aresta = obter_dado(no_arest);

			//ignora caso já exportada
			if (aresta_get_status(aresta) == EXPORTADA) {
				no_arest = obtem_proximo(no_arest);
				continue;
			}

			//marca como exportada esta aresta
			aresta_set_status(aresta, EXPORTADA);
			adjacente = aresta_get_adjacente(aresta);

			//marca contra-aresta também como exporta no caso de grafo não direcionados
			contra_aresta = procurar_adjacente(adjacente, vertice);
			aresta_set_status(contra_aresta, EXPORTADA);

			//obtem peso
			peso = aresta_get_peso(aresta);

			fprintf(file, "\t%d -- %d [label = %d];\n",
					vertice_get_id(vertice),
					vertice_get_id(adjacente),
					peso);

			no_arest = obtem_proximo(no_arest);
		}
		no_vert = obtem_proximo(no_vert);
	}
	fprintf(file, "}\n");
	fclose(file);
}

/**
  * @brief  Libera a memória utilizada pelo grafo
  * @param  grafo: ponteiro do grafo a ser exportado
  *
  * @retval Nenhum
  */
void libera_grafo (grafo_t *grafo){
	no_t *no_vert;
	no_t *no_arest;
	no_t *no_liberado;
	vertice_t *vertice;
	arestas_t *aresta;
	lista_enc_t *lista_arestas;

	if (grafo == NULL) {
		fprintf(stderr, "libera_grafo: grafo invalido\n");
		exit(EXIT_FAILURE);
	}

	//varre todos os vertices
	no_vert = obter_cabeca(grafo->vertices);
	while (no_vert){
		vertice = obter_dado(no_vert);

		//libera todas as arestas
		lista_arestas = vertice_get_arestas(vertice);
		no_arest = obter_cabeca(lista_arestas);
		while (no_arest){
			aresta = obter_dado(no_arest);

			//libera aresta
			free(aresta);

			//libera no da lsita
			no_liberado = no_arest;
			no_arest = obtem_proximo(no_arest);
			free(no_liberado);
		}

		//libera lista de arestas e vertice
		free(lista_arestas);
		free(vertice);

		//libera no da lista
		no_liberado = no_vert;
		no_vert = obtem_proximo(no_vert);
		free(no_liberado);
	}

	//libera grafo e vertice
	free(grafo->vertices);
	free(grafo);
}

/**
 * @brief Algoritmo de kruskal
 * @param *grafo: grafo onde sera realizada a busca
 * @return
 */
void kruskal_mst(grafo_t *grafo, grafo_t *g_out)
{
	int custo_total = 0;
	int peso_aresta;
	int id_aresta;

	no_t *no_vert;
	no_t *no_arest;
	vertice_t *vertice, *vertice_out;
	vertice_t *adjacente;
	arestas_t *aresta;
	lista_enc_t *lista_arestas;

	if(grafo == NULL)
	{
		fprintf(stderr, "kruskal_mst: ponteiros invalidos");
		exit(EXIT_FAILURE);
	}


	int i, j;

	//obtem todos os nos da lista
	no_vert = obter_cabeca(grafo->vertices);

	while (no_vert){
		vertice = obter_dado(no_vert);
		lista_arestas = vertice_get_arestas(vertice);
		no_arest = obter_cabeca(lista_arestas);
		int menor_peso = INFINITO;
		while (no_arest) {

			aresta = obter_dado(no_arest);								// Obtem a aresta
			printf("\nanalisando vertice %d", vertice_get_id(vertice));


			adjacente = aresta_get_adjacente(aresta);
			peso_aresta = aresta_get_peso(aresta);						// Obtem peso da aresta

#ifdef DEBUG_ON_KRUSKAL
				printf("\t%d -- %d [label = %d];\n",
				vertice_get_id(vertice),
				vertice_get_id(adjacente),
				peso_aresta);
#endif

			if(peso_aresta < menor_peso)
			{
				menor_peso = peso_aresta;								// Aresta de menor peso encontrada no vertice
				i = vertice_get_id(vertice);
				j = vertice_get_id(adjacente);
			}



			no_arest = obtem_proximo(no_arest);
		} // while(no_arest)

		no_vert = obtem_proximo(no_vert);		// Segue para o proximo vertice
	}// while(no_vert)
}

void prim_mst(grafo_t *grafo, int inicial)
{
	no_t *no_vert;
	no_t *no_arest;
	vertice_t *vertice, *vertice_out;
	vertice_t *adjacente;
	arestas_t *aresta;
	arestas_t *contra_aresta;
	lista_enc_t *lista_arestas;

	int id_aresta, change;
	int peso_aresta;

	if(grafo == NULL)
	{
		fprintf(stderr, "prim_mst: ponteiros invalidos");
		exit(EXIT_FAILURE);
	}
	//obtem todos os nos da lista
	no_vert = obter_cabeca(grafo->vertices);

	while (no_vert){
		vertice = obter_dado(no_vert);
		lista_arestas = vertice_get_arestas(vertice);
		no_arest = obter_cabeca(lista_arestas);

		int menor_peso = INFINITO;

		while (no_arest) {

			aresta = obter_dado(no_arest);										// Obtem a aresta
#ifdef DEBUG
			printf("\nanalisando vertice %d", vertice_get_id(vertice));
#endif

			if(vertice_visitado(vertice))
			{
				printf("\no vertice ja foi visitado\n");
				no_arest = obtem_proximo(no_arest);
				continue;
			}

			adjacente = aresta_get_adjacente(aresta);
			contra_aresta = procurar_adjacente(adjacente, vertice);

			peso_aresta = aresta_get_peso(aresta);						// Obtem peso da aresta
			int peso_contra_aresta = aresta_get_peso(contra_aresta);

			int menor_peso_temp;

			if(peso_aresta < peso_contra_aresta)
				menor_peso_temp = peso_aresta;
			else
				menor_peso_temp = peso_contra_aresta;


			if(peso_aresta < menor_peso)
			{
				change = 1;
				menor_peso = peso_aresta;
				id_aresta = vertice_get_id(adjacente);
#ifdef DEBUG
				printf("\nid_aresta %d, peso %d", id_aresta, menor_peso);
#endif
			}

			no_arest = obtem_proximo(no_arest);
		} // while(no_arest)

		if(change)
		{
			vertice_set_visited(adjacente);
			change = 0;
// #ifdef DEBUG
			printf("\nVertice %d marcado\n", vertice_get_id(adjacente));
// #endif
			printf("\t%d -- %d [label = %d];\n",
			vertice_get_id(vertice),
			id_aresta,
			menor_peso);

		}

		no_vert = obtem_proximo(no_vert);		// Segue para o proximo vertice
	}// while(no_vert)


}
