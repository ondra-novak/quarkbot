# account config

- **name** - jméno účtu, pokud chybí, bere se jméno souboru bez přípony
- **exchange** - (povinné) jméno modulu zajišťující spojení se směnárnou
- další položky - mohou obsahovat autorizaci (key, secret, nonce, etc)

account je typicky v samostatnem souboru

```
account1.ini:

[account]
name=account1
exchange=binance
key=329uejoidjw2d3292
secret=djnew82jdj98dj3d2
```


# instrument config
- **account** - (povinne) každý instrument je svázan s accounterm
- další položky - exchange spravující account dostane celý config



Instrumenty mohou byt definovany ve stejnem souboru jako account


```
account1.ini:

[account]
exchange=binance
key=329uejoidjw2d3292
secret=djnew82jdj98dj3d2

[instrument/bitcoin]
symbol=BTCUSDT

[instrument/ethereum]
symbol=ETHUSDT
```

pokud jsou definované v jiném souboru, musí obsahovat jméno accountu

```
[instrument/bitcoin]
symbol=BTCUSDT
account=account1
```


# strategy config

konfigurace strategie je v jednom ini souboru. Každá strategie má svůj ini soubor

- **instruments** - obsahuje seznam instrumentů, každý má vlastní label. Label by měl
být unikátní, ale instrumenty se mohou opakovat
- **parameters** - obsahuje parametry strategie
- **table** - obsahuje název databáze (tabulky, prefix)

```
[strategy]
table=string

[instruments]
main=account1.bitcoin
hedge=account1.ethereum

[parameters]
key=value
key=value
key=value
```

Strategie může instrumenty definovat inline

```
[instrument/xxy]
account=account_name
symbol=XXYUSDT
```

Pak tento instrument se též mapuje pod svým labelem=jménem



