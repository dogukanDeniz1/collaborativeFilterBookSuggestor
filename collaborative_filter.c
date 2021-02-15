#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define nameSize 100


typedef struct user {
	char name[nameSize];
	int* scores;
}USER;

//Kitaplar� okur.
void readBooks(FILE* stream, char** books, int bookNumber);
//Kullan�c�lar� okur.
void readUsers(FILE* stream, USER* users, int bookNumber);
//Dizideki kullan�calar� ve verdi�i skorlar� yazd�r�r.
void displayUsers(USER* users, int newUserSize, int bookNumber);
//2 Kullan�c� aras�ndaki benzerlik bulunur. Benzerlik de�eri d�ner.
float sim(USER a, USER b, int bookNumber);
//1 NU nun b�t�n U lara benzerli�ini �l�er. D�zg�nce diziye yerle�tirir. User d�ner.
USER pearson(USER* users, USER* newUsers, int bookNumber, char name[nameSize], int userSize, int newUserSize, int k, float* simResults, int* simIndexes);
//Okudu�u kitaplar �zerinden ortalama d�nd�r�r.
float getAverageScore(USER u, int bookNumber);
//NU nun okumad��� kitaplar i�in tahmini puan�n� belirler.
void predict(USER* users, USER nu, char** books, int bookNumber, int k, float* simResults, int* simIndexes);

