Entry hackabi-kilpailuun. ~ deffi420, deffi420@gmail.com
31.8.2013


HACKABI BOOTKIT

--[ ToC

  0 - Johdanto
  1 - Tekninen toteutus
    1.1 - Boottiprosessi
    1.2 - Payload
  2 - Hyökkäykseltä suojautuminen
  3 - Yhteenveto
  4 - Lähteet


--[ 0 - Johdanto

Bootkitillä tarkoitetaan rootkittiä, jonka suoritus aloitetaan ennen
käyttöjärjestelmän alustusta tietokoneen käynnistyksessä. Tähän tekniikkaan
perustuvat mm. vanhat DOS MBR-virukset, TDL/TDSS-bottiverkko, Kon-Boot sek�
Stoned Bootkit. [0-3]

Tässä esitän Hackabi-bootkitin, joka lataa Digabi-livekäyttöjärjestelmän ja
tekee muutoksia muistiin ladattuun kerneliin. Nämä muutokset antavat
hyökkääjälle root-oikeudet, kun hyökkääjä kirjoittaa valitsemansa salasanansa
stdout-virtaan esimerkiksi echo-ohjelmalla. Hyökkäys ei perustu virheeseen
Digabi-distrossa, eikä sitä voi siksi täydellisesti estää kieltämättä kokelaiden
omia päätelaitteita.

Hackabi-bootkit on kehitetty Digabi-livekäyttöjärjestelmän versiolle 1.0, eikä
luultavasti toimi suoraan poikkeavilla versioilla. Lähdekoodit sekö käännetty
levykuva löytyvöt osoitteesta:

  http://www.niksula.cs.hut.fi/~hirvolt1/hackabi/

Tai yhtenä pakettina:

  https://mega.co.nz/#!JFk2TBRK!PTDvDwSSeDVXZZNr3Od_M0RYmgMKQj6ldjGpk4KUANQ

Koodi k��ntyy FASM-assemblerilla [4] komennolla: fasm image.asm. Levykuvan voi
kirjoittaa CD:lle tai muistitikulle. Hackabi-bootkitti� voi kokeilla helposti
mm. VirtualBoxissa tai VMwaressa. N�iden lis�ksi Hackabi-bootkit on testattu
toimivaksi oikealla raudalla kirjoittamalla levykuva muistitikulle.

K�yt�nn�ss� Hackabi-bootkitti� k�ytet��n seuraavasti:

  1. Hy�kk��j� k�ynnist�� tietokoneen Hackabi-bootkitill� k�ytt�m�ll� USB-tikkua
     tai CD-levy�.

  2. N�yt�lle ilmestyy teksti, joka ohjeistaa poistamaan Hackabi-bootkitin
     asemasta ja lataamaan Digabi-k�ynnistysmedian samaan asemaan ja painamaan
     enter.

  3. T�m�n j�lkeen Digabi-livek�ytt�j�rjestelm� k�ynnistyy.

  4. Nyt hy�kk��j� saa root-oikeudet avaamalla p��tteen ja kirjoittamalla:
     echo "g1v3 m3 r00t b1tch". Lopuksi on suositeltavaa avata uusi shelli
     komennolla su.

Toisessa askeleessa vaaditaan, ett� Digabi-k�ynnistysmedia ladataan samaan
asemaan mist� Hackabi-bootkit k�ynnistettiin. T�m� rajoitus on nykyisess�
koodissa, ja se voidaan tarvittaessa poistaa.

Seuraavaksi esitell��n Hackabi-bootkitin teknist� toteutusta ja toimintatapaa.
T�m�n j�lkeen k�sittelen suojautumiskeinoja bootkitti� vastaan (tai l�hinn�
toimivien sellaisten puutetta). Viimeiseksi yhteenveto.


--[ 1 - Tekninen toteutus

Hackabi-bootkit on koodattu FASM-assemblerilla [4]. Bootkitin pit�isi k��nty�
suoraan komennolla: fasm image.asm. K��nt�minen tuottaa "El Torito"-speksin
mukaisen "no emulation" tilassa bootattavan CD-ROM -levykuvan. Lis�ksi levyn
ensimm�inen sektori sis�lt�� floppy-bootloaderin, joka mahdollistaa boottauksen
USB-tikulta. CD:n formaattiin liittyv�n koodin on alunperin kirjoittanut
Mike Gonta [5], mutta olen tehnyt siihen merkitt�vi� muutoksia. Kaikki muu koodi
on minun kirjoittamaani - osa koodista on kopioitu aikaisemmista projekteistani.

Tietokoneen k�ynnistyksess� BIOS lataa bootkitin muistiin. Bootkitin ensimm�inen
teht�v� on ladata oikea k�ytt�j�rjestelm� (t�ss� Digabi-livek�ytt�j�rjestelm�)
muistiin ja aloittaa sen alustus. T�m�n lis�ksi bootkitin on s�ilytt�v�
muistissa siihen asti, ett� k�ytt�j�rjestelm� on alustettu riitt�v�n pitk�lle
(esim. kernel ladattu muistiin) ja teht�v� lopuksi toivotut muutokset ladattuun
k�ytt�j�rjestelm��n. K�yt�nn�ss� bootkit hookkaa k�ytt�j�rjestelm�n alustuksessa
k�ytett�vi� rutiineja s�ilytt��kseen koodin suorituksen hallinnassaan kernelin
alustamiseen saakka. Hookkaamisella tarkoitetaan ohjelmiston muokkaamista siten,
ett� koodin suoritus palautuu hookkaajalle (bootkitille) kun hookattu kohta
koodista suoritetaan.

Kohdassa 1.1 k�sitell��n Hackabi-bootkitin hookkeja, joilla saavutetaan koodin
suoritus hieman ennen kernelin entry pointtia. T�m�n j�lkeen kuvataan kerneliin
teht�v�t muutokset kohdassa 1.2.


----[ 1.1 - Boottiprosessi

Hackabi-bootkitin suoritus alkaa, kun BIOS lataa bootkitin CDROM- tai
floppy-bootloaderin osoitteeseen 0x7C00. Bootloaderit lataavat
bootkit.inc-tiedoston koodin alkamaan osoitteesta 0x6000 ja hypp��v�t sen
alkuun. Valitsin osoitteen 0x6000, koska Digabin SYSLINUX-bootloader [6] ei
k�yt� lainkaan muistialuetta 0x6000-0x6C00 (3 KB tilaa).

Ennen k�ytt�j�rjestelm�n lataamista bootkit hookkaa IVT:st� (Interrupt Vector
Table [7]) keskeytyksen 0x13 (AH=2), joka on levysektorien lukemiseen
tarkoitettu BIOS-palvelu. Digabi kutsuu t�t� palvelua ensimm�isen kerran
boottivalikoiden sek� vastaanvapauslausekkeiden esitt�misen j�lkeen, mik� on
kohtalaisen my�h��n bootissa ja siksi hyv� ensimm�inen hookkauspaikka. En
k�ytt�nyt l�hdekoodeja reversauksessa, joten en ole varma mit� Digabi yritt��
lukea levyilt�. BIOS-palvelua kutsutaan per�kk�in levyille indekseill�
0x80-0x8F, joten kyseess� on luultavasti jokin luettavissa olevien kiintolevyjen
kartoitus. Kuitenkin, IVT:n hookkauksen j�lkeen bootkit lataa Digabin
SYSLINUX/ISOLINUX-bootloaderin osoitteeseen 0x7C00 ja hypp�� siihen. Nyt
Digabi-k�ytt�j�rjestelm�n alustus etenee normaalisti, kunnes boottivalikoiden
j�lkeen kutsutaan hookattua BIOS-palvelua, jolloin koodin suoritus palaa
bootkitille.

Keskeytyksen 0x13 hookissa on seuraavaksi hookattava protected modessa
suoritettava hyppy - uskoakseni - kernelin lataajaan. Hookkaus voidaan tehd�
ylikirjoittamalla jokin tai jotkin k�skyt hypyll� bootkitin koodiin. Hyppy
kernelin lataajaan sijaitsee osoitteessa 0x0010006A, joka on niin korkealla
osoiteavaruudessa, ettei siihen p��se k�siksi real modesta, jossa keskeytys 0x13
k�sitell��n. Ratkaisuksi Hackabi-bootkit hookkaa ensiksi kohdan 0x00013445,
jonka SYSLINUX-bootloader suorittaa pian protected modeen siirtymisen jälkeen.
Tässä hookissa hookataan kernelin lataajan hyppy. Tämän jälkeen hookataan
edelleen pari kohtaa, kunnes lopulta päästään lähelle kernelin entry pointtia.
Kaiken kaikkiaan bootkit toimii seuraavasti:

  1. Hookkaa IVT:st� INT 0x13 (AH=2, read sectors from a drive).
  
  2. Lataa Digabi-livekäyttöjärjestelmän bootloader osoitteeseen 0x7C00 ja
     aloita sen suoritus.

  3. IVT-hookin ensimmäisessä kutsussa hookkaa protected modessa suoritettava
     kohta 0x00013445.

  4. Edellisessä askeleessa asetetussa hookissa hookkaa hyppy kernelin
     lataajaan. Hyppy sijaitsee osoitteessa 0x0010006A.

  5. Seuraavaksi hookkaa jokin kohta, jossa hyppy kerneliin on ladattu muistiin.
     Hackabi-bootkit käyttää osoitetta 0x01587E0B.

  6. Viimeiseksi hookkaa hyppy purettuun kerneliin (hyppy @ 0x012A728F).

  7. Suorita payload.

Huom! En ole varma, voivatko yllämainittujen osoitteiden base-osoitteet muuttua.
Varsinaista ASLRia (Address Space Layout Randomization [8]) ei tavallisesti
käytetä bootloadereissa. Hackabi-bootkit on kuitenkin koodattu niin, että sen
pitäisi toimia, vaikka base-osoitteet muuttuisivatkin.

Joku saattaa ihmetellä, miksi on hookattava näin monta eri kohtaa. Eikä riitä,
jos heti alussa hookataan hyppy kerneliin? Tämä ei ole mahdollista, koska hyppy
kerneliin on ladattu muistiin vasta boottauksen viimeisissä vaiheissa.


----[ 1.2 - Payload

Hackabi-bootkitin payload on koodi, joka suoritetaan juuri ennen Digabin
kerneli�. Payload tekee lopulliset muutokset muistiin ladattuun kerneliin.
Tavoitteena on muokata kerneli� niin, ett� hy�kk��j� saa root-oikeudet ilman
root-tunnuksen salasanan tiet�mist�.

Otin l�hestymistavaksi hookata keskeytyksen 0x80, jota k�ytet��n
j�rjestelm�kutsujen l�hett�miseen. Esimerkiksi kutsua write(fd, buf, count)
vastaa j�rjestelm�kutsu parametreilla EAX=4, EBX=stdout, ECX=buf, EDX=count.
Hookkaamalla keskeytys 0x80 pystyt��n seuraamaan mit� parametreja writelle
sy�tet��n, mik� mahdollistaa esimerkiksi kutsuvan prosessin oikeuksien
korottamisen, jos buf sis�lt�� hy�kk��j�n valitseman salasanan. Muun muassa
echo-ohjelma kirjoittaa viestit stdout-virtaan k�ytt�m�ll�
write-j�rjestelm�kutsua. T�m�n ansiosta hy�kk��j� voi korottaa p��tteens�
oikeudet tulostamalla echo-ohjelmalla valitsemansa salasanan.

Tavallinen ja suoraviivainen tapa keskeytyksen 0x80 hookkaamiseen on korvata
IDT:n (Interrupt Descriptor Table) alkio 0x80, joka l�ydet��n SIDT-konek�skyn
avulla. T�m� ei kuitenkaan nyt toimi, koska bootkitin payload suoritetaan ennen
kerneli�, jolloin IDT ei ole viel� alustettu. Sen sijaan Hackabi-bootkit hookkaa
kernelist� osoitteen 0xC12B71B0, josta alkaa keskeytyksen 0x80 k�sittelij�.
Lis�ksi ongelmana on sivutaulun alustus, joka tyhjent�� bootkitin muistialueen
0x6000-0x6C00, kun kernelin suoritus aloitetaan. T�m�n takia j�rjestelm�kutsun
hookkiin liittyv� koodi on siirrett�v� muualle muistissa (muita osia bootkitin
koodista ei en�� tarvita ja niiden voidaan antaa pyyhkiyty� pois). Vapaata tilaa
l�ytyy riitt�v�sti alkaen osoitteesta: 0xC12BB6B4.

J�rjestelm�kutsu-hookissa oikeudet korotetaan asettamalla prosessin
task_struct-tietueen osoittaman cred-tietueen kent�t uid, gid, euid ja egid
nollaksi. Digabi-k�ytt�j�rjestelm�ss� osoitin aktiivisen prosessin
task_struct-tietueeseen l�ytyy FS-segmentin takaa osoitteesta 0xDDC. Oikeuksien
korottamisen j�lkeen writelle annetun puskurin sis�lt� voidaan korvata jollakin
viestill� merkiksi onnistumisesta.


--[ 2 - Hyökkäykseltä suojautuminen

Kuvailtua bootkit-hy�kk�yst� ei voida est�� muokkaamalla distroa, sill� hy�kk�ys
ei perustu distron haavoittuvuuksiin. Mielest�ni perimm�inen vika on
koetilanteessa. Ehdotetusta koej�rjestelyst� puuttuu chain of trust, koska
kokelas saa tuoda oman p��telaitteensa. Tietoturvan rakentaminen ilman chain of
trustia on pelkk�� masturbaatiota eik� johda mihink��n. Oikea ratkaisu on
kielt�� omat p��telaitteet ja bootata p��telaitteilla ainoastaan
allekirjoitettua koodia. T�ll�in k�ytt�j�rjestelm� voi luottaa alla olevaan
laitteestoon, ja siihen, ettei hy�kk��j� ole muokannut k�ytt�j�rjestelm��. N�in
estett�isiin my�s valtava m��r� muita hy�kk�yksi�.

Hy�kk�yst� voi kuitenkin yritt�� vaikeuttaa tekem�ll� muutoksia distroon.
Hackabi-bootkit k�ytt�� joitakin kovakoodattuja osoitteita, joiden takia bootkit
hajoaa esimerkiksi SYSLINUXin tai kernelin p�ivitt�misen j�lkeen. My�s
CD-bootissa ISOLINUX-bootloaderi ladataan kovakoodatusta osoitteesta levylt�,
mink� takia pienikin muutos distroon rikkoo bootkitin CD:ltä boottauksen.
Hackabi-bootkitistä on kuitenkin helppo laajentaa geneerinen versio, joka ei
käytä kovakoodattuja osoitteita ja selviää joistakin distron päivityksistä. Olen
koodannut Windows 7 -käyttöjärjestelmälle tämän tyyppisen bootkitin vuonna 2011,
ja se toimii edelleen uusimmissa päivitetyissä Win7-käyttöjärjestelmissä (2013).
Bootkitissä tarvittavien osoitteiden selvittämistä voi hankaloittaa esimerkiksi
obfuskaatiolla, mutta tämä ainoastaan vaikeuttaa bootkitin kehittämistä eikä
estä hyökkäystä.

Hackabi-bootkitin payload tekee muutoksia muistiin ladattuun kerneliin.
Perinteisesti näiltä muutoksilta suojaudutaan PaX- ja grsecurity-tyyppisillä
muokkauksilla, mutta ne eivät ole hyvä ratkaisu bootkittejä vastaan. Bootkit
suoritetaan suurimmilla mahdollisilla oikeuksilla sekä ennen kernelin ja
suojausten alustamista, minkä vuoksi bootkitillä on etulyöntiasema tällaisiin
suojauksiin nähden.

Hackabi-bootkitin voi bootata CD:ltä tai muistitikulta. Suojautumiseksi voisi
ehdottaa "ylimääräisten" boottimedioiden kieltämistä koetilanteessa. On
kuitenkin mahdollista muokata bootkitistä versio, joka kirjoitetaan kiintolevyn
MBR:ään (Master Boot Record [0,9]). Nyt asettamalla kiintolevy ensimmäiseksi
boottijärjestyksessä saadaan bootkit käynnistettyä täysin huomaamattomasti.

Toisaalta kyse on vain ylioppilaskirjoituksista, joten imo ihan sama vaikka joku
huijaa tai käytetyssä järjestelmässä on näin perustavaa laatua oleva aukko, jota
ei voi korjata. Käytännössä riittää, että huijaaminen on tarpeeksi hankalaa.


--[ 3 - Yhteenveto

Esittelin bootkit-tekniikkaan perustuvan hyökkäyksen, jonka avulla hyökkääjä saa
Digabi-käyttöjärjestelmään root-oikeudet valitsemallaan salasanalla. Entryn
tarkoitus oli havainnollistaa kokelaiden omien päätelaitteiden sallimiseen
liittyviä tietoturvaongelmia. Ainoa varma keino hyökkäyksen estämiseen on omien
päätelaitteiden kieltäminen.


--[ 4 - Lähteet

[0] Peter Szor. The Art of Computer Virus Research and Defense. 2005.
    http://computervirus.uw.hu/ch04lev1sec1.html
[1] ESET Team. TDSS part 3: Bootkit on the Other Foot. 2011.
    http://resources.infosecinstitute.com/?s=tdss
[2] Piotr Bania. Kon-Boot. 2011.
    http://www.piotrbania.com/all/kon-boot/
[3] Peter Kleissner. Stoned Bootkit. 2012.
    http://www.stoned-vienna.com/
[4] flat assembler.
    http://flatassembler.net/
[5] Mike Gonta. Bootable cdrom ISO9660/Joliet image code. 2008.
    http://www.911cd.net/forums//index.php?showtopic=21535
[6] SYSLINUX. The Syslinux Project. 2013.
    http://www.syslinux.org/
[7] OSDev.org. Interrupt Vector Table. 2013.
    http://wiki.osdev.org/Interrupt_Vector_Table
[8] Wikipedia. Address space layout randomization. 2013.
    http://en.wikipedia.org/wiki/Address_space_layout_randomization
[9] OSDev.org. MBR (x86). 2013.
    http://wiki.osdev.org/MBR_(x86)
