Nume:  Lungu-Stan Vlad-Constantin
Grupă: 334CB

# Tema 4

Organizare
-
1. Explicație pentru structura creată (sau soluția de ansamblu aleasă):


~~~~~~~~~~~~~~~~~~~~~~~~
     	STRUCTURI
~~~~~~~~~~~~~~~~~~~~~~~~

	Structura principală se găsește în fișierul so_scheduler_def.h, construit din cauza unor dependențe circulare
care făceau imposibilă compilarea. Acesta conține toate elementele necesare funcționării schedulerului, cu tot cu
bonusuri:
-time_quantum/supported_io_num, datele problemei
-MUTEX, un mutex folosit pentru excluderea mutuala cand se acceseaza resursele schedulerului
-ready_queue, cozile de așteptare pentru threadurile aflate în starea READY
-wait_queu, cozile de așteptare pentru threadurile aflate în starea WAITING
-current_thread, un pointer către structura corespunzătoare threadului de pe procesor
-thread_hash, o tabelă hash care face legătura între tid-uri și structurile mele de threaduri
-all_threads, o coadă auxiliară folosită pentru join-ul threadurilor
-enable_log, o variabilă care marchează folisirea mecanicii de log
-logger, un pointer către structura loggerului corespunzător schedulerului curent
-use_fcfs, un flag care îmi spune dacă este folosit algoritmul First Come First Served în loc de Round Robin

	Threadurile au asociată câte o structură de tipul so_thread, care conține resurse necesare pentru funcționarea
corectă a schedulerului. În primul rând am două semafoare, unul folosit de către threadul părinte pentru a aștepta
ajungerea threadului copil în starea ready, fork_sem, și unul pentru realizarea schedulingului propriu zis, numit
generic semaphore. Structura mai ține tid-ul, statusul, cuanta de timp curentă, prioritatea și handlerul către
funția pe care trebuie s-o execute.
	
	Folosesc ca structuri de date cozi simplu înlănțuite și hash-uri obișnuite, construite cu ajutorul cozilor
menționate anterior.

	Pentru bonus, am structurile so_logger, care are o coadă de lungime maximă setată de utilizator și un fișier de
log asociat și sched_info, care conține datele necesare configurării schedulerului și a loggerului.

	Fiecare program are un scheduler global, static. 

~~~~~~~~~~~~~~~~~~~~~~~~
	FUNCȚIILE TEMEI
~~~~~~~~~~~~~~~~~~~~~~~~
	Totul pornește din so_init. După ce este alocată structura de scheduler, se apelează funcția use_file, care 
deschide fișierul de configurație. Pentru ca nu pot adăuga vreun parameetru funcției so_init prin care să controlez
utilizarea sau nu a fișierului de configurație, primul element din el este un flag ce are valoarea 0 sau 1 prin care
știu dacă este sau nu folosit. Preventiv, citesc tot conținutul fișierului și populez o structură sched_info. Dacă
este foloit fișierul de configuraree, modific parametrii schedulerului conform acestuia, ignorând parametrii primiți
la inițializare, și, dacă este cazul, inițializez loggerul. După aceea afișez în consolă mesaje corespunzătoare dacă
a fost activată vreo componentă importantă. După aceea, știind parametrii cu care funcționează programul, inițializez
structurile care depind de aceștia.
	Sora acestei funcții este so_end. , în care dezaloc și distrug toate structurile create în schedulerul curent.
Prima dată, însă, această funcție se asigură că programul s-a terminat făcând join pe fiecare thread în parte. Aici
folosesc coada all_threads. În momentul în care un thread e creat, înainte de a-și începe execuția, acesta își
introduce structura în coada all_threads(nu tid-ul, pentru că a mea coadă ține so_struct). Dacă acesta face fork, va
aștepta ca fiul să se inițializeze înainte să treacă mai departe. Fiul își va introduce propria structură
în coadă înainte să deblocheze părintele. Astfel, am garanția că orice thread care este creat după el va fi în coadă
după el. so_end face join, astfel, pe rând la toate threadurile din program, aasigurând un flux de execuție corect. 
După ce am terminat cu joinurile pot fi sigur că programul poate fi terminat cu succes.
	Acestea două sunt folosite de threadul principal, care dă drumul programului. Celelalte funcții sunt folosite în
principal de threadurilor create cu fork.
	Pentru a crea un thread nou se apelează funcția so_fork. După ce verific validitatea parametrilor primiți creez o
nouă structură de tip so_thread pe care o inițializez și îmi caut în tabela hash structura corespunzătoare. Aici e o 
smecherie. Primul thread care face fork nu trebuie inclus în programul de scheduling. Reușesc să fac diferențierea
prin valoarea întoarsă de hash. Un singur thread, primul, nu va avea corespondent în tabelă. Mă pot folosi de valoarea
HASH_VALUE_NOT_FOUN(NULL) întoarsă pentru a sări peste etapa de așteptare, specifică fiecărei instrucțiuni. După aceea
așteept ca fiul meu să intre în starea ready  și finalizez prin check_scheduler și wait(dacă e cazul) funcția.
	La crearea threadului apelez funcția thread_function_wrapper, care populează structurile potrivite cu noul thread
