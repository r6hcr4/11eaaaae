# pus2021

## Jak zacząć

* projekt należy sklonować na swoim koncie na serwerze xor `git clone https://gitlab.com/mariusz.jarocki/pus2021.git`
* polecenie `make` stworzy plik wykonywalny serwera `dist/server`
* serwer uruchamiamy poleceniem `dist/server port` gdzie port jest liczbą całkowitą większą od 1024, nieużytą w tym momencie przez innego użytkownika *celem uniknięcia konfliktów, możemy przyjąć konwencję 1 miesiąc_urodzenia dzień_urodzenia czyli dla urodzonego 24 marca: 10324*
* uruchomiony serwer wyświetli prompt w postaci znaku % ; można go zakończyć naciskając Ctrl+D
* klienta javowego kompilować nie trzeba, bo w aktualnej postaci jest on skompilowany w katalogu `javaclient/dist`; gdyby potrzeba go było skompilować ponownie, należy wydać polecenie `make javaclient`
* plik `client.jar` należy ściągnąć do środowiska lokalnego wraz z plikiem `client.properties.example`
* należy zmienić nazwę pliku `client.properties.example` na `client.properties` i zmienić odpowiednio jego zawartość tak, aby łączył się z odpowiednim serwerem (`host:`) i portem (`port:`)
* uwaga: spoza sieci lokalnej nie połączycie się z portami xora poza 22 (ssh); aby połączyć się ze swoim serwerem pracującym na porcie powiedzmy 10324 należy zestawić tunel ssh poleceniem `ssh -L 10324:localhost:10324 uzytkownik@xor.math.uni.lodz.pl` i w pliku `client.properties` użyć jako host wartości `localhost`