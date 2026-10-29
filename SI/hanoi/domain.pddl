(define 
    (domain hanoi)
    (:requirements :adl)
    (:types krazek palik)
    (:constants x y z - palik)
    (:predicates
        (x_wiekszy_y ?x ?y - krazek)
        (na_paliku ?x - krazek ?y - palik) 
    )
    
    (:action przesun-na-pusty
        :parameters (?skad ?dokad - palik ?krazek - krazek)
        :precondition 
        (and 
            (na_paliku ?krazek ?skad)
            (not 
                (exists (?inny_krazek - krazek) 
                    (na_paliku ?inny_krazek ?dokad)
                )
            )
            (not 
                (exists (?inny_krazek2 - krazek) 
                    (and 
                        (x_wiekszy_y ?krazek ?inny_krazek2) 
                        (na_paliku ?inny_krazek2 ?skad)
                    )
                )
            )
        )
        :effect 
        (and 
            (na_paliku ?krazek ?dokad)
            (not (na_paliku ?krazek ?skad))    
        )
    )
    
    (:action przesun
        :parameters (?skad ?dokad - palik ?krazek1 ?krazek2 - krazek)
        :precondition 
        (and 
            (na_paliku ?krazek1 ?skad)
            (na_paliku ?krazek2 ?dokad)
            (x_wiekszy_y ?krazek2 ?krazek1)
            (not 
                (exists (?inny_krazek - krazek) 
                    (and 
                        (x_wiekszy_y ?krazek1 ?inny_krazek) 
                        (na_paliku ?inny_krazek ?skad)
                    )
                )
            )
            (not 
                (exists (?inny_krazek1 - krazek) 
                    (and 
                        (x_wiekszy_y ?krazek2 ?inny_krazek1) 
                        (na_paliku ?inny_krazek1 ?dokad)
                    )
                )
            )
        )
        :effect 
        (and 
            (not (na_paliku ?krazek1 ?skad))
            (na_paliku ?krazek1 ?dokad)
        )
    )
)