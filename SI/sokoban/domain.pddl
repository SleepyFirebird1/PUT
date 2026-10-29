(define
    (domain sokoban)
    (:requirements :adl)
    (:predicates
        (poziomo ?x ?y)
        (pionowo ?x ?y)
        (paczka ?x)
        (robot ?x)
        (cel ?x)
    )

    (:action idz
        :parameters (?x ?y)
        :precondition 
        (and 
            (robot ?x)
            (not (paczka ?y))
            (or
                (poziomo ?x ?y)
                (pionowo ?x ?y)
            )
        )
        :effect 
        (and 
            (robot ?y)
            (not (robot ?x))
        )
    )

    ; staraPozycjaPaczki == przyszlaPozycjaRobota
    (:action pchaj
        :parameters (?obecnaPozycjaRobota ?staraPozycjaPaczki ?przyszlaPozycjaPaczki)
        :precondition 
        (and 
            (not (= ?obecnaPozycjaRobota ?przyszlaPozycjaPaczki))
            (not (paczka ?przyszlaPozycjaPaczki))
            (robot ?obecnaPozycjaRobota)
            (paczka ?staraPozycjaPaczki)
            (or
                (and
                    (pionowo ?obecnaPozycjaRobota ?staraPozycjaPaczki)
                    (pionowo ?staraPozycjaPaczki ?przyszlaPozycjaPaczki)
                )
                (and
                    (poziomo ?obecnaPozycjaRobota ?staraPozycjaPaczki)
                    (poziomo ?staraPozycjaPaczki ?przyszlaPozycjaPaczki)
                )
            )
        )
        :effect 
        (and 
            (not (paczka ?staraPozycjaPaczki))
            (not (robot ?obecnaPozycjaRobota))
            (paczka ?przyszlaPozycjaPaczki)
            (robot ?staraPozycjaPaczki)
        )        
    ) 
)