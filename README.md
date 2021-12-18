# pus2021

## Jak zacząć

* projekt należy sklonować na swoim koncie na serwerze xor `git clone https://gitlab.com/mariusz.jarocki/pus2021.git`
* polecenie `make` stworzy plik wykonywalny serwera `dist/server`
* polecenie `make initdb` tworzy początkową postać bazy danych
* serwer uruchamiamy poleceniem `dist/server port` gdzie port jest liczbą całkowitą większą od 1024, nieużytą w tym momencie przez innego użytkownika *celem uniknięcia konfliktów, możemy przyjąć konwencję 1 miesiąc_urodzenia dzień_urodzenia czyli dla urodzonego 24 marca: 10324*
* uruchomiony serwer wyświetli prompt w postaci znaku % ; można go zakończyć naciskając Ctrl+D
* polecenie `make java` kompiluje klienta javowego
* plik `client.jar` należy ściągnąć do środowiska lokalnego wraz z plikiem `client.properties.example`
* należy zmienić nazwę pliku `client.properties.example` na `client.properties` i zmienić odpowiednio jego zawartość tak, aby łączył się z odpowiednim serwerem (`host:`) i portem (`port:`); w pliku tym będą umieszczane również dodatkowe informacje potrzebne klientowi do pracy
* uwaga: spoza sieci lokalnej nie połączycie się z portami xora poza 22 (ssh); aby połączyć się ze swoim serwerem pracującym na porcie powiedzmy 10324 należy zestawić tunel ssh poleceniem `ssh -L 10324:localhost:10324 uzytkownik@xor.math.uni.lodz.pl` i w pliku `client.properties` użyć jako host wartości `localhost`; niestety nie da się w ten sposób zasymulować transmisji UDP - zalecam użycie WSL,wirtualizacji albo użycie jako platformy uruchomieniowej systemu posixowego (Linux, MacOS X), gdzie klient javowy i serwer mogą pracować w ramach tej samej maszyny
* dla środowiska WSL opartego na Ubuntu celem zainstalowania narzędzi kompilacyjnych, należy wykonać polecenia: `sudo apt update; sudo apt install gcc make netcat net-tools sqlite3 libsqlite3-dev openjdk-8-jdk`
* komunikacja między Windows a WSL działa poprawnie, o ile używamy adresu prywatnego WSL a nie 127.0.0.1; można go wyświetlić używając na WSL polecenia `ip a`; adres powinien być wyświetlony przy interfejsie eth0
* VS Code należy wyposażyć w dodatek Remote WSL

## Zadania zaliczeniowe

# buforowanie (BC)

* po zalogowaniu użytkownik dostaje wszystkie wiadomości, które nie zostały odczytane na tym kliencie

# znajomi (BC)

* wysyłać wiadomość można tylko do tych osób, które wcześniej potwierdziły że chcą od nas je otrzymywać

# grupy (BC)

* definiujemy grupy użytkowników i możemy wysyłać wiadomości do wszystkich członków grupy naraz

# pliki (AB)

* użytkownik może wysłać plik z lokalnego systemu innemu użytkownikowi, który może go zapisać w swoim systemie lokalnym

# rozłączanie nieaktywnych połączeń TCP i ich ponowne zestawianie (AB)

* nieaktywna sesja TCP może być zamknięta i odtworzona w momencie powrocie aktywności użytkownika

# wieloserwerowość (A)

* dwa (conajmniej) współpracujące ze soba serwery, zsynchronizowana baza danych i zmodyfikowana reakcja na polecenia oraz przesyłanie wiadomości

Poziomy trudności: A - trudny, B - przeciętny, C - łatwy

Trzeba zaimplementować dowolne dwie funkcjonalności, ocena końcowa zależy od poziomów trudności oraz stopnia realizacji/poziomu kompromisu ;)