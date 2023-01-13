source setEnv.sh 


echo "\n\n\n"

for i in  1 2 3 4 5 6 7 8 9 10
do 
    echo "SIMULAZIONE $i..."
    make log
    echo "Done!\n"
done

echo "LA CONFIGURAZIONE 'Custom' SEMBRA FUNZIONARE\n\n\n"


source setEnv.sh 1

for i in  1 2 3 4 5 6 7 8 9 10
do 
    echo "SIMULAZIONE $i..."
    make log
    echo "Done!\n"
done

echo "LA CONFIGURAZIONE 'Dense, small ships' SEMBRA FUNZIONARE\n\n\n"

source setEnv.sh 2

for i in  1 2 3 4 5 6 7 8 9 10
do 
    echo "SIMULAZIONE $i..."
    make log
    echo "Done!\n"
done

echo "LA CONFIGURAZIONE 'Dense, small ships + trashing' SEMBRA FUNZIONARE\n\n\n"

source setEnv.sh 4

for i in  1 2 3 4 5 6 7 8 9 10
do 
    echo "SIMULAZIONE $i..."
    make log
    echo "Done!\n"
done

echo "LA CONFIGURAZIONE 'Cargos, big stuff' SEMBRA FUNZIONARE\n\n\n"

source setEnv.sh 5

for i in  1 2 3 4 5 6 7 8 9 10
do 
    echo "SIMULAZIONE $i..."
    make log
    echo "Done!\n"
done


echo "LA CONFIGURAZIONE 'Unlucky cargos' SEMBRA FUNZIONARE\n\n\n"


echo "\n\n\nSEMBRA FUNZIONARE TUTTO!"