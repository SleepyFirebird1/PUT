(define (problem p1) 
    (:domain world-of-blocks)
    (:objects a b c d e)
    (:init
        (clear e)
        (on-top e d)
        (not (clear d))
        (on-floor d)
        (clear c)
        (on-top c b)
        (not (clear b))
        (on-top b a)
        (not (clear a))
        (on-floor a)

    )

    (:goal 
        (and
            (exists (?p) (on-top b ?p))
            (on-top d b)
        )
    )
)
