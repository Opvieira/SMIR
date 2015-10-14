#include <wiringPi.h>
#include <stdint.h>
#include <time.h>
#include <mcp3004.h>
#include <wiringPiSPI.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sqlite3.h>

FILE *fp;

char *zErrMsg = 0, data[11];
int  rc;
short int dia, mes,hora3, min3, humidade, max_humidade=39, temperatura, max_temperatura=24, poten, max_poten=100, painel, max_painel=70, tensaoDC, tensaoAC;
char flag_struct1=0, flag_struct2=0, flag_struct3=0,flag_maxhumid=1, flag_maxtemp=0, flag_maxpoten=0, flag_maxpainel=0;
char dia_mes[7], hora_struct1[7], hora_struct2[7], hora_struct3[7];
float fahr;

typedef struct {
  char tipo;//iniciar a 1
  short int dia;
  short int mes;
  short int hora;
  short int min;
}registo1_t;

typedef struct {
  char tipo;//iniciar a 2
  short int hora;
  short int min;
  short int humidade;
  short int temperatura;
} registo2_t;

typedef struct {
  char tipo;//iniciar a 2
  short int hora;
  short int min;
  short int poten;
  short int painel;
} registo3_t;

typedef struct {
  short int tipo;//iniciar a 4
  short int hora;
  short int min;
  short int tensaoDC;
  short int tensaoAC;
}registo4_t;

void criar_db()//int argc, char* argv[]) //Cria database test.db
{
	sqlite3 *db;
	rc = sqlite3_open("test.db", &db);

	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
  	exit(0);
  }
  else{
  	fprintf(stderr, "\nBase de dados criada/aberta com sucesso\n");
  }
  sqlite3_close(db);
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
  for(i=0; i<argc; i++){
  	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
  printf("\n");
  return 0;
}

void criar_tabela()//(int argc, char* argv[])
{
  sqlite3 *db;
  char *sql;
   					// Open database
  rc = sqlite3_open("test.db", &db);
  if( rc ){
  	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
  	exit(0);
  }
  else{
		fprintf(stdout, "\nBase de dados aberta com sucesso para inserir tabela\n");
 	}

// Create SQL statement
 sql ="CREATE TABLE DADOS("  \
                        	"ID INTEGER PRIMARY KEY   AUTOINCREMENT,"\
                        	"Data		TEXT 	NOT NULL," \
                        	"Hora           TEXT    NOT NULL," \
                        	"Humidade       INT     NOT NULL," \
                        	"Max_Humidade   BOLEAN  NOT NULL," \
                        	"Temperatura    INT     NOT NULL," \
                        	"Fahrenheit	INT	NOT NULL," \
                        	"Max_Temp       BOLEAN  NOT NULL," \
                        	"Potenciometro  INT     NOT NULL," \
                        	"Max_Poten      BOLEAN  NOT NULL," \
                        	"Painel         INT     NOT NULL," \
                        	"Max_Painel     BOLEAN  NOT NULL," \
                        	"TensaoAC	INT     NOT NULL," \
                        	"TensaoDC       INT     NOT NULL );
                        	";
/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
 	if( rc != SQLITE_OK ){
 		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
 	}
 	else{
  	fprintf(stdout, "Tabelas criada com sucesso\n");
  }

/* Create SQL statement --- HARDWARE */
	sql ="CREATE TABLE HARDWARE("  \
	                            "ID INTEGER PRIMARY KEY   AUTOINCREMENT,"\
	                            "Data			TEXT 	NOT NULL," \
	                            "Hora        		TEXT    NOT NULL," \
	                            "Desumificador		BOLEAN  NOT NULL," \
	                            "ArCondicionado 	BOLEAN  NOT NULL," \
	                            "Bateria_Carregar	BOLEAN  NOT NULL," \
	                            "Painel_fonte		BOLEAN  NOT NULL );
	                            ";

/* Execute SQL statement */
 	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
 	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
 	}
 	else
  	fprintf(stdout, "Tabelas criada com sucesso\n");
  
  sqlite3_close(db);
}

