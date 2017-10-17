#include "Header.h"
DWORD _getProcPID(char* procName) {  // Ýstenen Ýþlemin ID'sini geri veriyor. Bu id'ye ihtiyaç duyma sebebimiz iþlemi OpenProcess ile açabilmek için fonksiyonun iþlem idsine ihtiyaç duymasý.
	PROCESSENTRY32 proc; //PROCESS ENTRY 32 structure(yapý) daha fazla bilgi için: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684839(v=vs.85).aspx
	HANDLE sShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //CreateToolhelp32Snapshot var olan iþlemlerin bir nevi ekran görüntüsünü çeker, baþarýlý durumda 'ekran görüntüsüne' ait bir handle döndürür baþarýsýz olduðu zaman INVALID_HANDLE_VALUE deðerini döndürür
	if (sShot == INVALID_HANDLE_VALUE) { //eðer dönüþ deðeri hatalýysa ~ hata oluþtumu diye kontrol
		printf_s("_getProcPID Error: %d\n", GetLastError()); // getlasterror() ile en son alýnan hataya ait bir numara alýyoruz bu hata kodundan hatayý Microsoft MSDN'den bulabiliriz
		return -2;
	}
	proc.dwSize = sizeof(PROCESSENTRY32); //.dwSize üyesi PROCESSENTRY32 yapýsýnýn büyüklüðünü byte olarak temsil eder, 0 kalmasý durumunda ortada bir yapý olmaz deðil mi ? sizeof(PROCESSENTRY32) ile PROCESSENTRY32 'nin büyüklüðünü alýyoruz(detaylý bilgi için msdn) ve buraya atýyoruz.  
	Process32First(sShot, &proc); //yukarda snapshot'da aldýðýmýz bir nevi ekran görüntüsünde 1.iþlemi seçiyoruz
	do
		if (!strcmp(proc.szExeFile, procName)) //strcmp  string(yazý) compare anlamýnda düþünülebilir 2tane stringi(yazýyu) karþýlaþtýrýr ayný iseler fonksiyon 0 yanýtýný verir.
			return proc.th32ProcessID; // eðer ayný ise iþlem idsini geri döndür.
	while (Process32Next(sShot, &proc)); //yukarda snapshot'da aldýðýmýz bir nevi ekran görüntüsünde bir sonraki seçiyoruz
	return -1;
}

DWORD _getProcModule(char* _module, DWORD procPID) { //iþlemdeki bir modülün baseAddr'ini(merkez Adres'ini) döndürür. Bu belirli bir modülde okuma yapmak istiyorsak bize yardýmcý olacaktýr ayrýca iþlemin merkez modülünü bulmamýzdada yardýmcý olur.
	MODULEENTRY32 mod; //bu kýsýmdada MODULE ENTRY 32 structure(yapýsý) bunu tekrar anlatmayacaðým yukardaki process entry 32 ile ayný mantýk yine sadece bu sefer process(iþlem) deðil module(modül).
	HANDLE sShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procPID); //yine yukardaki ayný mantýk ama bu sefer modüllerin ekran görüntüsü

	if (sShot == INVALID_HANDLE_VALUE) { //yukarýdaki _getProcPID fonksiyonu ile ayný mantýk
		printf_s("_getProcModule Error: %d\n", GetLastError());
		return -2;
	}
		
	mod.dwSize = sizeof(MODULEENTRY32); //bunu tekrar anlatmayacaðým yukardaki process entry 32 ile ayný mantýk yine sadece bu sefer process(iþlem) deðil module(modül).
	Module32First(sShot, &mod);//bunu tekrar anlatmayacaðým yukardaki process entry 32 ile ayný mantýk
	do
		if (!strcmp(mod.szModule, _module)) //bunuda tekrar anlatmama gerek yok yukarýdaki fonksiyonu incele hatýrlamýyorsan.
			return (DWORD)mod.modBaseAddr; //modülün merkez adresini döndürür
		else
			printf_s("Module: %s\n", mod.szModule); //eðer aradýðýmýz modül deðilse adýný chat'e yazsýn
	while (Module32Next(sShot, &mod));//bunu tekrar anlatmayacaðým yukardaki process entry 32 ile ayný mantýk

	return 0;
}

