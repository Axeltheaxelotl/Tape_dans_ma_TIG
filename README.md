<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/7/77/Elf-layout--en.svg/1083px-Elf-layout--en.svg.png" alt="ELF Layout" width="500"/>
</p>

---

## ELF header
- pour savoir ou commencer

## Program Headers
- pour savoir quelle partie mettre en memoire

- Il saute a l adresse d entree (`e_entry`)
- Le programme demarre

---

## LE FILS DE PUTE DE `.text`

- `.text` c est le  code machine
- `.data` c est les donnees ecrites
- `.rodata` chaine constantes
- `.bss` c est les donnees non initialisees

> le payload il modifie le `.text` pour chiffrer et dechiffrer

---

### un payload c un pti programme  
que tu met avant au debut d un autre programme  
pour lui faire faire quelque choses AVANT qu il demarre tes grand morts normalement

---

### pourquoi faire me dirier vous  
_"un doute sur l ortographe de dirier mais bon inshallah"_

- chiffrer le vrai code (`.text`)  
  pour que pas des petit fils de pute puissent lire ou copier le code

- avoir un pti programme pour decfiffrer le code au moment ou il est execute  
  pour qu il fonctionnent normalement

---

## COMMENT QUE CA FONCTIONNENT

1. À l’origine ton programme démarre à l’adresse **X**.
2. Tu changes ça → maintenant il démarre dans le payload.

3. Le payload :
   - retrouve l’endroit où est ton vrai code
   - le déchiffre avec la clé
   - répare la mémoire
   - saute à l’adresse **X** (le vrai début du programme)

4. Le programme continue normalement, comme si de rien n’était.
