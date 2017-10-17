#include "Header.h"
DWORD _getProcPID(char* procName) {  // �stenen ��lemin ID'sini geri veriyor. Bu id'ye ihtiya� duyma sebebimiz i�lemi OpenProcess ile a�abilmek i�in fonksiyonun i�lem idsine ihtiya� duymas�.
	PROCESSENTRY32 proc; //PROCESS ENTRY 32 structure(yap�) daha fazla bilgi i�in: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684839(v=vs.85).aspx
	HANDLE sShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //CreateToolhelp32Snapshot var olan i�lemlerin bir nevi ekran g�r�nt�s�n� �eker, ba�ar�l� durumda 'ekran g�r�nt�s�ne' ait bir handle d�nd�r�r ba�ar�s�z oldu�u zaman INVALID_HANDLE_VALUE de�erini d�nd�r�r
	if (sShot == INVALID_HANDLE_VALUE) { //e�er d�n�� de�eri hatal�ysa ~ hata olu�tumu diye kontrol
		printf_s("_getProcPID Error: %d\n", GetLastError()); // getlasterror() ile en son al�nan hataya ait bir numara al�yoruz bu hata kodundan hatay� Microsoft MSDN'den bulabiliriz
		return -2;
	}
	proc.dwSize = sizeof(PROCESSENTRY32); //.dwSize �yesi PROCESSENTRY32 yap�s�n�n b�y�kl���n� byte olarak temsil eder, 0 kalmas� durumunda ortada bir yap� olmaz de�il mi ? sizeof(PROCESSENTRY32) ile PROCESSENTRY32 'nin b�y�kl���n� al�yoruz(detayl� bilgi i�in msdn) ve buraya at�yoruz.  
	Process32First(sShot, &proc); //yukarda snapshot'da ald���m�z bir nevi ekran g�r�nt�s�nde 1.i�lemi se�iyoruz
	do
		if (!strcmp(proc.szExeFile, procName)) //strcmp  string(yaz�) compare anlam�nda d���n�lebilir 2tane stringi(yaz�yu) kar��la�t�r�r ayn� iseler fonksiyon 0 yan�t�n� verir.
			return proc.th32ProcessID; // e�er ayn� ise i�lem idsini geri d�nd�r.
	while (Process32Next(sShot, &proc)); //yukarda snapshot'da ald���m�z bir nevi ekran g�r�nt�s�nde bir sonraki se�iyoruz
	return -1;
}

DWORD _getProcModule(char* _module, DWORD procPID) { //i�lemdeki bir mod�l�n baseAddr'ini(merkez Adres'ini) d�nd�r�r. Bu belirli bir mod�lde okuma yapmak istiyorsak bize yard�mc� olacakt�r ayr�ca i�lemin merkez mod�l�n� bulmam�zdada yard�mc� olur.
	MODULEENTRY32 mod; //bu k�s�mdada MODULE ENTRY 32 structure(yap�s�) bunu tekrar anlatmayaca��m yukardaki process entry 32 ile ayn� mant�k yine sadece bu sefer process(i�lem) de�il module(mod�l).
	HANDLE sShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procPID); //yine yukardaki ayn� mant�k ama bu sefer mod�llerin ekran g�r�nt�s�

	if (sShot == INVALID_HANDLE_VALUE) { //yukar�daki _getProcPID fonksiyonu ile ayn� mant�k
		printf_s("_getProcModule Error: %d\n", GetLastError());
		return -2;
	}
		
	mod.dwSize = sizeof(MODULEENTRY32); //bunu tekrar anlatmayaca��m yukardaki process entry 32 ile ayn� mant�k yine sadece bu sefer process(i�lem) de�il module(mod�l).
	Module32First(sShot, &mod);//bunu tekrar anlatmayaca��m yukardaki process entry 32 ile ayn� mant�k
	do
		if (!strcmp(mod.szModule, _module)) //bunuda tekrar anlatmama gerek yok yukar�daki fonksiyonu incele hat�rlam�yorsan.
			return (DWORD)mod.modBaseAddr; //mod�l�n merkez adresini d�nd�r�r
		else
			printf_s("Module: %s\n", mod.szModule); //e�er arad���m�z mod�l de�ilse ad�n� chat'e yazs�n
	while (Module32Next(sShot, &mod));//bunu tekrar anlatmayaca��m yukardaki process entry 32 ile ayn� mant�k

	return 0;
}

