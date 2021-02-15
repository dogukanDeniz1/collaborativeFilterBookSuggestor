#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define nameSize 100


typedef struct user {
	char name[nameSize];
	int* scores;
}USER;

//Kitaplarý okur.
void readBooks(FILE* stream, char** books, int bookNumber);
//Kullanýcýlarý okur.
void readUsers(FILE* stream, USER* users, int bookNumber);
//Dizideki kullanýcalarý ve verdiði skorlarý yazdýrýr.
void displayUsers(USER* users, int newUserSize, int bookNumber);
//2 Kullanýcý arasýndaki benzerlik bulunur. Benzerlik deðeri döner.
float sim(USER a, USER b, int bookNumber);
//1 NU nun bütün U lara benzerliðini ölçer. Düzgünce diziye yerleþtirir. User döner.
USER pearson(USER* users, USER* newUsers, int bookNumber, char name[nameSize], int userSize, int newUserSize, int k, float* simResults, int* simIndexes);
//Okuduðu kitaplar üzerinden ortalama döndürür.
float getAverageScore(USER u, int bookNumber);
//NU nun okumadýðý kitaplar için tahmini puanýný belirler.
void predict(USER* users, USER nu, char** books, int bookNumber, int k, float* simResults, int* simIndexes);

int main() {
	int i = 0, j;
	int exit = 0;
	int menu; //Menu secimi
	int bookNumber; //Kitap sayýsý
	int userSize; //User sayýsý
	int newUserSize; //New User sayýsý
	float* simResults; //Benzerlik sonuçlarý
	int* simIndexes; //Benzerlik sonuçlarýnýna sahip kullanýcýlarýn indexleri

	FILE* stream = fopen("recdataset.csv", "r");
	if (!stream) {
		printf("File cannot openned.");
		return 0;
	}
	fscanf(stream, "%d\n", &bookNumber);
	fscanf(stream, "%d\n", &userSize);
	fscanf(stream, "%d\n", &newUserSize);

	char** books = (char**)calloc(bookNumber, sizeof(char*)); //Kitap isimleri
	for (i = 0; i < bookNumber; i++) {
		books[i] = (char*)calloc(nameSize, sizeof(char));
	}
	USER* users = (USER*)malloc(userSize * sizeof(USER)); //Userlerýn dizisi
	USER* newUsers = (USER*)malloc(newUserSize * sizeof(USER)); //New userlarýn dizisi

	readBooks(stream, books, bookNumber);
	readUsers(stream, users, bookNumber);
	readUsers(stream, newUsers, bookNumber);
	printf("Users are read.\n");
	while (!exit) {
		printf("\n  1. Get Recomendation.\n  2. Display all recomendations for new users.\n  3.Display users.\n  4.Display new users.\n  5.Exit");
		scanf("%d", &menu);
		system("CLS");
		if (menu == 1) {
			char nu[nameSize];
			int k;
			printf("Enter new user name (like NU1) : ");
			scanf("%s", nu);
			printf("Enter similiar user number (k) : ");
			scanf("%d", &k);
			system("CLS");
			simResults = (float*)calloc(k, sizeof(float));
			simIndexes = (int*)calloc(k, sizeof(int));
			USER u = pearson(users, newUsers, bookNumber, nu, userSize, newUserSize, k, simResults, simIndexes);
			predict(users, u, books, bookNumber, k, simResults, simIndexes);
			free(simIndexes);
			free(simResults);
		}
		else if (menu==2){
			for(i=0;i<newUserSize;i++){
				printf("\n**********************************************\n");
				simResults = (float*)calloc(3, sizeof(float));
				simIndexes = (int*)calloc(3, sizeof(int));
				USER u = pearson(users, newUsers, bookNumber, newUsers[i].name, userSize, newUserSize, 3, simResults, simIndexes);
				predict(users, u, books, bookNumber, 3, simResults, simIndexes);
				free(simIndexes);
				free(simResults);
				printf("\n**********************************************\n");
			}
		}
		else if (menu == 3) {
			displayUsers(users, userSize, bookNumber);
		}
		else if (menu == 4) {
			displayUsers(newUsers, newUserSize, bookNumber);
		}
		else {
			exit = 1;
		}
	}
	return 0;
}

