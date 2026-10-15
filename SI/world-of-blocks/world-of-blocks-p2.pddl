(define (problem p2) 
    (:domain world-of-blocks)
    (:objects a b c d e)
    (:init
        (not (clear a))
        (on-floor a)
        (not (clear b))
        (on-top b a)
        (not (on-floor b))
        (clear c)
        (on-top c b)
        (not (on-floor c))
        (not (clear d))
        (on-floor d)
        (clear e)
        (on-top e d)
        (not (on-floor e))

    )

    (:goal 
        (not (exists (?p) (not(on-floor ?p))))
    )
)
