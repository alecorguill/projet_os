###### COMPILATION #####
Pour compiler thread.c :
     make thread

Pour compiler les tests :
     make tests

Pour compiler un test en particulier :
     make testNumero
Par exemple pour compiler le test 01
    make test01

###### EXECUTION TEST #####
Pour éxecuter le test de numero n :
     ./n
###### TEST ######
Pour lancer les tests de comparaison avec pthread: 
     make check
Certains tests necessitent des arguments qui ont une valeur par defaut.
Pour éxecuter les tests avec des valeurs d'arguments différent faire par exemple :
     make check NB_FIBO=10

Voici la liste des nom des arguments :
NB_THREAD
NB_YIELD
NB_FIBO
NB_BIGSUM
NB_SORT

Pour lancer un test de comparaison avec pthread pour un test particulier :
     make ptestNumero