DWORD _searchVal(int value,HANDLE hProc, DWORD startAddr = 0x400000, DWORD endAddr = 0x1FF0000) {
	int temp = 0, found = 0; static DWORD arrList[5000]; // Bulunan adresler buraya kaydedilecek , 5000 sonu�luk bir array yeterli de�ilse artt�labilir.

	for (; startAddr < endAddr; startAddr += 0x4) { //klasik for d�ng�s�, ++ ile artt�rmak yerine nede += 0x4 diye artt�rd�m diye sorarsak, int de�i�keni ar�yoruz int de�i�keni 4 byte b�y�kl���nde bu y�zden veriyi okurken adres+4byte olarak okuyoruz o y�zden direkt 4 byte arttr�yoruz. �r: 0x000 adresinden int de�i�keni okuyoruz 0x000~0x003 aras�n� okuyacakt�r(4byte) bu y�zden o aray� tekrar kontrol etmeye gerek yok.
		ReadProcessMemory(hProc, (LPVOID)startAddr, &temp, 4, NULL); //ReadProcessMemory ad� �st�nde bir fonksiyon oldu�unu d���n�yorum, i�lemdeki haf�zay� okumam�z� sa�l�yor. 1.Parametre hProc i�lem idsi, 2.Parametre okumas�n� istedi�imiz adres noktas�, 3.Parametre okulan de�erin nereye kaydedilece�idir(scanf mant���), 4.Parametresini okumas�n� istedi�imiz byte say�s� 4 yap�yoruz ��nk� int de�i�keni okumak istiyoruz., 5.Parametre okunan bayt say�s� kaydedilece�i de�i�ken(scanf mant���) fakat bu gereksiz buna ihtiyac�m�z yok.
		if (temp == value) { //e�er aranan de�er istedi�imiz de�er ile e�le�iyorsa
			printf_s("Adding 0x%X (%d) to arrList[%d]\n", startAddr, temp, found); // de�erin bulundu�u adresi de�eri ve ka��nc� array'e ekledi�imizi yazd�r�yoruz.
			arrList[found] = startAddr; //adresi array'e yaz�yoruz.
			found++;//bulunan say�s�n� +1
		}
	}
	//Arama tamamland� devam etmek istiyormuyuz diye soruyoruz.
	goto1:printf_s("%d Adresses with Value %d, Continue? Type or -1\n", found, value); //sadece goto1 i ac�kalaca��m buray� bir nevi geri d�n�� noktas� olarak i�aretledik a�a��da goto goto1; ile tekrar buraya z�pl�yoruz.
	scanf_s("%d", &value); found = 0; 
	if (value > -1) { //e�er girilen de�er -1 den b�y�k ise aramaya devam ediyoruz.
		for (int i = 0; i < 5000; i++) { //yukardaki arraylerin i�ine bak�p adressleri tekrardan tar�yoruz i�lerindeki de�er de�i�timi ayn� m�
			if (arrList[i] == NULL) //e�er array'in i�inde bir de�er/adres yok ise
				continue; //bunu pas ge�ip bir sonraki d�ng�ye ge�elim.
			ReadProcessMemory(hProc, (LPVOID)arrList[i], &temp, 4, NULL); //adres i�indeki de�eri okuyoruz yukar�daki mant�kla ayn�
			if (temp != value) //e�er istedi�imiz yeni de�er ile ayn� de�il ise
				arrList[i] = NULL; //burdaki array'in adresini 0 yap�yoruz. | Not: NULL ve 0 ayn� �eydir, #define NULL 0
			else { // e�er ayn� ise
				printf_s("Still Same 0x%X : %d\n", arrList[i], temp); //bu adresin hala ayn� oldu�una dair yaz�yoruz (say� de�eri ile birlikte)
				arrList[found] = arrList[i]; //yukarda found = 0 yapt�k arrayi yeniden bulunan adreslerle dolduruyoruz.  
				if(found != i) //e�er bulnan ile de�ilse 0 yazd�r�yoruz
					arrList[i] = NULL; //bunun sebebi �r: found == 0 ve i == 1 yapt���m�z �ey arrList[0] = arrList[1]; ama arrList[1] hala adresi i�inde tutuyor.
				found++;//bulunanlar� +1 artt�r
			}
		}
		goto goto1; //arama sonras�nda tekrar yukar�daki goto1 noktas�na gidiyoruz ve soruyoruz devamm�? tamamm�?
	}
	else
		printf_s("Returning..\n"); //kullan�c� bu kadar yeter dedi ve -1 girdi fonksiyonun bitti�ine dair yaz� yazd�rmaktan zarar gelmez.
	return arrList[0]; //d�n�� olarak arrList[0] � d�nd�r�yoruz b�ylece e�er istedi�imiz say�y�/de�eri bulduysak bulundu�u adres art�k elimizde.
}

int main() {
	int temp = 0; 
	DWORD pid = _getProcPID("Discord.exe");/*Discord.exe'nin i�lem id'sini al�yorum*/ DWORD baseAddr = _getProcModule("d3d9.dll", pid);/*d3d9.dll' mod�l�n�n(DirectX9) merkez adresini al�yorum Discord.exe yaz�lsayd� Discord.exe nin merkez adresini alacakt�k buda genelde 0x400000 olur.*/
	printf_s("Result: Pid: 0x%X | baseAddr: 0x%X\n", pid, baseAddr); //buldu�umuz i�lem idsini ve merkez adresini bir yazd�ral�m bakal�m hata varm�

	goto2:printf_s("Enter search value:\n"); 
	scanf_s("%d", &temp);

	HANDLE hWnd = OpenProcess(PROCESS_VM_READ, 0, pid); 
	/*1.Parametre PROCESS_VM_READ sadece okuma yapaca��m�z� g�sterir,
	2.Parametre genelde 0 olarak tutulur 1 oldu�u zaman i�lem taraf�ndan olu�turulan i�lemler HANDLE'� miras al�r,
	3.Parametre yukar�da ald���m�zi�lem idsi.  | OpenProcess bir i�lemin handle'�n� d�n�� olarak verir.
	OpenProcess hakk�nda daha �ok bilgi i�in msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684320(v=vs.85).aspx */
	DWORD addr = _searchVal(temp, hWnd); //yukar�daki a��klad���m�z 'Arama' fonksiyonumuzu �a��r�r ve geri gelen d�n��� addr de�i�kenine koyar.

	getchar(); // klavyeden bir karakter girilmesini bekler girilen karakterin int de�erini d�n�� olarak verir. Bunun yaz�lma sebebi program sonuna geldi�inde direkt kapanmamas�n� sa�lamak.
	return 0;
}