int main() {
	int i = 0, j;
	int exit = 0;
	int menu; //Menu secimi
	int bookNumber; //Kitap say�s�
	int userSize; //User say�s�
	int newUserSize; //New User say�s�
	float* simResults; //Benzerlik sonu�lar�
	int* simIndexes; //Benzerlik sonu�lar�n�na sahip kullan�c�lar�n indexleri

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
	USER* users = (USER*)malloc(userSize * sizeof(USER)); //Userler�n dizisi
	USER* newUsers = (USER*)malloc(newUserSize * sizeof(USER)); //New userlar�n dizisi

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
	//New user�n ortalama skoru
	float rnu = getAverageScore(nu, bookNumber);
	//Denklemin pay k�sm�
	float uTotal = 0.0;
	//Denklemin payda k�sm�
	float bTotal = 0.0;
	//Tahmin
	float prediction = 0.0;
	//Maximum tahmin
	float maxPrediction = 0.0;
	//�nerilecek kitab�n indexi
	int recommendedBook;
	
	for (i = 0; i < bookNumber; i++) {
		//NU nun okumad��� kitaplar� se�iyoruz.
		if (nu.scores[i] == 0) {
			uTotal = 0.0;
			bTotal = 0.0;
			//O kitaplar� okuyan kullan�c�lar i�in denklemi ger�ekle�tiriyoruz.
			for (j = 0; j < k; j++) {
				USER u = users[simIndexes[j]];	
				if(u.scores[i]!=0){
					uTotal += simResults[j] * (u.scores[i] - (getAverageScore(u, bookNumber)));
					bTotal += simResults[j];
				}	
			}
			//Tahmin sonucunu hesapl�yoruz.
			prediction = rnu + (uTotal / bTotal);
			printf("%s -> %f\n", books[i], prediction);
			//Maximum skoru bulmak i�in
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
	//simResults dizisinin hangi indexine yerle�ice�ini tutar.
	int simLastIndex = 1;
	//Verilen isimli kullan�c�y� bulur.
	while (i < newUserSize && strcmp(name, newUsers[i].name)) {
		i++;
	}
	if (i == newUserSize) {
		printf("User not found.");
		return;
	}
	else {
		//NU nun ilk U ile benzerli�i yerle�tirilir.
		USER nu = newUsers[i];
		simResults[0] = sim(nu, users[0], bookNumber);
		simIndexes[0] = 0;
		//�lk k kadar benzerlik alaca��m�z i�in geri kalanlar� diziye alm�yoruz.
		for (i = 1; i < userSize; i++) {
			// K kadar benzerlik daha �l��lmemi�se diziye direk al�n�r.
			if (simLastIndex < k) {
				simResults[simLastIndex] = sim(nu, users[i], bookNumber);
				simIndexes[simLastIndex] = i;
			}
			//Diziye k kadar benzerlik al�nm��sa ve sonuncu elemandan b�y�kse onun yerine eklenir.
			else {
				simLastIndex--;
				if (simResults[simLastIndex] < sim(nu, users[i], bookNumber)) {
					simResults[simLastIndex] = sim(nu, users[i], bookNumber);
					simIndexes[simLastIndex] = i;
				}
			}
			//Son hanedeki eleman �ncekinden b�y�kse uygun yere gelene kadar swap i�lemi ger�ekle�tirilir.
			j = simLastIndex;
			while (j > 0 && simResults[j] > simResults[j - 1]) {
				//Sonu�lar swaplan�r.
				float tmp = simResults[j];
				simResults[j] = simResults[j - 1];
				simResults[j - 1] = tmp;
				//Userlar�n indexleri swaplan�r.
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
	//A n�n ortalamas� ile B nin ortalamas�. 
	float ra = 0.0, rb = 0.0;
	int counter = 0;
	//Ortak okudu�u kitaplar �zerinden skor ortalamalar� hesapland�.
	for (i = 0; i < bookNumber; i++) {
		if (a.scores[i] != 0 && b.scores[i] != 0) {
			ra += a.scores[i];
			rb += b.scores[i];
			counter++;
		}
	}
	ra /= counter;
	rb /= counter;

	float result = 0.0; //Genel sonu�
	float uTotal = 0.0; //Denklemin pay k�sm�
	float bTotal = 0.0;	//Denklemin payda k�sm�n�n sol taraf�
	float b2Total = 0.0;//Denklemin payda k�sm�n�n sa� taraf�

	for (i = 0; i < bookNumber; i++) {
		if (a.scores[i] != 0 && b.scores[i] != 0) {
			//Ortalamadan farklar� al�n�r.
			float RA = a.scores[i] - ra;
			float RB = b.scores[i] - rb;
			//Ortalamadan farklar� �arp�larak biriktirilir.
			uTotal += (float)(RA * RB);
			//Ortalamadan farklar�n�n kareleri ayr� ayr� biriktirilir.
			bTotal += (float)(pow(RA, 2));
			b2Total += (float)(pow(RB, 2));
		}
	}
	//Payda k�sm�n�n sonucu hesaplan�r.
	bTotal = sqrt(bTotal) * sqrt(b2Total);
	//Genel sonu� hesaplan�r.
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
	int userCounter = 0; //User dizisi i�in index tutar.
	char line[1024];
	//Sat�r� al�r.
	while (fgets(line, 1024, stream) && strcmp(line, ";;;;;;;;\n"))
	{
		USER u;
		//Kitaplar i�in alan a�ar.
		u.scores = (int*)calloc(bookNumber, sizeof(int));
		char token[10] = "";
		int bookCounter = 0;
		i = 0;
		//Kitap ad�n� token de�i�kenine al�r.
		while (line[i] != ';') {
			char tmp = line[i];
			strncat(token, &tmp, 1);
			i++;
		}
		i++;
		//User yap�s�na ismini kaydederiz.
		strcpy(u.name, token);
		
		//B�t�n kitaplar�na bakana kadar devam eder.
		while (bookCounter < bookNumber) {
			//E�er art arda ;; konulmu�sa veya sat�r sonundaysa buraya gelir.
			if (line[i] == ';' || line[i] == '\n') {
				u.scores[bookCounter] = 0;
			}
			// ; ; �eklindeyse buraya gelir.
			else if (line[i] == ' ') {
				u.scores[bookCounter] = 0;
				i++;
			}
			// ;4; �eklindeyse buraya gelir.
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

