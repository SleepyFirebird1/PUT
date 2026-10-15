(define 
    (domain world-of-blocks)
    (:requirements :adl)
    (:predicates
        (on-top ?x ?y)  ;czy x lezy na jakims klocku
        (on-floor ?x) ;czy lezy na podlodze
        (clear ?x) ;czy coś lezy na klocku x
        (holding ?x) ;czy jest zlapany przez ramie
    )
    (:action podnies-z-podlogi
        :parameters (?x)
        :precondition 
        (and 
            (not (exists (?p) (holding ?p)))
            (on-floor ?x)
            (clear ?x)
        )
        :effect 
        (and 
            (holding ?x)
            (not (on-floor ?x))
        )
    )
    (:action opusc-na-podloge
        :parameters (?x)
        :precondition 
        (and 
            (holding ?x)
            (not (on-floor ?x))
        )
        :effect 
        (and 
            (on-floor ?x)
            (not (holding ?x))
        )
    )
    (:action podnies-z-paczki
        :parameters (?x ?y)
        :precondition 
        (and 
            (clear ?x)
            (not (exists (?p) (holding ?p)))
            (on-top ?x ?y)
        )
        :effect 
        (and 
            (not (on-top ?x ?y))
            (clear ?y)
            (holding ?x)
        )
    )
    (:action opusc-na-paczke
        :parameters (?z ?x)
        :precondition 
        (and 
            (holding ?x)
            (clear ?z)
            (not (= ?x ?z))
        )
        :effect 
        (and 
            (on-top ?x ?z)
            (not (holding ?x))
            (not (clear ?z))
        )
    )
)
    