(inserează în hash, în coada corespunzătoare de ready și în coada pentru join). Acum știu că threadul meu se găsește
în starea readă și dau drumul părintelui, aștepându-mi rândul prin așteptarea la semaforul corespunzător threadului,
semaphore. Când îmi vine rândul, apelez funcția handler. Astfel, orice funcție din so_scheduler aș apela, am garanția
că atunci când încep execuția unei funcții, sunt unicul thread de pe procesor. Fiecare funcție urmărește, astfel, un
pattern. Intru în funcție, fac ce e de făcut și găsesc structura so_thread care-mi corespunde prin interograrea
tabelei hash cu tid-ul threadului curent(pe care  apoi apelez funcțiile specifice de sistem). Având această informație,
apelez check_scheduler pentru a vedea cine urmează pe procesor. După terminarea rutinei dată de handler știu că threadul
meu este terminat și marchez acest lucru în statusul asociat. Fiind terminat, scot din hash asocierea dintre tid și
structura mea(pentru că tid-urile se refolosesc) și dau drumul mai departe altui proces prin apelul funcției 
check_scheduler.
	Celelalte funcții sunt simple și urmăresc patternul de mai sus. So exec realmente nu face mai nimic( nu am știu ce
do-work să pun, așa că am lăsat gol), da drumul semaforului următorului thread și așteaptă.
	so_wait marcheză starea de waiting, intră în coada corespunzătoare, cedează procesorul  și așteaptă să fie trezită. 
	so_signal scoate toate elementele din coada evenimentului și le pune în cozile ready.
	
	O funcție importantă este check_scheduler. Aceasta este apelată la finalul fiecărei instrucțiuni și reprezintă
partea care face realmente scheduling-ul. Are drept parametru structura threadului curent. Dacă este apelată de 
threadul inițial sau de către un thread terminat, planifică imediat următorul proces de drept. Altfel verifică dacă
a expirat cuanta de timp. Dacă nu, rămân pe procesor și îmi incrementez cuanta de timp. Dacă da, resetez cuanta, intru
în starea ready și ma introduc în coada de așteptare corespunzătoare. Noul thread este marcat ca fiind RUNNING și
semaforul său este incrementat, trezind, practic, threadul potrivit.
	
~~~~~~~~~~~~~~~~~~~~~~~~
	   ROUND ROBIN
~~~~~~~~~~~~~~~~~~~~~~~~
	Pentru implementarea algoritmului de scheduling propriu zis folosesc cozi de așteptare. Numărul de priorități fiind
garantat mic(0-5), este mai eficient să împart threadurile în pooluri(implementate prin cozi) de priorități. Când un 
thread intră în starea READY este plasat în coada priorității pe care o are. Când este căutat următorul thread care va
rula pe procesor, se parcurg cozile de la cea cu prioritate cea mai mare la cea cu prioritate cea mai mică. Primul 
thread întâlnit este automat cel cu prioritate cea mai mare, care a așteptat cel mai mult. Acesta este planificat pe
procesor. Astfel, algoritmul este preemptiv(un thread nu e lăsat să ruleze pe procesor până se termină cu ajutorul
cuantei de timp) și cât de cât echitabil(dacă un thread de aceeași prioritatee așteaptă mai mult decât mine, va fi 
băgat mai repede pe procesor).

~~~~~~~~~~~~~~~~~~~~~~~~
		BONUS
~~~~~~~~~~~~~~~~~~~~~~~~
	Pentru această temă am implementat toate bonusurile propuse. În primul rând codul este multi-platform linux-windows.
Diferențele dintre cele două platforme sunt mici( în principiu doar structurile de sincronizare). Cu define-uri potrivite
și funcții wrapper, găsite la începutul fișierului so_scheduler.c și so_scheduler.h, plasate în ifdef-uri, am reușit să
fac acest lucru. Desigur, am două makefile-uri, fix ca la prima temă.
	Totodată, așa cum am menționat și mai sus, schedulerul poate fi configurat prin editarea unui fișier, numit .config,
