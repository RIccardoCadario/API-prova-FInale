//
//  main.c
//  progettoAPI
//
//  Created by Riccardo Maria Cadario on 09/07/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RICETTE_SIZE 1000000
#define MAGAZZINO_SIZE 1000000

//------------------------RICETTARIO:-------------------------
// struttura base ingredienti per ricettario
typedef struct Ingredienti {
    char * nome;
    int quantita;
    struct Ingredienti* next;
} Ingredienti;

// struttura base ricette per ricettario
typedef struct Ricetta {
    char * nome;
    Ingredienti* ingredienti;
    struct Ricetta *next;
} Ricetta;

// struttura per la tabella hash del ricettario
typedef struct HashTable {
    Ricetta* buckets[RICETTE_SIZE];
} HashTable;
//-------------------------MAGAZZINO:---------------------------
// Struttura per un lotto di ingredienti
typedef struct Lotto {
    int quantita;
    int scadenza;
    struct Lotto* next;
} Lotto;

// Struttura per un ingrediente nel magazzino
typedef struct IngredienteMagazzino {
    char *nome;
    int quantita;
    Lotto* lotti;
    struct IngredienteMagazzino* next;
} IngredienteMagazzino;

// Struttura per la tabella hash del magazzino
typedef struct Magazzino {
    IngredienteMagazzino* buckets[MAGAZZINO_SIZE];
} Magazzino;

//----------------------CODA ORDINI:--------------------------

// Struttura per un ordine
typedef struct Ordine {
    char *nome_ricetta;
    int numero_elementi_ordinati;
    int istante_arrivo;
    struct Ordine* next;
} Ordine;

// Struttura per la coda degli ordini
typedef struct CodaOrdini_attesa {
    Ordine* head;
    Ordine* tail;
} CodaOrdini_attesa;

// Struttura per la coda degli ordini
typedef struct CodaOrdini_pronti {
    Ordine* head;
    Ordine* tail;
} CodaOrdini_pronti;


//-----------------------RICETTARIO FUNZIONI:--------------------------

Ricetta* find_recipe(HashTable* table, char* nome);
int processa_ordine(HashTable* , Magazzino* , CodaOrdini_attesa* ,CodaOrdini_pronti* , char* , int ,int);
void remove_recipe(HashTable* , CodaOrdini_attesa* ,CodaOrdini_pronti* ,char* );
void stampa_coda(CodaOrdini_attesa* );
void stampa_coda_pronta(CodaOrdini_pronti* );
void print_all_ingredienti(Magazzino* );
int calcola_peso_ordine(HashTable* ,Ordine* );

// funzione di hash del ricettario
unsigned int hash(char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + *key++;
    }
    return hash % RICETTE_SIZE;
}

