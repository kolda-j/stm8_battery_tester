# stm8_battery_tester

## Schéma zapojení
<p align="center"><img src="media/schematic.png"><p>
<p>Obr č.1</p>
  
---
  
## STM8S Dero Board Pinout
<p align="center"><img src="media/pinout_1.png"><p>
<p>Tab č.1</p>
  
---
  
## Seznam Součástek
<p align="left"><img src="media/BOM.png"><p>
<p>Tab č.2</p>

---
  
## Úprava nabíječky TP4056
<p>Tato sekce popisuje úpravy, které byly provedeny na výše zmíněném modulu TP4056.</p>
<p>Je třeba odpájet rezistor R3(Rprog), jeho hodnota nastavuje nabíjecí proud (viz tabulka č.3). Místo něj se pájecí plošky připojí na piny digitálního potenciometru VH a VW. Díky této úpravě můžeme nastavit nabíjecí proud pomocí STM8</p>
<p>Dále se musí odpájet CE(Pin8). Připojení pinu CE na logickou 1 (TTL logika) uvede čip do normální funkce, Díky této modifikaci můžeme nabíječku zapínat a vypínat pomocí výstupu STM8</p>
<p>Poslední úprava na desce je odpájení pinu CHRG(Pin7) a jeho připojení na pin STM8. V případě že se baterie nabíjí je pin CHRG nastaven na logickou 0, v opačném případě je nastaven na logickou 1. Nyní může program reagovat na ukončení nabíjení.</p>


<p align="left"><img src="media/tp4056.jpg"><p>
  <p>Obr č.2</p>
<p align="left"><img src="media/Rprog.png"><p>
  <p>Tab č.3</p>

---

## Krabička
<p align="center"><img src="media/enclosure.png"><p>
<p>Obr č.3</p>
