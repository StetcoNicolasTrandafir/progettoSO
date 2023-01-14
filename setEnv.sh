if [ -z "$1" ] 
    then
    echo "Setting custom configuration..."
    export SO_NAVI=100
    export SO_PORTI=50
    export SO_MERCI=2
    export SO_MIN_VITA=100000
    export SO_MAX_VITA=10000000
    export SO_SIZE=10
    export SO_SPEED=50
    export SO_LATO=10000
    export SO_CAPACITY=10
    export SO_BANCHINE=10
    export SO_FILL=100000
    export SO_LOADSPEED=100 
    export SO_DAYS=10
    export SO_STORM_DURATION=36
    export SO_SWELL_DURATION=69
    export SO_MEALSTROM=30
    echo "Done!"
    else if [ $1 == 1 ] 
        then
            echo "Setting 'dense, small ships' configuration..."
            export SO_NAVI=1000
            export SO_PORTI=100
            export SO_MERCI=1
            export SO_SIZE=1
            export SO_MIN_VITA=50
            export SO_MAX_VITA=50
            export SO_LATO=1000
            export SO_SPEED=500
            export SO_CAPACITY=10
            export SO_BANCHINE=2
            export SO_FILL=500000
            export SO_LOADSPEED=200
            export SO_DAYS=10
            export SO_STORM_DURATION=6
            export SO_SWELL_DURATION=24
            export SO_MEALSTROM=1
            echo "Done!"
        else if [ $1 == 2 ] 
            then
                echo "Setting 'dense, small ship + trashing' configuration..."
                export SO_NAVI=1000
                export SO_PORTI=100
                export SO_MERCI=10
                export SO_SIZE=1
                export SO_MIN_VITA=3
                export SO_MAX_VITA=10
                export SO_LATO=1000
                export SO_SPEED=500
                export SO_CAPACITY=10
                export SO_BANCHINE=2
                export SO_FILL=500000
                export SO_LOADSPEED=200
                export SO_DAYS=10
                export SO_STORM_DURATION=6
                export SO_SWELL_DURATION=24
                export SO_MEALSTROM=1
                echo "Done!"
            else if [ $1 == 3 ] 
                then
                    echo "Setting 'born tu run' configuration..."
                    export SO_NAVI=10
                    export SO_PORTI=400
                    export SO_MERCI=100
                    export SO_SIZE=100
                    export SO_MIN_VITA=3
                    export SO_MAX_VITA=10
                    export SO_LATO=1000
                    export SO_SPEED=2000
                    export SO_CAPACITY=1000
                    export SO_BANCHINE=10
                    export SO_FILL=1000000
                    export SO_LOADSPEED=500
                    export SO_DAYS=10
                    export SO_STORM_DURATION=6
                    export SO_SWELL_DURATION=24
                    export SO_MEALSTROM=60
                    echo "Done!"
                else if [ $1 == 4 ] 
                    then
                        echo "Setting 'cargos, big stuff' configuration..."
                        export SO_NAVI=100
                        export SO_PORTI=5
                        export SO_MERCI=10
                        export SO_SIZE=100
                        export SO_MIN_VITA=3
                        export SO_MAX_VITA=10
                        export SO_LATO=1000
                        export SO_SPEED=500
                        export SO_CAPACITY=1000
                        export SO_BANCHINE=10
                        export SO_FILL=1000000
                        export SO_LOADSPEED=200
                        export SO_DAYS=10
                        export SO_STORM_DURATION=6
                        export SO_SWELL_DURATION=24
                        export SO_MEALSTROM=24
                        echo "Done!"
                    else if [ $1 == 5 ] 
                        then
                            echo "Setting 'unlucky cargos' configuration..."
                            export SO_NAVI=100
                            export SO_PORTI=5
                            export SO_MERCI=10
                            export SO_SIZE=100
                            export SO_MIN_VITA=3
                            export SO_MAX_VITA=10
                            export SO_LATO=1000
                            export SO_SPEED=500
                            export SO_CAPACITY=1000
                            export SO_BANCHINE=10
                            export SO_FILL=1000000
                            export SO_LOADSPEED=200
                            export SO_DAYS=10
                            export SO_STORM_DURATION=12
                            export SO_SWELL_DURATION=10
                            export SO_MEALSTROM=1
                            echo "Done!"
                        else if [ $1 == 0 ] 
                            then
                                echo "Setting custom configuration..."
                                export SO_NAVI=1
                                export SO_PORTI=2
                                export SO_MERCI=69
                                export SO_MIN_VITA=69
                                export SO_MAX_VITA=10
                                export SO_SIZE=10
                                export SO_SPEED=40
                                export SO_LATO=100
                                export SO_CAPACITY=10
                                export SO_BANCHINE=10
                                export SO_FILL=69
                                export SO_LOADSPEED=69 
                                export SO_DAYS=30
                                export SO_STORMD_URATION=10
                                export SO_SWELL_DURATION=69
                                export SO_MEALSTROM=69 
                                echo "Done!"
                            else 
                                echo "Invalid argument: please insert a number of the interval [0,5]"
                        fi  
                    fi
                fi
            fi               
        fi
    fi 
fi