void predict(USER* users, USER nu, char** books, int bookNumber, int k, float* simResults, int* simIndexes) {
	int i, j;
	//New userýn ortalama skoru
	float rnu = getAverageScore(nu, bookNumber);
	//Denklemin pay kýsmý
	float uTotal = 0.0;
	//Denklemin payda kýsmý
	float bTotal = 0.0;
	//Tahmin
	float prediction = 0.0;
	//Maximum tahmin
	float maxPrediction = 0.0;
	//Önerilecek kitabýn indexi
	int recommendedBook;
	
	for (i = 0; i < bookNumber; i++) {
		//NU nun okumadýðý kitaplarý seçiyoruz.
		if (nu.scores[i] == 0) {
			uTotal = 0.0;
			bTotal = 0.0;
			//O kitaplarý okuyan kullanýcýlar için denklemi gerçekleþtiriyoruz.
			for (j = 0; j < k; j++) {
				USER u = users[simIndexes[j]];	
				if(u.scores[i]!=0){
					uTotal += simResults[j] * (u.scores[i] - (getAverageScore(u, bookNumber)));
					bTotal += simResults[j];
				}	
			}
			//Tahmin sonucunu hesaplýyoruz.
			prediction = rnu + (uTotal / bTotal);
			printf("%s -> %f\n", books[i], prediction);
			//Maximum skoru bulmak için
			if(maxPrediction<prediction){
				maxPrediction = prediction;
				recommendedBook = i;
			}
		}
	}
	printf("\n   Recommended book is %s", books[recommendedBook]);
}

float getAverageScore(USER u, int bookNumber) {
	int i, j;
	float avg = 0.0;
	int counter = 0;
	for (i = 0; i < bookNumber; i++) {	
		if(u.scores[i] != 0){
			avg += u.scores[i];
			counter++;
		}
	}
	avg /= counter;
	return avg;
}

USER pearson(USER* users, USER* newUsers, int bookNumber, char name[nameSize], int userSize, int newUserSize, int k, float* simResults, int* simIndexes) {
	int i = 0, j;
	//simResults dizisinin hangi indexine yerleþiceðini tutar.
	int simLastIndex = 1;
	//Verilen isimli kullanýcýyý bulur.
	while (i < newUserSize && strcmp(name, newUsers[i].name)) {
		i++;
	}
	if (i == newUserSize) {
		printf("User not found.");
		return;
	}
	else {
		//NU nun ilk U ile benzerliði yerleþtirilir.
		USER nu = newUsers[i];
		simResults[0] = sim(nu, users[0], bookNumber);
		simIndexes[0] = 0;
		//Ýlk k kadar benzerlik alacaðýmýz için geri kalanlarý diziye almýyoruz.
		for (i = 1; i < userSize; i++) {
			// K kadar benzerlik daha ölçülmemiþse diziye direk alýnýr.
			if (simLastIndex < k) {
				simResults[simLastIndex] = sim(nu, users[i], bookNumber);
				simIndexes[simLastIndex] = i;
			}
			//Diziye k kadar benzerlik alýnmýþsa ve sonuncu elemandan büyükse onun yerine eklenir.
			else {
				simLastIndex--;
				if (simResults[simLastIndex] < sim(nu, users[i], bookNumber)) {
					simResults[simLastIndex] = sim(nu, users[i], bookNumber);
					simIndexes[simLastIndex] = i;
				}
			}
			//Son hanedeki eleman öncekinden büyükse uygun yere gelene kadar swap iþlemi gerçekleþtirilir.
			j = simLastIndex;
			while (j > 0 && simResults[j] > simResults[j - 1]) {
				//Sonuçlar swaplanýr.
				float tmp = simResults[j];
				simResults[j] = simResults[j - 1];
				simResults[j - 1] = tmp;
				//Userlarýn indexleri swaplanýr.
				int temp = simIndexes[j];
				simIndexes[j] = simIndexes[j - 1];
				simIndexes[j - 1] = temp;
				j--;
			}
			if (simLastIndex < k) {
				simLastIndex++;
			}
		}
		printf("Closest users for %s : \n", name);
		for (i = 0; i < k; i++) {
			printf("  %2d. %5s - %10f\n", i + 1, users[simIndexes[i]].name, simResults[i]);
		}
		return nu;
	}
	return;
}