DWORD _searchVal(int value,HANDLE hProc, DWORD startAddr = 0x400000, DWORD endAddr = 0x1FF0000) {
	int temp = 0, found = 0; static DWORD arrList[5000]; // Bulunan adresler buraya kaydedilecek , 5000 sonuçluk bir array yeterli deðilse arttýlabilir.

	for (; startAddr < endAddr; startAddr += 0x4) { //klasik for döngüsü, ++ ile arttýrmak yerine nede += 0x4 diye arttýrdým diye sorarsak, int deðiþkeni arýyoruz int deðiþkeni 4 byte büyüklüðünde bu yüzden veriyi okurken adres+4byte olarak okuyoruz o yüzden direkt 4 byte arttrýyoruz. Ör: 0x000 adresinden int deðiþkeni okuyoruz 0x000~0x003 arasýný okuyacaktýr(4byte) bu yüzden o arayý tekrar kontrol etmeye gerek yok.
		ReadProcessMemory(hProc, (LPVOID)startAddr, &temp, 4, NULL); //ReadProcessMemory adý üstünde bir fonksiyon olduðunu düþünüyorum, iþlemdeki hafýzayý okumamýzý saðlýyor. 1.Parametre hProc iþlem idsi, 2.Parametre okumasýný istediðimiz adres noktasý, 3.Parametre okulan deðerin nereye kaydedileceðidir(scanf mantýðý), 4.Parametresini okumasýný istediðimiz byte sayýsý 4 yapýyoruz çünkü int deðiþkeni okumak istiyoruz., 5.Parametre okunan bayt sayýsý kaydedileceði deðiþken(scanf mantýðý) fakat bu gereksiz buna ihtiyacýmýz yok.
		if (temp == value) { //eðer aranan deðer istediðimiz deðer ile eþleþiyorsa
			printf_s("Adding 0x%X (%d) to arrList[%d]\n", startAddr, temp, found); // deðerin bulunduðu adresi deðeri ve kaçýncý array'e eklediðimizi yazdýrýyoruz.
			arrList[found] = startAddr; //adresi array'e yazýyoruz.
			found++;//bulunan sayýsýný +1
		}
	}
	//Arama tamamlandý devam etmek istiyormuyuz diye soruyoruz.
	goto1:printf_s("%d Adresses with Value %d, Continue? Type or -1\n", found, value); //sadece goto1 i acýkalacaðým burayý bir nevi geri dönüþ noktasý olarak iþaretledik aþaðýda goto goto1; ile tekrar buraya zýplýyoruz.
	scanf_s("%d", &value); found = 0; 
	if (value > -1) { //eðer girilen deðer -1 den büyük ise aramaya devam ediyoruz.
		for (int i = 0; i < 5000; i++) { //yukardaki arraylerin içine bakýp adressleri tekrardan tarýyoruz içlerindeki deðer deðiþtimi ayný mý
			if (arrList[i] == NULL) //eðer array'in içinde bir deðer/adres yok ise
				continue; //bunu pas geçip bir sonraki döngüye geçelim.
			ReadProcessMemory(hProc, (LPVOID)arrList[i], &temp, 4, NULL); //adres içindeki deðeri okuyoruz yukarýdaki mantýkla ayný
			if (temp != value) //eðer istediðimiz yeni deðer ile ayný deðil ise
				arrList[i] = NULL; //burdaki array'in adresini 0 yapýyoruz. | Not: NULL ve 0 ayný þeydir, #define NULL 0
			else { // eðer ayný ise
				printf_s("Still Same 0x%X : %d\n", arrList[i], temp); //bu adresin hala ayný olduðuna dair yazýyoruz (sayý deðeri ile birlikte)
				arrList[found] = arrList[i]; //yukarda found = 0 yaptýk arrayi yeniden bulunan adreslerle dolduruyoruz.  
				if(found != i) //eðer bulnan ile deðilse 0 yazdýrýyoruz
					arrList[i] = NULL; //bunun sebebi ör: found == 0 ve i == 1 yaptýðýmýz þey arrList[0] = arrList[1]; ama arrList[1] hala adresi içinde tutuyor.
				found++;//bulunanlarý +1 arttýr
			}
		}
		goto goto1; //arama sonrasýnda tekrar yukarýdaki goto1 noktasýna gidiyoruz ve soruyoruz devammý? tamammý?
	}
	else
		printf_s("Returning..\n"); //kullanýcý bu kadar yeter dedi ve -1 girdi fonksiyonun bittiðine dair yazý yazdýrmaktan zarar gelmez.
	return arrList[0]; //dönüþ olarak arrList[0] ý döndürüyoruz böylece eðer istediðimiz sayýyý/deðeri bulduysak bulunduðu adres artýk elimizde.
}

int main() {
	int temp = 0; 
	DWORD pid = _getProcPID("Discord.exe");/*Discord.exe'nin iþlem id'sini alýyorum*/ DWORD baseAddr = _getProcModule("d3d9.dll", pid);/*d3d9.dll' modülünün(DirectX9) merkez adresini alýyorum Discord.exe yazýlsaydý Discord.exe nin merkez adresini alacaktýk buda genelde 0x400000 olur.*/
	printf_s("Result: Pid: 0x%X | baseAddr: 0x%X\n", pid, baseAddr); //bulduðumuz iþlem idsini ve merkez adresini bir yazdýralým bakalým hata varmý

	goto2:printf_s("Enter search value:\n"); 
	scanf_s("%d", &temp);

	HANDLE hWnd = OpenProcess(PROCESS_VM_READ, 0, pid); 
	/*1.Parametre PROCESS_VM_READ sadece okuma yapacaðýmýzý gösterir,
	2.Parametre genelde 0 olarak tutulur 1 olduðu zaman iþlem tarafýndan oluþturulan iþlemler HANDLE'ý miras alýr,
	3.Parametre yukarýda aldýðýmýziþlem idsi.  | OpenProcess bir iþlemin handle'ýný dönüþ olarak verir.
	OpenProcess hakkýnda daha çok bilgi için msdn: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684320(v=vs.85).aspx */
	DWORD addr = _searchVal(temp, hWnd); //yukarýdaki açýkladýðýmýz 'Arama' fonksiyonumuzu çaðýrýr ve geri gelen dönüþü addr deðiþkenine koyar.

	getchar(); // klavyeden bir karakter girilmesini bekler girilen karakterin int deðerini dönüþ olarak verir. Bunun yazýlma sebebi program sonuna geldiðinde direkt kapanmamasýný saðlamak.
	return 0;
}