găsit în arhivă. Acesta poate fii editat atât de mână, cât și prin programul config_params, care întreabă utiliztorul ce
valoare preferă pentru fiecare parametru în parte și actualizează conținutul fișierului. Structura sa e următoarea:
Pe fiecare linie se găsește câte un parametru. Primul spune daca fișierul de configurare este folosit(1) sau nu. Urmează 
cuanta de timp maximă permisă de scheduler și numărul maxim de dispozitive. După aceea vine din nou un flag care spune
dacă este(1) sau nu folosit mecanismul de logging, urmat de numărul de mesaje de log pe care utilizatorul le vrea. 
În cele din urmă se află flagul care îmi spune dacă algoritmul de scheduling rămâne Round Robin(0), sau este înlocuit
de First Come First Served(1). 
	De asemenea, am inclus mecanica de logging. Aceasta poate fi folosită prin utilizarea fișierului de configurare și
setarea flagului corespunzător pe 1. Tot din fișier ia dimensiunea ferestrei. Loggerul păstrează într-o coadă un numar
maxim de window_size mesaje de tipul char pointer. Când numarul de mesaje este depășit, elimin cel mai vechi mesaj
pentru a păstra numărul de mesaje în intervalul dorit. Astfel, obțin o fereastră de mesaje maximă și, în consecință, un
fișier de logging de dimensiune limtată. Acesta din urmă este scris la distrugerea loggerului, prin apelul funcției
flush_log. Acest logger permite mesaje standard pentru fork, temrinate, preempt, exec, wait și signal, dar și mesaje
custom prin intermediul lui log_error. Aceste funcții sunt apelate în locuri bine alese de către so_scheduler, însă doar
dacă flagul de logging este activat. 
	Nu în cele din urmă am impleemeetat și un alt algoritm de scheduling, anume First Come First Served pentru sistemele
cu task-uri care nu se bazează pe interacțiune cu utilizatorul și nu au nevoie de fairness. Astfel, primul proces care vine
își face toată treaba și apoi îl lasă pe următorul. Singura excepție e când așteaptă după vreun eveniment. Pentru 
implementarea lui mă folosesc de aceleași funcții și resurse ca în cazul lui Round Robin, dar puțin schimbat. Astfel, în
loc de SO_MAX_PRIO cozi, mă folosesc de una singură, de coada 0. Cuanta de timp e setată artificial la 1, adică se face
mereu ”un pas”, care conșine o grămadă de operații. De aștepat la semafoare aștept imediat după ce am fost creat sau 
când fac wait. Dau drumul mai departe altor procese fie când aștept, fie când am terminat. Astfel, coada 0 păstreează
ordinea în care  procesele cer accesul pe procesor. Fiecare proces își așteaptă  rândul, ruleează cât poate de mult și
dă drumul mai departe următorului proces din coadă. 
	Schimbările la cod sunt aproap nonexistente. Diferența dintre algoritmi este făcută cu if-uri a căror activitate e
guvernată de flagul use_fcfs a schedulerului, la inserările în coadă, așteptarea la semafor și apelul check_scheduler.

~~~~~~~~~~~~~~~~~~~~~~~~
		EFICIENȚĂ
~~~~~~~~~~~~~~~~~~~~~~~~ 
	Algoritmul meu este unul eficient. Numărul mic de posibile priorități duce la un timp mic de alegere a următorului 
candidat, O(5), pentru că inserările și extragerile din cozi se fac în timp constant, O(1). De asemenea, interogările
tabelei hash sunt în O(1) amortizat. Cheile sunt tid-urile, unsigned long-uri, iar funcția de hash e tid%dimensiune_hash,
care e un număr prim. Astfel, dispersia este una bună(nu optimă, dar nu asta căutam). De asemena, regiunile critice sunt
în general mici și concurența e dată în principal de threadul curent care rulează și threadul principal care face
ocazional joinuri. De aceea, abordarea mi se pare mie una eficientă.
	De menționat este că și în cazul în care SO_MAX_PRIO este mai mare, abordarea rămâne tot una bună. Uzual prioritățile
sunt de câteva zeci. Tot cred că e mai bine să am O(1) pe operație decât un O(logN).

~~~~~~~~~~~~~~~~~~~~~~~~
	CERUTE MAI JOS
~~~~~~~~~~~~~~~~~~~~~~~~
	Da, consider că tema este utilă, plăcută și interesantă. A fost o plăcere să mă bat cu ea și să găsesc o modalitate
,zic eu, elegantă de rezolvare. 
	Consider implementarea eficientă. Structuri de date mai bune pentru problemă nu pot implementa(O(1)). Singura parte
care poate fi îmbunătățită este dimensiune tabelei hash. Cu puțină cercetaree se poate determina o valoare care să ducă
la o dispersie superioară( eu folosesc 1013 drept număr prim. E mic și sigur duce la coliziuni. N-am vrut să folosesc
ceva de tipul 66607 pentru că tema asta e mai mult demonstrativă).
	Corner case-uri nu cred că am :-?.
	Menționez că întreg enunțul temei este implementat și că obține punctajul maxim pe vmchecker.
	Funcționalitățile extra le-am detaliat mai sus, în regiunea bonus. Cum se terstează, imediat. 
	Motivație?
	-logging, absolut necesar, mai ales ppt debugging
	-fișier configurație, pentru ulurință și mai ales ca să mă lase să dau enable/disable la debugging
	-cod multi-platform? ca să nu mă încurc în arhive și că nu am făcut prea des asta, așa că mi s-a părut interesant
	-FCFS? pentru că mi s-a părut atât de ușor și de elegant de inclus și algoritmul asta în implementarea mea 
	 încât a trebuit să o fac
	 
	Dificultățilee întâmpinate au fost de natură conceptuală. A fost oarecum dificil să gândesc un mecanism bun de
