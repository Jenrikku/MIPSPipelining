# MIPS Pipeline Simulator

Fent ús d'un set reduït d'instruccions de l'arquitectura MIPS, aquest programa pot interpretar i executar codi assemblador a més de trobar possibles problemes de dades relacionats amb la *pipeline* de 5 fases de MIPS.

Aquest projecte es va fer com a tasca per a la [UIB](https://uib.cat).

## Guia de l'usuari (Català)

### Interfície de línia de comandes

El programa s'utilitza en la seva totalitat mitjançant una interfície de línia de comandes. Espera rebre les següents opcions, totes opcionals, per a modificar el seu comportament:

- **-i --input**: Permet especificar el fitxer d'entrada des d'on es llegirà el codi assemblador.
- **-o --output**: Permet especificar el fitxer de sortida on s'escriuran tots els resultats del programa.
- **-n --nops**: En comptes de mostrar un diagrama de *pipeline*, afegeix `NOP`s al codi de tal forma que no hi hagi problemes de dades.
- **-d --branch-in-dec**: Simula que els *branch* es calculen durant la fase de *decode*, és a dir, que ja es sap quina és la següent instrucció a executar tan bon punt acaba la fase de *decode*.
- **-u --unlimited**: Per a evitar que hi hagi bucles infinits, hi ha un nombre màxim d'instruccions que es poden executar al simulador. Aquesta opció anuŀla aquest límit.
- **-t --tabs**: Utilitza tabulacions en comptes d'espais a l'hora de separar les fases del diagrama.
- **-f --forwarding**: Permet especificar el tipus de *forwarding* a utilitzar d'entre els següents:
    - **no**: No hi ha *forwarding* (per defecte).
    - **alu**: Només hi ha *forwarding* a les fases d'execució.
    - **full**: S'implementa *forwarding* a totes les fases. Si s'especifica l'opció sense cap valor, s'utilitzarà aquest.
- **-b --branch**: Permet especificar el tipus de predicció de *branch* d'entre els següents:
    - **no**: No hi ha predicció de *branch* (per defecte).
    - **p**: Predicció de *branch* perfecte. Mai ocorren aturades al *pipeline* per culpa dels *branch*.
    - **t**: Sempre es prediu que s'agafarà el *branch*.
    - **nt**: Sempre es prediu que mai s'agafarà el *branch*.

Les opcions segueixen l'estàndard POSIX juntament amb les [extensions del GNU](https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html).  
Notau que si no especificau un fitxer d'entrada, llavors s'utilitzaran les dades que entren per terminal. El programa començarà la simulació tan bon punt trobi el final del fitxer, que es pot enviar a la majoria de terminals prement Ctrl+D.

#### Exemple d'ús

```
mipspipeline -falu -bnt -d -n -i "basic.asm" -o "output.txt"
```

En aquest exemple s'utilitza *forwarding* només a les fases d'execució, predicció de *branch* com que mai s'agafaran, es simula que els *branch* es calculen a la fase de decode, s'afegeixen `NOP`s en comptes de mostrar el diagrama, es llegeix el fitxer "basic.asm" i s'escriu el resultat a "output.txt".

### Codi d'entrada

El codi d'entrada pot fer un ús lliure de les majúscules i minúscules ja que el programa no les diferencia.  
Per a referir-se a registres, es pot utilitzar tant el símbol `$`, com la lletra `R`. De manera que `$9` i `R9` són equivalents.  
Per a escriure afegir comentaris es poden utilitzar els següents símbols: `;`, `/`, `#`, `@`. Tot caràcter després d'aquests símbols i fins al final de la línia serà ignorat.
  
Cada línia comença de manera opcional per una etiqueta de la següent manera:

```
LABEL: XORI $4, $3, 4
```

Aquestes etiquetes s'utilitzen a les instruccions de *branch* i *jump*.

#### Instruccions suportades

El codi d'entrada pot contenir les següents instruccions:

| Instrucció     | Ús                | Descripció                                                                                      |
| -------------- | ----------------- | ----------------------------------------------------------------------------------------------- |
| `NOP` / `NOOP` | `NOP`             | Cap operació.                                                                                   |
| `ADD`          | `ADD $d, $s, $t`  | Suma els valors de `$s` i `$t` i posa el resultat a `$d`.                                       |
| `ADDI`         | `ADDI $t, $s, i`  | Suma el valor immediat `i` a `$s` i posa el resultat a `$t`.                                    |
| `ADDU`         | `ADDU $d, $s, $t` | Equivalent a `ADD` però amb nombres sense signe.                                                |
| `ADDIU`        | `ADDIU $t, $s, i` | Equivalent a `ADDI` però amb nombres sense signe.                                               |
| `AND`          | `AND $d, $s, $t`  | Fa una operació *and* dels bits de `$s` i `$t` i posa el resultat a `$d`.                       |
| `ANDI`         | `ANDI $d, $s, i`  | Fa una *and* dels bits de `$s` i el valor immediat `i` i posa el resultat a `$t`.               |
| `OR`           | `OR $d, $s, $t`   | Fa una operació *or* dels bits de `$s` i `$t` i posa el resultat a `$d`.                        |
| `ORI`          | `ORI $d, $s, i`   | Fa una *or* dels bits de `$s` i el valor immediat `i` i posa el resultat a `$t`.                |
| `NOR`          | `NOR $d, $s, $t`  | Fa una operació *nor* dels bits de `$s` i `$t` i posa el resultat a `$d`.                       |
| `NORI`         | `NORI $d, $s, i`  | Fa una *nor* dels bits de `$s` i el valor immediat `i` i posa el resultat a `$t`.               |
| `XOR`          | `XOR $d, $s, $t`  | Fa una operació *xor* dels bits de `$s` i `$t` i posa el resultat a `$d`.                       |
| `XORI`         | `XORI $d, $s, i`  | Fa una *xor* dels bits de `$s` i el valor immediat `i` i posa el resultat a `$t`.               |
| `SUB`          | `SUB $d, $s, $t`  | Resta el valor de `$t` al valor de `$s` (s - t) i posa el resultat a `$d`.                      |
| `SUBU`         | `SUBU $d, $s, $t` | Equivalent a `SUB` però amb nombres sense signe.                                                |
| `LB`           | `LB $t, i($s)`    | Carrega un byte (8 bits) a `$t` des de la posició de memòria apuntada per `$s + i`.             |
| `LH`           | `LH $t, i($s)`    | Carrega un half word (16 bits) a `$t` des de la posició de memòria apuntada per `$s + i`.       |
| `LW`           | `LW $t, i($s)`    | Carrega un word (32 bits) a `$t` des de la posició de memòria apuntada per `$s + i`.            |
| `SB`           | `SB $t, i($s)`    | Guarda un byte (8 bits) des de `$t` a la posició de memòria apuntada per `$s + i`.              |
| `SH`           | `SH $t, i($s)`    | Guarda un half word (16 bits) des de `$t` a la posició de memòria apuntada per `$s + i`.        |
| `SW`           | `SW $t, i($s)`    | Guarda un word (32 bits) des de `$t` a la posició de memòria apuntada per `$s + i`.             |
| `BEQ`          | `BEQ $s, $t, lab` | Si els valors de `$s` i `$t` són iguals, llavors salta a la instrucció amb l'etiqueta `lab`.    |
| `BNE`          | `BNE $s, $t, lab` | Si els valors de `$s` i `$t` són diferents, llavors salta a la instrucció amb l'etiqueta `lab`. |
| `BGEZ`         | `BGEZ $s, lab`    | Si el valor de `$s` és major o igual a 0, llavors salta a la instrucció amb l'etiqueta `lab`.   |
| `BGTZ`         | `BGTZ $s, lab`    | Si el valor de `$s` és major que 0, llavors salta a la instrucció amb l'etiqueta `lab`.         |
| `BLEZ`         | `BLEZ $s, lab`    | Si el valor de `$s` és menor o igual a 0, llavors salta a la instrucció amb l'etiqueta `lab`.   |
| `BLTZ`         | `BLTZ $s, lab`    | Si el valor de `$s` és menor que 0, llavors salta a la instrucció amb l'etiqueta `lab`.         |
| `J`            | `J lab`           | Sempre salta a la instrucció amb l'etiqueta `lab`.                                              |

#### Reserva de la memòria

Per a reservar memòria, es poden utilitzar les següents directrius del simulador, sempre **abans del codi** que s'executa.

| Directriu      | Ús                | Descripció                                                                                                |
| -------------- | ----------------- | --------------------------------------------------------------------------------------------------------- |
| `DEFB`         | `DEFB $s, i`      | Afegeix una variable d'1 byte (8 bits) amb valor `i` i posa la seva adreça al registre `$s`.              |
| `DEFH`         | `DEFH $s, i`      | Afegeix una variable de 2 bytes (16 bits) amb valor `i` i posa la seva adreça al registre `$s`.           |
| `DEFW`         | `DEFW $s, i`      | Afegeix una variable de 4 bytes (32 bits) amb valor `i` i posa la seva adreça al registre `$s`.           |
| `DEVB`         | `DEVB $s, i`      | Reserva una *array* de `i` valors cada un d'1 byte (8 bits) i posa l'adreça del primer element a `$s`.    |
| `DEVH`         | `DEVH $s, i`      | Reserva una *array* de `i` valors cada un de 2 bytes (16 bits) i posa l'adreça del primer element a `$s`. |
| `DEVW`         | `DEVW $s, i`      | Reserva una *array* de `i` valors cada un de 4 bytes (32 bits) i posa l'adreça del primer element a `$s`. |

Totes les *arrays* s'inicialitzen a 0.

### Consideracions

S'ha considerat que les instruccions de salt incondicional (`J`) sempre afegeixen una aturada al *pipeline* ja que no sabem que es tracta d'un salt fins a la fase de *decode*.  
Per defecte i si no s'especifica l'opció `branch-in-dec`, llavors a les instruccions de *branch* no sabem si hem de saltar fins a la fase d'execució.
