(define 
    (domain merry)
    (:requirements :adl)
    (:types
        pokoj kolor kulka pozycja klocek
    )
    (:predicates
        (merry-w ?r - pokoj)
        (kulka-w ?r - pokoj ?b - kulka)
        (jest-koloru ?b - kulka ?c - kolor)
        (drzwi ?skad - room ?dokad - room ?c - kolor)
        (merry-ma ?b - kulka)

        ;; Pokój, w którym jest łamigłówka
        (puzzle-w ?r - pokoj)
        (na ?k - klocek ?p - pozycja)
        (puste ?p - pozycja)
        (sasiednia ?p1 - pozycja ?p2 - pozycja)
    )

    (:action wez
        :parameters
        (
            ?c - kolor ?k - kulka ?p - pokoj
        )
        :precondition
        (and
            (merry-w ?p)
            (kulka-w ?p ?k)
            (jest-koloru ?k ?c)
        )
        :effect
        (and
            (merry-ma ?k)
            (not (kulka-w ?p ?k))
        )
    )
    (:action idz
        :parameters
        (
            ?dokad - pokoj ?skad - pokoj ?k - kulka ?c - kolor
        )
        :precondition
        (and
            (merry-w ?skad)
            (drzwi ?skad ?dokad ?c)
            (merry-ma ?k)
            (jest-koloru ?k ?c)
        )
        :effect 
        (and 
            (not (merry-w ?skad))
            (merry-w ?dokad)
            (not (merry-ma ?k))    
        )
    )
    (:action przesun
        :parameters
        (
            ?k - klocek ?skad - pozycja ?dokad - pozycja ?p - pokoj
        )
        :precondition
        (and
            (merry-w ?p)
            (puzzle-w ?p)
            (na ?k ?skad)
            (sasiednia ?skad ?dokad)
            (puste ?dokad)
        )
        :effect
        (and
            (not (na ?k ?skad))
            (na ?k ?dokad)
            (not (puste ?dokad))
            (puste ?skad)
        )
    )
)