void inserir_dados(){
	sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
	char sqlStr[256];

/* Open database */
 	rc = sqlite3_open("test.db", &db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
 	}
 	else
 	  fprintf(stderr, "\nBase de dados aberta com sucesso para inserir dados\n");
   	
/* Create SQL statement  -------   DADOS*/
  if (flag_maxhumid==0){
	  if (flag_maxtemp==0){
		  if (flag_maxpoten==0){
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'FALSE', %d, 'FALSE', %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'FALSE', %d, 'FALSE' , %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
		  else{
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'FALSE', %d, 'TRUE', %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'FALSE', %d, 'TRUE' , %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
	  }
	  else{
		  if (flag_maxpoten==0){
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'TRUE', %d, 'FALSE' , %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'TRUE', %d, 'FALSE' , %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
		  else{
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'TRUE', %d, 'TRUE' , %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoDC, TensaoAC) VALUES (%s, %s, %d, 'FALSE', %d, %2.1f, 'TRUE', %d, 'TRUE' , %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
	  }
  }
  else{
	  if (flag_maxtemp==0){
		  if (flag_maxpoten==0){
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s , %s, %d, 'TRUE' , %d, %2.1f, 'FALSE', %d, 'FALSE' , %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s , %s, %d, 'TRUE' , %d, %2.1f, 'FALSE', %d, 'FALSE' , %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
		  else{
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s , %s, %d, 'TRUE' , %d, %2.1f, 'FALSE', %d, 'TRUE' , %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s , %s, %d, 'TRUE' , %d, %2.1f, 'FALSE', %d, 'TRUE' , %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
	  }
	  else{
		  if (flag_maxpoten==0){
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'TRUE', %d, %2.1f, 'TRUE', %d, 'FALSE', %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr,  poten, painel, tensaoAC, tensaoDC);
			  else
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'TRUE', %d, %2.1f, 'TRUE', %d, 'FALSE', %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
		  else{
			  if (flag_maxpainel==0)
				  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'TRUE', %d, %2.1f, 'TRUE', %d, 'TRUE', %d, 'FALSE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
			  else
			  sprintf(sqlStr, "INSERT INTO DADOS(Data, Hora, Humidade, Max_Humidade, Temperatura, Fahrenheit, Max_Temp, Potenciometro, Max_Poten, Painel, Max_Painel, TensaoAC, TensaoDC) VALUES (%s, %s, %d, 'TRUE', %d, %2.1f, 'TRUE', %d, 'TRUE', %d, 'TRUE', %d, %d)", dia_mes, hora_struct3, humidade, temperatura, fahr, poten, painel, tensaoAC, tensaoDC);
		  }
	  }
  }
/* Execute SQL statement */
	rc = sqlite3_exec(db, sqlStr, callback, 0, &zErrMsg);
  if( rc != SQLITE_OK ){
  	fprintf(stderr, "SQL error: %s\n", zErrMsg);
  	sqlite3_free(zErrMsg);
  }
  else
		fprintf(stdout, "Tabela dados gravados com sucesso\n");

/* Create SQL statement  -------  HARDWARE   */
  if (flag_maxhumid==1){
	  if (flag_maxtemp==1){
		  if (flag_maxpoten==1){
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'TRUE', 'FALSE','TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'TRUE', 'FALSE', 'FALSE')", dia_mes, hora_struct3);
			  }
		  else{
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'TRUE', 'TRUE','TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'TRUE', 'TRUE','FALSE')", dia_mes, hora_struct3);
		  }
	  }
	  else{
		  if (flag_maxpoten==1){
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'FALSE', 'FALSE','TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'FALSE', 'FALSE','FALSE')", dia_mes, hora_struct3);
	  	}
		  else{
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE', 'FALSE', 'TRUE','TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'TRUE	', 'FALSE', 'TRUE', 'FALSE')", dia_mes, hora_struct3);
		  }
	  }
  }
  else{
	  if (flag_maxtemp==1){
		  if (flag_maxpoten==1){
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'TRUE', 'FALSE', 'TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'TRUE', 'FALSE', 'FALSE')", dia_mes, hora_struct3);
		  }
		  else{
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'TRUE', 'TRUE', 'TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'TRUE', 'TRUE', 'FALSE')", dia_mes, hora_struct3);
		  }
	  }
	  else{
		  if (flag_maxpoten==1){
			  if (flag_maxpainel==1)
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'FALSE', 'FALSE', 'TRUE')", dia_mes, hora_struct3);
			  else
				  sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'FALSE', 'FALSE', 'FALSE')", dia_mes, hora_struct3);
		  }
		  else{
			  if (flag_maxpainel==1)
	  			sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'FALSE', 'TRUE', 'TRUE')", dia_mes, hora_struct3);
		  	else
			  	sprintf(sqlStr, "INSERT INTO HARDWARE(Data, Hora, Desumificador, ArCondicionado, Bateria_Carregar, Painel_fonte) VALUES (%s , %s, 'FALSE', 'FALSE', 'TRUE', 'FALSE')", dia_mes, hora_struct3);
		  } 
  	}
  }
/* Execute SQL statement */
	rc = sqlite3_exec(db, sqlStr, callback, 0, &zErrMsg);
 	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
 	}
 	else
		fprintf(stdout, "Tabela hardware gravados com sucesso\n");
   	
 	sqlite3_close(db);
}

/***********************************MOSTRAR BASE DE DADOS***********************************/
void selecionar_dados(){
  sqlite3 *db;
  char *sql;
  const char* data = "Callback function called";

/* Open database */
	rc = sqlite3_open("test.db", &db);
 	if( rc ){
		fprintf(stderr, "Não foi possivel abrir base de dados: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else
  	fprintf(stderr, "Base de dados aberta com sucesso!\n");

/* Create SQL statement */
	printf("====================\nTabela de Dados Lidos\n");
	sql= "SELECT * from DADOS";

/* Execute SQL statement */
  rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
  if( rc != SQLITE_OK ){
  	fprintf(stderr, "SQL error: %s\n", zErrMsg);
  	sqlite3_free(zErrMsg);
	}
	else
	  fprintf(stdout, "Tabela Completa\n====================\n");
   	
/* Create SQL statement */
  printf("====================\nTabela de Hardware\n");
  sql= "SELECT * from HARDWARE";

/* Execute SQL statement */
  rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  else
    fprintf(stdout, "Tabela Completa\n====================\n");

 	sqlite3_close(db);
}

/***********************************************LÊ DO FICHEIRO E GUARDA NA BD***************************************************************/
void guardar()
{
  registo1_t r,r1;
  registo2_t r2;
  registo3_t r3;
	registo4_t r4;

	fp=fopen("file.dat","r");

	while((fread(&r,sizeof(registo1_t),1,fp))!=0)
  {
    switch(r.tipo)
		{
      case 1: ; //deixar estar os ponto e virgula (;) a seguir aos case (tens de ter em caso de declaraçao de variavel a seguir ao case)
			        memcpy(&r1,&r,sizeof(registo1_t));
			        sprintf(dia_mes, "%02d.%02d", r1.dia, r1.mes);
			        sprintf(hora_struct3, "%02d.%02d", r3.hora, r3.min);
			        //flag_struct1=1;
			        break;

     case 2: ;
              memcpy(&r2,&r,sizeof(registo1_t));
              sprintf(hora_struct3, "%02d.%02d", r3.hora, r3.min);
        			humidade=r2.humidade;
	            fahr=(r2.temperatura*1.8+32);
			        temperatura=r2.temperatura;
			        //flag_struct2=1;
			        break;

		  case 3: ;
			        memcpy(&r3,&r,sizeof(registo1_t));
			        sprintf(hora_struct3,"%02d.%02d", r3.hora, r3.min);
			        poten=(r3.poten*100)/255;
			        painel=(r3.painel*50)/255;
			        //flag_struct3=1;
			        break;

  		case 4: ;
        			memcpy(&r4,&r,sizeof(registo1_t));
	 		        sprintf(hora_struct3,"%02d.%02d", r4.tensaoDC, r4.tensaoAC);
	         		tensaoDC=(r4.tensaoDC*12)/255;
        	    tensaoAC=(r4.tensaoAC*50)/255;
              //flag_struct4=1;
			        break;
    }
//		ant_tipo=r.tipo;

		if(flag_struct3==1)
		{
      if (r2.humidade<max_humidade || r2.humidade==max_humidade)
        flag_maxhumid=0;
      else
        flag_maxhumid=1;

	   if (r2.temperatura<max_temperatura || r2.temperatura==max_temperatura)
    	flag_maxtemp=0;
	   else
    	flag_maxtemp=1;

	   if (r3.poten<max_poten || r3.poten==max_poten)
    	flag_maxpoten=0;
	   else
    	flag_maxpoten=1;

      if (r3.painel<max_painel || r3.painel==max_painel)
    	  flag_maxpainel=0;
      else
      	flag_maxpainel=1;

	  inserir_dados();

		flag_struct1=0;
    flag_struct2=0;
		flag_struct3=0;
		}
	}
  fclose(fp);
}

char op_menu() {
  char op;
  printf("\nEscolha uma opção:\n");
  printf("1-Mostrar base de dados\n");
  printf("2-Preencher a base de dados\n");
	printf("0-Sair\n");
  op=getchar();
  getchar();
  return op;
}

int main(void)
{
  char opcao;
  printf("SMIR - Servidor de Monitorização e Intervenção Remoto\n");
  if(wiringPiSetup()==-1)
    exit(1);
  criar_db();
	criar_tabela();
  while(1)
  {
	  opcao=op_menu();
    switch (opcao)
    {
      case '0': printf("\nO programa terminado\n\n");
                exit(1);

      case '1': selecionar_dados();
					      break;

      case '2': guardar();
			          break;

      default:  printf("\n Comando invalido");
                getchar();
                break;
    }
  }
}