//crea la tabella hash ricettario-
HashTable* create_hashtable(void) {
    HashTable* table = malloc(sizeof(HashTable));
    for (int i = 0; i < RICETTE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

// crea un nuovo elemento nella lista degli ingredienti del ricettario-
Ingredienti * create_ingrediente(char * nome_ingrediente, int quantita) {
    Ingredienti * ingrediente = malloc(sizeof(Ingredienti));
    ingrediente->nome = strdup(nome_ingrediente);  // Duplicate the string to avoid overwriting
    ingrediente->quantita = quantita;
    ingrediente->next = NULL;
    return ingrediente;
}

// crea una nuova ricetta per il ricettario
Ricetta* create_recipe(char * nome_ricetta) {
    Ricetta * ricetta = malloc(sizeof(Ricetta));
    ricetta->nome = strdup(nome_ricetta);  // Duplicate the string to avoid overwriting
    ricetta->ingredienti = NULL;
    ricetta->next = NULL;
    return ricetta;
}

// Funzione per stampare gli ingredienti di una ricetta
void print_ingredients(Ricetta* ricetta) {
    Ingredienti* ingrediente = ricetta->ingredienti;
    while (ingrediente != NULL) {
        printf("Ingrediente: %s, Quantita: %d\n", ingrediente->nome, ingrediente->quantita);
        ingrediente = ingrediente->next;
    }
}

// Funzione per stampare tutte le ricette
void print_all_recipes(HashTable* table) {
    for (int i = 0; i < RICETTE_SIZE; i++) {
        Ricetta* ricetta = table->buckets[i];
        while (ricetta != NULL) {
            printf("Ricetta: %s\n", ricetta->nome);
            print_ingredients(ricetta);
            ricetta = ricetta->next;
        }
    }
}

// Funzione per aggiungere un ingrediente a una ricetta
void add_ingredient(Ricetta* ricetta, char* nome, int quantita) {
    Ingredienti* ingrediente = create_ingrediente(nome, quantita);
    if (ricetta->ingredienti == NULL) {
        ricetta->ingredienti = ingrediente;
    } else {
        Ingredienti* current = ricetta->ingredienti;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = ingrediente;
    }
}

// Funzione per inserire una ricetta nel ricettario
void insert_recipe(HashTable* table, char* nome_ricetta) {
    unsigned int index = hash(nome_ricetta);
    Ricetta* ricetta = create_recipe(nome_ricetta);
    
    ricetta->next = table->buckets[index];
    table->buckets[index] = ricetta;
}

// Funzione per trovare una ricetta nel ricettario
Ricetta* find_recipe(HashTable* table, char* nome) {
    unsigned int index = hash(nome);
    Ricetta* ricetta = table->buckets[index];
    while (ricetta != NULL && strcmp(ricetta->nome, nome) != 0) {
        ricetta = ricetta->next;
    }
    return ricetta;
}

//-------------------MAGAZZINO FUNZIONI:------------------------

//funzione di hash del magazzino
unsigned int new_hash(char *key) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % MAGAZZINO_SIZE;
}

// Funzione per creare il magazzino
Magazzino* create_magazzino(void) {
    Magazzino* magazzino = malloc(sizeof(Magazzino));
    for (int i = 0; i < MAGAZZINO_SIZE; i++) {
        magazzino->buckets[i] = NULL;
    }
    return magazzino;
}

// Funzione per creare un nuovo lotto
Lotto* create_lotto(int quantita, int scadenza) {
    Lotto* lotto = malloc(sizeof(Lotto));
    lotto->quantita = quantita;
    lotto->scadenza = scadenza;
    lotto->next = NULL;
    return lotto;
}

// Funzione per creare un nuovo ingrediente nel magazzino
IngredienteMagazzino* create_ingrediente_magazzino(char* nome) {
    IngredienteMagazzino* ingrediente = malloc(sizeof(IngredienteMagazzino));
    ingrediente->nome = strdup(nome);
    ingrediente->quantita=0;
    ingrediente->lotti = NULL;
    ingrediente->next = NULL;
    return ingrediente;
}


// Funzione per aggiungere un lotto di ingredienti al magazzino
void add_lotto(Magazzino* magazzino, char* nome, int quantita, int scadenza) {
    unsigned int index = new_hash(nome);
    IngredienteMagazzino* ingrediente = magazzino->buckets[index];
    
    // Trova l'ingrediente nel magazzino, se esiste
    while (ingrediente != NULL && strcmp(ingrediente->nome, nome) != 0) {
        ingrediente = ingrediente->next;

    }
    
    // Se l'ingrediente non esiste, crea una nuova voce
    if (ingrediente == NULL) {
        ingrediente = create_ingrediente_magazzino(nome);
        ingrediente->next = magazzino->buckets[index];
        magazzino->buckets[index] = ingrediente;
    }
    
    // Crea il nuovo lotto e lo inserisce nella lista ordinata
    Lotto* lotto = create_lotto(quantita, scadenza);
    if (ingrediente->lotti == NULL || ingrediente->lotti->scadenza > scadenza) {
        lotto->next = ingrediente->lotti;
        ingrediente->lotti = lotto;
        ingrediente->quantita = ingrediente->quantita + lotto->quantita;
    } else {
        Lotto* current = ingrediente->lotti;
        while (current->next != NULL && current->next->scadenza <= scadenza) {
            current = current->next;
        }
        lotto->next = current->next;
        current->next = lotto;
    }
}

// Funzione per stampare i lotti di un ingrediente
void print_lotti(IngredienteMagazzino* ingrediente) {
    Lotto* lotto = ingrediente->lotti;
    while (lotto != NULL) {
        printf("Quantita: %d, Scadenza: %d\n", lotto->quantita, lotto->scadenza);
        lotto = lotto->next;
    }
}

// Funzione per stampare tutti gli ingredienti e i loro lotti nel magazzino
void print_all_ingredienti(Magazzino* magazzino) {
    for (int i = 0; i < MAGAZZINO_SIZE; i++) {
        IngredienteMagazzino* ingrediente = magazzino->buckets[i];
        while (ingrediente != NULL) {
            printf("Ingrediente: %s\n", ingrediente->nome);
            print_lotti(ingrediente);
            ingrediente = ingrediente->next;
        }
    }
}


//-------------------LIBERA MEMORIA:------------------------

// Funzione per liberare la memoria allocata per un lotto
void free_lotto(Lotto* lotto) {
    while (lotto != NULL) {
        Lotto* temp = lotto;
        lotto = lotto->next;
        free(temp);
    }
}

// Funzione per liberare la memoria allocata per un ingrediente nel magazzino
void free_ingrediente_magazzino(IngredienteMagazzino* ingrediente) {
    free_lotto(ingrediente->lotti);
    free(ingrediente->nome);
    free(ingrediente);
}

// Funzione per liberare la memoria allocata per il magazzino
void free_magazzino(Magazzino* magazzino) {
    for (int i = 0; i < MAGAZZINO_SIZE; i++) {
        while (magazzino->buckets[i] != NULL) {
            IngredienteMagazzino* temp = magazzino->buckets[i];
            magazzino->buckets[i] = magazzino->buckets[i]->next;
            free_ingrediente_magazzino(temp);
        }
    }
    free(magazzino);
}

// Funzione per liberare la memoria allocata per una ricetta
void free_recipe(Ricetta* ricetta) {
    while (ricetta->ingredienti != NULL) {
        Ingredienti* temp = ricetta->ingredienti;
        ricetta->ingredienti = ricetta->ingredienti->next;
        free(temp->nome);
        free(temp);
    }
    free(ricetta->nome);
    free(ricetta);
}

//controlla se ci sono ordini in attesa per quella ricetta, altrimenti la remove_recipe la cancellera
int ordini_in_sospeso(CodaOrdini_attesa* coda_ordini_attesa, char* nome_ricetta) {
    Ordine* current = coda_ordini_attesa->head;
    //printf("sono entrato in ordine in sospeso\n");
    while (current != NULL) {
        if (strcmp(current->nome_ricetta, nome_ricetta) == 0) {
            return 1; // Ordine in sospeso trovato
        }
        current = current->next;
    }
    return 0; // Nessun ordine in sospeso per questa ricetta
}

//controlla se ci sono ordini in attesa per quella ricetta, altrimenti la remove_recipe la cancellera
int ordini_in_sospeso_pronti(CodaOrdini_pronti* coda_ordini_pronti, char* nome_ricetta) {
    Ordine* current = coda_ordini_pronti->head;
    //printf("sono entrato in ordine in sospeso\n");
    while (current != NULL) {
        if (strcmp(current->nome_ricetta, nome_ricetta) == 0) {
            return 1; // Ordine in sospeso trovato
        }
        current = current->next;
    }
    return 0; // Nessun ordine in sospeso per questa ricetta
}

//rimuove la ricetta dal ricettario con il nome dato in ingresso,per caso 2:manca gestione caso ordine non ancora evaso ->in coda
void remove_recipe(HashTable* table, CodaOrdini_attesa* coda_ordini_attesa,CodaOrdini_pronti* coda_ordini_pronti,char* nome_ricetta) {
    
    if (ordini_in_sospeso(coda_ordini_attesa, nome_ricetta)==1) {
        printf("ordini in sospeso\n");
        return;
    }else if (ordini_in_sospeso_pronti(coda_ordini_pronti, nome_ricetta)==1){
        printf("ordini in sospeso\n");
        return;
    }
    
    unsigned int index = hash(nome_ricetta);
    Ricetta* ricetta = table->buckets[index];
    Ricetta* prev = NULL;
    
    while (ricetta != NULL && strcmp(ricetta->nome, nome_ricetta) != 0) {
        prev = ricetta;
        ricetta = ricetta->next;
    }
    
    if (ricetta != NULL) {
        if (prev == NULL) {
            table->buckets[index] = ricetta->next;
        } else {
            prev->next = ricetta->next;
        }
        free_recipe(ricetta);
        printf("rimossa\n");//togliere solo per debug
    } else {
        printf("non presente\n");//togliere solo per debug
    }
}

// Funzione per liberare la memoria allocata per la tabella hash
void free_table(HashTable* table) {
    for (int i = 0; i < RICETTE_SIZE; i++) {
        while (table->buckets[i] != NULL) {
            Ricetta* temp = table->buckets[i];
            table->buckets[i] = table->buckets[i]->next;
            free_recipe(temp);
        }
    }
    free(table);
}

//------------------PROCESSA ORDINE:--------------------

// Funzione per creare una nuova coda di ordini
CodaOrdini_attesa* create_coda_ordini_attesa(void) {
    CodaOrdini_attesa* coda = malloc(sizeof(CodaOrdini_attesa));
    coda->head = NULL;
    coda->tail = NULL;
    return coda;
}

// Funzione per creare una nuova coda di ordini
CodaOrdini_pronti* create_coda_ordini_pronti(void) {
    CodaOrdini_pronti* coda = malloc(sizeof(CodaOrdini_pronti));
    coda->head = NULL;
    coda->tail = NULL;
    return coda;
}

// Funzione per aggiungere un ordine alla coda
void enqueue_ordine_attesa(CodaOrdini_attesa* coda, char* nome_ricetta, int numero_elementi_ordinati,int istante_arrivo) {
    Ordine* ordine = malloc(sizeof(Ordine));
    ordine->nome_ricetta = strdup(nome_ricetta);
    ordine->numero_elementi_ordinati = numero_elementi_ordinati;
    ordine->istante_arrivo=istante_arrivo;
    ordine->next = NULL;
    
    if (coda->tail == NULL) {
        coda->head = ordine;
        coda->tail = ordine;
    } else {
        coda->tail->next = ordine;
        coda->tail = ordine;
    }
}

void enqueue_ordine_pronti(CodaOrdini_pronti* coda, char* nome_ricetta, int numero_elementi_ordinati, int istante_arrivo) {
    Ordine* nuovo_ordine = malloc(sizeof(Ordine));
    nuovo_ordine->nome_ricetta = strdup(nome_ricetta);
    nuovo_ordine->numero_elementi_ordinati = numero_elementi_ordinati;
    nuovo_ordine->istante_arrivo = istante_arrivo;
    nuovo_ordine->next = NULL;

    // Se la coda è vuota, inserisci il nuovo ordine come unico elemento
    if (coda->head == NULL) {
        coda->head = nuovo_ordine;
        coda->tail = nuovo_ordine;
    } else if (coda->head->istante_arrivo > istante_arrivo) {
        // Se il nuovo ordine ha l'istante di arrivo minore del primo elemento
        nuovo_ordine->next = coda->head;
        coda->head = nuovo_ordine;
    } else {
        // Inserisci il nuovo ordine nella posizione corretta
        Ordine* current = coda->head;
        while (current->next != NULL && current->next->istante_arrivo <= istante_arrivo) {
            current = current->next;
        }
        nuovo_ordine->next = current->next;
        current->next = nuovo_ordine;
        if (current == coda->tail) {
            coda->tail = nuovo_ordine;
        }
    }
}

void enqueue_ordine_pronti_in_testa(CodaOrdini_pronti* coda, char* nome_ricetta, int numero_elementi_ordinati, int istante_arrivo) {
    Ordine* new_ordine = malloc(sizeof(Ordine));
    new_ordine->nome_ricetta = strdup(nome_ricetta);
    new_ordine->numero_elementi_ordinati = numero_elementi_ordinati;
    new_ordine->istante_arrivo = istante_arrivo;
    new_ordine->next = coda->head;
    coda->head = new_ordine;
    if (coda->tail == NULL) {
        coda->tail = new_ordine;
    }
}

void enqueue_ordine_attesa_in_testa(CodaOrdini_attesa* coda, char* nome_ricetta, int numero_elementi_ordinati, int istante_arrivo) {
    Ordine* new_ordine = malloc(sizeof(Ordine));
    new_ordine->nome_ricetta = strdup(nome_ricetta);
    new_ordine->numero_elementi_ordinati = numero_elementi_ordinati;
    new_ordine->istante_arrivo = istante_arrivo;
    new_ordine->next = coda->head;
    coda->head = new_ordine;
    if (coda->tail == NULL) {
        coda->tail = new_ordine;
    }
}

// Funzione per rimuovere un ordine dalla coda
Ordine* dequeue_ordine_attesa(CodaOrdini_attesa* coda) {
    if (coda->head == NULL) {
        return NULL;
    }
    Ordine* ordine = coda->head;
    coda->head = coda->head->next;
    if (coda->head == NULL) {
        coda->tail = NULL;
    }
    return ordine;
}

// Funzione per rimuovere un ordine dalla coda
Ordine* dequeue_ordine_pronti(CodaOrdini_pronti* coda) {
    if (coda->head == NULL) {
        return NULL;
    }
    Ordine* ordine = coda->head;
    coda->head = coda->head->next;
    if (coda->head == NULL) {
        coda->tail = NULL;
    }
    return ordine;
}

// Funzione per processare un ordine
int processa_ordine(HashTable* ricettario, Magazzino* magazzino, CodaOrdini_attesa* coda,CodaOrdini_pronti* codap, char* nome_ricetta, int numero_elementi_ordinati,int istante_arrivo){
    
    //trova la ricetta nel ricettario
    Ricetta* ricetta = find_recipe(ricettario, nome_ricetta);
    
    if (ricetta == NULL) {
        return 2;
    }
    
    //cerca ingrediente nel ricettario
    Ingredienti* ingrediente = ricetta->ingredienti;
    //qui c'ho gli ingredienti della ricetta che devo preparare, che posso controllare tutti
    
    while (ingrediente != NULL) {
        unsigned int index = new_hash(ingrediente->nome);//cerca il codice hash del nome ingrediente nel magazzino
        IngredienteMagazzino* ingr_magazzino = magazzino->buckets[index];//trova l'ingrediente nel magazzino
        
        while (ingr_magazzino != NULL && strcmp(ingr_magazzino->nome, ingrediente->nome) != 0) {
            ingr_magazzino = ingr_magazzino->next;
        }
        
        if (ingr_magazzino == NULL) {
            return 0;
        }
        
        Lotto* l = ingr_magazzino->lotti;
        int quantita_totale_disponibile = 0;
        int quantita_necessaria = ingrediente->quantita * numero_elementi_ordinati;
        
        //trovo la quantita totale disponibile nel magazzino per quell'ingrediente
        while (l != NULL) {
            if(l->scadenza>istante_arrivo){//conto la quantita solo se il lotto non è ancora scaduto
                quantita_totale_disponibile += l->quantita;
            }
            
            l = l->next;
        }
        
        if (quantita_totale_disponibile < quantita_necessaria) {
            //ordine accettato ma messo nella lista di attesa
            return 0;
        }
        
        //ingr_magazzino->lotti = lotto;
        ingrediente = ingrediente->next;
    }
    
    Ingredienti* ingredientz = ricetta->ingredienti;
    
    while (ingredientz!=NULL) {
        unsigned int index = new_hash(ingredientz->nome);//cerca il codice hash del nome ingrediente nel magazzino
        IngredienteMagazzino* ingr_magazzino = magazzino->buckets[index];
        while (ingr_magazzino != NULL && strcmp(ingr_magazzino->nome, ingredientz->nome) != 0) {
            ingr_magazzino = ingr_magazzino->next;
        }
        
        int quantita_necessaria = ingredientz->quantita * numero_elementi_ordinati;
        
        //nel caso eista l'ingrediente e ce ne sia abbastanza nel magazzino, allora elimino il lotti che utilizzo
        Lotto* lotto = ingr_magazzino->lotti;
        Lotto* lotto_precedente = NULL;
        
        while (lotto != NULL && quantita_necessaria > 0) {//
            
            if (lotto->scadenza > istante_arrivo) { // Se il lotto non è scaduto
                            
                            if (lotto->quantita < quantita_necessaria) { // Controlla la quantità se il lotto non ne ha abbastanza
                                quantita_necessaria -= lotto->quantita;
                                lotto->quantita = 0;
                                
                                // Rimuovi il lotto dalla lista
                                if (lotto_precedente == NULL) {
                                    ingr_magazzino->lotti = lotto->next;
                                } else {
                                    lotto_precedente->next = lotto->next;
                                }
                                
                                Lotto* lotto_da_rimuovere = lotto;
                                lotto = lotto->next;
                                free(lotto_da_rimuovere);
                                
                            } else {
                                lotto->quantita -= quantita_necessaria;
                                quantita_necessaria = 0;
                                lotto_precedente = lotto;
                                lotto = lotto->next;
                            }
                            
                        } else { // Lotto scaduto
                            // Rimuovi il lotto dalla lista
                            if (lotto_precedente == NULL) {
                                ingr_magazzino->lotti = lotto->next;
                            } else {
                                lotto_precedente->next = lotto->next;
                            }
                            
                            Lotto* lotto_da_rimuovere = lotto;
                            lotto = lotto->next;
                            free(lotto_da_rimuovere);
                        }
        }
        ingredientz = ingredientz->next;
    }
    
    return 1;
}

// Funzione per smaltire gli ordini in attesa
void smaltisci_ordini_in_attesa(HashTable* ricettario, Magazzino* magazzino, CodaOrdini_attesa* coda, CodaOrdini_pronti* codap, char* nome_ricetta, int numero_elementi_ordinati, int istante_arrivo) {
    //stampa_coda(coda);
    //stampa_coda_pronta(codap);
    Ordine* ordine;
    Ordine* primo_ordine = NULL;
    int flag=0;
    // Ciclo finché ci sono ordini in attesa
    while ((ordine = dequeue_ordine_attesa(coda)) != NULL ) {
        //printf("%s\n", ordine->nome_ricetta);
        
        // Tenta di processare l'ordine
        int risultato = processa_ordine(ricettario, magazzino, coda, codap, ordine->nome_ricetta, ordine->numero_elementi_ordinati, istante_arrivo);

        // Se l'ordine rimane in attesa ed è il primo
        if (risultato != 1 && flag==0) {
            
            enqueue_ordine_attesa(coda, ordine->nome_ricetta, ordine->numero_elementi_ordinati, ordine->istante_arrivo);
            flag=1;
            primo_ordine=ordine;
        }
        //
        else if(flag==1 ){
            //se sono tornato al primo
            if(strcmp(primo_ordine->nome_ricetta, ordine->nome_ricetta)==0 && primo_ordine->istante_arrivo==ordine->istante_arrivo){
                enqueue_ordine_attesa_in_testa(coda,ordine->nome_ricetta, ordine->numero_elementi_ordinati, ordine->istante_arrivo);
                free(ordine->nome_ricetta);
                free(ordine);
                break;
            }//se invece è un altro
            else if(risultato==1){
                enqueue_ordine_pronti(codap, ordine->nome_ricetta, ordine->numero_elementi_ordinati, ordine->istante_arrivo);
                free(ordine->nome_ricetta);
                free(ordine);
            }
            else if(risultato==0){
                enqueue_ordine_attesa(coda, ordine->nome_ricetta, ordine->numero_elementi_ordinati, ordine->istante_arrivo);
                free(ordine->nome_ricetta);
                free(ordine);
            }
        }
        else {
            enqueue_ordine_pronti(codap, ordine->nome_ricetta, ordine->numero_elementi_ordinati, ordine->istante_arrivo);
            free(ordine->nome_ricetta);
            free(ordine);
        }
    }
}

void smaltisci_ordini_in_attesa_prova(HashTable* ricettario, Magazzino* magazzino, CodaOrdini_attesa* coda, CodaOrdini_pronti* codap, int istante_arrivo) {
    Ordine* ordine_precedente = NULL;
    Ordine* ordine_corrente = coda->head;
    
    while (ordine_corrente != NULL) {
        int risultato = processa_ordine(ricettario, magazzino, coda, codap, ordine_corrente->nome_ricetta, ordine_corrente->numero_elementi_ordinati, istante_arrivo);

        if (risultato == 1) {
            enqueue_ordine_pronti(codap, ordine_corrente->nome_ricetta, ordine_corrente->numero_elementi_ordinati, ordine_corrente->istante_arrivo);
            
            if (ordine_precedente == NULL) {
                // Se l'ordine corrente è il primo nella lista
                coda->head = ordine_corrente->next;
                if (coda->head == NULL) {
                    // Se la lista diventa vuota, aggiornare la coda->tail
                    coda->tail = NULL;
                }
            } else {
                // Se l'ordine corrente è in mezzo o alla fine della lista
                ordine_precedente->next = ordine_corrente->next;
                if (ordine_corrente->next == NULL) {
                    // Se l'ordine corrente è l'ultimo, aggiornare coda->tail
                    coda->tail = ordine_precedente;
                }
            }

            Ordine* ordine_da_rimuovere = ordine_corrente;
            ordine_corrente = ordine_corrente->next;

            free(ordine_da_rimuovere->nome_ricetta);
            free(ordine_da_rimuovere);
        } else {
            ordine_precedente = ordine_corrente;
            ordine_corrente = ordine_corrente->next;
        }
    }
}

HashTable* global_table;
int compare_ordini(const void* a, const void* b) {
    Ordine* ordineA = *(Ordine**)a;
    Ordine* ordineB = *(Ordine**)b;
    int pesoA = calcola_peso_ordine(global_table, ordineA);
    int pesoB = calcola_peso_ordine(global_table, ordineB);
    
    if (pesoA > pesoB) {
        return -1; // Peso decrescente
    } else if (pesoA < pesoB) {
        return 1; // Peso decrescente
    } else {
        // Se i pesi sono uguali, ordina per istante di arrivo crescente
        if (ordineA->istante_arrivo < ordineB->istante_arrivo) {
            return -1;
        } else if (ordineA->istante_arrivo > ordineB->istante_arrivo) {
            return 1;
        } else {
            return 0;
        }
    }

}

int calcola_peso_ordine(HashTable* table,Ordine* ordine) {
    int peso = 0;
    
    Ricetta* ricetta=find_recipe(table, ordine->nome_ricetta);//da errore qui
    Ingredienti* ingrediente = ricetta->ingredienti;
    while (ingrediente != NULL) {
        
        //printf("ingrediente:%s -- quantita:%d\n",ricetta->ingredienti->nome,ricetta->ingredienti->quantita);
        peso += ingrediente->quantita * ordine->numero_elementi_ordinati;
        ingrediente = ingrediente->next;
    }
    return peso;
}

Ordine** carica_ordine_sul_camion(CodaOrdini_pronti* coda_ordini_pronti,HashTable* table,int capienza,int* num_ordini_caricati) {
    int capacita_rimanente=capienza;
    int max_ordini = 10; // Iniziamo con una dimensione arbitraria
    Ordine** vettore_ordini_corriere = malloc(max_ordini * sizeof(Ordine*));
    int i=0;
    
    while (capacita_rimanente > 0) {
        Ordine* ordine = dequeue_ordine_pronti(coda_ordini_pronti);
        if (ordine == NULL && capacita_rimanente==capienza) {
            printf("camioncino vuoto\n");
            break;
        }
        else if (ordine == NULL) {
            break;
        }
        int peso_ordine = calcola_peso_ordine(table,ordine);
        //printf("peso:%d\n",peso_ordine);
        //printf("capacita rimanente:%d\n",capacita_rimanente);
        
        if (capacita_rimanente>=peso_ordine) {
            if (i >= max_ordini) {
                max_ordini *= 2;
                vettore_ordini_corriere = realloc(vettore_ordini_corriere, max_ordini * sizeof(Ordine*));
            }
            
            vettore_ordini_corriere[i]=ordine;
            //camion->ordini_caricati[camion->num_ordini++] = ordine;
            capacita_rimanente = capacita_rimanente - peso_ordine;
            //printf("Ordine caricato: %s\n", ordine->nome_ricetta);
            i++;
        } else {
            
            enqueue_ordine_pronti_in_testa(coda_ordini_pronti, ordine->nome_ricetta,ordine->numero_elementi_ordinati,ordine->istante_arrivo); // Rimetti l'ordine in coda
            break;
        }
    }
    *num_ordini_caricati = i;
    global_table = table;
    qsort(vettore_ordini_corriere, *num_ordini_caricati, sizeof(Ordine*), compare_ordini);
        
    return vettore_ordini_corriere;
}

// Funzione per stampare la coda
void stampa_coda(CodaOrdini_attesa* coda) {
    Ordine* current = coda->head;
    while (current != NULL) {
        Ordine* ordine = current;
        printf("Nome Ricetta: %s, Numero Elementi Ordinati attesa: %d, istate di arrivo:%d\n", ordine->nome_ricetta, ordine->numero_elementi_ordinati,ordine->istante_arrivo);
        current = current->next;
    }
}

// Funzione per stampare la coda
void stampa_coda_pronta(CodaOrdini_pronti* coda) {
    Ordine* current = coda->head;
    while (current != NULL) {
        Ordine* ordine = current;
        printf("Nome Ricetta: %s, Numero Elementi Ordinati: %d, istante di arrivo:%d\n", ordine->nome_ricetta, ordine->numero_elementi_ordinati,ordine->istante_arrivo);
        current = current->next;
    }
}

void stampa_ordini_caricati(Ordine** ordini, int num_ordini) {
    for (int i = 0; i < num_ordini; i++) {
        printf("%d %s %d\n", ordini[i]->istante_arrivo,
                ordini[i]->nome_ricetta, ordini[i]->numero_elementi_ordinati);
    }
}


//---------------------------MAIN:------------------------

int main(int argc, const char * argv[]) {
    HashTable* tabella_ricette = create_hashtable();//crea tabella ricette
    Magazzino* magazzino = create_magazzino();//crea magazzino
    CodaOrdini_attesa* coda_ordini_attesa = create_coda_ordini_attesa(); //crea coda ordini in attesa
    CodaOrdini_pronti* coda_ordini_pronti = create_coda_ordini_pronti();
    char scelta[17];
    char nome_ricetta[20];
    char nome_ingrediente[25];
    int quantita_ingrediente=0;
    int data_scadenza=0;
    int numero_elementi_ordinati=0;
    int tempo_contatore=0;
    int periodicita=0,capienza=0;
    int flag=0;
    int a=0;
    //Ordine* vettore_ordini_corriere=NULL;
    
    a=scanf("%d",&periodicita);
    a=scanf(" %d",&capienza);
    
    while ((a=scanf("%s", scelta)==1)) {
        
        //caso 1: aggiungi ricetta al ricettario
        /*if ((tempo_contatore%periodicita==0) && tempo_contatore!=0){
            //printf("tempo:%d\n",tempo_contatore);
            
            //carica_ordine_sul_camion(coda_ordini_pronti,tabella_ricette,capienza);
            tempo_contatore++;
            continue;
        }*/
        
        if (strcmp(scelta, "aggiungi_ricetta") == 0) { //in caso ricettario enorme-> chaining..
            a=scanf(" %s", nome_ricetta);
            if ((tempo_contatore%periodicita==0) && tempo_contatore!=0){
                //printf("tempo:%d\n",tempo_contatore);
                int numero_ordini_caricati=0;
                /*printf("tempo:%d\n",tempo_contatore);
                printf("------ORDINI PRONTI:-------\n");
                stampa_coda_pronta(coda_ordini_pronti);
                printf("------ORDINI IN ATTESA:-------\n");
                stampa_coda(coda_ordini_attesa);*/
                Ordine** ordini_caricati = carica_ordine_sul_camion(coda_ordini_pronti,tabella_ricette,capienza,&numero_ordini_caricati);
                stampa_ordini_caricati(ordini_caricati, numero_ordini_caricati);
                            
                for (int i = 0; i < numero_ordini_caricati; i++) {
                    free(ordini_caricati[i]->nome_ricetta);
                    free(ordini_caricati[i]);
                }
                free(ordini_caricati);
            }
            /*printf("------ORDINI PRONTI:-------\n");
            stampa_coda_pronta(coda_ordini_pronti);
            printf("------ORDINI IN ATTESA:-------\n");
            stampa_coda(coda_ordini_attesa);
            printf("------MAGAZZINO:-------\n");
            print_all_ingredienti(magazzino);*/
            if (find_recipe(tabella_ricette, nome_ricetta) == NULL) {
                insert_recipe(tabella_ricette, nome_ricetta);
                Ricetta* ricetta = find_recipe(tabella_ricette, nome_ricetta);
                
                while (getchar() != '\n' ) {
                    a=scanf(" %s", nome_ingrediente);
                    if (scanf(" %d", &quantita_ingrediente) == 1) {
                        add_ingredient(ricetta, nome_ingrediente, quantita_ingrediente);
                    }else {
                        break;  // Esce dal ciclo se non c'è un numero dopo il nome dell'ingrediente
                    }
                }
                printf("aggiunta\n");
            }
            else{
                printf("ignorato\n");
                // Skip the rest of the line for this recipe
                while (getchar() != '\n') {
                    continue;
                }
            }
            tempo_contatore++;
        }
        
        //caso 2: rimuovi ricetta dal ricettario, manca gestione caso ordine non ancora evaso ->in coda
        else if (strcmp(scelta, "rimuovi_ricetta") == 0) {
            a=scanf("%s", nome_ricetta);
            if ((tempo_contatore%periodicita==0) && tempo_contatore!=0){
                printf("tempo:%d\n",tempo_contatore);
                int numero_ordini_caricati=0;
                Ordine** ordini_caricati = carica_ordine_sul_camion(coda_ordini_pronti,tabella_ricette,capienza,&numero_ordini_caricati);
                stampa_ordini_caricati(ordini_caricati, numero_ordini_caricati);
                            
                for (int i = 0; i < numero_ordini_caricati; i++) {
                    free(ordini_caricati[i]->nome_ricetta);
                    free(ordini_caricati[i]);
                }
                free(ordini_caricati);
            }
            
            remove_recipe(tabella_ricette, coda_ordini_attesa,coda_ordini_pronti,nome_ricetta);
            tempo_contatore++;
        }
        
        //caso 3: rifornimento magazzino e devi controllare se puoi evadere ordini in attesa
        else if(strcmp(scelta, "rifornimento") == 0){
            if ((tempo_contatore%periodicita==0) && tempo_contatore!=0){

                
                int numero_ordini_caricati=0;
                Ordine** ordini_caricati = carica_ordine_sul_camion(coda_ordini_pronti,tabella_ricette,capienza,&numero_ordini_caricati);
                stampa_ordini_caricati(ordini_caricati, numero_ordini_caricati);
                            
                for (int i = 0; i < numero_ordini_caricati; i++) {
                    free(ordini_caricati[i]->nome_ricetta);
                    free(ordini_caricati[i]);
                }
                free(ordini_caricati);
            }
            while (getchar() != '\n') {
                a=scanf("%s",nome_ingrediente);
                a=scanf(" %d",&quantita_ingrediente);
                a=scanf(" %d",&data_scadenza);
                add_lotto(magazzino, nome_ingrediente, quantita_ingrediente, data_scadenza);
                
                //stampa_coda(coda_ordini_attesa);
            }
            
            
            smaltisci_ordini_in_attesa_prova(tabella_ricette, magazzino, coda_ordini_attesa,coda_ordini_pronti,tempo_contatore);
            /*printf("------ORDINI PRONTI POST SMALTIMENTO:-------\n");
            stampa_coda_pronta(coda_ordini_pronti);
            printf("------ORDINI IN ATTESA POST SMALTIMENTO:-------\n");
            stampa_coda(coda_ordini_attesa);
            //stampa_coda(coda_ordini_attesa);
            //stampa_coda_pronta(coda_ordini_pronti);
            printf("------MAGAZZINO POST:-------\n");
            print_all_ingredienti(magazzino);*/
            printf("rifornito\n");
            tempo_contatore++;
        }
        
        //ordinazione di una ricetta
        else if(strcmp(scelta, "ordine") == 0){
            
            a=scanf(" %s",nome_ricetta);
            a=scanf(" %d",&numero_elementi_ordinati);
            if ((tempo_contatore%periodicita==0) && tempo_contatore!=0){
                int numero_ordini_caricati=0;
                Ordine** ordini_caricati = carica_ordine_sul_camion(coda_ordini_pronti,tabella_ricette,capienza,&numero_ordini_caricati);
                stampa_ordini_caricati(ordini_caricati, numero_ordini_caricati);
                            
                for (int i = 0; i < numero_ordini_caricati; i++) {
                    free(ordini_caricati[i]->nome_ricetta);
                    free(ordini_caricati[i]);
                }
                free(ordini_caricati);
            }
            
            flag=processa_ordine(tabella_ricette, magazzino, coda_ordini_attesa,coda_ordini_pronti, nome_ricetta, numero_elementi_ordinati,tempo_contatore);
            if(flag==2){
                printf("rifiutato\n");
            }else if (flag==0){
                enqueue_ordine_attesa(coda_ordini_attesa, nome_ricetta, numero_elementi_ordinati,tempo_contatore);
                /*printf("tempo:%d\n",tempo_contatore);
                printf("------ORDINI PRONTI:-------\n");
                stampa_coda_pronta(coda_ordini_pronti);
                printf("------ORDINI IN ATTESA:-------\n");
                stampa_coda(coda_ordini_attesa);
                printf("------MAGAZZINO:-------\n");
                print_all_ingredienti(magazzino);*/
                printf("accettato\n");
            }
            else {
                enqueue_ordine_pronti(coda_ordini_pronti, nome_ricetta, numero_elementi_ordinati,tempo_contatore);
                
                printf("accettato\n");
            }
            /*printf("------ORDINI PRONTI:-------\n");
            stampa_coda_pronta(coda_ordini_pronti);
            printf("------ORDINI IN ATTESA:-------\n");
            stampa_coda(coda_ordini_attesa);*/
                
            //stampa_coda_pronta(coda_ordini_pronti);
            //printf("------MAGAZZINO ord:-------\n");
            //print_all_ingredienti(magazzino);
            
            tempo_contatore++;
            //stampa_coda_pronta(coda_ordini_pronti);
        }
        else
            printf("parola non autorizzata\n");
    }
    if ((tempo_contatore%periodicita==0) && tempo_contatore!=0){
        //printf("tempo:%d\n",tempo_contatore);
        int numero_ordini_caricati=0;
        Ordine** ordini_caricati = carica_ordine_sul_camion(coda_ordini_pronti,tabella_ricette,capienza,&numero_ordini_caricati);
        stampa_ordini_caricati(ordini_caricati, numero_ordini_caricati);
                    
        for (int i = 0; i < numero_ordini_caricati; i++) {
            free(ordini_caricati[i]->nome_ricetta);
            free(ordini_caricati[i]);
        }
        free(ordini_caricati);
    }
    
    
    free_magazzino(magazzino);// Libera la memoria allocata
    free_table(tabella_ricette);  // Free all allocated memory
    
    return 0;
}