float sim(USER a, USER b, int bookNumber) {
	int i;
	//A nýn ortalamasý ile B nin ortalamasý. 
	float ra = 0.0, rb = 0.0;
	int counter = 0;
	//Ortak okuduðu kitaplar üzerinden skor ortalamalarý hesaplandý.
	for (i = 0; i < bookNumber; i++) {
		if (a.scores[i] != 0 && b.scores[i] != 0) {
			ra += a.scores[i];
			rb += b.scores[i];
			counter++;
		}
	}
	ra /= counter;
	rb /= counter;

	float result = 0.0; //Genel sonuç
	float uTotal = 0.0; //Denklemin pay kýsmý
	float bTotal = 0.0;	//Denklemin payda kýsmýnýn sol tarafý
	float b2Total = 0.0;//Denklemin payda kýsmýnýn sað tarafý

	for (i = 0; i < bookNumber; i++) {
		if (a.scores[i] != 0 && b.scores[i] != 0) {
			//Ortalamadan farklarý alýnýr.
			float RA = a.scores[i] - ra;
			float RB = b.scores[i] - rb;
			//Ortalamadan farklarý çarpýlarak biriktirilir.
			uTotal += (float)(RA * RB);
			//Ortalamadan farklarýnýn kareleri ayrý ayrý biriktirilir.
			bTotal += (float)(pow(RA, 2));
			b2Total += (float)(pow(RB, 2));
		}
	}
	//Payda kýsmýnýn sonucu hesaplanýr.
	bTotal = sqrt(bTotal) * sqrt(b2Total);
	//Genel sonuç hesaplanýr.
	result = uTotal / bTotal;
	return result;
}

void displayUsers(USER* users, int newUserSize, int bookNumber) {
	int i, j;
	for (i = 0; i < newUserSize; i++) {
		printf(" %4s - ", users[i].name);
		for (j = 0; j < bookNumber; j++) {
			printf(" %d,", users[i].scores[j]);
		}
		printf("\n");
	}
}


void readUsers(FILE* stream, USER* users, int bookNumber) {
	int i = 0;
	int userCounter = 0; //User dizisi için index tutar.
	char line[1024];
	//Satýrý alýr.
	while (fgets(line, 1024, stream) && strcmp(line, ";;;;;;;;\n"))
	{
		USER u;
		//Kitaplar için alan açar.
		u.scores = (int*)calloc(bookNumber, sizeof(int));
		char token[10] = "";
		int bookCounter = 0;
		i = 0;
		//Kitap adýný token deðiþkenine alýr.
		while (line[i] != ';') {
			char tmp = line[i];
			strncat(token, &tmp, 1);
			i++;
		}
		i++;
		//User yapýsýna ismini kaydederiz.
		strcpy(u.name, token);
		
		//Bütün kitaplarýna bakana kadar devam eder.
		while (bookCounter < bookNumber) {
			//Eðer art arda ;; konulmuþsa veya satýr sonundaysa buraya gelir.
			if (line[i] == ';' || line[i] == '\n') {
				u.scores[bookCounter] = 0;
			}
			// ; ; þeklindeyse buraya gelir.
			else if (line[i] == ' ') {
				u.scores[bookCounter] = 0;
				i++;
			}
			// ;4; þeklindeyse buraya gelir.
			else {
				char tmp = line[i];
				u.scores[bookCounter] = (int)tmp - 48;
				i++;
			}
			i++;
			bookCounter++;
		}
		//Users dizisinde biriktiririz.
		users[userCounter] = u;
		userCounter++;
	}
}

void readBooks(FILE* stream, char** books, int bookNumber) {
	int i = 0;

	char line[1024];
	fgets(line, 1024, stream);
	char* token = strtok(line, ";");
	token = strtok(NULL, ";");
	while (token != NULL) {
		strcpy(books[i], token);
		if (i == bookNumber - 2) {
			token = strtok(NULL, ";\n");
		}
		else {
			token = strtok(NULL, ";");
		}
		i++;
	}
}