excludere mutuală și scheduling, dar a fost foarte interesant.
	Lucruri interesante? Cum poți să faci un scheduler(habar n-aveam), un exercișiu în plus de sincronizare de threaduri,
aprofundarea noțiunilor de so, scris cod multi-platform cât să fie și elegant( nu cu multe ifdef-uri si cod duplicat) și
combinare de round-robin și fcfs atât de simplă. 

~~~~~~~~~~~~~~~~~~~~~~~~
		Compilare
~~~~~~~~~~~~~~~~~~~~~~~~
	Ambele makefile-uri au regula de build care realizează crearea biblioteci libloader.so/dll+lib. Doar apelând make/nmake
se face toată treaba. Acestea au asociate regula de clean care șterge fișierele folosite și biblioteca. 
	Pentru bonus am inclus și regula ”bonus_tests” care generează biblioteca și trei executabile, descrise mai jos.
Existaă și regula de clean, numită clean_bonus, care șterge toate fișierele rezultate.
	Pentru a merge bonusul, trebuie compilate testele prin comanda "make bonus_tests". Aceasta va da enable la folosirea
fisierului pentru configurare. Dupa aceea trebuie editat fisierul de configurare, dupa cum puteti vedea mai jos. Pe Linux
se poate sa fie nevoie de "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.". Rularea se face cu ./, fara parametrii.

-config_params, un program interactiv prin care utilizatorul setează parametrii din fișierul de configurație
	Se rulează ./config_params sau ./config_params.exe șis se urmează instrucțiunile. Recomand valorile 1 5 256 1 2000 1
	și 1 5 256 1 2000 0, pentru a vedea diferențele dintre round-robin și fcfs. Inițial primul parametru e 0 pentru a nu
	influența rularea pe checker.

	Ambele sunt făcute pentru algoritmul de scheduling fcfs. Primul merge cu siguranță rulat și pentru round-robin pentru
a observa loggingul. Nu au argumente.
-bonus_test_exec: am copiat stress test-ul vostru (18, parcă). Se creează multe threaduri. În bonus_test_exec.c se poate
	altera numărul schimbând define-ul. Eu am pus 69. 
-bonus_test_wait: am făcut un test în care câteva threaduri să se aștepte unul pe celălalt. E făcut în principal pentru
	algoritmul fcfs. Nu garantez să meargă mereu pentru round-robin.
	
Rularea fiecărui executabil va duce la crearea(dacă a fost selectată, evident) a câte unui fișier de logging. Astfel se
poate observa funcționalitatea tuturor elementelor de bonus implementate. Voi include screenshoturi cu rezultatele rulării
pentru a fi sigur.

~~~~~~~~~~~~~~~~~~
Git-ul este https://github.com/Longman-Stan/So-tema4, însă va fi pe privat până când va trece deadline-ul hard al temei.
***Obligatoriu:*** 
* De făcut referință la abordarea generală menționată în paragraful de mai sus. Aici se pot băga bucăți de cod/funcții - etc.
* Consideri că tema este utilă?
* Consideri implementarea naivă, eficientă, se putea mai bine?

***Opțional:***
* De menționat cazuri speciale (corner cases), nespecificate în enunț și cum au fost tratate.


Implementare
-

* De specificat dacă întregul enunț al temei e implementat
* Dacă există funcționalități extra, pe lângă cele din enunț - descriere succintă (maximum 3-4 rânduri/funcționalitate) + motivarea lor (maximum o frază)
* De specificat funcționalitățile lipsă din enunț (dacă există) și menționat dacă testele reflectă sau nu acest lucru
* Dificultăți întâmpinate
* Lucruri interesante descoperite pe parcurs

Cum se compilează și cum se rulează?
-
* Explicație, ce biblioteci linkează, cum se face build
* Cum se rulează executabilul, se rulează cu argumente (sau nu)

Bibliografie
-

* Resurse utilizate - toate resursele publice de pe internet/cărți/code snippets, chiar dacă sunt laboratoare de SO

Git
-
1. Link către repo-ul de git

Ce să **NU**
-
* Detalii de implementare despre fiecare funcție/fișier în parte
* Fraze lungi care să ocolească subiectul în cauză
* Răspunsuri și idei neargumentate
* Comentarii și *TODO*-uri
